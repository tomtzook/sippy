#pragma once

#include <iostream>

namespace sippy::sdp {

enum class version {
    version_0
};

enum class media_type {
    audio,
    video,
    text,
    application,
    message
};

enum class transport_protocol {
    udp,
    rtp_avp,
    rtp_savp,
    rtp_savpf
};

enum class network_type {
    in
};

enum class address_type {
    ipv4,
    ipv6
};

std::istream& operator>>(std::istream& is, version& version);
std::ostream& operator<<(std::ostream& os, version version);
std::istream& operator>>(std::istream& is, media_type& media_type);
std::ostream& operator<<(std::ostream& os, media_type media_type);
std::istream& operator>>(std::istream& is, transport_protocol& transport_protocol);
std::ostream& operator<<(std::ostream& os, transport_protocol transport_protocol);
std::istream& operator>>(std::istream& is, network_type& network_type);
std::ostream& operator<<(std::ostream& os, network_type network_type);
std::istream& operator>>(std::istream& is, address_type& address_type);
std::ostream& operator<<(std::ostream& os, address_type address_type);

}
