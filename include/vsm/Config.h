#pragma once

/**
 * @file Config.h
 * @brief Application configuration (Singleton pattern).
 */

#include <filesystem>
#include <string>

#include "vsm/Settings.h"

namespace vsm {

/**
 * @brief Holds the configuration read from config.json.
 *
 * Singleton: exactly one configuration exists for the whole program and it is
 * reachable through Config::instance(). It is only used at the "edge" of the
 * program (main); the business classes receive plain Settings objects, so
 * they remain testable.
 *
 * Supported config.json keys: "username", "world_name", "supabase_url" or
 * "supabase_project_id", "supabase_key", "bucket", "save_dir".
 */
class Config {
public:
    /**
     * @brief Returns the single Config instance (created on first use).
     * @return Reference to the instance.
     */
    static Config& instance();

    /**
     * @brief Loads the configuration from a JSON file.
     * @param path Path of the config.json file.
     * @throws std::runtime_error if the file is missing or not valid JSON.
     */
    void loadFromFile(const std::filesystem::path& path);

    /**
     * @brief Loads the configuration from JSON text.
     * @param jsonText The JSON document.
     * @throws std::runtime_error if the text is not valid JSON.
     */
    void loadFromText(const std::string& jsonText);

    /**
     * @brief Lets the VSM_SUPABASE_KEY environment variable override the key.
     *
     * This allows keeping the secret out of any file in the repository.
     */
    void applyEnvironment();

    /** @brief Restores the default values (used by tests). */
    void reset();

    /** @brief Returns the user name and world name as a Settings object. */
    Settings settings() const;

    /** @brief Base URL of the Supabase project. */
    const std::string& supabaseUrl() const { return supabaseUrl_; }
    /** @brief Supabase API key. */
    const std::string& supabaseKey() const { return supabaseKey_; }
    /** @brief Name of the Supabase storage bucket. */
    const std::string& bucket() const { return bucket_; }
    /** @brief Optional override of the Valheim save directory (may be empty). */
    const std::string& saveDir() const { return saveDir_; }

    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

private:
    /** @brief Private constructor: only instance() creates the object. */
    Config() { reset(); }

    std::string username_;
    std::string worldName_;
    std::string supabaseUrl_;
    std::string supabaseKey_;
    std::string bucket_;
    std::string saveDir_;
};

}  // namespace vsm
