#include <sip/headers.h>

#include "serialization/matchers.h"
#include "serialization/reader.h"


using namespace sippy;

static constexpr bool is_end_of_uri(const char c) {
    return c == '>';
}

static constexpr bool is_data_val(const char c) {
    return serialization::is_alphanumeric(c) || c == '/';
}

template<typename T>
static inline void read_from_to(std::istream& is, T& h) {
    serialization::reader reader(is);

    if (!reader.peek('<')) {
        h.display_name = reader.read_until(serialization::is_whitespace);
    }

    reader.eat_while(serialization::is_whitespace);
    reader.eat('<');
    h.uri = reader.read_until(is_end_of_uri);
    reader.eat('>');
    reader.eat_while(serialization::is_whitespace);

    if (reader.peek(';')) {
        reader.eat(';');
        reader.eat_while(serialization::is_whitespace);
        reader.eat("tag=");
        h.tag = reader.read_while(serialization::is_alphanumeric);
    }
}

template<typename T>
static inline void write_from_to(std::ostream& os, const T& h) {
    if (h.display_name) {
        os << h.display_name.value();
        os << ' ';
    }

    os << '<' << h.uri << '>';

    if (h.tag) {
        os << ";tag=" << h.tag.value();
    }
}

DEFINE_SIP_HEADER_READ(from) {
    read_from_to(is, h);
}

DEFINE_SIP_HEADER_WRITE(from) {
    write_from_to(os, h);
}

DEFINE_SIP_HEADER_READ(to) {
    read_from_to(is, h);
}

DEFINE_SIP_HEADER_WRITE(to) {
    write_from_to(os, h);
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
    h.value = reader.read_while(serialization::is_textual_sequence);
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

DEFINE_SIP_HEADER_READ(content_length) {
    is >> h.length;
}

DEFINE_SIP_HEADER_WRITE(content_length) {
    os << h.length;
}

DEFINE_SIP_HEADER_READ(content_type) {
    serialization::reader reader(is);
    h.type = reader.read_while(is_data_val);
}

DEFINE_SIP_HEADER_WRITE(content_type) {
    os << h.type;
}
