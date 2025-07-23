#pragma once

#include <sip/message.h>

namespace sippy::sip {

message_ptr create_request(
    sip::status_code code,
    const sip::message& original_request,
    uint32_t expires,
    uint32_t max_forwards,
    std::optional<std::string_view> phrase = std::nullopt);

}
