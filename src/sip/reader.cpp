
#include <regex>

#include <sip/message.h>

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

reader::reader(std::istream& is)
    : m_is(is)
    , m_message()
{}

void reader::reset() {
    m_message = std::make_unique<message>();
}

message& reader::get() {
    return *m_message;
}

message_ptr reader::release() {
    return std::move(m_message);
}

void reader::parse_headers() {
    parse_start_line();
    while (parse_next_header());
}

bool reader::can_parse_body() {
    const auto len = get_body_length();
    if (len < 1) {
        return true;
    }

    const auto current_pos = m_is.tellg();
    m_is.seekg(0, std::ios::end);
    const auto end = m_is.tellg();
    m_is.seekg(current_pos, std::ios::beg);

    if ((end - current_pos) < len) {
        return false;
    }

    return true;
}

void reader::parse_body() {
    serialization::reader reader(m_is);
    reader.eat("\r\n");

    const auto len = get_body_length();
    if (len < 1) {
        return;
    }

    auto body = reader.read(len);
    if (m_message->has_header<headers::content_type>()) {
        const auto& type = m_message->header<headers::content_type>().type;
        load_body(type, std::move(body));

        m_message->remove_headers<headers::content_length>();
        m_message->remove_headers<headers::content_type>();
    } else {
        throw missing_content_type();
    }
}

void reader::parse_start_line() {
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
        ss >> request_line;
        m_message->set_request_line(std::move(request_line));
    } else if (match[4].matched && match[5].matched && match[6].matched) {
        // status
        std::stringstream ss(str);

        sip::status_line status_line;
        ss >> status_line;
        m_message->set_status_line(std::move(status_line));
    } else {
        throw bad_start_line();
    }
}

bool reader::parse_next_header() {
    header_reader header_reader(m_is);
    auto nameOpt = header_reader.read_header_name();
    if (!nameOpt.has_value()) {
        return false;
    }
    auto& name = nameOpt.value();

    auto valueOpt = header_reader.read_header_value();
    if (!valueOpt.has_value()) {
        throw missing_header_value();
    }
    auto& value = valueOpt.value();

    fixup_header_name(name);
    load_header_values(name, std::move(value));

    return true;
}

void reader::load_header_values(const std::string& name, std::string&& value) {
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
        reader.eat_while(serialization::is_whitespace);

        auto holder = def->create();
        holder->operator>>(ss);
        m_message->_add_header(name, std::move(holder));

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

uint32_t reader::get_body_length() const {
    if (!m_message->has_header<headers::content_length>()) {
        // no body then
        return 0;
    }

    return m_message->header<headers::content_length>().length;
}

void reader::load_body(const std::string& type, std::string&& value) {
    const auto defOpt = bodies::storage::get_body(type);
    if (!defOpt.has_value()) {
        throw unknown_body();
    }

    const auto& def = defOpt.value();

    std::stringstream ss(std::move(value));

    auto holder = def->create();
    holder->operator>>(ss);

    if (!ss.eof()) {
        throw body_trailing_data();
    }

    m_message->_set_body(std::move(holder));
}

}
