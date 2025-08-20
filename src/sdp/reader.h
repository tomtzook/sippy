#pragma once

#include <sdp/message.h>

#include "serialization/reader.h"

namespace sippy::sdp {

class reader {
public:
    explicit reader(std::istream& is);
    description_message read();

private:
    template<fields::meta::_field_type T>
    bool peek() {
        const auto expected_name = fields::meta::_field_detail<T>::name();
        return m_reader.peek(expected_name);
    }

    template<typename T>
    bool read_next_field(T& field) {
        if (peek<T>()) {
            return false;
        }

        const auto expected_name = fields::meta::_field_detail<T>::name();
        m_reader.eat(expected_name);

        m_reader.eat('=');
        m_is >> field;
        m_reader.eat('\r');
        m_reader.eat('\n');

        return true;
    }
    template<typename T>
    void read(T& field) {
        if (!read_next_field(field)) {
            throw std::runtime_error("Missing required field");
        }
    }
    template<typename T>
    void read_optional(std::optional<T>& field) {
        T val{};
        if (read_next_field(val)) {
            field = val;
        }
    }
    template<typename T>
    void read_vector(std::vector<T>& field) {
        bool read = false;
        do {
            T val{};
            read = read_next_field(val);
            if (read) {
                field.push_back(std::move(val));
            }
        } while (read);
    }

    void read_description(time_repeat_description& description);
    void read_description(media_time_description& description);
    void read_description(message_media_description& description);

    bool peek_attribute();
    attributes::storage::_attribute_holder_ptr read_attribute();
    void read_attributes(attributes::attribute_container& attributes);

    std::istream& m_is;
    serialization::reader m_reader;
};

}
