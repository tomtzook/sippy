#pragma once

#include <map>
#include <vector>

#include <sip/types.h>
#include <sip/headers.h>

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

class message {
public:
    message() = default;
    message(const message&) = delete;
    message(message&&) = default;
    ~message() = default;

    [[nodiscard]] bool is_valid() const;
    [[nodiscard]] bool is_request() const;
    [[nodiscard]] bool is_response() const;

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

    [[nodiscard]] size_t _header_count(const std::string& name) const;
    [[nodiscard]] const headers::storage::_base_header_holder* _get_header(const std::string& name, size_t index) const;
    headers::storage::_base_header_holder* _get_header(const std::string& name, size_t index);
    void _add_header(const std::string& name, headers::storage::_header_holder_ptr holder);
    void _copy_headers(const std::string& name, const message& other);

private:
    std::optional<sip::request_line> m_request_line;
    std::optional<sip::status_line> m_status_line;
    std::map<std::string, std::vector<headers::storage::_header_holder_ptr>> m_headers;
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

    add_header(name, std::move(holder));
}

template<headers::meta::_header_type T>
void message::copy_headers(const message& other) {
    const auto& name = headers::meta::_header_detail<T>::name();
    _copy_headers(name, other);
}

}
