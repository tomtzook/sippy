#pragma once

#include <string>
#include <optional>
#include <cstdint>
#include <vector>
#include <span>

#include <sdp/types.h>
#include <sdp/fields.h>

namespace sippy::sdp {

struct time_repeat_description {
    fields::repeat_times repeat;
    std::optional<fields::timezone> timezone;
};

struct media_time_description {
    fields::time_active times;
    std::vector<time_repeat_description> repeat; // at least one required
};

struct message_media_description {
    fields::media_description media;
    std::optional<fields::session_information> information;
    std::vector<fields::connection_information> connections;
    std::vector<fields::bandwidth_information> bandwidths;
    std::vector<fields::attribute> attributes;
};

struct description_message {
    fields::version version;
    fields::origin origin;
    fields::session_name name;
    std::optional<fields::session_information> information;
    std::optional<fields::uri> uri;
    std::vector<fields::email> emails;
    std::vector<fields::phone_number> phone_numbers;
    std::optional<fields::connection_information> connection_information;
    std::vector<fields::bandwidth_information> bandwidths;
    std::vector<media_time_description> time_descriptions; // at least one required
    std::vector<fields::attribute> attributes;
    std::vector<message_media_description> media_descriptions;

    // [ ] = optional one
    // * = 0+ times
    // 1* = 1+ times
    //
    // version
    // origin
    // session
    // [information]
    // [uri]
    // *email
    // *phone
    // [connection]
    // *bandwidth
    // 1*time desc
    //  time field
    //      1*repeat
    //          [zone]
    // [key field] (deprectated)
    // *attribute
    // *media description
    //  media field
    //  [information]
    //  *connection
    //  *bandwidth
    //  [key]
    //  *attribute
};

void validate_message(const description_message& message);

description_message parse(std::istream& is);
description_message parse(std::span<const uint8_t> buffer);

void write(std::ostream& os, const description_message& message);
ssize_t write(std::span<uint8_t> buffer, const description_message& message);

}
