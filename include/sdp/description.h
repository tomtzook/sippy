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

struct media_information {
    std::optional<std::string> information;
    std::vector<connection_information> connections;
    std::map<std::string, std::string> bandwidths;
    // todo: attributes
};

class session_description;

class time_description {
private:
    struct timezone {
        uint64_t adjustment_time;
        uint64_t offset;
        bool offset_back;
    };

    struct repeat {
        uint64_t interval;
        uint64_t active_duration;
        std::vector<uint64_t> offsets;
        std::vector<timezone> timezones;
    };

public:
    class repeat_builder {
    public:
        explicit repeat_builder(time_description& parent, uint64_t interval, uint64_t active_duration);
        ~repeat_builder();

        repeat_builder& add_offset(uint64_t offset);
        repeat_builder& tz_adjust(uint64_t adjustment_time, uint64_t offset, bool forward = true);

    private:
        time_description& m_parent;
        repeat m_repeat;
    };

    time_description(std::chrono::milliseconds start_time, std::chrono::milliseconds stop_time);

    repeat_builder add_repeat(uint64_t interval, uint64_t active_duration);

private:
    std::chrono::milliseconds m_start_time;
    std::chrono::milliseconds m_stop_time;
    std::vector<repeat> m_repeats;

    friend class session_description;
};

class media_description {
public:
    explicit media_description(sdp::media_type type);

    void set_protocol(transport_protocol protocol, uint16_t port, uint16_t port_count = 1);
    void add_format(std::string_view format);
    void add_format(uint16_t format);
    void set_information(std::string_view info);
    void add_connection(connection_information&& information);
    void add_connection_ipv4(std::string_view base_address, uint16_t count = 1, uint16_t ttl = 0);
    void set_bandwidth(std::string_view type, std::string_view bandwidth);

private:
    sdp::media_type m_type;
    uint16_t m_port;
    uint64_t m_number_of_ports;
    transport_protocol m_protocol;
    std::vector<std::string> m_formats;
    media_information m_media;

    friend class session_description;
};

class session_description {
public:
    session_description();

    void set_id(std::string_view id);
    void set_version(uint64_t version);
    void set_name(std::string_view name);
    void set_owner(std::string_view username, net_address&& address);
    void set_uri(std::string_view uri);
    void add_email(std::string_view email);
    void add_phone_number(std::string_view phone_number);

    void set_information(std::string_view info);
    void set_connection(connection_information&& information);
    void set_connection_ipv4(std::string_view base_address, uint16_t count = 1, uint16_t ttl = 0);
    void set_bandwidth(std::string_view type, std::string_view bandwidth);

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

    media_information m_session_level_media;
    std::vector<time_description> m_times;
    std::vector<media_description> m_medias;
};

}
