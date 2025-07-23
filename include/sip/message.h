#pragma once

#include <map>
#include <span>
#include <vector>

#include <sip/types.h>
#include <sip/headers.h>
#include <sip/bodies.h>

namespace sippy::sip {

struct request_line {
    request_line();
    request_line(sip::method method, std::string_view uri, sip::version version);

    sip::method method;
    std::string uri;
    sip::version version;
};

struct status_line {
    status_line();
    status_line(sip::version version, sip::status_code code, std::string_view reason_phrase);

    sip::version version;
    sip::status_code code;
    std::string reason_phrase;
};

std::istream& operator>>(std::istream& is, request_line& line);
std::ostream& operator<<(std::ostream& os, const request_line& line);
std::istream& operator>>(std::istream& is, status_line& line);
std::ostream& operator<<(std::ostream& os, const status_line& line);

class reader;
class writer;
class message;
using message_ptr = std::unique_ptr<message>;

message_ptr parse(std::istream& is);
message_ptr parse(std::span<const uint8_t> buffer);

void write(std::ostream& os, message_ptr message);
ssize_t write(std::span<uint8_t> buffer, message_ptr message);

class message {
public:
    message() = default;
    message(const message&) = delete;
    message(message&&) = default;
    ~message() = default;

    [[nodiscard]] bool is_valid() const;
    [[nodiscard]] bool is_request() const;
    [[nodiscard]] bool is_response() const;
    [[nodiscard]] bool has_body() const;

    [[nodiscard]] const sip::request_line& request_line() const;
    [[nodiscard]] sip::request_line& request_line();
    void set_request_line(const sip::request_line& line);
    void set_request_line(sip::request_line&& line);

    [[nodiscard]] const sip::status_line& status_line() const;
    [[nodiscard]] sip::status_line& status_line();
    void set_status_line(const sip::status_line& line);
    void set_status_line(sip::status_line&& line);

    template<headers::meta::_header_type T>
    [[nodiscard]] bool has_header() const;
    template<headers::meta::_header_type T>
    [[nodiscard]] size_t header_count() const;
    template<headers::meta::_header_type T>
    const T& header(size_t index = 0) const;
    template<headers::meta::_header_type T>
    T& header(size_t index = 0);

    template<headers::meta::_header_type T>
    void add_header(const T& header);
    template<headers::meta::_header_type T>
    void add_header(T&& header);
    template<headers::meta::_header_type T>
    void copy_headers(const message& other);
    template<headers::meta::_header_type T>
    bool remove_header(size_t index = 0);
    template<headers::meta::_header_type T>
    bool remove_headers();

    template<bodies::meta::_body_type T>
    [[nodiscard]] bool is_body() const;
    template<bodies::meta::_body_type T>
    const T& body() const;
    template<bodies::meta::_body_type T>
    T& body();

    template<bodies::meta::_body_type T>
    void set_body(const T& body);
    template<bodies::meta::_body_type T>
    void set_body(T&& body);
    void remove_body();

private:
    [[nodiscard]] size_t _header_count(const std::string& name) const;
    [[nodiscard]] const headers::storage::_base_header_holder* _get_header(const std::string& name, size_t index) const;
    headers::storage::_base_header_holder* _get_header(const std::string& name, size_t index);
    void _add_header(const std::string& name, headers::storage::_header_holder_ptr holder);
    void _copy_headers(const std::string& name, const message& other);
    bool _remove_header(const std::string& name, size_t index);
    bool _remove_headers(const std::string& name);

    [[nodiscard]] bool _is_body(const std::string& type) const;
    [[nodiscard]] const bodies::storage::_base_body_holder* _get_body(const std::string& type) const;
    [[nodiscard]] bodies::storage::_base_body_holder* _get_body(const std::string& type);
    void _set_body(bodies::storage::_body_holder_ptr body);

    std::optional<sip::request_line> m_request_line;
    std::optional<sip::status_line> m_status_line;
    std::map<std::string, std::vector<headers::storage::_header_holder_ptr>> m_headers;
    bodies::storage::_body_holder_ptr m_body;

    friend class reader;
    friend class writer;
};

template<headers::meta::_header_type T>
bool message::has_header() const {
    return header_count<T>() > 0;
}

template<headers::meta::_header_type T>
size_t message::header_count() const {
    const auto& name = headers::meta::_header_detail<T>::name();
    return _header_count(name);
}

template<headers::meta::_header_type T>
const T& message::header(size_t index) const {
    const auto& name = headers::meta::_header_detail<T>::name();

    auto holder = reinterpret_cast<const headers::storage::_header_holder<T>*>(_get_header(name, index));
    return holder->value;
}

template<headers::meta::_header_type T>
T& message::header(size_t index) {
    const auto& name = headers::meta::_header_detail<T>::name();

    auto holder = reinterpret_cast<headers::storage::_header_holder<T>*>(_get_header(name, index));
    return holder->value;
}

template<headers::meta::_header_type T>
void message::add_header(const T& header) {
    T header_copy = header;
    add_header(std::move(header_copy));
}

template<headers::meta::_header_type T>
void message::add_header(T&& header) {
    const auto& name = headers::meta::_header_detail<T>::name();

    auto holder = std::make_unique<headers::storage::_header_holder<T>>();
    holder->value = std::forward<T>(header);

    _add_header(name, std::move(holder));
}

template<headers::meta::_header_type T>
void message::copy_headers(const message& other) {
    const auto& name = headers::meta::_header_detail<T>::name();
    _copy_headers(name, other);
}

template<headers::meta::_header_type T>
bool message::remove_header(size_t index) {
    const auto& name = headers::meta::_header_detail<T>::name();
    return _remove_header(name, index);
}

template<headers::meta::_header_type T>
bool message::remove_headers() {
    const auto& name = headers::meta::_header_detail<T>::name();
    return _remove_headers(name);
}

template<bodies::meta::_body_type T>
bool message::is_body() const {
    const auto& type = bodies::meta::_body_detail<T>::app_type();
    return _is_body(type);
}

template<bodies::meta::_body_type T>
const T& message::body() const {
    const auto& type = bodies::meta::_body_detail<T>::app_type();

    auto holder = reinterpret_cast<const bodies::storage::_body_holder<T>*>(_get_body(type));
    return holder->value;
}

template<bodies::meta::_body_type T>
T& message::body() {
    const auto& type = bodies::meta::_body_detail<T>::app_type();

    auto holder = reinterpret_cast<bodies::storage::_body_holder<T>*>(_get_body(type));
    return holder->value;
}

template<bodies::meta::_body_type T>
void message::set_body(const T& body) {
    T body_copy = body;
    set_body(std::move(body_copy));
}

template<bodies::meta::_body_type T>
void message::set_body(T&& body) {
    auto holder = std::make_unique<bodies::storage::_body_holder<T>>();
    holder->value = std::forward<T>(body);

    _set_body(std::move(holder));
}

}
