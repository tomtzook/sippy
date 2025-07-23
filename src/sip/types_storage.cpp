
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


void __attribute__((constructor)) register_known_types() {
    headers::register_header<headers::from>();
    headers::register_header<headers::to>();
    headers::register_header<headers::contact>();
    headers::register_header<headers::cseq>();
    headers::register_header<headers::call_id>();
    headers::register_header<headers::via>();
    headers::register_header<headers::content_length>();
    headers::register_header<headers::content_type>();
    headers::register_header<headers::max_forwards>();
    headers::register_header<headers::expires>();
    headers::register_header<headers::min_expires>();
    headers::register_header<headers::record_route>();
    headers::register_header<headers::route>();
    headers::register_header<headers::subject>();
    headers::register_header<headers::allow>();
    headers::register_header<headers::server>();
    headers::register_header<headers::authorization>();
    headers::register_header<headers::www_authorization>();

    bodies::register_body<bodies::test>();
}

}
