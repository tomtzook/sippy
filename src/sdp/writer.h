#pragma once

#include <sdp/message.h>

namespace sippy::sdp {

class writer {
public:
    explicit writer(std::ostream& os);

    void write(const description_message& message);

private:
    template<typename T>
    void write_field(const T& field) {
        m_os << fields::meta::_field_detail<T>::name();
        m_os << '=';
        m_os << field;
        m_os << "\r\n";
    }

    template<typename T>
    void write_field_opt(const std::optional<T>& field) {
        if (field.has_value()) {
            write_field(field.value());
        }
    }

    template<typename T>
    void write_field_vec(const std::vector<T>& field) {
        for (const auto& fld : field) {
            write_field(fld);
        }
    }

    std::ostream& m_os;
};

}
