
#include <ranges>
#include <sstream>

#include "writer.h"

#include <algorithm>

namespace sippy::sip {

class invalid_message final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "invalid message";
    }
};

writer::writer(std::ostream& os)
    : m_os(os)
    , m_request_line()
    , m_status_line()
    , m_headers()
    , m_body()
    , m_body_str()
    , m_body_type()
{}

void writer::attach(message_ptr msg) {
    if (!msg->is_valid()) {
        throw invalid_message();
    }

    const auto message = std::move(msg);
    load(message);
}

void writer::write() {
    compose_body();
    add_necessary_headers();

    write_start_line();
    write_headers();

    m_os << "\r\n";
    m_os << m_body_str;
}

void writer::load(const message_ptr& msg) {
    m_headers.clear();
    m_body.reset();
    m_body_str.clear();
    m_body_type.clear();

    m_request_line = std::move(msg->m_request_line);
    m_status_line = std::move(msg->m_status_line);

    for (auto &holders: msg->m_headers | std::views::values) {
        for (auto& holder : holders) {
            m_headers.push_back(std::move(holder));
        }
    }

    m_body = std::move(msg->m_body);
}

void writer::add_necessary_headers() {
    auto content_length = std::make_unique<headers::storage::_header_holder<headers::content_length>>();
    content_length->value.length = m_body_str.size();
    m_headers.push_back(std::move(content_length));

    if (!m_body_type.empty()) {
        auto content_type = std::make_unique<headers::storage::_header_holder<headers::content_type>>();
        content_type->value.type = std::move(m_body_type);
        m_headers.push_back(std::move(content_type));
    }
}

void writer::write_start_line() {
    if (m_request_line.has_value()) {
        m_os << m_request_line->method;
        m_os << ' ';
        m_os << m_request_line->uri;
        m_os << ' ';
        m_os << m_request_line->version;
    } else {
        m_os << m_status_line->version;
        m_os << ' ';
        m_os << m_status_line->code;
        m_os << ' ';
        m_os << m_status_line->reason_phrase;
    }

    m_os << "\r\n";
}

void writer::write_headers() {
    std::ranges::sort(m_headers, [](const headers::storage::_header_holder_ptr& lhs, const headers::storage::_header_holder_ptr& rhs) {
        const auto lhs_flags = lhs->flags();
        const auto rhs_flags = rhs->flags();

        if ((lhs_flags & headers::flag_priority_top) != 0) {
            return true;
        }
        if ((rhs_flags & headers::flag_priority_top) != 0) {
            return false;
        }

        return std::greater<>()(lhs->name(), rhs->name());
    });

    for (const auto& header : m_headers) {
        m_os << header->name() << ": ";
        header->operator<<(m_os);
        m_os << "\r\n";
    }
}

void writer::compose_body() {
    if (m_body) {
        std::stringstream ss;
        m_body->operator<<(ss);

        m_body_str = ss.str();
        m_body_type = m_body->type();
    } else {
        m_body_str.clear();
        m_body_type.clear();
    }

    m_body.reset();
}

}
