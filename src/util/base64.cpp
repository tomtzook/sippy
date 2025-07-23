#include <array>
#include <cstdint>

#include "base64.h"


namespace sippy::util {

constexpr char encode_table[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
    'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
    'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
    's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2',
    '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

constexpr char decode_table[] = {
    0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
    0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64,
    0x64, 0x64, 0x64, 0x64, 0x64, 0x3E, 0x64, 0x64, 0x64, 0x3F, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C,
    0x3D, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x64, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A,
    0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x64, 0x64, 0x64, 0x64,
    0x64, 0x64, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A,
    0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x64, 0x64, 0x64, 0x64, 0x64
};

static std::array<char, 4> encode_triplet(const uint8_t a, const uint8_t b, const uint8_t c) {
    const uint32_t concat_bits = (a << 16) | (b << 8) | c;

    const auto b64_char1 = encode_table[(concat_bits >> 18) & 0b0011'1111];
    const auto b64_char2 = encode_table[(concat_bits >> 12) & 0b0011'1111];
    const auto b64_char3 = encode_table[(concat_bits >> 6) & 0b0011'1111];
    const auto b64_char4 = encode_table[concat_bits & 0b0011'1111];
    return {b64_char1, b64_char2, b64_char3, b64_char4};
}

static std::array<uint8_t, 3> decode_quad(const char a, const char b, const char c, const char d) {
     const uint32_t concat_bytes =
        (decode_table[a] << 18) | (decode_table[b] << 12) |
        (decode_table[c] << 6) | decode_table[d];

    const uint8_t byte1 = (concat_bytes >> 16) & 0b1111'1111;
    const uint8_t byte2 = (concat_bytes >> 8) & 0b1111'1111;
    const uint8_t byte3 = concat_bytes & 0b1111'1111;
    return {byte1, byte2, byte3};
}

std::vector<uint8_t> base64_decode(const std::string_view data) {
    const auto result_len = (data.size() * 3/4) + 8;
    std::vector<uint8_t> result(result_len);

    const auto* data_ptr = data.data();
    auto* out_ptr = result.data();
    size_t out_idx = 0;
    size_t i;
    for (i = 0; i + 3 < data.size(); i += 4) {
        const auto decoded = decode_quad(
            data_ptr[i], data_ptr[i + 1], data_ptr[i + 2], data_ptr[i + 3]);
        out_ptr[out_idx++] = decoded[0];
        out_ptr[out_idx++] = decoded[1];
        out_ptr[out_idx++] = decoded[2];
    }

    if (i < data.size()) {
        char a, b, c, d;

        a = data_ptr[i];

        if (i + 1 < data.size()) {
            b = data_ptr[i + 1];
        } else {
            b = '=';
        }

        if (i + 2 < data.size()) {
            c = data_ptr[i + 2];
        } else {
            c = '=';
        }

        if (i + 3 < data.size()) {
            d = data_ptr[i + 3];
        } else {
            d = '=';
        }

        const auto decoded = decode_quad(a, b, c, d);
        out_ptr[out_idx++] = decoded[0];
        out_ptr[out_idx++] = decoded[1];
        out_ptr[out_idx++] = decoded[2];
    }

    out_ptr[out_idx++] = 0;
    result.resize(out_idx);
    return std::move(result);
}

}
