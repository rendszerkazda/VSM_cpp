#pragma once

/**
 * @file IStorage.h
 * @brief Abstract storage backend (Strategy pattern).
 */

#include <optional>
#include <string>

#include "vsm/Common.h"

namespace vsm {

/**
 * @brief Interface of a place where save files can be stored.
 *
 * This is the "Strategy" of the design: the rest of the program only knows
 * this interface, so the real cloud (SupabaseStorage), a plain folder
 * (LocalFolderStorage) or a test double can be swapped without touching the
 * business logic (Open-Closed and Dependency Inversion principles).
 */
class IStorage {
public:
    virtual ~IStorage() = default;

    /**
     * @brief Stores data under the given name, overwriting any existing data.
     *
     * @param name Object name, for example "Arena19_db.gz".
     * @param data The bytes to store.
     * @throws std::runtime_error if the operation fails.
     */
    virtual void put(const std::string& name, const Bytes& data) = 0;

    /**
     * @brief Reads the data stored under the given name.
     *
     * @param name Object name.
     * @return The data, or std::nullopt if no such object exists.
     * @throws std::runtime_error on errors other than "not found".
     */
    virtual std::optional<Bytes> get(const std::string& name) = 0;
};

}  // namespace vsm
