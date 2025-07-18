
#include <sip/message.h>

#include "serialization/matchers.h"
#include "serialization/reader.h"
#include "sip/base.h"

namespace sippy::sip {

class not_a_request final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "not a request";
    }
};

class not_a_response final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "not a response";
    }
};

class no_body final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "no body";
    }
};

class wrong_body_type final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "wrong body type";
    }
};

request_line::request_line()
    : method(sip::method::invite)
    , uri()
    , version(version::version_2_0)
{}

request_line::request_line(const sip::method method, const std::string_view uri, const sip::version version)
    : method(method)
    , uri(uri)
    , version(version)
{}

status_line::status_line()
    : version(version::version_2_0)
    , code(status_code::trying)
    , reason_phrase()
{}

status_line::status_line(const sip::version version, const sip::status_code code, const std::string_view reason_phrase)
    : version(version)
    , code(code)
    , reason_phrase(reason_phrase)
{}

std::istream& operator>>(std::istream& is, request_line& line) {
    serialization::reader reader(is);

    is >> line.method;
    reader.read_while(serialization::is_whitespace);
    line.uri = reader.read_until(serialization::is_whitespace);
    reader.read_while(serialization::is_whitespace);
    is >> line.version;

    return is;
}

std::ostream& operator<<(std::ostream& os, const request_line& line) {
    os << line.method;
    os << ' ';
    os << line.uri;
    os << ' ';
    os << line.version;

    return os;
}

std::istream& operator>>(std::istream& is, status_line& line) {
    serialization::reader reader(is);

    is >> line.version;
    reader.read_while(serialization::is_whitespace);
    is >> line.code;
    reader.read_while(serialization::is_whitespace);
    line.reason_phrase = reader.read_until(serialization::is_new_line);

    return is;
}

std::ostream& operator<<(std::ostream& os, const status_line& line) {
    os << line.version;
    os << ' ';
    os << line.code;
    os << ' ';
    os << line.reason_phrase;

    return os;
}

bool message::is_valid() const {
    return is_request() || is_response();
}

bool message::is_request() const {
    return m_request_line.has_value();
}

bool message::is_response() const {
    return m_status_line.has_value();
}

bool message::has_body() const {
    return m_body.get() != nullptr;
}

const sip::request_line& message::request_line() const {
    if (m_request_line.has_value()) {
        return m_request_line.value();
    }

    throw not_a_request();
}

sip::request_line& message::request_line() {
    if (m_request_line.has_value()) {
        return m_request_line.value();
    }

    throw not_a_request();
}

void message::set_request_line(const sip::request_line& line) {
    auto copy = line;
    set_request_line(std::move(copy));
}

void message::set_request_line(sip::request_line&& line) {
    m_request_line = std::move(line);
    m_status_line = std::nullopt;
}

const sip::status_line& message::status_line() const {
    if (m_status_line.has_value()) {
        return m_status_line.value();
    }

    throw not_a_response();
}

sip::status_line& message::status_line() {
    if (m_status_line.has_value()) {
        return m_status_line.value();
    }

    throw not_a_response();
}

void message::set_status_line(const sip::status_line& line) {
    auto copy = line;
    set_status_line(std::move(copy));
}

void message::set_status_line(sip::status_line&& line) {
    m_status_line = std::move(line);
    m_request_line = std::nullopt;
}

size_t message::_header_count(const std::string& name) const {
    const auto it = m_headers.find(name);
    if (it != m_headers.end()) {
        return it->second.size();
    }

    return 0;
}

void message::remove_body() {
    m_body.reset();
}

const headers::storage::_base_header_holder* message::_get_header(const std::string& name, size_t index) const {
    const auto it = m_headers.find(name);
    if (it == m_headers.end()) {
        throw headers::header_not_found();
    }
    if (it->second.empty()) {
        throw headers::header_not_found();
    }
    if (index >= it->second.size()) {
        throw headers::header_not_found();
    }

    return it->second[index].get();
}

headers::storage::_base_header_holder* message::_get_header(const std::string& name, const size_t index) {
    const auto it = m_headers.find(name);
    if (it == m_headers.end()) {
        throw headers::header_not_found();
    }
    if (it->second.empty()) {
        throw headers::header_not_found();
    }
    if (index >= it->second.size()) {
        throw headers::header_not_found();
    }

    return it->second[index].get();
}

void message::_add_header(const std::string& name, headers::storage::_header_holder_ptr holder) {
    const auto it = m_headers.find(name);
    if (it == m_headers.end()) {
        std::vector<headers::storage::_header_holder_ptr> vector;
        vector.push_back(std::move(holder));
        m_headers[name] = std::move(vector);
    } else {
        it->second.push_back(std::move(holder));
    }
}

void message::_copy_headers(const std::string& name, const message& other) {
    const auto it = other.m_headers.find(name);
    if (it != other.m_headers.end()) {
        for (const auto& holder : it->second) {
            _add_header(name, holder->copy());
        }
    }
}

bool message::_remove_header(const std::string& name, const size_t index) {
    const auto it = m_headers.find(name);
    if (it == m_headers.end()) {
        return false;
    }
    if (it->second.empty()) {
        return false;
    }
    if (index >= it->second.size()) {
        return false;
    }

    it->second.erase(it->second.begin() + static_cast<ptrdiff_t>(index));
    return true;
}

bool message::_remove_headers(const std::string& name) {
    const auto it = m_headers.find(name);
    if (it == m_headers.end()) {
        return false;
    }
    if (it->second.empty()) {
        return false;
    }

    it->second.clear();
    return true;
}

bool message::_is_body(const std::string& type) const {
    if (!has_body()) {
        return false;
    }

    return m_body->is_of_type(type);
}

const bodies::storage::_base_body_holder* message::_get_body(const std::string& type) const {
    if (!has_body()) {
        throw no_body();
    }
    if (!m_body->is_of_type(type)) {
        throw wrong_body_type();
    }

    return m_body.get();
}

bodies::storage::_base_body_holder* message::_get_body(const std::string& type) {
    if (!has_body()) {
        throw no_body();
    }
    if (!m_body->is_of_type(type)) {
        throw wrong_body_type();
    }

    return m_body.get();
}

void message::_set_body(bodies::storage::_body_holder_ptr body) {
    m_body = std::move(body);
}

}
