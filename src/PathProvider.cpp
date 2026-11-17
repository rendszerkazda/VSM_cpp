#include "vsm/PathProvider.h"

#include <cstdlib>
#include <stdexcept>
#include <utility>

namespace vsm {

WindowsPathProvider::WindowsPathProvider(std::string userProfile)
    : userProfile_(std::move(userProfile)) {
    if (userProfile_.empty()) {
        if (const char* env = std::getenv("USERPROFILE")) {
            userProfile_ = env;
        }
    }
}

std::filesystem::path WindowsPathProvider::saveDir() const {
    if (userProfile_.empty()) {
        throw std::runtime_error(
            "USERPROFILE is not set. Use --save-dir or \"save_dir\" in config.json.");
    }
    return std::filesystem::path(userProfile_) / "AppData" / "LocalLow" / "IronGate" / "Valheim" /
           "worlds_local";
}

FixedPathProvider::FixedPathProvider(std::filesystem::path dir) : dir_(std::move(dir)) {}

std::filesystem::path FixedPathProvider::saveDir() const {
    return dir_;
}

std::unique_ptr<IPathProvider> makePathProvider(const std::string& overrideDir) {
    if (!overrideDir.empty()) {
        return std::make_unique<FixedPathProvider>(overrideDir);
    }
    return std::make_unique<WindowsPathProvider>();
}

}  // namespace vsm
