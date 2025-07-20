
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

static constexpr bool is_text(const char c) {
    return is_letter(c) || is_number(c) || c == '.' || c == '-' || c == '_';
}

tokenizer::tokenizer(std::istream& is)
    : m_is(is)
    , m_done(false)
{}

tokenizer::token tokenizer::next() {
    token token{};

    if (m_done) {
        token.type = token_type::eof;
        return token;
    }

    const auto ch_int = next_char();
    if (ch_int < 0) {
        // eof
        m_done = true;
        token.type = token_type::eof;
        return token;
    }

    const auto ch = static_cast<char>(ch_int);
    if (is_whitespace(ch)) {
        eat_whitespaces();
        token.type = token_type::whitespace;
    } else if (is_tab(ch)) {
        token.type = token_type::tab;
    } else if (ch == '\r') {
        if (eat('\n')) {
            token.type = token_type::crlf;
        } else {
            token.type = token_type::cr;
        }
    } else if (ch == '\n') {
        token.type = token_type::lf;
    } else if (ch == ':') {
        token.type = token_type::colon;
    } else if (ch == ',') {
        token.type = token_type::coma;
    } else if (ch == '/') {
        token.type = token_type::forward_slash;
    } else if (ch == '\\') {
        token.type = token_type::backward_slash;
    } else if (ch == '<') {
        token.type = token_type::less_than;
    } else if (ch == '>') {
        token.type = token_type::greater_than;
    } else if (is_text(ch)) {
        token.type = token_type::string;
        token.str = read_string(ch);
    } else if (ch == '"') {
        token.type = token_type::open_string;
        token.str = read_open_string();
        if (!eat('"')) {
            throw unexpected_character();
        }
    } else {
        throw unexpected_character();
    }

    return token;
}

std::string tokenizer::read_string(const char initial_ch) {
    std::string result;
    result.reserve(16);
    result.push_back(initial_ch);

    auto peeked = m_is.peek();
    while (peeked != EOF && is_text(static_cast<char>(peeked))) {
        result.push_back(static_cast<char>(next_char()));
        peeked = m_is.peek();
    }

    return std::move(result);
}

std::string tokenizer::read_open_string() {
    std::string result;
    result.reserve(16);

    bool is_escaped = false;
    auto peeked = m_is.peek();
    while (peeked != EOF && (!is_escaped || peeked != '\"')) {
        if (!is_escaped && peeked == '\\') {
            is_escaped = true;
        } else if (is_escaped) {
            is_escaped = false;
        }

        result.push_back(static_cast<char>(next_char()));
        peeked = m_is.peek();
    }

    return std::move(result);
}

bool tokenizer::eat(const char ch) {
    const auto peeked = m_is.peek();
    if (peeked == ch) {
        next_char();
        return true;
    } else {
        return false;
    }
}

void tokenizer::eat_whitespaces() {
    while (eat(' '));
}

int tokenizer::next_char() {
    char ch;
    if (m_is.get(ch)) {
        return ch;
    } else {
        return EOF;
    }
}

}
