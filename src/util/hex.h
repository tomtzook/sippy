#pragma once

#include <string>
#include <span>
#include <cstdint>
#include <vector>

namespace sippy::util {

std::string to_hex_string(std::span<const uint8_t> buffer);
std::vector<uint8_t> from_hex_string(std::string_view str);

}
