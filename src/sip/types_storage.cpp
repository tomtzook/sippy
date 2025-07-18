
#include <unordered_map>

#include "types_storage.h"

namespace sippy::sip {

namespace headers::storage {

static std::unordered_map<std::string, std::shared_ptr<_base_header_def>>& _get_storage() {
    static std::unordered_map<std::string, std::shared_ptr<_base_header_def>> _headers;
    return _headers;
}

void _register_header_internal(const std::string& name, std::shared_ptr<_base_header_def> ptr) {
    _get_storage()[name] = std::move(ptr);
}

std::optional<std::shared_ptr<_base_header_def> > get_header(const std::string &name) {
    const auto it = _get_storage().find(name);
    if (it != _get_storage().end()) {
        return it->second;
    }
    return std::nullopt;
}

}

namespace bodies::storage {

static std::unordered_map<std::string, std::shared_ptr<_base_body_def>>& _get_storage() {
    static std::unordered_map<std::string, std::shared_ptr<_base_body_def>> _headers;
    return _headers;
}

void _register_body_internal(const std::string& name, std::shared_ptr<_base_body_def> ptr) {
    _get_storage()[name] = std::move(ptr);
}

std::optional<std::shared_ptr<_base_body_def>> get_body(const std::string &name) {
    const auto it = _get_storage().find(name);
    if (it != _get_storage().end()) {
        return it->second;
    }
    return std::nullopt;
}

}

}
