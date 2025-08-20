
#include <sstream>

#include <sdp/description.h>

namespace sippy::sdp {

static fields::connection_information connection_to_field(const connection_information& info) {
    fields::connection_information field{};
    field.net_type = info.address.network_type;
    field.addr_type = info.address.address_type;
    field.base_address = info.address.address_value;

    if (info.count > 1) {
        field.num_of_addresses = info.count;
    } else {
        field.num_of_addresses = std::nullopt;
    }

    if (info.ttl > 0) {
        field.ttl = info.ttl;
    } else {
        field.ttl = std::nullopt;
    }

    return field;
}

static connection_information connection_from_field(const fields::connection_information& field) {
    connection_information info{};
    info.address.network_type = field.net_type;
    info.address.address_type = field.addr_type;
    info.address.address_value = field.base_address;

    if (field.num_of_addresses.has_value()) {
        info.count = field.num_of_addresses.value();
    } else {
        info.count = 1;
    }

    if (field.ttl.has_value()) {
        info.ttl = field.ttl.value();
    } else {
        info.ttl = 0;
    }

    return info;
}

void media_information::set_information(const std::string_view info) {
    m_information = info;
}

void media_information::add_connection(connection_information&& information) {
    m_connections.push_back(std::move(information));
}

void media_information::add_connection_ipv4(const std::string_view base_address, const uint16_t count, const uint16_t ttl) {
    connection_information info{};
    info.address.network_type = network_type::in;
    info.address.address_type = address_type::ipv4;
    info.address.address_value = base_address;

    if (info.count > 1) {
        info.count = count;
    }

    if (ttl != 0) {
        info.ttl = ttl;
    }

    add_connection(std::move(info));
}

void media_information::set_bandwidth(std::string_view type, std::string_view bandwidth) {
    m_bandwidths.emplace(type, bandwidth);
}

time_description::repeat_builder::repeat_builder(time_description& parent, const std::chrono::seconds interval, const std::chrono::seconds active_duration)
    : m_parent(parent)
    , m_repeat() {
    m_repeat.interval = interval;
    m_repeat.active_duration = active_duration;
}

time_description::repeat_builder::~repeat_builder() {
    m_parent.m_repeats.push_back(std::move(m_repeat));
}

time_description::repeat_builder& time_description::repeat_builder::add_offset(const std::chrono::seconds offset) {
    m_repeat.offsets.push_back(offset);
    return *this;
}

time_description::repeat_builder& time_description::repeat_builder::tz_adjust(const std::chrono::seconds adjustment_time, const std::chrono::seconds offset) {
    timezone tz{};
    tz.adjustment_time = adjustment_time;
    tz.offset = offset;
    m_repeat.timezones.push_back(std::move(tz));

    return *this;
}

time_description::time_description(const std::chrono::seconds start_time, const std::chrono::seconds stop_time)
    : m_start_time(start_time)
    , m_stop_time(stop_time)
    , m_repeats()
{}

time_description::repeat_builder time_description::add_repeat(const std::chrono::seconds interval, const std::chrono::seconds active_duration) {
    return repeat_builder(*this, interval, active_duration);
}

media_description::media_description(const media_type type)
    : m_type(type)
    , m_port()
    , m_number_of_ports()
    , m_protocol()
    , m_formats()
{}

void media_description::set_protocol(const transport_protocol protocol, const uint16_t port, const uint16_t port_count) {
    m_protocol = protocol;
    m_port = port;
    m_number_of_ports = port_count;
}

void media_description::add_format(const uint16_t format) {
    auto it = m_formats.find(format);
    if (it != m_formats.end()) {
        throw std::invalid_argument("format already exists");
    }

    media_description::format fmt_strct{};
    fmt_strct.id = format;
    m_formats.emplace(format, fmt_strct);
}

void media_description::add_rtpmap(const uint16_t format, const std::string_view name, const uint16_t clock_rate, const uint8_t channels) {
    auto it = m_formats.find(format);
    if (it == m_formats.end()) {
        throw std::invalid_argument("no such format");
    }

    it->second.name = name;
    it->second.clock_rate = clock_rate;
    it->second.channels = channels;
}

void media_description::add_fmtp(const uint16_t format, const std::string_view name, const std::string_view value) {
    auto it = m_formats.find(format);
    if (it == m_formats.end()) {
        throw std::invalid_argument("no such format");
    }

    it->second.params.emplace(name, value);
}

session_description::session_description()
    : m_id()
    , m_version()
    , m_name()
    , m_owner_username()
    , m_owner_address()
    , m_uri()
    , m_emails()
    , m_phone_numbers()
    , m_medias()
{}

void session_description::set_id(const std::string_view id) {
    m_id = id;
}

void session_description::set_version(const uint64_t version) {
    m_version = version;
}

void session_description::set_name(const std::string_view name) {
    m_name = name;
}

void session_description::set_owner(const std::string_view username, net_address&& address) {
    m_owner_username = username;
    m_owner_address = std::move(address);
}

void session_description::set_uri(const std::string_view uri) {
    m_uri = uri;
}

void session_description::add_email(const std::string_view email) {
    m_emails.emplace_back(email);
}

void session_description::add_phone_number(const std::string_view phone_number) {
    m_phone_numbers.emplace_back(phone_number);
}

void session_description::add_time(time_description&& time) {
    m_times.emplace_back(std::move(time));
}

void session_description::add_media(media_description&& media) {
    m_medias.emplace_back(std::move(media));
}

description_message session_description::to_message() const {
    description_message msg{};
    msg.version.ver = version::version_0;
    msg.origin.session_id = m_id;
    msg.origin.session_version = m_version;
    msg.origin.username = m_owner_username;
    msg.origin.net_type = m_owner_address.network_type;
    msg.origin.addr_type = m_owner_address.address_type;
    msg.origin.unicast_address = m_owner_address.address_value;
    msg.name.name = m_name;

    if (m_information.has_value()) {
        msg.information = fields::session_information{.information = m_information.value()};
    }
    if (m_uri.has_value()) {
        msg.uri = fields::uri{.uri_str = m_uri.value()};
    }

    for (const auto& email : m_emails) {
        fields::email field{};
        field.email_address = email;
        msg.emails.push_back(std::move(field));
    }

    for (const auto& phone_number : m_phone_numbers) {
        fields::phone_number field{};
        field.number = phone_number;
        msg.phone_numbers.push_back(std::move(field));
    }

    if (!m_connections.empty()) {
        const auto& con = m_connections[0];
        msg.connection_information = connection_to_field(con);
    }

    for (const auto& [type, bandwidth] : m_bandwidths) {
        fields::bandwidth_information field{};
        field.bw_type = type;
        field.bandwidth = bandwidth;
        msg.bandwidths.push_back(std::move(field));
    }

    for (const auto& desc : m_times) {
        media_time_description field{};
        field.times.start_time = desc.m_start_time.count();
        field.times.stop_time = desc.m_stop_time.count();

        for (const auto& rep_desc : desc.m_repeats) {
            time_repeat_description rep_field{};
            rep_field.repeat.repeat_interval = rep_desc.interval.count();
            rep_field.repeat.active_duration = rep_desc.active_duration.count();

            for (const auto& offset : rep_desc.offsets) {
                rep_field.repeat.offsets.push_back(offset.count());
            }

            fields::timezone tz_field{};
            for (const auto& tz_desc : rep_desc.timezones) {
                fields::timezone_adjustment adjust{};
                adjust.adjustment_time = tz_desc.adjustment_time.count();
                adjust.offset = tz_desc.offset.count();
                tz_field.adjustments.emplace_back(std::move(adjust));
            }

            if (!tz_field.adjustments.empty()) {
                rep_field.timezone = tz_field;
            }

            field.repeat.push_back(std::move(rep_field));
        }

        msg.time_descriptions.push_back(std::move(field));
    }

    // todo: attributes

    for (const auto& desc : m_medias) {
        message_media_description field{};

        field.media.media_type = desc.m_type;
        field.media.protocol = desc.m_protocol;
        field.media.port = desc.m_port;
        field.media.number_of_ports = desc.m_number_of_ports;

        for (const auto& [id, format] : desc.m_formats) {
            field.media.formats.push_back(id);

            if (format.name.has_value() && format.clock_rate.has_value()) {
                attributes::rtpmap attr{};
                attr.payload_type = id;
                attr.encoding_name = format.name.value();
                attr.clock_rate = format.clock_rate.value();
                attr.channels = format.channels.value_or(0);

                field.attributes.add(std::move(attr));
            }

            if (!format.params.empty()) {
                attributes::fmtp attr{};
                for (const auto& [name, value] : format.params) {
                    attr.params.emplace(name, value);
                }
                field.attributes.add(std::move(attr));
            }
        }

        if (desc.m_information.has_value()) {
            field.information = fields::session_information{.information = desc.m_information.value()};
        }

        for (const auto& con : desc.m_connections) {
            field.connections.push_back(connection_to_field(con));
        }

        for (const auto& [type, bandwidth] : desc.m_bandwidths) {
            fields::bandwidth_information bw_field{};
            bw_field.bw_type = type;
            bw_field.bandwidth = bandwidth;
            field.bandwidths.push_back(std::move(bw_field));
        }

        // todo: attributes

        msg.media_descriptions.push_back(std::move(field));
    }

    return std::move(msg);
}

void session_description::from_message(const description_message& msg) {
    m_id = msg.origin.session_id;
    m_version = msg.origin.session_version;
    m_owner_username = msg.origin.username;
    m_owner_address.network_type = msg.origin.net_type;
    m_owner_address.address_type = msg.origin.addr_type;
    m_owner_address.address_value = msg.origin.unicast_address;
    m_name = msg.name.name;

    if (msg.information.has_value()) {
        m_information = msg.information.value().information;
    }
    if (msg.uri.has_value()) {
        m_uri = msg.uri.value().uri_str;
    }

    for (const auto& email : msg.emails) {
        add_email(email.email_address);
    }

    for (const auto& phone_number : msg.phone_numbers) {
        add_phone_number(phone_number.number);
    }

    if (msg.connection_information.has_value()) {
        const auto& con_field = msg.connection_information.value();
        add_connection(connection_from_field(con_field));
    }

    for (const auto& bw_field : msg.bandwidths) {
        set_bandwidth(bw_field.bw_type, bw_field.bandwidth);
    }

    for (const auto& time_field : msg.time_descriptions) {
        time_description time_desc(
            std::chrono::seconds(time_field.times.start_time),
            std::chrono::seconds(time_field.times.stop_time));

        for (const auto& rep_field : time_field.repeat) {
            time_description::repeat rep_desc{};
            rep_desc.interval = std::chrono::seconds(rep_field.repeat.repeat_interval);
            rep_desc.active_duration = std::chrono::seconds(rep_field.repeat.active_duration);

            for (const auto& offset : rep_field.repeat.offsets) {
                rep_desc.offsets.emplace_back(offset);
            }

            if (rep_field.timezone.has_value()) {
                for (const auto& tz_field : rep_field.timezone.value().adjustments) {
                    time_description::timezone tz{};
                    tz.adjustment_time = std::chrono::seconds(tz_field.adjustment_time);
                    tz.offset = std::chrono::seconds(tz_field.offset);
                    rep_desc.timezones.emplace_back(std::move(tz));
                }
            }

            time_desc.m_repeats.push_back(std::move(rep_desc));
        }
    }

    // todo: attributes

    for (const auto& media : msg.media_descriptions) {
        media_description desc(media.media.media_type);
        desc.set_protocol(media.media.protocol, media.media.port, media.media.number_of_ports);

        for (const auto& format : media.media.formats) {
            desc.add_format(format);
        }

        if (media.information.has_value()) {
            desc.set_information(media.information.value().information);
        }

        for (const auto& con_field : media.connections) {
            desc.add_connection(connection_from_field(con_field));
        }

        for (const auto& bw_field : media.bandwidths) {
            desc.set_bandwidth(bw_field.bw_type, bw_field.bandwidth);
        }

        for (auto it = media.attributes.begin_attr<attributes::rtpmap>();
            it != media.attributes.end_attr<attributes::rtpmap>(); ++it) {
            desc.add_rtpmap(it->payload_type, it->encoding_name, it->clock_rate, it->channels);
        }
        for (auto it = media.attributes.begin_attr<attributes::fmtp>();
            it != media.attributes.end_attr<attributes::fmtp>(); ++it) {
            for (const auto& [name, value] : it->params) {
                desc.add_fmtp(it->payload_type, name, value);
            }
        }

        add_media(std::move(desc));
    }
}

}
