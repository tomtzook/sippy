
#include <regex>

#include <sip/message.h>

#include "serialization/matchers.h"
#include "header_storage.h"
#include "parser.h"

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

static void trim(std::istream& is) {
    serialization::reader reader(is);
    reader.eat_while(serialization::is_whitespace);
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
    m_reader.eat_while(serialization::is_whitespace);
    if (data.empty()) {
        // empty line, so next is the body
        return std::nullopt;
    }

    m_reader.eat(':');

    return {std::move(data)};
}

std::optional<std::string> header_reader::read_header_value() {
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

parser::parser(std::istream& is)
    : m_is(is)
{}

void parser::parse_start_line() {
    header_reader reader(m_is);
    const auto lineOpt = reader.read_start_line();
    if (!lineOpt.has_value()) {
        throw bad_start_line();
    }

    const auto& str = lineOpt.value();

    const std::regex pattern(R"(^(?:(?:(\w+)\s(.+)\sSIP\/(2\.0))|(?:SIP\/(2\.0)\s(\d+)\s(.+)))$)");
    std::smatch match;
    if (!std::regex_match(str, match, pattern)) {
        throw bad_start_line();
    }

    if (match[1].matched && match[2].matched && match[3].matched) {
        // request
        std::stringstream ss(str);

        sip::request_line request_line;
        ss >> request_line; // todo: return/store value
    } else if (match[4].matched && match[5].matched && match[6].matched) {
        // status
        std::stringstream ss(str);

        sip::status_line status_line;
        ss >> status_line; // todo: return/store value
    } else {
        throw bad_start_line();
    }
}

void parser::parse_next_header() {
    header_reader header_reader(m_is);
    auto nameOpt = header_reader.read_header_name();
    if (!nameOpt.has_value()) {
        return;
    }
    auto& name = nameOpt.value();

    auto valueOpt = header_reader.read_header_value();
    if (!valueOpt.has_value()) {
        throw missing_header_value();
    }
    auto& value = valueOpt.value();

    fixup_header_name(name);
    load_header_values(name, std::move(value));
}

void parser::parse_body() {

}

void parser::load_header_values(const std::string& name, std::string&& value) {
    const auto defOpt = headers::storage::get_header(name);
    if (!defOpt.has_value()) {
        // unknown header, ignore it
        return;
    }

    const auto& def = defOpt.value();
    const auto can_multiple = (def->flags() & headers::flag_allow_multiple) != 0;

    std::stringstream ss(std::move(value));
    serialization::reader reader(ss);
    do {
        auto holder = def->create();
        holder->operator>>(ss);
        // todo: save header

        reader.eat_while(serialization::is_whitespace);

        if (!ss.eof()) {
            if (!can_multiple || !reader.peek(',')) {
                throw header_value_trailing_data();
            }

            // more header values
            reader.eat(',');
            reader.eat_while(serialization::is_whitespace);

            // next loop run will parse the header
        }
    } while (!ss.eof());
}

}
