#pragma once

#include <istream>
#include <vector>

namespace sippy::lang {

enum class token_type {
    eof,
    whitespace,
    tab,
    cr,
    lf,
    crlf,
    colon,
    coma,
    forward_slash,
    backward_slash,
    less_than,
    greater_than,
    string,
    open_string
};

struct token {
    token_type type;
    std::string_view str;
};

class lexer {
public:
    explicit lexer(std::istream& is);
    explicit lexer(const std::vector<token>& tokens);

    token_type peek();
    token next();
    void go_back(size_t count);

private:
    struct token_storage {
        token_type type;
        std::string str;
    };

    token_storage new_raw_token();
    std::string read_string(char initial_ch);
    std::string read_open_string();

    bool eat(char ch);
    void eat_whitespaces();
    int next_char();

    std::istream* m_is;
    bool m_done;
    std::vector<token_storage> m_read_tokens;
    size_t m_token_index;
};

}
