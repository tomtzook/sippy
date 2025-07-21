
#include "lexer.h"

namespace sippy::lang {

class unexpected_character final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "unexpected character";
    }
};

static constexpr bool is_letter(const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static constexpr bool is_whitespace(const char c) {
    return c == ' ';
}

static constexpr bool is_tab(const char c) {
    return c == '\t';
}

static constexpr bool is_number(const char c) {
    return c >= '0' && c <= '9';
}

static constexpr bool is_text(const char c) {
    return is_letter(c) || is_number(c) || c == '.' || c == '-' || c == '_';
}

lexer::lexer(std::istream& is)
    : m_is(&is)
    , m_done(false)
    , m_read_tokens()
    , m_token_index(0)
{}

lexer::lexer(const std::vector<token>& tokens)
    : m_is(nullptr)
    , m_done(true)
    , m_read_tokens()
    , m_token_index(0) {
    // copy all the given tokens
    m_read_tokens.reserve(tokens.size());
    for (const auto& token : tokens) {
        token_storage storage{};
        storage.type = token.type;
        storage.str = token.str;
        m_read_tokens.push_back(std::move(storage));
    }
}

token_type lexer::peek() {
    const auto type = next().type;
    go_back(1);

    return type;
}

token lexer::next() {
    m_token_index++;
    if (m_token_index >= m_read_tokens.size()) {
        if (m_done) {
            m_token_index = m_read_tokens.size();
            throw std::runtime_error("nothing more to read");
        }

        auto token = new_raw_token();
        m_read_tokens.push_back(std::move(token));
    }

    const auto& storage = m_read_tokens[m_token_index-1];

    token token{};
    token.type = storage.type;
    token.str = storage.str;
    return token;
}

void lexer::go_back(const size_t count) {
    if (count > m_token_index) {
        throw std::runtime_error("not enough history to go back requested amount");
    }

    m_token_index -= count;
}

lexer::token_storage lexer::new_raw_token() {
    token_storage token{};

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

    return std::move(token);
}

std::string lexer::read_string(const char initial_ch) {
    std::string result;
    result.reserve(16);
    result.push_back(initial_ch);

    auto peeked = m_is->peek();
    while (peeked != EOF && is_text(static_cast<char>(peeked))) {
        result.push_back(static_cast<char>(next_char()));
        peeked = m_is->peek();
    }

    return std::move(result);
}

std::string lexer::read_open_string() {
    std::string result;
    result.reserve(16);

    bool is_escaped = false;
    auto peeked = m_is->peek();
    while (peeked != EOF && (!is_escaped || peeked != '\"')) {
        if (!is_escaped && peeked == '\\') {
            is_escaped = true;
        } else if (is_escaped) {
            is_escaped = false;
        }

        result.push_back(static_cast<char>(next_char()));
        peeked = m_is->peek();
    }

    return std::move(result);
}

bool lexer::eat(const char ch) {
    const auto peeked = m_is->peek();
    if (peeked == ch) {
        next_char();
        return true;
    } else {
        return false;
    }
}

void lexer::eat_whitespaces() {
    while (eat(' '));
}

int lexer::next_char() {
    if (m_is == nullptr) {
        throw std::runtime_error("no stream to read from");
    }

    char ch;
    if (m_is->get(ch)) {
        return ch;
    } else {
        return EOF;
    }
}

}
