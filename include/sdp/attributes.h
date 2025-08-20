#pragma once

#include <type_traits>
#include <memory>
#include <map>
#include <vector>

#include <sdp/types.h>

namespace sippy::sdp::attributes {

struct _attribute {};

enum flags : uint32_t {
    flag_none = 0,
    flag_session_level = 1,
    flag_media_level = 2,
    flag_charset_dependent = 4,
    flag_allow_multiple = 8,
};

namespace meta {

template<typename T>
concept _attribute_type = std::is_base_of_v<_attribute, T>;

template<typename T>
struct _attribute_detail {};
template<typename T>
struct _attribute_reader {};
template<typename T>
struct _attribute_writer {};

}

namespace storage {

struct _base_attribute_holder;

// shared_ptr explicitly to allow playing around with attributes
using _attribute_holder_ptr = std::shared_ptr<_base_attribute_holder>;

struct _base_attribute_holder {
    virtual ~_base_attribute_holder() = default;

    [[nodiscard]] virtual const char* name() const = 0;
    [[nodiscard]] virtual uint32_t flags() const = 0;

    virtual std::istream& operator>>(std::istream& is) = 0;
    virtual std::ostream& operator<<(std::ostream& os) const = 0;
};

template<meta::_attribute_type T>
struct _attribute_holder final : _base_attribute_holder {
    [[nodiscard]] const char* name() const override {
        return meta::_attribute_detail<T>::name();
    }
    [[nodiscard]] uint32_t flags() const override {
        return meta::_attribute_detail<T>::flags();
    }

    std::istream& operator>>(std::istream& is) override {
        is >> value;
        return is;
    }
    std::ostream& operator<<(std::ostream& os) const override {
        os << value;
        return os;
    }

    T value;
};

struct _base_attribute_def {
    virtual ~_base_attribute_def() = default;

    [[nodiscard]] virtual const char* name() const = 0;
    [[nodiscard]] virtual _attribute_holder_ptr create() const = 0;
};

template<meta::_attribute_type T>
struct _attribute_def final : _base_attribute_def {
    [[nodiscard]] const char* name() const override {
        return meta::_attribute_detail<T>::name();
    }
    [[nodiscard]] _attribute_holder_ptr create() const override {
        return std::make_shared<_attribute_holder<T>>();
    }
};

void _register_attribute_internal(const std::string& name, std::shared_ptr<_base_attribute_def> ptr);

}

template<meta::_attribute_type T>
void register_attribute() {
    const auto& name = meta::_attribute_detail<T>::name();
    auto def = std::make_shared<storage::_attribute_def<T>>();
    storage::_register_attribute_internal(name, std::move(def));
}

template<meta::_attribute_type T>
storage::_attribute_holder_ptr create_ptr(T&& t) {
    auto ptr = std::make_shared<storage::_attribute_holder<T>>();
    ptr->value = std::move(t);
    return std::move(ptr);
}

class attribute_container {
public:
    using attr_list = std::vector<storage::_attribute_holder_ptr>;
    using attr_map = std::map<std::string, attr_list, std::less<>>;

    struct const_iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = const storage::_base_attribute_holder;
        using pointer           = value_type*;
        using reference         = value_type&;

        const_iterator(attr_map::const_iterator map_it, attr_list::const_iterator lst_it)
            : m_map_it(map_it)
            , m_lst_it(lst_it)
        {}
        explicit const_iterator(attr_map::const_iterator map_it)
            : m_map_it(map_it)
            , m_lst_it(map_it->second.cbegin())
        {}

        reference operator*() const { return m_lst_it->operator*(); }
        pointer operator->() { return m_lst_it->operator->(); }

        const_iterator& operator++() {
            ++m_lst_it;
            if (m_lst_it == m_map_it->second.cend()) {
                ++m_map_it;
                m_lst_it = m_map_it->second.cbegin();
            }

            return *this;
        }
        const_iterator operator++(int) { const_iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator==(const const_iterator& a, const const_iterator& b) { return a.m_map_it == b.m_map_it && a.m_lst_it == b.m_lst_it; };
        friend bool operator!=(const const_iterator& a, const const_iterator& b) { return a.m_map_it != b.m_map_it && a.m_lst_it != b.m_lst_it; };

    private:
        attr_map::const_iterator m_map_it;
        attr_list::const_iterator m_lst_it;
    };

