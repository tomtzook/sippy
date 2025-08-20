
#include <unordered_map>

#include "types_storage.h"

namespace sippy::sdp::attributes::storage {

static std::unordered_map<std::string, std::shared_ptr<_base_attribute_def>>& _get_storage() {
    static std::unordered_map<std::string, std::shared_ptr<_base_attribute_def>> _headers;
    return _headers;
}

void _register_attribute_internal(const std::string& name, std::shared_ptr<_base_attribute_def> ptr) {
    _get_storage()[name] = std::move(ptr);
}

std::optional<std::shared_ptr<_base_attribute_def>> get_attribute(const std::string& name) {
    const auto it = _get_storage().find(name);
    if (it != _get_storage().end()) {
        return it->second;
    }
    return std::nullopt;
}

void __attribute__((constructor)) register_known_types() {
    register_attribute<tool>();
    register_attribute<ptime>();
    register_attribute<maxptime>();
    register_attribute<rtpmap>();
}

}
