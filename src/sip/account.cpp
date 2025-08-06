
#include <format>

#include <sip/account.h>


namespace sippy::sip {

static std::string create_host_from_mccmnc(uint16_t mcc, uint16_t mnc) {
    return std::format("ims.mnc{:03}.mcc{:03}.3gppnetwork.org", mcc, mnc);
}

sim_account::sim_account(
    const std::string_view imsi,
    const uint16_t mcc,
    const uint16_t mnc,
    const std::string_view ki,
    const std::string_view opc,
    const std::string_view amf)
    : m_imsi(imsi)
    , m_host(create_host_from_mccmnc(mcc, mnc))
    , m_ki()
    , m_opc()
    , m_amf() {
    ki_from_hex(ki, m_ki);
    opc_from_hex(opc, m_opc);
    amf_from_hex(amf, m_amf);
}

std::string_view sim_account::username() const {
    return m_imsi;
}

std::string_view sim_account::host() const {
    return m_host;
}

headers::authorization sim_account::create_auth_header() {
    return create_request_authorization(auth_scheme::digest, auth_algorithm::aka, m_host, m_imsi);
}

headers::authorization sim_account::create_auth_response_header(const headers::www_authorization& auth_header) {

    return create_request_authorization(auth_header, m_host, m_imsi, m_ki, m_opc, m_amf);
}

}
