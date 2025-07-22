#include <regex>
#include <sip/headers.h>

#include "serialization/reader.h"


using namespace sippy::sip;

static std::smatch parse(const std::string& data, const std::string_view pattern) {
    std::regex regex(pattern.data());
    std::smatch matches;
    if (std::regex_match(data, matches, regex)) {
        return matches;
    } else {
        throw std::invalid_argument("header data not matching pattern");
    }
}

DEFINE_SIP_HEADER_READ(from) {
    serialization::reader reader(is);
    auto str = reader.read_until(serialization::is_new_line);

    const auto match = parse(str, R"(^(?:(\"?.+\"?)\s+)?<?(\w+:[\w@\-\.\+]+)>?\s*(?:;\s*tag=(.+))?$)");
    if (match[1].matched) {
        h.display_name = match[1].str();
    } else {
        h.display_name = std::nullopt;
    }
    h.uri = match[2].str();
    if (match[3].matched) {
        h.tag = match[3].str();
    } else {
        h.tag = std::nullopt;
    }
}

DEFINE_SIP_HEADER_WRITE(from) {
    if (h.display_name) {
        os << h.display_name.value();
        os << ' ';
    }

    os << '<' << h.uri << '>';

    if (h.tag) {
        os << ";tag=" << h.tag.value();
    }
}

DEFINE_SIP_HEADER_READ(to) {
    serialization::reader reader(is);
    auto str = reader.read_until(serialization::is_new_line);

    const auto match = parse(str, R"(^(?:(\"?.+\"?)\s+)?<?(\w+:[\w@\-\.\+]+)>?\s*(?:;\s*tag=(.+))?$)");
    if (match[1].matched) {
        h.display_name = match[1].str();
    } else {
        h.display_name = std::nullopt;
    }
    h.uri = match[2].str();
    if (match[3].matched) {
        h.tag = match[3].str();
    } else {
        h.tag = std::nullopt;
    }
}

DEFINE_SIP_HEADER_WRITE(to) {
    if (h.display_name) {
        os << h.display_name.value();
        os << ' ';
    }

    os << '<' << h.uri << '>';

    if (h.tag) {
        os << ";tag=" << h.tag.value();
    }
}

DEFINE_SIP_HEADER_READ(content_length) {
    is >> h.length;
}

DEFINE_SIP_HEADER_WRITE(content_length) {
    os << h.length;
}

DEFINE_SIP_HEADER_READ(content_type) {
    serialization::reader reader(is);
    auto str = reader.read_until(serialization::is_new_line);

    h.type = std::move(str);
}

DEFINE_SIP_HEADER_WRITE(content_type) {
    os << h.type;
}

DEFINE_SIP_HEADER_READ(cseq) {
    serialization::reader reader(is);

    is >> h.seq_num;
    reader.eat_while(serialization::is_whitespace);
    is >> h.method;
}

DEFINE_SIP_HEADER_WRITE(cseq) {
    os << h.seq_num;
    os << ' ';
    os << h.method;
}


DEFINE_SIP_HEADER_READ(call_id) {
    serialization::reader reader(is);
    auto str = reader.read_until(serialization::is_new_line);

    h.value = std::move(str);
}

DEFINE_SIP_HEADER_WRITE(call_id) {
    os << h.value;
}

DEFINE_SIP_HEADER_READ(max_forwards) {
    is >> h.value;
}

DEFINE_SIP_HEADER_WRITE(max_forwards) {
    os << h.value;
}

DEFINE_SIP_HEADER_READ(min_expires) {
    is >> h.value;
}

DEFINE_SIP_HEADER_WRITE(min_expires) {
    os << h.value;
}

DEFINE_SIP_HEADER_READ(expires) {
    is >> h.value;
}

DEFINE_SIP_HEADER_WRITE(expires) {
    os << h.value;
}
