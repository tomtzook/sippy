#pragma once

#include <iostream>
#include <memory>

namespace sippy::sip::bodies {

struct _body {};

namespace meta {

template<typename T>
concept _body_type = std::is_base_of_v<_body, T>;

template<typename T>
struct _body_detail {};
template<typename T>
struct _body_reader {};
template<typename T>
struct _body_writer {};

}

namespace storage {

struct _base_body_holder;

using _body_holder_ptr = std::unique_ptr<_base_body_holder>;

struct _base_body_holder {
    virtual ~_base_body_holder() = default;

    [[nodiscard]] virtual const char* type() const = 0;
    [[nodiscard]] virtual bool is_of_type(const std::string& type) const = 0;
    virtual std::istream& operator>>(std::istream& is) = 0;
    virtual std::ostream& operator<<(std::ostream& os) = 0;
};

template<meta::_body_type T>
struct _body_holder final : _base_body_holder {
    [[nodiscard]] const char* type() const override {
        return meta::_body_detail<T>::app_type();
    }
    [[nodiscard]] bool is_of_type(const std::string& type) const override {
        return type == meta::_body_detail<T>::app_type();
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

struct _base_body_def {
    virtual ~_base_body_def() = default;

    [[nodiscard]] virtual const char* application_type() const = 0;
    [[nodiscard]] virtual _body_holder_ptr create() const = 0;
};

template<meta::_body_type T>
struct _body_def final : _base_body_def {
    [[nodiscard]] const char* application_type() const override {
        return meta::_body_detail<T>::app_type();
    }
    [[nodiscard]] _body_holder_ptr create() const override {
        return std::make_unique<_body_holder<T>>();
    }
};

void _register_body_internal(const std::string& name, std::shared_ptr<_base_body_def> ptr);

}

template<meta::_body_type T>
void register_body() {
    const auto& app_type = meta::_body_detail<T>::app_type();
    auto def = std::make_shared<storage::_body_def<T>>();
    storage::_register_body_internal(app_type, std::move(def));
}

}

#define DECLARE_SIP_BODY(b_name, app_type_str) \
    namespace sippy::sip::bodies { \
        struct b_name; \
        std::istream& operator>>(std::istream& is, b_name & b); \
        std::ostream& operator<<(std::ostream& os, b_name & b); \
        namespace meta { \
            template<> struct _body_detail<sippy::sip::bodies::b_name> { \
                static constexpr const char* app_type() { return (app_type_str) ; } \
            }; \
            template<> struct _body_reader<sippy::sip::bodies::b_name> { \
                static void read(std::istream& is, sippy::sip::bodies::b_name & h) { is >> h; } \
            }; \
            template<> struct _body_writer<sippy::sip::bodies::b_name> { \
                static void write(std::ostream& os, sippy::sip::bodies::b_name & h) { os << h; } \
            }; \
        } \
    } \
    struct sippy::sip::bodies::b_name : sippy::sip::bodies::_body

#define DEFINE_SIP_BODY_READ(b_name) \
    namespace sippy::sip::bodies { \
        static void read_body_ ##b_name(std::istream& is, b_name & b); \
        std::istream& operator>>(std::istream& is, b_name & b) { \
            read_body_ ##b_name(is, b); \
            return is; \
        } \
    } \
    static void sippy::sip::bodies::read_body_ ##b_name(std::istream& is, b_name & b)

#define DEFINE_SIP_BODY_WRITE(b_name) \
    namespace sippy::sip::bodies { \
        static void write_body_ ##b_name(std::ostream& os, b_name & b); \
        std::ostream& operator<<(std::ostream& os, b_name & b) { \
            write_body_ ##b_name(os, b); \
            return os; \
        } \
    } \
    static void sippy::sip::bodies::write_body_ ##b_name(std::ostream& os, b_name & b)


DECLARE_SIP_BODY(test, "application/test") {
    std::string v;
};
