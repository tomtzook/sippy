#pragma once

#include <optional>

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

class reader {
public:
    explicit reader(std::istream& is);

    void reset();
    message& get();
    message_ptr release();

    void parse_headers();
    bool can_parse_body();
    void parse_body();

private:
    void parse_start_line();
    bool parse_next_header();

    void load_header_values(const std::string& name, std::string&& value);

    [[nodiscard]] uint32_t get_body_length() const;
    void load_body(const std::string& type, std::string&& value);

    std::istream& m_is;
    message_ptr m_message;

};

}
