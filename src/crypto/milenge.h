#pragma once

#include <cstdint>
#include <span>


namespace crypto::milenge {

using ki = uint8_t[16];
using opc = uint8_t[16];
using amf = uint8_t[2];
using rand = uint8_t[16];
using sqn = uint8_t[6];
using xres = uint8_t[8];
using ak = uint8_t[6];
using net_auth = uint8_t[8];
using resync_auth = uint8_t[8];

template<size_t N>
void buffer_xor(const uint8_t first[N], const uint8_t second[N], uint8_t result[N]) {
    for (size_t i = 0; i < N; i++) {
        result[i] = first[i] ^ second[i];
    }
}

template<size_t N>
void buffer_rotate(const uint8_t first[N], const uint8_t second[N], uint8_t result[N]) {
    constexpr size_t N_2 = N / 2;
    for (size_t i = 0; i < N; i++) {
        result[(i+N_2) % N] = first[i] ^ second[i];
    }
}

void f1(const ki key, const sqn sqn, const rand rand, const opc opc, const amf amf, net_auth mac_a, resync_auth mac_s);
void f2_f5(const ki key, const rand rand, const opc opc, xres res, ak ak);

}