    template<meta::_attribute_type T>
    struct const_attr_type_iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = const T;
        using pointer           = value_type*;
        using reference         = value_type&;

        explicit const_attr_type_iterator(attr_list::const_iterator lst_it)
            : m_lst_it(lst_it)
        {}

        reference operator*() const { return reinterpret_cast<reference>(reinterpret_cast<const storage::_attribute_holder<T>*>(m_lst_it->operator->())->value); }
        pointer operator->() { return reinterpret_cast<pointer>(&reinterpret_cast<const storage::_attribute_holder<T>*>(m_lst_it->operator->())->value); }

        const_attr_type_iterator& operator++() { ++m_lst_it; return *this; }
        const_attr_type_iterator operator++(int) { const_attr_type_iterator tmp = *this; ++(*this); return tmp; }

        friend bool operator==(const const_attr_type_iterator& a, const const_attr_type_iterator& b) { return a.m_lst_it == b.m_lst_it; };
        friend bool operator!=(const const_attr_type_iterator& a, const const_attr_type_iterator& b) { return a.m_lst_it != b.m_lst_it; };

    private:
        attr_list::const_iterator m_lst_it;
    };

    attribute_container() = default;
    attribute_container(const attribute_container&) = delete;
    attribute_container(attribute_container&&) = default;
    ~attribute_container() = default;

    attribute_container& operator=(const attribute_container&) = delete;
    attribute_container& operator=(attribute_container&&) = default;

    template<meta::_attribute_type T>
    [[nodiscard]] bool has() const;
    template<meta::_attribute_type T>
    [[nodiscard]] size_t count() const;
    template<meta::_attribute_type T>
    const T& get(size_t index = 0) const;
    template<meta::_attribute_type T>
    T& get(size_t index = 0);

    template<meta::_attribute_type T>
    void add(const T& attr);
    template<meta::_attribute_type T>
    void add(T&& attr);
    template<meta::_attribute_type T>
    void copy(const attribute_container& other);
    template<meta::_attribute_type T>
    bool remove_one(size_t index = 0);
    template<meta::_attribute_type T>
    bool remove();

    [[nodiscard]] size_t count(std::string_view name) const;
    void add(attribute_container&& other);
    void add(storage::_attribute_holder_ptr holder);

    [[nodiscard]] const_iterator begin() const;
    [[nodiscard]] const_iterator end() const;

    template<meta::_attribute_type T>
    [[nodiscard]] const_attr_type_iterator<T> begin_attr() const;
    template<meta::_attribute_type T>
    [[nodiscard]] const_attr_type_iterator<T> end_attr() const;

private:
    [[nodiscard]] size_t _count(const std::string& name) const;
    [[nodiscard]] const storage::_base_attribute_holder* _get(const std::string& name, size_t index) const;
    storage::_base_attribute_holder* _get(const std::string& name, size_t index);
    void _add(const std::string& name, storage::_attribute_holder_ptr holder);
    void _copy(const std::string& name, const attribute_container& other);
    bool _remove(const std::string& name, size_t index);
    bool _remove(const std::string& name);

    attr_map m_container;
};

template<meta::_attribute_type T>
bool attribute_container::has() const {
    return count<T>() > 0;
}

template<meta::_attribute_type T>
size_t attribute_container::count() const {
    const auto& name = meta::_attribute_detail<T>::name();
    return _count(name);
}

template<meta::_attribute_type T>
const T& attribute_container::get(size_t index) const {
    const auto& name = meta::_attribute_detail<T>::name();

    auto holder = reinterpret_cast<const storage::_attribute_holder<T>*>(_get(name, index));
    return holder->value;
}

template<meta::_attribute_type T>
T& attribute_container::get(size_t index) {
    const auto& name = meta::_attribute_detail<T>::name();

    auto holder = reinterpret_cast<storage::_attribute_holder<T>*>(_get(name, index));
    return holder->value;
}

template<meta::_attribute_type T>
void attribute_container::add(const T& attr) {
    T attr_copy = attr;
    add(std::move(attr_copy));
}

