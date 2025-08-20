#pragma once

#include <string>
#include <optional>
#include <memory>

#include <sdp/attributes.h>

namespace sippy::sdp::attributes::storage {

std::optional<std::shared_ptr<_base_attribute_def>> get_attribute(const std::string& name);

}
