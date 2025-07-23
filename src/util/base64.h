#pragma once

#include <string>
#include <vector>

namespace sippy::util {

std::vector<uint8_t> base64_decode(std::string_view data);

}
