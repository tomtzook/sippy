#pragma once

#include <sip/requests.h>

namespace sippy::sip {

class account {
public:
    virtual ~account() = default;

    [[nodiscard]] virtual std::string_view username() const = 0;
    [[nodiscard]] virtual std::string_view host() const = 0;

    virtual headers::authorization create_auth_header() = 0;
    virtual headers::authorization create_auth_response_header(const headers::www_authorization& auth_header) = 0;
};

using account_ptr = std::shared_ptr<account>;

class sim_account final : public account {
public:
    sim_account(
        std::string_view imsi,
        uint16_t mcc, uint16_t mnc,
        std::string_view ki, std::string_view opc, std::string_view amf);

    [[nodiscard]] std::string_view username() const override;
    [[nodiscard]] std::string_view host() const override;

    headers::authorization create_auth_header() override;
    headers::authorization create_auth_response_header(const headers::www_authorization& auth_header) override;

private:
    std::string m_imsi;
    std::string m_host;
    ki m_ki;
    opc m_opc;
    amf m_amf;
};

}
