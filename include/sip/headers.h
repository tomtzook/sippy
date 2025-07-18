#pragma once

#include <exception>
#include <optional>
#include <memory>

#include <sip/types.h>

namespace sippy::sip::headers {

class header_not_found final : std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "header not found";
    }
};

struct _header {};

enum flags : uint32_t {
    flag_priority_top = 1,
    flag_priority_normal = 2,
    flag_allow_multiple = 4,
};

namespace meta {

template<typename T>
concept _header_type = std::is_base_of_v<_header, T>;

template<typename T>
struct _header_detail {};
template<typename T>
struct _header_reader {};
template<typename T>
struct _header_writer {};

}

namespace storage {

struct _base_header_holder;

using _header_holder_ptr = std::unique_ptr<_base_header_holder>;

struct _base_header_holder {
    virtual ~_base_header_holder() = default;

    [[nodiscard]] virtual _header_holder_ptr copy() const = 0;
    virtual std::istream& operator>>(std::istream& is) = 0;
    virtual std::ostream& operator<<(std::ostream& os) = 0;
};

template<meta::_header_type T>
struct _header_holder final : _base_header_holder {
    [[nodiscard]] _header_holder_ptr copy() const override {
        auto cpy = std::make_unique<_header_holder>();
        cpy->value = value;
        return std::move(cpy);
    }
    std::istream& operator>>(std::istream& is) override {
        is >> value;
        return is;
    }
    std::ostream& operator<<(std::ostream& os) override {
        os << value;
        return os;
    }

    T value;
};

struct _base_header_def {
    virtual ~_base_header_def() = default;

    [[nodiscard]] virtual const char* name() const = 0;
    [[nodiscard]] virtual uint32_t flags() const = 0;
    [[nodiscard]] virtual _header_holder_ptr create() const = 0;
};

template<meta::_header_type T>
struct _header_def final : _base_header_def {
    [[nodiscard]] const char* name() const override {
        return meta::_header_detail<T>::name();
    }
    [[nodiscard]] uint32_t flags() const override {
        return meta::_header_detail<T>::flags();
    }
    [[nodiscard]] _header_holder_ptr create() const override {
        return std::make_unique<_header_holder<T>>();
    }
};

void _register_header_internal(const std::string& name, std::shared_ptr<_base_header_def> ptr);

}

template<meta::_header_type T>
void register_header() {
    const auto& name = meta::_header_detail<T>::name();
    auto def = std::make_shared<storage::_header_def<T>>();
    storage::_register_header_internal(name, std::move(def));
}

}

#define DECLARE_SIP_HEADER(h_name, str_name, flags_int) \
    namespace sippy::sip::headers { \
        struct h_name; \
        std::istream& operator>>(std::istream& is, h_name & h); \
        std::ostream& operator<<(std::ostream& os, h_name & h); \
        namespace meta { \
            template<> struct _header_detail<sippy::sip::headers::h_name> { \
                static constexpr const char* name() { return (str_name) ; } \
                static constexpr uint32_t flags() { return (flags_int) ; } \
            }; \
            template<> struct _header_reader<sippy::sip::headers::h_name> { \
                static void read(std::istream& is, sippy::sip::headers::h_name & h) { is >> h; } \
            }; \
            template<> struct _header_writer<sippy::sip::headers::h_name> { \
                static void write(std::ostream& os, sippy::sip::headers::h_name & h) { os << h; } \
            }; \
        } \
    } \
    struct sippy::sip::headers::h_name : sippy::sip::headers::_header

#define DEFINE_SIP_HEADER_READ(h_name) \
    namespace sippy::sip::headers { \
        static void read_header_ ##h_name(std::istream& is, h_name & h); \
        std::istream& operator>>(std::istream& is, h_name & h) { \
            read_header_ ##h_name(is, h); \
            return is; \
        } \
    } \
    static void sippy::sip::headers::read_header_ ##h_name(std::istream& is, h_name & h)

#define DEFINE_SIP_HEADER_WRITE(h_name) \
    namespace sippy::sip::headers { \
        static void write_header_ ##h_name(std::ostream& os, h_name & h); \
        std::ostream& operator<<(std::ostream& os, h_name & h) { \
            write_header_ ##h_name(os, h); \
            return os; \
        } \
    } \
    static void sippy::sip::headers::write_header_ ##h_name(std::ostream& os, h_name & h)


DECLARE_SIP_HEADER(from, "From", flag_priority_normal) {
    std::optional<std::string> display_name;
    std::string uri;
    std::optional<std::string> tag;
};

DECLARE_SIP_HEADER(to, "To", flag_priority_normal) {
    std::optional<std::string> display_name;
    std::string uri;
    std::optional<std::string> tag;
};

DECLARE_SIP_HEADER(cseq, "CSeq", flag_priority_normal) {
    uint32_t seq_num;
    sip::method method;
};

DECLARE_SIP_HEADER(content_length, "Content-Length", flag_priority_top) {
    uint32_t length;
};

DECLARE_SIP_HEADER(content_type, "Content-Type", flag_priority_top) {
    std::string type;
};
