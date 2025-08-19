
#include "string_helper.h"


namespace sippy::util {

bool is_numeric_string(const std::string_view str) {
    for (const auto& ch : str) {
        if (!std::isdigit(ch)) {
            return false;
        }
    }

    return true;
}

}
