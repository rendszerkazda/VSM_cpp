#include "vsm/LocalFolderStorage.h"

#include <stdexcept>
#include <utility>

#include "vsm/FileUtil.h"
#include "vsm/Settings.h"

namespace vsm {

LocalFolderStorage::LocalFolderStorage(std::filesystem::path root) : root_(std::move(root)) {}

void LocalFolderStorage::put(const std::string& name, const Bytes& data) {
    if (!isValidName(name)) {
        throw std::runtime_error("Invalid object name: " + name);
    }
    writeFile(root_ / name, data);
}

std::optional<Bytes> LocalFolderStorage::get(const std::string& name) {
    if (!isValidName(name)) {
        throw std::runtime_error("Invalid object name: " + name);
    }
    const auto path = root_ / name;
    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }
    return readFile(path);
}

}  // namespace vsm
