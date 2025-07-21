
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

static void verify_token_type(const serialization::token& token, const serialization::token_type type) {
    if (token.type != type) {
        throw unexpected_token();
    }
}

parser::parser(serialization::lexer& lexer)
    : m_lexer(lexer)
{}

/*template<>
parser::space parser::read<parser::space>() {
    next_token(serialization::token_type::whitespace);
    return {};
}

template<>
parser::new_line parser::read<parser::new_line>() {
    next_token(serialization::token_type::crlf);
    return {};
}

template<>
parser::colon parser::read<parser::colon>() {
    next_token(serialization::token_type::colon);
    return {};
}

template<>
std::string_view parser::read<std::string_view>() {
    return next_token(serialization::token_type::string).str;
}

template<>
version parser::read<version>() {
    const auto& token = next_token(serialization::token_type::string);
    const auto ver_opt = try_read_version(token);
    if (ver_opt) {
        return ver_opt.value();
    }

    throw unexpected_token();
}

template<>
method parser::read<method>() {
    const auto& token = next_token(serialization::token_type::string);
    const auto method_opt = try_get_method(token.str);
    if (method_opt) {
        return method_opt.value();
    }

    throw unexpected_token();
}*/

void parser::consume_whitespace() {
    const auto token = next_token();
    if (token.type != serialization::token_type::whitespace) {
        m_lexer.go_back(1);
    }
}

serialization::token parser::next_token() {
    return m_lexer.next();
}

serialization::token parser::next_token(const serialization::token_type expected_type) {
    const auto token = next_token();
    verify_token_type(token, expected_type);
    return token;
}

std::optional<version> parser::try_read_version(const serialization::token& c_token) {
    if (c_token.type != serialization::token_type::string || strcasecmp(c_token.str.data(), "SIP") != 0) {
        return std::nullopt;
    }

    {
        const auto token = next_token();
        if (token.type != serialization::token_type::forward_slash) {
            m_lexer.go_back(1);
            return std::nullopt;
        }
    }

    {
        const auto token = next_token();
        if (token.type != serialization::token_type::string) {
            m_lexer.go_back(2);
            return std::nullopt;
        }

        const auto version_opt = try_get_version(token.str);
        if (version_opt) {
            return version_opt.value();
        } else {
            m_lexer.go_back(2);
            return std::nullopt;
        }
    }
}

msg_parser::msg_parser(std::istream& is)
    : m_lexer(is)
{}

void msg_parser::parse_header_line() {
    // name reading
    std::string_view name;
    {
        parser parser(m_lexer);
        name = "";//parser.read<std::string_view>();
        parser.consume_whitespace();
        //parser.read<parser::colon>();
        parser.consume_whitespace();
    }

    // value loading
    /*bool first = true;
    std::vector<serialization::token> value_tokens;
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

        auto holder = def->create();
        {
            serialization::lexer val_lexer(value_tokens);
            //holder->read(val_lexer);
        }
        //m_message->_add_header(name, std::move(holder));
    } while (!value_tokens.empty());*/
}

std::vector<serialization::token> msg_parser::next_header_value_tokens() {
    std::vector<serialization::token> tokens;
    bool first = true;
    bool done = false;
    do {
        auto token = next();
        // ignore initial whitespaces
        if (first) {
            if (token.type == serialization::token_type::whitespace) {
                continue;
            }

            first = false;
        }

        if (token.type == serialization::token_type::crlf) {
            if (util::is_any_of(peek_next(), serialization::token_type::whitespace, serialization::token_type::tab)) {
                // more data to read in next line
                next(); // get rid of first sp or only tab
                first = true;
                done = false;
            } else {
                done = true;
            }
        } else if (token.type == serialization::token_type::coma) {
            done = true;
        } else {
            tokens.push_back(std::move(token));
        }
    } while (done);

    return tokens;
}

void msg_parser::load() {
    serialization::token token;
    do {
        token = m_lexer.next();
        m_tokens.push_back(token);
    } while (token.type != serialization::token_type::eof);
}

serialization::token_type msg_parser::peek_next() const {
    if (m_tokens.empty()) {
        return serialization::token_type::eof;
    }

    return m_tokens.front().type;
}

serialization::token msg_parser::next() {
    auto token = std::move(m_tokens.front());
    m_tokens.pop_front();
    return std::move(token);
}

}
