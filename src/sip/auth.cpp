
#include <cstring>
#include <sstream>
#include <iomanip>

#include <sip/auth.h>

#include "crypto/milenge.h"
#include "util/base64.h"
#include "util/hash.h"
#include "util/hex.h"

namespace sippy::sip {

#pragma pack(push, 1)

struct nonce_data {
    crypto::milenge::rand rand;
    uint8_t sqnxoraka[6];
    crypto::milenge::amf amf;
    crypto::milenge::net_auth mac;
};

#pragma pack(pop)

class bad_server_info final : std::exception {
public:
    const char* what() const noexcept override {
        return "Server information does not match local information";
    }
};

static std::string nc_to_string(const uint32_t nc) {
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(8) << nc;
    return ss.str();
}

static std::vector<uint8_t> hash_md5_first(
    const std::string_view username,
    const std::span<const uint8_t> password,
    const std::string_view realm) {
    util::hash_md5 hash;
    hash.update(username);
    hash.update(":");
    hash.update(realm);
    hash.update(":");
    hash.update(password);

    return hash.finalize();
}

static std::vector<uint8_t> hash_md5_second(
    const sip::method method,
    const std::string_view uri) {
    std::stringstream ss;
    ss << method;

    util::hash_md5 hash;
    hash.update(ss.str());
    hash.update(":");
    hash.update(uri);

    return hash.finalize();
}

void sim_keys_to_password(const ki ki, const opc opc, const amf amf, const std::string_view nonce, res out) {
    const auto nonce_decoded = util::base64_decode(nonce);
    const auto* nonce_part = reinterpret_cast<const nonce_data*>(nonce_decoded.data());

    crypto::milenge::xres xres;
    crypto::milenge::ak ak;
    crypto::milenge::f2_f5(ki, nonce_part->rand, opc, xres, ak);

    crypto::milenge::sqn sqn;
    crypto::milenge::buffer_xor<sizeof(sqn)>(nonce_part->sqnxoraka, ak, sqn);

    crypto::milenge::net_auth xmac;
    crypto::milenge::resync_auth resync;
    crypto::milenge::f1(ki, sqn, nonce_part->rand, opc, amf, xmac, resync);

    if (memcmp(xmac, nonce_part->mac, sizeof(xmac)) != 0) {
        // network information does not match ours
        throw bad_server_info();
    }

    memcpy(out, xres, sizeof(xres));
}

std::string auth_md5(
    const std::string_view username,
    const std::span<const uint8_t> password,
    const sip::method method,
    const std::string_view realm,
    const std::string_view uri,
    const std::string_view nonce,
    const std::string_view cnonce,
    const uint32_t nc,
    const std::string_view qop) {
    auto a1 = hash_md5_first(username, password, realm);
    auto a2 = hash_md5_second(method, uri);

    util::hash_md5 hash;
    hash.update(util::to_hex_string(a1));
    hash.update(":");
    hash.update(nonce);
    hash.update(":");
    hash.update(nc_to_string(nc));
    hash.update(":");
    hash.update(cnonce);
    hash.update(":");
    hash.update(qop);
    hash.update(":");
    hash.update(util::to_hex_string(a2));

    auto resp = hash.finalize();
    return util::to_hex_string(resp);
}

std::string auth_aka(
    const std::string_view username,
    const ki ki,
    const opc opc,
    const amf amf,
    const sip::method method,
    const std::string_view realm,
    const std::string_view uri,
    const std::string_view nonce,
    const std::string_view cnonce,
    const uint32_t nc,
    const std::string_view qop) {
    res password;
    sim_keys_to_password(ki, opc, amf, nonce, password);

    return auth_md5(username, password, method, realm, uri, nonce, cnonce, nc, qop);
}

}
