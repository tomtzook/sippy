#pragma once

#include <string>

#include <sip/headers.h>
#include <sip/bodies.h>

namespace sippy::sip {

namespace headers::storage {

std::optional<std::shared_ptr<_base_header_def>> get_header(const std::string& name);

}

namespace bodies::storage {

std::optional<std::shared_ptr<_base_body_def>> get_body(const std::string& name);

}

}
