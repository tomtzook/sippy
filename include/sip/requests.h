#pragma once

#include <sip/message.h>
#include <sip/auth.h>

namespace sippy::sip {

message_ptr create_request(
    sip::method method,
    std::string_view target_uri,
    std::string_view from_uri,
    std::string_view to_uri,
    std::string_view call_id,
    uint32_t sequence_num,
    uint32_t expires,
    uint32_t max_forwards);

message_ptr create_request_register(
    std::string_view target_uri,
    std::string_view from_uri,
    std::string_view call_id,
    uint32_t sequence_num,
    uint32_t expires,
    uint32_t max_forwards
);

headers::authorization create_request_authorization(
    auth_scheme scheme,
    auth_algorithm algorithm,
    std::string_view user_host,
    std::string_view username);

headers::authorization create_request_authorization(
    const headers::www_authorization& auth_header,
    std::string_view user_host,
    std::string_view username,
    std::span<const uint8_t> password,
    std::optional<std::string_view> cnonce = std::nullopt,
    std::optional<uint32_t> nc = std::nullopt);

headers::authorization create_request_authorization(
    const headers::www_authorization& auth_header,
    std::string_view user_host,
    std::string_view username,
    const ki ki,
    const opc opc,
    const amf amf,
    std::optional<std::string_view> cnonce = std::nullopt,
    std::optional<uint32_t> nc = std::nullopt);

}
