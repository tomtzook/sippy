
#include <format>

#include <sip/requests.h>
#include "util/hex.h"

namespace sippy::sip {

static std::string generate_cnonce(const size_t length) {
    return util::random_hex_string(length);
}

static headers::authorization create_request_authorization(
    const headers::www_authorization& auth_details,
    const std::string_view user_host,
    const std::string_view username,
    const std::string_view cnonce,
    const uint32_t nc,
    std::string&& response) {
    headers::authorization auth{};
    auth.scheme = auth_details.scheme;
    auth.algorithm = auth_details.algorithm;
    auth.uri = std::format("sip:{}", user_host);
    auth.username = username;
    auth.realm = user_host;
    auth.qop = auth_details.qop;
    auth.nonce = auth_details.nonce;
    auth.cnonce = cnonce;
    auth.nc = nc;
    auth.response = std::move(response);

    return std::move(auth);
}

message_ptr create_request(
    const sip::method method,
    const std::string_view target_uri,
    const std::string_view from_uri,
    const std::string_view to_uri,
    const std::string_view call_id,
    const uint32_t sequence_num,
    const uint32_t expires,
    const uint32_t max_forwards) {
    auto msg = std::make_unique<sip::message>();

    {
        request_line line;
        line.method = method;
        line.version = version::version_2_0;
        line.uri = target_uri;
        msg->set_request_line(std::move(line));
    }
    {
        headers::from from;
        from.uri = from_uri;
        msg->add_header(std::move(from));
    }
    {
        headers::to to;
        to.uri = to_uri;
        msg->add_header(std::move(to));
    }
    {
        headers::call_id call_id_h;
        call_id_h.value = call_id;
        msg->add_header(std::move(call_id_h));
    }
    {
        headers::cseq cseq;
        cseq.seq_num = sequence_num;
        cseq.method = method;
        msg->add_header(std::move(cseq));
    }
    {
        headers::expires expires_h;
        expires_h.value = expires;
        msg->add_header(std::move(expires_h));
    }
    {
        headers::max_forwards max_forwards_h;
        max_forwards_h.value = max_forwards;
        msg->add_header(std::move(max_forwards_h));
    }

    return std::move(msg);
}

message_ptr create_request_register(
    const std::string_view target_uri,
    const std::string_view from_uri,
    const std::string_view call_id,
    const uint32_t sequence_num,
    const uint32_t expires,
    const uint32_t max_forwards) {
    return create_request(
        sip::method::register_,
        target_uri,
        from_uri,
        from_uri,
        call_id,
        sequence_num,
        expires,
        max_forwards);
}

headers::authorization create_request_authorization(
    const auth_scheme scheme,
    const auth_algorithm algorithm,
    const std::string_view user_host,
    const std::string_view username) {
    headers::authorization auth{};
    auth.scheme = scheme;
    auth.algorithm = algorithm;
    auth.uri = std::format("sip:{}", user_host);
    auth.username = std::format("{}@{}", username, user_host);
    auth.realm = user_host;
    auth.qop = "auth";

    return std::move(auth);
}

headers::authorization create_request_authorization(
    const headers::www_authorization& auth_header,
    const std::string_view user_host,
    const std::string_view username,
    const std::span<const uint8_t> password,
    const std::optional<std::string_view> cnonce,
    const std::optional<uint32_t> nc) {
    std::string cnonce_value;
    if (cnonce.has_value()) {
        cnonce_value = cnonce.value();
    } else {
        cnonce_value = generate_cnonce(auth_header.nonce.length());
    }

    uint32_t nc_value;
    if (nc.has_value()) {
        nc_value = nc.value();
    } else {
        nc_value = 1;
    }

    const auto uri = std::format("sip:{}", user_host);
    auto response = create_auth_response_md5_for_register(username, uri, password, auth_header, cnonce_value, nc_value);
    return create_request_authorization(auth_header, user_host, username, cnonce_value, nc_value, std::move(response));
}

headers::authorization create_request_authorization(
    const headers::www_authorization& auth_header,
    const std::string_view user_host,
    const std::string_view username,
    const ki ki,
    const opc opc,
    const amf amf,
    const std::optional<std::string_view> cnonce,
    const std::optional<uint32_t> nc) {
    std::string cnonce_value;
    if (cnonce.has_value()) {
        cnonce_value = cnonce.value();
    } else {
        cnonce_value = generate_cnonce(auth_header.nonce.length());
    }

    uint32_t nc_value;
    if (nc.has_value()) {
        nc_value = nc.value();
    } else {
        nc_value = 1;
    }

    const auto uri = std::format("sip:{}", user_host);
    auto response = create_auth_response_aka_for_register(username, uri, ki, opc, amf, auth_header, cnonce_value, nc_value);
    return create_request_authorization(auth_header, user_host, username, cnonce_value, nc_value, std::move(response));
}

}
