
#include <format>

#include <sip/requests.h>

namespace sippy::sip {

message_ptr create_request(
    const sip::method method,
    const std::string_view target_uri,
    const std::string_view from_uri,
    const std::string_view to_uri,
    const sip::transport transport,
    const std::string_view via_address,
    const uint16_t via_port,
    const std::string_view tag,
    const std::string_view branch,
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
        from.tag = tag;
        msg->add_header(std::move(from));
    }
    {
        headers::to to;
        to.uri = to_uri;
        msg->add_header(std::move(to));
    }
    {
        headers::via via;
        via.version = version::version_2_0;
        via.transport = transport;
        via.host = via_address;
        via.port = via_port;
        via.tags["branch"] = branch;
        msg->add_header(std::move(via));
    }
    {
        headers::contact contact;
        contact.uri = std::format("sip:{}@{}", via_address, via_port);
        msg->add_header(std::move(contact));
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
    const sip::transport transport,
    const std::string_view via_address,
    const uint16_t via_port,
    const std::string_view tag,
    const std::string_view branch,
    const std::string_view call_id,
    const uint32_t sequence_num,
    const uint32_t expires,
    const uint32_t max_forwards) {
    return create_request(
        sip::method::register_,
        target_uri,
        from_uri,
        from_uri,
        transport,
        via_address,
        via_port,
        tag,
        branch,
        call_id,
        sequence_num,
        expires,
        max_forwards);
}

headers::authorization create_request_authorization(
    const auth_scheme scheme,
    const auth_algorithm algorithm,
    const std::string_view user_name,
    const std::string_view user_host) {
    headers::authorization auth{};
    auth.scheme = scheme;
    auth.algorithm = algorithm;
    auth.uri = std::format("sip:{}", user_host);
    auth.username = std::format("{}@{}", user_name, user_host);
    auth.realm = user_host;
    auth.qop = "auth";

    return std::move(auth);
}

headers::authorization create_request_authorization(
    const headers::www_authorization& auth_details,
    std::string_view user_name,
    std::string_view user_host) {
    headers::authorization auth{};
    auth.scheme = auth_details.scheme;
    auth.algorithm = auth_details.algorithm;
    auth.uri = std::format("sip:{}", user_host);
    auth.username = user_name;
    auth.realm = user_host;
    auth.qop = auth_details.qop;
    auth.nonce = auth_details.nonce;
    auth.nc = 1;

    return std::move(auth);
}

}
