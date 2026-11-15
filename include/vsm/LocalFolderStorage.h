#pragma once

/**
 * @file LocalFolderStorage.h
 * @brief IStorage implementation that keeps objects as files in a folder.
 */

#include <filesystem>

#include "vsm/IStorage.h"

namespace vsm {

/**
 * @brief Stores objects as plain files inside a root directory.
 *
 * Useful for tests, for offline use, and for synchronised folders
 * (OneDrive, Syncthing...).
 */
class LocalFolderStorage : public IStorage {
public:
    /**
     * @brief Creates the storage.
     * @param root Directory that holds the objects; created on first write.
     */
    explicit LocalFolderStorage(std::filesystem::path root);

    /** @copydoc IStorage::put */
    void put(const std::string& name, const Bytes& data) override;

    /** @copydoc IStorage::get */
    std::optional<Bytes> get(const std::string& name) override;

private:
    /** @brief Directory that holds the object files. */
    std::filesystem::path root_;
};

}  // namespace vsm
