#pragma once


namespace sippy::serialization {

using matcher = bool(*)(char);

static constexpr bool is_letter(const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static constexpr bool is_dash(const char c) {
    return c == '-';
}

static constexpr bool is_whitespace(const char c) {
    return c == ' ';
}

static constexpr bool is_tab(const char c) {
    return c == '\t';
}

static constexpr bool is_colon(const char c) {
    return c == ':';
}

static constexpr bool is_colon_or_semicolon(const char c) {
    return c == ':' || c == ';';
}

static constexpr bool is_number(const char c) {
    return c >= '0' && c <= '9';
}

static constexpr bool is_dot(const char c) {
    return c == '.';
}

static constexpr bool is_semicolon(const char c) {
    return c == ';';
}

static constexpr bool is_new_line(const char ch) {
    return ch == '\r' || ch == '\n';
}

static constexpr bool is_letter_or_dash(const char c) {
    return is_letter(c) || is_dash(c);
}

static constexpr bool is_letter_number_or_dash(const char c) {
    return is_letter(c) || is_number(c) || is_dash(c);
}

static constexpr bool is_number_or_dot(const char c) {
    return is_number(c) || is_dot(c);
}

static constexpr bool is_whitespace_or_tab(const char c) {
    return is_whitespace(c) || is_tab(c);
}

static constexpr bool is_alphanumeric(const char c) {
    return is_letter(c) || is_number(c);
}

}
