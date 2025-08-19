
#include <sdp/fields.h>
#include <sdp/message.h>

#include "serialization/reader.h"
#include "sip/reader.h"
#include "util/string_helper.h"


static bool is_valid_string(const std::string_view str, const bool allow_whitespace = false) {
    if (str.empty()) {
        return false;
    }

    if (str.find('\r') != std::string::npos || str.find('\n') != std::string::npos) {
        return false;
    }

    if (!allow_whitespace && str.find(' ') != std::string::npos) {
        return false;
    }

    return true;
}


DEFINE_SDP_FIELD_VALIDATOR(version) {
    return f.ver == sdp::version::version_0;
}

DEFINE_SDP_FIELD_READ(version) {
    is >> f.ver;
}

DEFINE_SDP_FIELD_WRITE(version) {
    os << f.ver;
}

DEFINE_SDP_FIELD_VALIDATOR(origin) {
    if (!is_valid_string(f.username)) {
        return false;
    }

    if (!is_valid_string(f.session_id) || !util::is_numeric_string(f.session_id)) {
        return false;
    }

    // todo: validate address by type
    if (!is_valid_string(f.unicast_address)) {
        return false;
    }

    return true;
}

DEFINE_SDP_FIELD_READ(origin) {
    serialization::reader reader(is);
    f.username = reader.read_until(serialization::is_whitespace);
    f.session_id = reader.read_until(serialization::is_whitespace);
    is >> f.session_version;
    reader.eat(' ');
    is >> f.net_type;
    reader.eat(' ');
    is >> f.addr_type;
    reader.eat(' ');
    f.unicast_address = reader.read_until(serialization::is_new_line);
}

DEFINE_SDP_FIELD_WRITE(origin) {
    os << f.username;
    os << ' ';
    os << f.session_id;
    os << ' ';
    os << f.session_version;
    os << ' ';
    os << f.net_type;
    os << ' ';
    os << f.addr_type;
    os << ' ';
    os << f.unicast_address;
}

DEFINE_SDP_FIELD_VALIDATOR(session_name) {
    return is_valid_string(f.name, true);
}

DEFINE_SDP_FIELD_READ(session_name) {
    is >> f.name;
}

DEFINE_SDP_FIELD_WRITE(session_name) {
    os << f.name;
}

DEFINE_SDP_FIELD_VALIDATOR(session_information) {
    return is_valid_string(f.information, true);
}

DEFINE_SDP_FIELD_READ(session_information) {
    is >> f.information;
}

DEFINE_SDP_FIELD_WRITE(session_information) {
    os << f.information;
}

DEFINE_SDP_FIELD_VALIDATOR(uri) {
    // todo: verify URI
    return is_valid_string(f.uri_str, true);
}

DEFINE_SDP_FIELD_READ(uri) {
    is >> f.uri_str;
}

DEFINE_SDP_FIELD_WRITE(uri) {
    os << f.uri_str;
}

DEFINE_SDP_FIELD_VALIDATOR(email) {
    if (f.display_name.has_value() && !is_valid_string(f.display_name.value(), true)) {
        return false;
    }

    if (!is_valid_string(f.email_address)) {
        return false;
    }

    return true;
}

DEFINE_SDP_FIELD_READ(email) {
    static const auto regex1 = R"(^(?:[\w\d\s._-]+\s)<?([\w\d._]+@[\w]+\.[\w\d]+)>?$)";
    static const auto regex2 = R"(^([\w\d._]+@[\w]+\.[\w\d]+)(?:\s\([\w\d\s._-]+\))$)";

    serialization::reader reader(is);
    auto str = reader.read_until(serialization::is_new_line);

    auto match_opt = serialization::try_parse(str, regex1);
    if (match_opt) {
        const auto& match = match_opt.value();
        if (match[1].matched) {
            f.display_name = match[1].str();
        } else {
            f.display_name = std::nullopt;
        }

        f.email_address = match[2].str();
    } else {
        const auto match = serialization::parse(str, regex2);
        if (match[1].matched) {
            f.display_name = match[1].str();
        } else {
            f.display_name = std::nullopt;
        }

        f.email_address = match[2].str();
    }
}

