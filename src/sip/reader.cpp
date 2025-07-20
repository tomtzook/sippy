
#include "reader.h"

#include <regex>
#include <string.h>

#include <sip/message.h>

#include "util/meta.h"
#include "serialization/matchers.h"
#include "types_storage.h"
#include "reader.h"

namespace sippy::sip {

class bad_start_line final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "bad start line";
    }
};

class missing_header_value final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "missing header value";
    }
};

class header_value_trailing_data final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "trailing data in header";
    }
};

class body_trailing_data final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "trailing data in body";
    }
};

class missing_content_type final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "missing content type";
    }
};

class unknown_body final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "unknown body";
    }
};

class unexpected_token final : public std::exception {
public:
    [[nodiscard]] const char* what() const noexcept override {
        return "unexpected token";
    }
};

static void fixup_header_name(std::string& name) {
    bool word_start = true;
    for (char& ch : name) {
        if (word_start) {
            ch = static_cast<char>(std::toupper(ch));
            word_start = false;
        } else if (serialization::is_letter(ch)) {
            ch = static_cast<char>(std::tolower(ch));
        } else if (serialization::is_dash(ch)) {
            word_start = true;
        }
    }
}

header_reader::header_reader(std::istream& is)
    : m_reader(is)
{}

std::optional<std::string> header_reader::read_start_line() {
    auto data = m_reader.read_until(serialization::is_new_line);
    eat_new_line();

    return {std::move(data)};
}

std::optional<std::string> header_reader::read_header_name() {
    m_reader.eat_while(serialization::is_whitespace);

    auto data = m_reader.read_while(serialization::is_letter_or_dash);
    if (data.empty()) {
        // empty line, so next is the body
        return std::nullopt;
    }

    m_reader.eat_while(serialization::is_whitespace);
    m_reader.eat(':');

    return {std::move(data)};
}

std::optional<std::string> header_reader::read_header_value() {
    m_reader.eat_while(serialization::is_whitespace_or_tab);
    auto data = m_reader.read_until(serialization::is_new_line);
    eat_new_line();

    // it is possible that next line is with more value
    while (m_reader.peek(' ') || m_reader.peek('\t')) {
        // we have more data in the next line
        m_reader.eat_while(serialization::is_whitespace_or_tab);
        const auto more_data = m_reader.read_until(serialization::is_new_line);
        eat_new_line();

        data += more_data;
    }

    return {std::move(data)};
}

void header_reader::eat_new_line() {
    m_reader.eat('\r');
    m_reader.eat('\n');
}

static void verify_token_type(const lexer::token& token, const lexer::token_type type) {
    if (token.type != type) {
        throw unexpected_token();
    }
}

lexer::lexer(std::istream& is)
    : m_tokenizer(is)
    , m_read_tokens()
    , m_token_index(0)
{}

template<>
version lexer::next<version>() {
    return next(token_type::version).v.version;
}

template<>
method lexer::next<method>() {
    return next(token_type::method).v.method;
}

lexer::token lexer::next() {
    const auto& c_token = next_token();

    token token{};
    switch (c_token.type) {
        case serialization::tokenizer::token_type::eof:
            token.type = token_type::eof;
            break;
        case serialization::tokenizer::token_type::whitespace:
            token.type = token_type::whitespace;
            break;
        case serialization::tokenizer::token_type::tab:
            token.type = token_type::tab;
            break;
        case serialization::tokenizer::token_type::crlf:
            token.type = token_type::new_line;
            break;
        case serialization::tokenizer::token_type::colon:
            token.type = token_type::colon;
            break;
        case serialization::tokenizer::token_type::coma:
            token.type = token_type::coma;
            break;
        case serialization::tokenizer::token_type::string: {
            const auto ver_opt = try_read_version(c_token);
            if (ver_opt) {
                token.type = token_type::version;
                token.v.version = ver_opt.value();
                break;
            }

            const auto method_opt = try_get_method(c_token.str);
            if (method_opt) {
                token.type = token_type::method;
                token.v.method = method_opt.value();
                break;
            }

            token.type = token_type::string;
            token.str = c_token.str;
            break;
        }
        default:
            throw unexpected_token();
    }

    return token;
}

lexer::token lexer::next(const token_type expected_type, const bool eat_whitespaces) {
    token token;
    do {
        token = next();
    } while (eat_whitespaces && token.type == token_type::whitespace);

    verify_token_type(token, expected_type);
    return std::move(token);
}

