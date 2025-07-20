#pragma once

#include <optional>
#include <deque>

#include <sip/message.h>

#include "serialization/reader.h"

namespace sippy::sip {

class header_reader {
public:
    explicit header_reader(std::istream& is);

    std::optional<std::string> read_start_line();
    std::optional<std::string> read_header_name();
    std::optional<std::string> read_header_value();

private:
    void eat_new_line();

    serialization::reader m_reader;
};

class lexer {
public:
    enum class token_type {
        eof,
        whitespace,
        tab,
        colon,
        coma,
        new_line,
        string,
        version,
        method
    };

    struct token {
        token_type type = token_type::eof;
        union {
            sip::version version;
            sip::method method;
        } v;
        std::string_view str;
    };

    explicit lexer(std::istream& is);

    template<typename T>
    T next() = delete;
    template<>
    version next();
    template<>
    method next();

    token next();
    token next(token_type expected_type, bool eat_whitespaces = false);

private:
    std::optional<version> try_read_version(const serialization::tokenizer::token& c_token);

    const serialization::tokenizer::token& next_token();
    void go_back(size_t count);

    serialization::tokenizer m_tokenizer;
    std::vector<serialization::tokenizer::token> m_read_tokens;
    size_t m_token_index;
};

class header_line_parser {
public:
    explicit header_line_parser(lexer& lexer);
private:
    void parse();

    enum class state {
        start,
        name_value,
        name_end,
        value_start
    };

    lexer& m_lexer;
    state m_state;
};

class parser {
public:
    explicit parser(std::istream& is);

private:
    enum class state {
        start_line_start,
        request_line_uri,
        request_line_version,
        response_line_code,
        response_line_phrase,
        header_name_start,
        header_name_end,
        header_value
    };

    void parse_next();
    void parse_header_line();
    std::vector<lexer::token> next_header_value_tokens();

    void load();
    [[nodiscard]] lexer::token_type peek_next() const;
    lexer::token next();

    lexer m_lexer;
    std::deque<lexer::token> m_tokens;
};

}
