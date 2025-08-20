#pragma once

#include <type_traits>
#include <optional>
#include <vector>
#include <cstdint>

#include <sdp/types.h>

namespace sippy::sdp::fields {

struct _field {};

static constexpr char field_name_attribute = 'a';

enum flags : uint32_t {
    flag_none = 0,
    flag_optional = 1,
    flag_allow_multiple = 4
};

struct timezone_adjustment {
    uint64_t adjustment_time; // seconds
    int64_t offset; // seconds
};

namespace meta {

template<typename T>
concept _field_type = std::is_base_of_v<_field, T>;

template<typename T>
struct _field_detail {};
template<typename T>
struct _field_validator {};
template<typename T>
struct _field_reader {};
template<typename T>
struct _field_writer {};

}

}

#define DECLARE_SDP_FIELD(f_name, ch_name) \
    namespace sippy::sdp::fields { \
        struct f_name; \
        bool _validate_field_ ##f_name(const f_name & f);\
        std::istream& operator>>(std::istream& is, f_name & f); \
        std::ostream& operator<<(std::ostream& os, const f_name & f); \
        namespace meta { \
            template<> struct _field_detail<sippy::sdp::fields::f_name> { \
                static constexpr const char name() { return (ch_name) ; } \
            }; \
            template<> struct _field_validator<sippy::sdp::fields::f_name> { \
                static bool validate(const sippy::sdp::fields::f_name & f) { return _validate_field_ ##f_name(f); } \
            }; \
            template<> struct _field_reader<sippy::sdp::fields::f_name> { \
                static void read(std::istream& is, sippy::sdp::fields::f_name & f) { is >> f; } \
            }; \
            template<> struct _field_writer<sippy::sdp::fields::f_name> { \
                static void write(std::ostream& os, sippy::sdp::fields::f_name & f) { os << f; } \
            }; \
        } \
    } \
    struct sippy::sdp::fields::f_name : sippy::sdp::fields::_field


#define DEFINE_SDP_FIELD_VALIDATOR(f_name) \
    bool sippy::sdp::fields::_validate_field_ ##f_name(const f_name & f)


#define DEFINE_SDP_FIELD_READ(f_name) \
    namespace sippy::sdp::fields { \
        static void read_field_ ##f_name(std::istream& is, f_name & f); \
        std::istream& operator>>(std::istream& is, f_name & f) { \
            read_field_ ##f_name(is, f); \
            return is; \
        } \
    } \
    static void sippy::sdp::fields::read_field_ ##f_name(std::istream& is, f_name & f)


#define DEFINE_SDP_FIELD_WRITE(f_name) \
    namespace sippy::sdp::fields { \
        static void write_field_ ##f_name(std::ostream& os, const f_name & f); \
        std::ostream& operator<<(std::ostream& os, const f_name & f) { \
            write_field_ ##f_name(os, f); \
            return os; \
        } \
    } \
    static void sippy::sdp::fields::write_field_ ##f_name(std::ostream& os, const f_name & f)



// TODO: SESSION LEVEL AND MEDIA LEVEL FIELDS
// TODO: ATTRIBUTES
// TODO: STRING ENCODING AND CHARSETS

DECLARE_SDP_FIELD(version, 'v') {
    sdp::version ver;
};

DECLARE_SDP_FIELD(origin, 'o') {
    std::string username;
    std::string session_id;
    uint64_t session_version;
    network_type net_type;
    address_type addr_type;
    std::string unicast_address;
};

DECLARE_SDP_FIELD(session_name, 's') {
    std::string name;
};

DECLARE_SDP_FIELD(session_information, 'i') {
    std::string information;
};

DECLARE_SDP_FIELD(uri, 'u') {
    std::string uri_str;
};

DECLARE_SDP_FIELD(email, 'e') {
    std::optional<std::string> display_name;
    std::string email_address;
};

DECLARE_SDP_FIELD(phone_number, 'p') {
    std::optional<std::string> display_name;
    std::string number;
};

DECLARE_SDP_FIELD(connection_information, 'c') {
    network_type net_type;
    address_type addr_type;
    std::string base_address;
    std::optional<uint16_t> ttl;
    std::optional<uint16_t> num_of_addresses;
};

DECLARE_SDP_FIELD(bandwidth_information, 'b') {
    std::string bw_type;
    std::string bandwidth;
};

DECLARE_SDP_FIELD(time_active, 't') {
    uint64_t start_time; // seconds
    uint64_t stop_time; // seconds
};

DECLARE_SDP_FIELD(repeat_times, 'r') {
    uint64_t repeat_interval; // seconds
    uint64_t active_duration; // seconds
    std::vector<uint64_t> offsets; // seconds
};

DECLARE_SDP_FIELD(timezone, 'z') {
    std::vector<timezone_adjustment> adjustments; // at least one required
};

DECLARE_SDP_FIELD(media_description, 'm') {
    sdp::media_type media_type;
    uint16_t port;
    uint64_t number_of_ports;
    transport_protocol protocol;
    std::vector<uint16_t> formats;
};
