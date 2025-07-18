#pragma once

#include <memory>

#include <sip/message.h>

namespace sippy::sip {

class writer {
public:
    explicit writer(std::ostream& os);

    void attach(message_ptr msg);
    void write();

private:
    void load(const message_ptr& msg);
    void add_necessary_headers();
    void write_start_line();
    void write_headers();
    void compose_body();

    std::ostream& m_os;
    std::optional<request_line> m_request_line;
    std::optional<status_line> m_status_line;
    std::vector<headers::storage::_header_holder_ptr> m_headers;
    bodies::storage::_body_holder_ptr m_body;
    std::string m_body_str;
    std::string m_body_type;
};

}
