#pragma once

#include <istream>
#include <cstdint>
#include <exception>
#include <optional>
#include <regex>

#include "matchers.h"

namespace sippy::serialization {

class character_eof final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "character is eof";
    }
};

class no_character final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "no character";
    }
};

class unexpected_character final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "unexpected character";
    }
};

class not_enough_characters final : public std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "not enough characters";
    }
};

using character = int16_t;
static constexpr character eof = -1;
static constexpr character uneaten = -2;

static constexpr bool is_char(const character ch) {
    return ch >= 0;
}

static constexpr char to_char(const character ch) {
    if (is_char(ch)) {
        return static_cast<char>(ch);
    } else if (ch == eof) {
        throw character_eof();
    } else {
        throw no_character();
    }
}

class reader {
public:
    explicit reader(std::istream& is);

    bool peek(char ch);

    void eat(char ch);
    void eat(std::string_view str);
    bool eat_one_if(matcher matcher);
    void eat_while(matcher matcher);

    std::string read(size_t length);
    character read_one_if(matcher matcher);
    character read_one_if_not(matcher matcher);
    std::string read_while(matcher matcher);
    std::string read_until(matcher matcher);

    template<typename T>
    T read() {
        T t;
        m_is >> t;
        return t;
    }
private:
    std::istream& m_is;
};

std::smatch parse(const std::string& data, std::string_view pattern);
std::optional<std::smatch> try_parse(const std::string& data, std::string_view pattern);

}
