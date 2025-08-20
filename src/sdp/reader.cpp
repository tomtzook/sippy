
#include "types_storage.h"
#include "reader.h"


namespace sippy::sdp {

reader::reader(std::istream &is)
    : m_is(is)
    , m_reader(is)
{}

description_message reader::read() {
    description_message description{};

    read(description.version);
    read(description.origin);
    read_optional(description.information);
    read_optional(description.uri);
    read_vector(description.emails);
    read_vector(description.phone_numbers);
    read_optional(description.connection_information);
    read_vector(description.bandwidths);

    do {
        media_time_description time_description{};
        read_description(time_description);
        description.time_descriptions.push_back(std::move(time_description));
    } while (peek<fields::time_active>());

    read_attributes(description.attributes);

    while (peek<fields::media_description>()) {
        message_media_description media_description{};
        read_description(media_description);
        description.media_descriptions.push_back(std::move(media_description));
    };

    validate_message(description);

    return description;
}

void reader::read_description(time_repeat_description& description) {
    read(description.repeat);
    read_optional(description.timezone);
}

void reader::read_description(media_time_description& description) {
    read(description.times);

    while (peek<fields::repeat_times>()) {
        time_repeat_description repeat_description{};
        read_description(repeat_description);
        description.repeat.push_back(std::move(repeat_description));
    };
}

void reader::read_description(message_media_description& description) {
    read(description.media);
    read_optional(description.information);
    read_vector(description.connections);
    read_vector(description.bandwidths);
    read_attributes(description.attributes);
}

bool reader::peek_attribute() {
    return m_reader.peek(fields::field_name_attribute);
}

attributes::storage::_attribute_holder_ptr reader::read_attribute() {
    m_reader.eat('=');

    attributes::storage::_attribute_holder_ptr ptr;

    auto value = m_reader.read_until(serialization::is_colon_or_newline);
    if (m_reader.eat_one_if(serialization::is_colon)) {
        auto name = std::move(value);
        value = m_reader.read_until(serialization::is_new_line);

        const auto def = attributes::storage::get_attribute(name);
        if (def) {
            ptr = def.value()->create();
            std::stringstream ss(value);
            ptr->operator>>(ss);
        }
    } else {
        // todo: HANDLE
    }

    m_reader.eat('\r');
    m_reader.eat('\n');

    return ptr;
}

void reader::read_attributes(attributes::attribute_container& attributes) {
    while (peek_attribute()) {
        auto ptr = read_attribute();
        if (ptr) {
            attributes.add(std::move(ptr));
        }
    }
}

}
