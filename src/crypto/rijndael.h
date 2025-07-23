#pragma once

#include <cstdint>

namespace crypto::rijndael {

void key_schedule(const uint8_t key[16], uint32_t ekey[44]);
void encrypt(const uint8_t in[16], const uint32_t ekey[44], uint8_t out[16]);

}