DEFINE_SDP_FIELD_WRITE(email) {
    bool enclose_email = false;
    if (f.display_name.has_value()) {
        os << f.display_name.value();
        os << ' ';
        enclose_email = true;
    }

    if (enclose_email) os << '<';
    os << f.email_address;
    if (enclose_email) os << '>';
}

DEFINE_SDP_FIELD_VALIDATOR(phone_number) {
    if (f.display_name.has_value() && !is_valid_string(f.display_name.value(), true)) {
        return false;
    }

    // todo: valid phone number
    if (!is_valid_string(f.number)) {
        return false;
    }

    return true;
}

DEFINE_SDP_FIELD_READ(phone_number) {
    static const auto regex1 = R"(^(?:[\w\d\s._-]+\s)<?(+?[\w\d._]+)>?$)";
    static const auto regex2 = R"(^(+?[\w\d._]+)(?:\s\([\w\d\s._-]+\))$)";

    serialization::reader reader(is);
    auto str = reader.read_until(serialization::is_new_line);

    auto match_opt = serialization::try_parse(str, regex1);
    if (match_opt) {
        const auto& match = match_opt.value();
        if (match[1].matched) {
            f.display_name = match[1].str();
        } else {
            f.display_name = std::nullopt;
        }

        f.number = match[2].str();
    } else {
        const auto match = serialization::parse(str, regex2);
        if (match[1].matched) {
            f.display_name = match[1].str();
        } else {
            f.display_name = std::nullopt;
        }

        f.number = match[2].str();
    }
}

DEFINE_SDP_FIELD_WRITE(phone_number) {
    bool enclose_number = false;
    if (f.display_name.has_value()) {
        os << f.display_name.value();
        os << ' ';
        enclose_number = true;
    }

    if (enclose_number) os << '<';
    os << f.number;
    if (enclose_number) os << '>';
}

DEFINE_SDP_FIELD_VALIDATOR(connection_information) {
    // todo: valid base address based on type
    // todo: validate net and addr types

    if (f.addr_type != address_type::ipv4 && f.ttl.has_value()) {
        return false;
    }

    return true;
}

DEFINE_SDP_FIELD_READ(connection_information) {
    serialization::reader reader(is);

    is >> f.net_type;
    reader.eat(' ');
    is >> f.addr_type;
    reader.eat(' ');
    f.base_address = reader.read_until(serialization::is_slash_or_new_line);

    const auto expect_ttl = f.addr_type == address_type::ipv4;
    f.ttl = std::nullopt;
    f.num_of_addresses = std::nullopt;

    if (reader.eat_one_if(serialization::is_slash)) {
        uint16_t num;
        is >> num;

        if (expect_ttl) {
            f.ttl = num;
        } else {
            f.num_of_addresses = num;
        }
    }
    if (expect_ttl && reader.eat_one_if(serialization::is_slash)) {
        uint16_t num;
        is >> num;
        f.num_of_addresses = num;
    }
}

DEFINE_SDP_FIELD_WRITE(connection_information) {
    os << f.net_type;
    os << ' ';
    os << f.addr_type;
    os << ' ';
    os << f.base_address;

    if (f.ttl.has_value()) {
        os << '/';
        os << f.ttl.value();
    }
    if (f.num_of_addresses.has_value()) {
        os << '/';
        os << f.num_of_addresses.value();
    }
}

DEFINE_SDP_FIELD_VALIDATOR(bandwidth_information) {
    // todo: validate
    return true;
}

DEFINE_SDP_FIELD_READ(bandwidth_information) {
    serialization::reader reader(is);
    f.bw_type = reader.read_until(serialization::is_colon);
    reader.eat(':');
    f.bandwidth = reader.read_until(serialization::is_new_line);
}

DEFINE_SDP_FIELD_WRITE(bandwidth_information) {
    os << f.bw_type;
    os << ':';
    os << f.bandwidth;
}

