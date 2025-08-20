
#include <sdp/message.h>

#include "reader.h"
#include "writer.h"
#include "util/streams.h"

namespace sippy::sdp {

template<typename T>
static bool is_valid_opt(const std::optional<T>& opt) {
    return !opt.has_value() || fields::meta::_field_validator<T>::validate(opt.value());
}

template<typename T>
static bool is_valid_vec(const std::vector<T>& vec) {
    for (const auto& v : vec) {
        if (!fields::meta::_field_validator<T>::validate(v)) {
            return false;
        }
    }

    return true;
}

void validate_message(const description_message& message) {
    if (!fields::meta::_field_validator<fields::version>::validate(message.version)) {
        throw std::invalid_argument("Invalid version field");
    }
    if (!fields::meta::_field_validator<fields::origin>::validate(message.origin)) {
        throw std::invalid_argument("Invalid origin field");
    }
    if (!fields::meta::_field_validator<fields::session_name>::validate(message.name)) {
        throw std::invalid_argument("Invalid session name field");
    }
    if (!is_valid_opt(message.information)) {
        throw std::invalid_argument("Invalid session level information field");
    }
    if (!is_valid_opt(message.uri)) {
        throw std::invalid_argument("Invalid session level uri field");
    }
    if (!is_valid_vec(message.emails)) {
        throw std::invalid_argument("Invalid session level emails field");
    }
    if (!is_valid_vec(message.phone_numbers)) {
        throw std::invalid_argument("Invalid session level phone numbers field");
    }
    if (!is_valid_opt(message.connection_information)) {
        throw std::invalid_argument("Invalid session level connection info field");
    }
    if (!is_valid_vec(message.bandwidths)) {
        throw std::invalid_argument("Invalid session level bandwiths field");
    }

    for (const auto& attr : message.attributes) {
        if ((attr.flags() & attributes::flag_session_level) == 0) {
            throw std::invalid_argument("Invalid attribute in session level");
        }
        if (message.attributes.count(attr.name()) > 1 && (attr.flags() & attributes::flag_allow_multiple) == 0) {
            throw std::invalid_argument("Attribute has multiple entries but only one allowed");
        }
    }

    for (const auto& time_desc : message.time_descriptions) {
        if (!fields::meta::_field_validator<fields::time_active>::validate(time_desc.times)) {
            throw std::invalid_argument("Invalid times field");
        }

        for (const auto& repeat_desc : time_desc.repeat) {
            if (!fields::meta::_field_validator<fields::repeat_times>::validate(repeat_desc.repeat)) {
                throw std::invalid_argument("Invalid repeat field");
            }
            if (!is_valid_opt(repeat_desc.timezone)) {
                throw std::invalid_argument("Invalid timezone field");
            }
        }
    }

    for (const auto& media_desc : message.media_descriptions) {
        if (!fields::meta::_field_validator<fields::media_description>::validate(media_desc.media)) {
            throw std::invalid_argument("Invalid media level media field");
        }
        if (!is_valid_opt(media_desc.information)) {
            throw std::invalid_argument("Invalid media level information field");
        }
        if (!is_valid_vec(media_desc.connections)) {
            throw std::invalid_argument("Invalid media level connections field");
        }
        if (!is_valid_vec(media_desc.bandwidths)) {
            throw std::invalid_argument("Invalid media level bandwidths field");
        }

        for (const auto& attr : media_desc.attributes) {
            if ((attr.flags() & attributes::flag_media_level) == 0) {
                throw std::invalid_argument("Invalid attribute in media level");
            }
            if (media_desc.attributes.count(attr.name()) > 1 && (attr.flags() & attributes::flag_allow_multiple) == 0) {
                throw std::invalid_argument("Attribute has multiple entries but only one allowed");
            }
        }
    }
}

description_message parse(std::istream& is) {
    reader reader(is);
    return reader.read();
}

description_message parse(const std::span<const uint8_t> buffer) {
    util::istream_buff buff(buffer);
    std::istream is(&buff);
    return parse(is);
}

void write(std::ostream& os, const description_message& message) {
    writer writer(os);
    writer.write(message);
}

ssize_t write(const std::span<uint8_t> buffer, const description_message& message) {
    util::ostream_buff buff(buffer);
    std::ostream os(&buff);
    write(os, message);

    if (os.bad()) {
        throw std::runtime_error("write failed: bad");
    }
    if (os.fail()) {
        throw std::runtime_error("write failed: fail");
    }
    if (os.eof()) {
        throw std::runtime_error("write failed: eof");
    }

    return os.tellp();
}

}
