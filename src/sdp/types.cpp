
#include <map>
#include <cstdint>

#include <sdp/types.h>

#include "serialization/reader.h"

namespace sippy::sdp {

class unknown_version final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "Unknown version";
    }
};

class unknown_media_type final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "Unknown media type";
    }
};

class unknown_transport_protocol final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "Unknown transport protocol";
    }
};

class unknown_network_type final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "Unknown network type";
    }
};

class unknown_address_type final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "Unknown address type";
    }
};

class unknown_media_direction final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "Unknown media direction";
    }
};

std::map<std::string, media_type, std::less<>> m_str_to_media_type = {
    {"audio", media_type::audio},
    {"video", media_type::video},
    {"application", media_type::application},
    {"text", media_type::text},
    {"message", media_type::message},
};

std::map<std::string, transport_protocol, std::less<>> m_str_to_transport_proto = {
    {"UDP", transport_protocol::udp},
    {"RTP/AVP", transport_protocol::rtp_avp},
    {"RTP/SAVP", transport_protocol::rtp_savp},
    {"RTP/SAVPF", transport_protocol::rtp_savpf}
};

std::map<std::string, network_type, std::less<>> m_str_to_net_type = {
    {"IN", network_type::in},
};

std::map<std::string, address_type, std::less<>> m_str_to_addr_type = {
    {"IP4", address_type::ipv4},
    {"IP6", address_type::ipv6},
};

std::map<std::string, media_direction, std::less<>> m_str_to_media_direction = {
    {"recvonly", media_direction::recvonly},
    {"sendrecv", media_direction::sendrecv},
    {"sendonly", media_direction::sendonly},
    {"inactive", media_direction::inactive},
};

std::istream& operator>>(std::istream& is, version& version) {
    uint16_t i;
    is >> i;

    switch (i) {
        case 0:
            version = version::version_0;
            break;
        default:
            throw unknown_version();
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const version version) {
    switch (version) {
        case version::version_0:
            os << '0';
            break;
    }

    return os;
}

std::istream& operator>>(std::istream& is, media_type& media_type) {
    serialization::reader reader(is);
    const auto line = reader.read_while(serialization::is_letter);

    const auto it = m_str_to_media_type.find(line);
    if (it != m_str_to_media_type.end()) {
        media_type = it->second;
    } else {
        throw unknown_media_type();
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const media_type media_type) {
    switch (media_type) {
        case media_type::audio:
            os << "audio";
            break;
        case media_type::video:
            os << "video";
            break;
        case media_type::text:
            os << "text";
            break;
        case media_type::application:
            os << "application";
            break;
        case media_type::message:
            os << "message";
            break;
        default:
            throw unknown_media_type();
    }

    return os;
}

std::istream& operator>>(std::istream& is, transport_protocol& transport_protocol) {
    serialization::reader reader(is);
    const auto line = reader.read_while(serialization::is_letter_or_slash);

    const auto it = m_str_to_transport_proto.find(line);
    if (it != m_str_to_transport_proto.end()) {
        transport_protocol = it->second;
    } else {
        throw unknown_transport_protocol();
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const transport_protocol transport_protocol) {
    switch (transport_protocol) {
        case transport_protocol::udp:
            os << "UDP";
            break;
        case transport_protocol::rtp_avp:
            os << "RTP/AVP";
            break;
        case transport_protocol::rtp_savp:
            os << "RTP/SAVP";
            break;
        case transport_protocol::rtp_savpf:
            os << "RTP/SAVPF";
            break;
        default:
            throw unknown_transport_protocol();
    }

    return os;
}

std::istream& operator>>(std::istream& is, network_type& network_type) {
    serialization::reader reader(is);
    const auto line = reader.read_while(serialization::is_letter_or_slash);

    const auto it = m_str_to_net_type.find(line);
    if (it != m_str_to_net_type.end()) {
        network_type = it->second;
    } else {
        throw unknown_network_type();
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const network_type network_type) {
    switch (network_type) {
        case network_type::in:
            os << "IN";
            break;
        default:
            throw unknown_network_type();
    }

    return os;
}

std::istream& operator>>(std::istream& is, address_type& address_type) {
    serialization::reader reader(is);
    const auto line = reader.read_while(serialization::is_letter_or_slash);

    const auto it = m_str_to_addr_type.find(line);
    if (it != m_str_to_addr_type.end()) {
        address_type = it->second;
    } else {
        throw unknown_address_type();
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const address_type address_type) {
    switch (address_type) {
        case address_type::ipv4:
            os << "IP4";
            break;
        case address_type::ipv6:
            os << "IP6";
            break;
        default:
            throw unknown_address_type();
    }

    return os;
}

std::istream& operator>>(std::istream& is, media_direction& media_direction) {
    serialization::reader reader(is);
    const auto line = reader.read_while(serialization::is_letter);

    const auto it = m_str_to_media_direction.find(line);
    if (it != m_str_to_media_direction.end()) {
        media_direction = it->second;
    } else {
        throw unknown_media_direction();
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const media_direction media_direction) {
    switch (media_direction) {
        case media_direction::recvonly:
            os << "recvonly";
            break;
        case media_direction::sendrecv:
            os << "sendrecv";
            break;
        case media_direction::sendonly:
            os << "sendonly";
            break;
        case media_direction::inactive:
            os << "inactive";
            break;
        default:
            throw unknown_media_direction();
    }

    return os;
}

}
