
#include "writer.h"

#include <cstring>

namespace sippy::sdp {

writer::writer(std::ostream& os)
    : m_os(os)
{}

void writer::write(const description_message& message) {
    validate_message(message);

    write_field(message.version);
    write_field(message.origin);
    write_field(message.name);
    write_field_opt(message.information);
    write_field_opt(message.uri);
    write_field_vec(message.emails);
    write_field_vec(message.phone_numbers);
    write_field_opt(message.connection_information);
    write_field_vec(message.bandwidths);

    for (const auto& time_desc : message.time_descriptions) {
        write_field(time_desc.times);

        for (const auto& repeat_desc : time_desc.repeat) {
            write_field(repeat_desc.repeat);
            write_field_opt(repeat_desc.timezone);
        }
    }

    for (const auto& attr : message.attributes) {
        write_attribute(attr);
    }

    for (const auto& media_desc : message.media_descriptions) {
        write_field(media_desc.media);
        write_field_opt(media_desc.information);
        write_field_vec(media_desc.connections);
        write_field_vec(media_desc.bandwidths);

        for (const auto& attr : media_desc.attributes) {
            write_attribute(attr);
        }
    }
}

void writer::write_attribute(const attributes::storage::_base_attribute_holder& attr) {
    m_os << fields::field_name_attribute << '=';

    const auto name = attr.name();
    if (strlen(name) > 0) {
        m_os << name;
        m_os << ':';
    }

    attr.operator<<(m_os);
    m_os << "\r\n";
}

}
