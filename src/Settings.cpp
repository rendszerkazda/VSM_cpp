#include "vsm/Settings.h"

#include <cctype>

namespace vsm {

bool isValidName(const std::string& name) {
    if (name.empty() || name.find("..") != std::string::npos) {
        return false;
    }
    for (unsigned char c : name) {
        const bool ok = std::isalnum(c) != 0 || c == '_' || c == '-' || c == '.' || c == ' ';
        if (!ok) {
            return false;
        }
    }
    return true;
}

}  // namespace vsm
