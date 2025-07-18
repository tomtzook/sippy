#pragma once

#include <string>

#include <sip/headers.h>

namespace sippy::sip::headers::storage {

std::optional<std::shared_ptr<_base_header_def>> get_header(const std::string& name);

}
