
#include "reader.h"

namespace sippy::serialization {

reader::reader(std::istream &is)
    : m_is(is)
{}

bool reader::peek(const char ch) {
    const auto peeked = m_is.peek();
    return peeked == ch;
}

void reader::eat(const char ch) {
    char cch;
    m_is.get(cch);

    if (cch != ch) {
        throw unexpected_character();
    }
}

void reader::eat(const std::string_view str) {
    for (const char i : str) {
        eat(i);
    }
}

bool reader::eat_one_if(const matcher matcher) {
    const auto peeked = m_is.peek();
    if (matcher(static_cast<char>(peeked))) {
        char cch;
        m_is.get(cch);
        return true;
    } else {
        return false;
    }
}

void reader::eat_while(const matcher matcher) {
    while (eat_one_if(matcher));
}

std::string reader::read(const size_t length) {
    std::string str;
    str.reserve(length);

    for (int i = 0; i < length; ++i) {
        char ch;
        if (!m_is.get(ch)) {
            throw not_enough_characters();
        }

        str.push_back(ch);
    }

    return str;
}

character reader::read_one_if(const matcher matcher) {
    const auto peeked = m_is.peek();
    if (peeked == EOF) {
        return eof;
    } else if (matcher(static_cast<char>(peeked))) {
        char ch;
        m_is.get(ch);
        return ch;
    } else {
        return uneaten;
    }
}

character reader::read_one_if_not(const matcher matcher) {
    const auto peeked = m_is.peek();
    if (peeked == EOF) {
        return eof;
    } else if (!matcher(static_cast<char>(peeked))) {
        char ch;
        m_is.get(ch);
        return ch;
    } else {
        return uneaten;
    }
}

std::string reader::read_while(const matcher matcher) {
    std::string result;
    result.reserve(16);

    character ch;
    while (((ch = read_one_if(matcher))) && is_char(ch)) {
        result.push_back(to_char(ch));
    }

    return std::move(result);
}

std::string reader::read_until(const matcher matcher) {
    std::string result;
    result.reserve(16);

    character ch;
    while (((ch = read_one_if_not(matcher))) && is_char(ch)) {
        result.push_back(to_char(ch));
    }

    return std::move(result);
}


}
