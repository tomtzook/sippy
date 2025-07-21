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

class parser {
public:
    struct space {};
    struct new_line {};
    struct colon {};

    explicit parser(serialization::lexer& lexer);

    /*template<typename T>
    T read() = delete;
    template<>
    space read<space>();
    template<>
    new_line read();
    template<>
    colon read();
    template<>
    std::string_view read();
    template<>
    version read();
    template<>
    method read();*/

    void consume_whitespace();

    serialization::token next_token();
    serialization::token next_token(serialization::token_type expected_type);

private:
    std::optional<version> try_read_version(const serialization::token& c_token);

    serialization::lexer& m_lexer;
};

class msg_parser {
public:
    explicit msg_parser(std::istream& is);

private:

    void parse_header_line();
    std::vector<serialization::token> next_header_value_tokens();

    void load();
    [[nodiscard]] serialization::token_type peek_next() const;
    serialization::token next();

    serialization::lexer m_lexer;
    std::deque<serialization::token> m_tokens;
};

}