std::optional<version> lexer::try_read_version(const serialization::tokenizer::token& c_token) {
    if (c_token.type != serialization::tokenizer::token_type::string || strcasecmp(c_token.str.c_str(), "SIP") != 0) {
        return std::nullopt;
    }

    {
        const auto& token = next_token();
        if (token.type != serialization::tokenizer::token_type::forward_slash) {
            go_back(1);
            return std::nullopt;
        }
    }

    {
        const auto& token = next_token();
        if (token.type != serialization::tokenizer::token_type::string) {
            go_back(2);
            return std::nullopt;
        }

        const auto version_opt = try_get_version(token.str);
        if (version_opt) {
            return version_opt.value();
        } else {
            go_back(2);
            return std::nullopt;
        }
    }
}

const serialization::tokenizer::token& lexer::next_token() {
    m_token_index++;
    if (m_token_index >= m_read_tokens.size()) {
        auto token = m_tokenizer.next();
        m_read_tokens.push_back(std::move(token));
    }

    return m_read_tokens[m_token_index-1];
}

void lexer::go_back(const size_t count) {
    if (count > m_token_index) {
        throw std::runtime_error("not enough history to go back requested amount");
    }

    m_token_index -= count;
}

void header_line_parser::parse() {
    switch (m_state) {
        case state::start:
            m_state = state::name_value;
            break;
        case state::name_value: {
            const auto token = m_lexer.next();
            if (token.type != lexer::token_type::whitespace) {
                verify_token_type(token, lexer::token_type::string);
                m_state = state::name_end;
            }
            break;
        }
    }
}

parser::parser(std::istream& is)
    : m_lexer(is)
    , m_state(state::start_line_start)
{}

void parser::parse_next() {
    const auto token = m_lexer.next();

    switch (m_state) {
        case state::start_line_start:
            if (token.type == lexer::token_type::method) {
                m_state_data.request_line.method = token.v.method;
                m_state = state::request_line_uri;
            } else if (token.type == lexer::token_type::version) {
                m_state_data.response_line.version = token.v.version;
                m_state = state::response_line_code;
            } else {
                throw unexpected_token();
            }
            break;
        case state::request_line_uri:
            verify_token_type(token, lexer::token_type::uri);
            break;
        case state::request_line_version:
            verify_token_type(token, lexer::token_type::version);
            m_state_data.response_line.version = token.v.version;

            break;
        case state::header_name_start:
            verify_token_type(token, lexer::token_type::string);
            m_state_data.header_name = token.str;
            m_state = state::header_name_end;
            break;
        case state::header_name_end:
            verify_token_type(token, lexer::token_type::colon);
            m_state = state::header_value;
            break;
    }

}

void parser::parse_header_line() {
    // name reading
    const auto name = m_lexer.next(lexer::token_type::string).str;
    m_lexer.next(lexer::token_type::colon, true);

    // value loading
    bool first = true;
    std::vector<lexer::token> value_tokens;
    do {
        value_tokens = next_header_value_tokens();

        const auto defOpt = headers::storage::get_header(name);
        if (!defOpt.has_value()) {
            // unknown header, ignore it
            return;
        }

        const auto& def = defOpt.value();
        const auto can_multiple = (def->flags() & headers::flag_allow_multiple) != 0;
        if (!first && !can_multiple) {
            throw header_value_trailing_data();
        }

        if (first) {
            first = false;
        }
    } while (!value_tokens.empty());
}

std::vector<lexer::token> parser::next_header_value_tokens() {
    std::vector<lexer::token> tokens;
    bool first = true;
    bool done = false;
    do {
        auto token = next();
        // ignore initial whitespaces
        if (first) {
            if (token.type == lexer::token_type::whitespace) {
                continue;
            }

            first = false;
        }

        if (token.type == lexer::token_type::new_line) {
            if (util::is_any_of(peek_next(), lexer::token_type::whitespace, lexer::token_type::tab)) {
                // more data to read in next line
                next(); // get rid of first sp or only tab
                first = true;
                done = false;
            } else {
                done = true;
            }
        } else if (token.type == lexer::token_type::coma) {
            done = true;
        } else {
            tokens.push_back(std::move(token));
        }
    } while (done);

    return tokens;
}

void parser::load() {
    lexer::token token;
    do {
        token = m_lexer.next();
        m_tokens.push_back(token);
    } while (token.type != lexer::token_type::eof);
}

lexer::token_type parser::peek_next() const {
    if (m_tokens.empty()) {
        return lexer::token_type::eof;
    }

    return m_tokens.front().type;
}

lexer::token parser::next() {
    auto token = std::move(m_tokens.front());
    m_tokens.pop_front();
    return std::move(token);
}

}
