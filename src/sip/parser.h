#pragma once

#include <optional>

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
    explicit parser(std::istream& is);

private:
    void parse_start_line();
    void parse_next_header();
    void parse_body();

    void load_header_values(const std::string& name, std::string&& value);

    std::istream& m_is;
};

}
