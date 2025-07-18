#pragma once

#include <sip/headers.h>

#include "serialization/matchers.h"
#include "serialization/reader.h"


static constexpr bool is_end_of_uri(const char c) {
    return c == '>';
}

DEFINE_SIP_HEADER_READ(from) {
    serialization::reader reader(is);

    if (!reader.peek('<')) {
        h.display_name = reader.read_until(serialization::is_whitespace);
    }

    reader.eat_while(serialization::is_whitespace);
    reader.eat('<');
    h.uri = reader.read_until(is_end_of_uri);
    reader.eat('>');

    if (reader.peek(';')) {
        reader.eat(';');
        h.tag = reader.read_while(serialization::is_alphanumeric);
    }
}

DEFINE_SIP_HEADER_WRITE(from) {
    if (h.display_name) {
        os << h.display_name.value();
        os << ' ';
    }

    os << '<' << h.uri << '>';

    if (h.tag) {
        os << ';' << h.tag.value();
    }
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
