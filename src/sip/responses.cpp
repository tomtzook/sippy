
#include <sip/responses.h>

namespace sippy::sip {

message_ptr create_request(
    const sip::status_code code,
    const sip::message& original_request,
    const uint32_t expires,
    const uint32_t max_forwards,
    const std::optional<std::string_view> phrase) {
    auto msg = std::make_unique<sip::message>();

    {
        status_line line;
        line.version = version::version_2_0;
        line.code = code;
        line.reason_phrase = phrase.has_value() ? phrase.value() : status_code_reason_phrase(code);
        msg->set_status_line(std::move(line));
    }

    msg->copy_headers<headers::from>(original_request);
    msg->copy_headers<headers::to>(original_request);
    msg->copy_headers<headers::call_id>(original_request);
    msg->copy_headers<headers::via>(original_request);
    msg->copy_headers<headers::record_route>(original_request);
    msg->copy_headers<headers::cseq>(original_request);

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

}
