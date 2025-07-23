#pragma once

#include <span>
#include <string>
#include <cstdint>

#include <sip/types.h>

namespace sippy::sip {

using ki = uint8_t[16];
using opc = uint8_t[16];
using amf = uint8_t[2];
using res = uint8_t[8];

void sim_keys_to_password(const ki ki, const opc opc, const amf amf, std::string_view nonce, res out);

std::string auth_md5(
    std::string_view username,
    std::span<const uint8_t> password,
    sip::method method,
    std::string_view realm,
    std::string_view uri,
    std::string_view nonce,
    std::string_view cnonce,
    uint32_t nc,
    std::string_view qop);

std::string auth_aka(
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

}
