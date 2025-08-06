
#include <cstring>

#include "milenge.h"

#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/types.h>

namespace crypto::milenge {

#pragma pack(push, 1)

static constexpr size_t n1_size = (sizeof(sqn) + sizeof(amf)) * 2;
union in1 {
    struct {
        milenge::sqn sqn1;
        milenge::amf amf1;
        milenge::sqn sqn2;
        milenge::amf amf2;
    };
    uint8_t data[n1_size];
};

#pragma pack(pop)

static size_t aes128encrypt(const uint8_t* key, const std::span<const uint8_t> data, uint8_t* out) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if(ctx == nullptr) {
        throw std::runtime_error("Failed to allocate");
    }

    uint8_t iv[16] = {};
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, key, iv)) {
        throw std::runtime_error("Failed to init the encryption");
    }

    EVP_CIPHER_CTX_set_padding(ctx, 0);

    size_t total_out_len = 0;
    int out_len;
    if(1 != EVP_EncryptUpdate(ctx, out, &out_len, data.data(), data.size_bytes())) {
        throw std::runtime_error("Failed to update the encryption");
    }
    total_out_len += out_len;

    if(1 != EVP_EncryptFinal_ex(ctx, out + total_out_len, &out_len)) {
        throw std::runtime_error("Failed to finalize the encryption");
    }
    total_out_len += out_len;

    EVP_CIPHER_CTX_free(ctx);

    return total_out_len;
}

void f1(const ki key, const sqn sqn, const rand rand, const opc opc, const amf amf, net_auth mac_a, resync_auth mac_s) {
    uint8_t rijndaelInput[16];
    uint8_t temp[16];
    uint8_t out1[16];

    // TEMP = E_K(RAND XOR OP_C)
    buffer_xor<sizeof(rijndaelInput)>(rand, opc, rijndaelInput);
    // rijndael::encrypt(rijndaelInput, rijndaelEkey, temp);
    aes128encrypt(key, rijndaelInput, temp);

    // IN1 = SQN || AMF || SQN || AMF
    milenge::in1 in1{};
    memcpy(in1.sqn1, sqn, sizeof(in1.sqn1));
    memcpy(in1.sqn2, sqn, sizeof(in1.sqn2));
    memcpy(in1.amf1, amf, sizeof(in1.amf1));
    memcpy(in1.amf2, amf, sizeof(in1.amf2));

    // OUT1 = E_K(TEMP XOR rotate(IN1 XOR OP_C, r1) XOR c1) XOR OP_C
    buffer_rotate<sizeof(rijndaelInput)>(in1.data, opc, rijndaelInput);
    buffer_xor<sizeof(rijndaelInput)>(rijndaelInput, temp, rijndaelInput);

    aes128encrypt(key, rijndaelInput, out1);
    buffer_xor<sizeof(out1)>(out1, opc, out1);

    // MAC-A = f1 = OUT1[0] .. OUT1[63]
    // MAC-S = f1* = OUT1[64] .. OUT1[127]
    memcpy(mac_a, out1, sizeof(net_auth));
    memcpy(mac_s, out1 + sizeof(net_auth), sizeof(resync_auth));
}

void f2_f5(const ki key, const rand rand, const opc opc, xres res, ak ak) {
    uint8_t rijndaelInput[16];
    uint8_t temp[16];
    uint8_t out[16];

    // TEMP = E_K(RAND XOR OP_C)
    buffer_xor<16>(rand, opc, rijndaelInput);

    aes128encrypt(key, rijndaelInput, temp);

    // OUT2 = E_K(rotate(TEMP XOR OP_C, r2) XOR c2) XOR OP_C
    buffer_xor<16>(temp, opc, rijndaelInput);
    rijndaelInput[15] ^= 1;

    aes128encrypt(key, rijndaelInput, out);
    buffer_xor<16>(out, opc, out);

    // res = f2 = OUT2[64] ... OUT2[127]
    // ak = f5 = OUT2[0] ... OUT2[47]
    memcpy(res, out + 8, sizeof(xres));
    memcpy(ak, out, sizeof(milenge::ak));
}

}
