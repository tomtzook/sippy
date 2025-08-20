#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <map>
#include <chrono>

#include <sdp/types.h>
#include <sdp/message.h>

namespace sippy::sdp {

struct net_address {
    sdp::network_type network_type;
    sdp::address_type address_type;
    std::string address_value;
};

struct connection_information {
    net_address address;
    uint16_t count;
    uint16_t ttl;
};

class session_description;

class media_information {
public:
    void set_information(std::string_view info);
    void add_connection(connection_information&& information);
    void add_connection_ipv4(std::string_view base_address, uint16_t count = 1, uint16_t ttl = 0);
    void set_bandwidth(std::string_view type, std::string_view bandwidth);

private:
    std::optional<std::string> m_information;
    std::vector<connection_information> m_connections;
    std::map<std::string, std::string> m_bandwidths;
    // todo: attributes

    friend class session_description;
};

class time_description {
private:
    struct timezone {
        std::chrono::seconds adjustment_time;
        std::chrono::seconds offset;
    };

    struct repeat {
        std::chrono::seconds interval;
        std::chrono::seconds active_duration;
        std::vector<std::chrono::seconds> offsets;
        std::vector<timezone> timezones;
    };

public:
    class repeat_builder {
    public:
        explicit repeat_builder(time_description& parent, std::chrono::seconds interval, std::chrono::seconds active_duration);
        ~repeat_builder();

        repeat_builder& add_offset(std::chrono::seconds offset);
        repeat_builder& tz_adjust(std::chrono::seconds adjustment_time, std::chrono::seconds offset);

    private:
        time_description& m_parent;
        repeat m_repeat;
    };

    time_description(std::chrono::seconds start_time, std::chrono::seconds stop_time);

    repeat_builder add_repeat(std::chrono::seconds interval, std::chrono::seconds active_duration);

private:
    std::chrono::seconds m_start_time;
    std::chrono::seconds m_stop_time;
    std::vector<repeat> m_repeats;

    friend class session_description;
};

class media_description final : public media_information {
public:
    explicit media_description(sdp::media_type type);

    void set_protocol(transport_protocol protocol, uint16_t port, uint16_t port_count = 1);
    void add_format(uint16_t format);

    void add_rtpmap(uint16_t format, std::string_view name, uint16_t clock_rate, uint8_t channels = 0);
    void add_fmtp(uint16_t format, std::string_view name, std::string_view value);

private:
    struct format {
        uint16_t id;
        std::optional<std::string> name;
        std::optional<uint16_t> clock_rate;
        std::optional<uint8_t> channels;
        std::map<std::string, std::string, std::less<>> params;
    };

    sdp::media_type m_type;
    uint16_t m_port;
    uint64_t m_number_of_ports;
    transport_protocol m_protocol;
    std::map<uint16_t, format> m_formats;

    friend class session_description;
};

class session_description final : public media_information {
public:
    session_description();

    void set_id(std::string_view id);
    void set_version(uint64_t version);
    void set_name(std::string_view name);
    void set_owner(std::string_view username, net_address&& address);
    void set_uri(std::string_view uri);
    void add_email(std::string_view email);
    void add_phone_number(std::string_view phone_number);

    void add_time(time_description&& time);
    void add_media(media_description&& media);

    [[nodiscard]] description_message to_message() const;
    void from_message(const description_message& msg);

private:
    std::string m_id;
    uint64_t m_version;
    std::string m_name;
    std::string m_owner_username;
    net_address m_owner_address;

    std::optional<std::string> m_uri;
    std::vector<std::string> m_emails;
    std::vector<std::string> m_phone_numbers;

    std::vector<time_description> m_times;
    std::vector<media_description> m_medias;
};

}
