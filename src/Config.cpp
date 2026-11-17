#include "vsm/Config.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace vsm {

Config& Config::instance() {
    static Config config;
    return config;
}

void Config::reset() {
    username_.clear();
    worldName_ = "Arena19";
    supabaseUrl_.clear();
    supabaseKey_.clear();
    bucket_ = "valheim-saves";
    saveDir_.clear();
}

void Config::loadFromFile(const std::filesystem::path& path) {
    std::ifstream in(path);
    if (!in) {
        throw std::runtime_error("Cannot open config file: " + path.string());
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    loadFromText(buffer.str());
}

void Config::loadFromText(const std::string& jsonText) {
    nlohmann::json json;
    try {
        json = nlohmann::json::parse(jsonText);
    } catch (const nlohmann::json::exception& e) {
        throw std::runtime_error(std::string("Invalid config JSON: ") + e.what());
    }
    if (!json.is_object()) {
        throw std::runtime_error("Invalid config JSON: top level must be an object");
    }

    reset();
    username_ = json.value("username", username_);
    worldName_ = json.value("world_name", worldName_);
    supabaseKey_ = json.value("supabase_key", supabaseKey_);
    bucket_ = json.value("bucket", bucket_);
    saveDir_ = json.value("save_dir", saveDir_);

    if (json.contains("supabase_url")) {
        supabaseUrl_ = json.value("supabase_url", std::string());
    } else if (json.contains("supabase_project_id")) {
        supabaseUrl_ = "https://" + json.value("supabase_project_id", std::string()) + ".supabase.co";
    }
}

void Config::applyEnvironment() {
    if (const char* key = std::getenv("VSM_SUPABASE_KEY")) {
        if (*key != '\0') {
            supabaseKey_ = key;
        }
    }
}

Settings Config::settings() const {
    return Settings{username_, worldName_};
}

}  // namespace vsm
