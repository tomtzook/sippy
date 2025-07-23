#pragma once

#include <span>
#include <string_view>
#include <memory>
#include <vector>
#include <openssl/evp.h>

namespace sippy::util {

struct evp_md_ctx_free {
    void operator()(EVP_MD_CTX* ctx) const {
        EVP_MD_CTX_free(ctx);
    }
};

using evp_md_ctx = std::unique_ptr<EVP_MD_CTX, evp_md_ctx_free>;

class hash_md5 {
public:
    hash_md5();

    void update(std::span<const uint8_t> data);
    void update(std::string_view data);
    std::vector<uint8_t> finalize();

private:
    evp_md_ctx m_ctx;
};

}
