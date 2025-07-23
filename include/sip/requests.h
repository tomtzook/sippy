#pragma once

#include <sip/message.h>

namespace sippy::sip {

message_ptr create_request(
    sip::method method,
    std::string_view target_uri,
    std::string_view from_uri,
    std::string_view to_uri,
    sip::transport transport,
    std::string_view via_address,
    uint16_t via_port,
    std::string_view tag,
    std::string_view branch,
    std::string_view call_id,
    uint32_t sequence_num,
    uint32_t expires,
    uint32_t max_forwards);

message_ptr create_request_register(
    std::string_view target_uri,
    std::string_view from_uri,
    sip::transport transport,
    std::string_view via_address,
    uint16_t via_port,
    std::string_view tag,
    std::string_view branch,
    std::string_view call_id,
    uint32_t sequence_num,
    uint32_t expires,
    uint32_t max_forwards
);

headers::authorization create_request_authorization(
    auth_scheme scheme,
    auth_algorithm algorithm,
    std::string_view user_name,
    std::string_view user_host);

headers::authorization create_request_authorization(
    const headers::www_authorization& auth_details,
    std::string_view user_name,
    std::string_view user_host);

}