DEFINE_SDP_FIELD_VALIDATOR(time_active) {
    // todo: validate
    return true;
}

DEFINE_SDP_FIELD_READ(time_active) {
    serialization::reader reader(is);
    is >> f.start_time;
    reader.eat(' ');
    is >> f.stop_time;
}

DEFINE_SDP_FIELD_WRITE(time_active) {
    os << f.start_time;
    os << ' ';
    os << f.stop_time;
}

DEFINE_SDP_FIELD_VALIDATOR(repeat_times) {
    // todo: validate
    return true;
}

DEFINE_SDP_FIELD_READ(repeat_times) {
    // todo: support time units
    serialization::reader reader(is);
    is >> f.repeat_interval;
    reader.eat(' ');
    is >> f.active_duration;
    reader.eat(' ');
    is >> f.offset1;
    reader.eat(' ');
    is >> f.offset2;
}

DEFINE_SDP_FIELD_WRITE(repeat_times) {
    os << f.repeat_interval;
    os << ' ';
    os << f.active_duration;
    os << ' ';
    os << f.offset1;
    os << ' ';
    os << f.offset2;
}

DEFINE_SDP_FIELD_VALIDATOR(timezone) {
    if (f.adjustments.empty()) {
        return false;
    }

    return true;
}

static sippy::sdp::fields::timezone_adjustment read_timezone_adjustment(std::istream& is, sippy::serialization::reader& reader) {
    using namespace sippy;

    sdp::fields::timezone_adjustment adjustment{};
    is >> adjustment.adjustment_time;
    reader.eat(' ');
    adjustment.offset_back = reader.eat_one_if(serialization::is_dash);
    is >> adjustment.offset;

    return adjustment;
}

DEFINE_SDP_FIELD_READ(timezone) {
    // todo: support time units
    serialization::reader reader(is);

    // must have one
    do {
        f.adjustments.push_back(read_timezone_adjustment(is, reader));
    } while (reader.read_one_if(serialization::is_whitespace));
}

DEFINE_SDP_FIELD_WRITE(timezone) {
    for (auto it = f.adjustments.cbegin(); it != f.adjustments.cend();) {
        os << it->adjustment_time;
        os << ' ';

        if (it->offset_back) {
            os << '-';
        }
        os << it->offset;

        ++it;
        if (it != f.adjustments.cend()) {
            os << ' ';
        }
    }
}

DEFINE_SDP_FIELD_VALIDATOR(media_description) {
    return true;
}

DEFINE_SDP_FIELD_READ(media_description) {
    serialization::reader reader(is);
    is >> f.media_type;
    reader.eat(' ');

    is >> f.port;
    if (reader.eat_one_if(serialization::is_slash)) {
        is >> f.number_of_ports;
    }
    reader.eat(' ');

    is >> f.protocol;

    while (reader.eat_one_if(serialization::is_whitespace)) {
        auto format = reader.read_while(serialization::is_alphanumeric);
        f.formats.push_back(format);
    }
}

DEFINE_SDP_FIELD_WRITE(media_description) {
    os << f.media_type;
    os << ' ';
    os << f.port;
    if (f.number_of_ports > 1) {
        os << '/';
        os << f.number_of_ports;
    }
    os << ' ';
    os << f.protocol;

    for (auto it = f.formats.cbegin(); it != f.formats.cend(); ) {
        os << *it;

        ++it;
        if (it != f.formats.cend()) {
            os << ' ';
        }
    }
}

DEFINE_SDP_FIELD_VALIDATOR(attribute) {
    return true;
}

DEFINE_SDP_FIELD_READ(attribute) {
    serialization::reader reader(is);
    auto value = reader.read_until(serialization::is_colon);
    if (reader.eat_one_if(serialization::is_colon)) {
        f.name.emplace(std::move(value));

        value = reader.read_until(serialization::is_new_line);
        f.value = value;
    }
}

DEFINE_SDP_FIELD_WRITE(attribute) {
    if (f.name.has_value()) {
        os << f.name.value();
        os << ':';
    }
    os << f.value;
}
