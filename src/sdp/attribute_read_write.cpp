
#include <sdp/attributes.h>

#include "serialization/reader.h"


DEFINE_SDP_ATTRIBUTE_READ(tool) {
    serialization::reader reader(is);
    a.name = reader.read_until(serialization::is_whitespace);
    reader.eat(' ');
    a.version = reader.read_until(serialization::is_whitespace);
}

DEFINE_SDP_ATTRIBUTE_WRITE(tool) {
    os << a.name;
    os << ' ';
    os << a.version;
}

DEFINE_SDP_ATTRIBUTE_READ(ptime) {
    is >> a.time;
}

DEFINE_SDP_ATTRIBUTE_WRITE(ptime) {
    os << a.time;
}

DEFINE_SDP_ATTRIBUTE_READ(maxptime) {
    is >> a.time;
}

DEFINE_SDP_ATTRIBUTE_WRITE(maxptime) {
    os << a.time;
}

DEFINE_SDP_ATTRIBUTE_READ(rtpmap) {
    serialization::reader reader(is);
    is >> a.payload_type;
    reader.eat(' ');
    a.encoding_name = reader.read_until(serialization::is_slash);
    reader.eat('/');
    is >> a.clock_rate;

    if (reader.eat_one_if(serialization::is_slash)) {
        is >> a.channels;
    } else {
        a.channels = 0;
    }
}

DEFINE_SDP_ATTRIBUTE_WRITE(rtpmap) {
    os << a.payload_type;
    os << ' ';
    os << a.encoding_name;
    os << '/';
    os << a.clock_rate;

    if (a.channels > 1) {
        os << '/';
        os << a.channels;
    }
}

DEFINE_SDP_ATTRIBUTE_READ(fmtp) {
    serialization::reader reader(is);
    is >> a.payload_type;
    reader.eat(' ');

    while (true) {
        auto name = reader.read_until(serialization::is_equal);
        if (!reader.eat_one_if(serialization::is_equal)) {
            break;
        }

        auto value = reader.read_until(serialization::is_colon_or_newline);
        a.params.emplace(std::move(name), std::move(value));
    }
}

DEFINE_SDP_ATTRIBUTE_WRITE(fmtp) {
    os << a.payload_type;
    os << ' ';

    bool first = true;
    for (const auto& [name, value] : a.params) {
        if (first) {
            first = false;
        } else {
            os << ';';
        }

        os << name << '=' << value;
    }
}
