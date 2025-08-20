
#include <sdp/attributes.h>

namespace sippy::sdp::attributes {

class attribute_not_found final : std::exception {
public:
    [[nodiscard]] const char *what() const noexcept override {
        return "attribute not found";
    }
};

size_t attribute_container::count(const std::string_view name) const {
    const auto it = m_container.find(name);
    if (it != m_container.end()) {
        return it->second.size();
    }

    return 0;
}

void attribute_container::add(attribute_container&& other) {
    for (auto& [name, holder] : other.m_container) {
        for (auto& header : holder) {
            _add(name, std::move(header));
        }
    }
}

void attribute_container::add(storage::_attribute_holder_ptr holder) {
    _add(holder->name(), std::move(holder));
}

attribute_container::const_iterator attribute_container::begin() const {
    return const_iterator{m_container.begin()};
}

attribute_container::const_iterator attribute_container::end() const {
    return const_iterator{m_container.end()};
}

size_t attribute_container::_count(const std::string& name) const {
    const auto it = m_container.find(name);
    if (it != m_container.end()) {
        return it->second.size();
    }

    return 0;
}

const storage::_base_attribute_holder* attribute_container::_get(const std::string& name, size_t index) const {
    const auto it = m_container.find(name);
    if (it == m_container.end()) {
        throw attribute_not_found();
    }
    if (it->second.empty()) {
        throw attribute_not_found();
    }
    if (index >= it->second.size()) {
        throw attribute_not_found();
    }

    return it->second[index].get();
}

storage::_base_attribute_holder* attribute_container::_get(const std::string& name, size_t index) {
    const auto it = m_container.find(name);
    if (it == m_container.end()) {
        throw attribute_not_found();
    }
    if (it->second.empty()) {
        throw attribute_not_found();
    }
    if (index >= it->second.size()) {
        throw attribute_not_found();
    }

    return it->second[index].get();
}

void attribute_container::_add(const std::string& name, storage::_attribute_holder_ptr holder) {
    const auto it = m_container.find(name);
    if (it == m_container.end()) {
        std::vector<storage::_attribute_holder_ptr> vector;
        vector.push_back(std::move(holder));
        m_container[name] = std::move(vector);
    } else {
        it->second.push_back(std::move(holder));
    }
}

void attribute_container::_copy(const std::string& name, const attribute_container& other) {
    const auto it = other.m_container.find(name);
    if (it != other.m_container.end()) {
        for (const auto& holder : it->second) {
            _add(name, holder);
        }
    }
}

bool attribute_container::_remove(const std::string& name, size_t index) {
    const auto it = m_container.find(name);
    if (it == m_container.end()) {
        return false;
    }
    if (it->second.empty()) {
        return false;
    }
    if (index >= it->second.size()) {
        return false;
    }

    it->second.erase(it->second.begin() + static_cast<ptrdiff_t>(index));
    return true;
}

bool attribute_container::_remove(const std::string& name) {
    const auto it = m_container.find(name);
    if (it == m_container.end()) {
        return false;
    }
    if (it->second.empty()) {
        return false;
    }

    it->second.clear();
    return true;
}

}
