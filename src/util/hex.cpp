
#include <sstream>
#include <iomanip>

#include "hex.h"

namespace sippy::util {

std::string to_hex_string(const std::span<const uint8_t> buffer) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < buffer.size_bytes(); i++) {
        ss << std::setw(2) << static_cast<int>(buffer[i]);
    }

    return ss.str();
}

std::vector<uint8_t> from_hex_string(const std::string_view str) {
    if (str.length() % 2 != 0) {
        throw std::invalid_argument("Hex string must have an even number of characters.");
    }

    std::vector<uint8_t> buffer;
    buffer.reserve(str.length() / 2);

    char hex_byte[3];
    hex_byte[2] = '\0';

    for (size_t i = 0; i < str.length(); i += 2) {
        hex_byte[0] = str[i];
        hex_byte[1] = str[i + 1];
        const auto byte = static_cast<uint8_t>(std::stoi(hex_byte, nullptr, 16));
        buffer.push_back(byte);
    }

    return std::move(buffer);
}

}