template<meta::_attribute_type T>
void attribute_container::add(T&& attr) {
    const auto& name = meta::_attribute_detail<T>::name();

    auto holder = std::make_unique<storage::_attribute_holder<T>>();
    holder->value = std::forward<T>(attr);

    _add(name, std::move(holder));
}

template<meta::_attribute_type T>
void attribute_container::copy(const attribute_container& other) {
    const auto& name = meta::_attribute_detail<T>::name();
    _copy(name, other);
}

template<meta::_attribute_type T>
bool attribute_container::remove_one(size_t index) {
    const auto& name = meta::_attribute_detail<T>::name();
    return _remove(name, index);
}

template<meta::_attribute_type T>
bool attribute_container::remove() {
    const auto& name = meta::_attribute_detail<T>::name();
    return _remove(name);
}

template<meta::_attribute_type T>
attribute_container::const_attr_type_iterator<T> attribute_container::begin_attr() const {
    const auto& name = meta::_attribute_detail<T>::name();

    auto it = m_container.find(name);
    if (it == m_container.end()) {
        return const_attr_type_iterator<T>{{}};
    }

    return const_attr_type_iterator<T>{it->second.cbegin()};
}

template<meta::_attribute_type T>
attribute_container::const_attr_type_iterator<T> attribute_container::end_attr() const {
    const auto& name = meta::_attribute_detail<T>::name();

    auto it = m_container.find(name);
    if (it == m_container.end()) {
        return const_attr_type_iterator<T>{{}};
    }

    return const_attr_type_iterator<T>{it->second.cend()};
}

}

#define DECLARE_SDP_ATTRIBUTE(a_name, str_name, flags_int) \
    namespace sippy::sdp::attributes { \
        struct a_name; \
        std::istream& operator>>(std::istream& is, a_name & a); \
        std::ostream& operator<<(std::ostream& os, const a_name & a); \
        namespace meta { \
            template<> struct _attribute_detail<sippy::sdp::attributes::a_name> { \
                static constexpr const char* name() { return (str_name) ; } \
                static constexpr uint32_t flags() { return (flags_int) ; } \
            }; \
            template<> struct _attribute_reader<sippy::sdp::attributes::a_name> { \
                static void read(std::istream& is, sippy::sdp::attributes::a_name & a) { is >> a; } \
            }; \
            template<> struct _attribute_writer<sippy::sdp::attributes::a_name> { \
                static void write(std::ostream& os, sippy::sdp::attributes::a_name & a) { os << a; } \
            }; \
        } \
    } \
    struct sippy::sdp::attributes::a_name : sippy::sdp::attributes::_attribute

#define DEFINE_SDP_ATTRIBUTE_READ(a_name) \
    namespace sippy::sdp::attributes { \
        static void read_attribute_ ##a_name(std::istream& is, a_name & a); \
        std::istream& operator>>(std::istream& is, a_name & a) { \
            read_attribute_ ##a_name(is, a); \
            return is; \
        } \
    } \
    static void sippy::sdp::attributes::read_attribute_ ##a_name(std::istream& is, a_name & a)


#define DEFINE_SDP_ATTRIBUTE_WRITE(a_name) \
    namespace sippy::sdp::attributes { \
        static void write_attribute_ ##a_name(std::ostream& os, const a_name & a); \
        std::ostream& operator<<(std::ostream& os, const a_name & a) { \
            write_attribute_ ##a_name(os, a); \
            return os; \
        } \
    } \
    static void sippy::sdp::attributes::write_attribute_ ##a_name(std::ostream& os, const a_name & a)


DECLARE_SDP_ATTRIBUTE(tool, "tool", flag_session_level) {
    std::string name;
    std::string version;
};

DECLARE_SDP_ATTRIBUTE(ptime, "ptime", flag_media_level) {
    uint64_t time;
};

DECLARE_SDP_ATTRIBUTE(maxptime, "maxptime", flag_media_level) {
    uint64_t time;
};

DECLARE_SDP_ATTRIBUTE(rtpmap, "rtpmap", flag_media_level | flag_allow_multiple) {
    uint32_t payload_type;
    std::string encoding_name;
    uint16_t clock_rate;
    uint8_t channels;
};

DECLARE_SDP_ATTRIBUTE(fmtp, "fmtp", flag_media_level | flag_allow_multiple) {
    uint32_t payload_type;
    std::map<std::string, std::string> params;
};
