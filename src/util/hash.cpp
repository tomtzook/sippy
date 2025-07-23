
#include "hash.h"

namespace sippy::util {

hash_md5::hash_md5()
    : m_ctx(nullptr) {
    auto ctx = evp_md_ctx(EVP_MD_CTX_new());
    if (1 != EVP_DigestInit_ex(ctx.get(), EVP_md5(), nullptr)) {
        throw std::runtime_error("evp init failed");
    }

    m_ctx = std::move(ctx);
}

void hash_md5::update(const std::span<const uint8_t> data) {
    if (1 != EVP_DigestUpdate(m_ctx.get(), data.data(), data.size())) {
        throw std::runtime_error("evp digest update failed");
    }
}

void hash_md5::update(const std::string_view data) {
    if (1 != EVP_DigestUpdate(m_ctx.get(), data.data(), data.size())) {
        throw std::runtime_error("evp digest update failed");
    }
}

std::vector<uint8_t> hash_md5::finalize() {
    std::vector<uint8_t> buffer;
    buffer.resize(EVP_MAX_MD_SIZE);
    uint32_t size;
    if (1 != EVP_DigestFinal_ex(m_ctx.get(), buffer.data(), &size)) {
        throw std::runtime_error("evp finalize failed");
    }

    buffer.resize(size);

    return std::move(buffer);
}

}
