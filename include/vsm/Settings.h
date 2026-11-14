#pragma once

/**
 * @file Settings.h
 * @brief Plain data describing who is working on which Valheim world.
 */

#include <string>

namespace vsm {

/**
 * @brief The user-level settings needed by every operation.
 *
 * This is a simple value type that is passed around explicitly (dependency
 * injection), so that the business logic does not depend on the Config
 * singleton and stays easy to test.
 */
struct Settings {
    /** @brief Name of the player, written to the activity log. */
    std::string username;
    /** @brief Name of the Valheim world (the file name without extension). */
    std::string worldName;
};

/**
 * @brief Checks that a name is safe to use in file names and URLs.
 *
 * Only letters, digits, '_', '-', '.' and ' ' are accepted, the name must not be
 * empty and must not contain "..". This also prevents path traversal and
 * shell injection through the world name.
 *
 * @param name The name to check.
 * @return true if the name is acceptable.
 */
bool isValidName(const std::string& name);

}  // namespace vsm
