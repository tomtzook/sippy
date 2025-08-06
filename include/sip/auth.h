#pragma once

#include <span>
#include <string>

#include <sip/types.h>
#include <sip/headers.h>

namespace sippy::sip {

using ki = uint8_t[16];
using opc = uint8_t[16];
using amf = uint8_t[2];
using res = uint8_t[8];

void ki_from_hex(std::string_view str, ki ki);
void opc_from_hex(std::string_view str, opc opc);
void amf_from_hex(std::string_view str, amf amf);

void sim_keys_to_password(const ki ki, const opc opc, const amf amf, std::string_view nonce, res out);

std::string create_auth_response_md5(
    std::string_view username,
    std::span<const uint8_t> password,
    sip::method method,
    std::string_view realm,
    std::string_view uri,
    std::string_view nonce,
    std::string_view cnonce,
    uint32_t nc,
    std::string_view qop);

std::string create_auth_response_aka(
    std::string_view username,
    const ki ki,
    const opc opc,
    const amf amf,
    sip::method method,
    std::string_view realm,
    std::string_view uri,
    std::string_view nonce,
    std::string_view cnonce,
    uint32_t nc,
    std::string_view qop);

std::string create_auth_response_md5_for_register(
    std::string_view username,
    std::string_view uri,
    std::span<const uint8_t> password,
    const headers::www_authorization& auth_header,
    std::string_view cnonce,
    uint32_t nc);

std::string create_auth_response_aka_for_register(
    std::string_view username,
    std::string_view uri,
    const ki ki,
    const opc opc,
    const amf amf,
    const headers::www_authorization& auth_header,
    std::string_view cnonce,
    uint32_t nc);

}
