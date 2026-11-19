#include "vsm/ActivityLog.h"

#include <ctime>
#include <exception>
#include <utility>

#include <nlohmann/json.hpp>

namespace vsm {

namespace {

const char* const kLogName = "log.json";

nlohmann::json loadEntries(IStorage& storage) {
    const auto data = storage.get(kLogName);
    if (!data) {
        return nlohmann::json::array();
    }
    auto parsed = nlohmann::json::parse(data->begin(), data->end(), nullptr, false);
    if (parsed.is_discarded() || !parsed.is_array()) {
        return nlohmann::json::array();
    }
    return parsed;
}

}  // namespace

ActivityLog::ActivityLog(IStorage& storage, Clock clock)
    : storage_(storage), clock_(std::move(clock)) {}

std::string ActivityLog::defaultClock() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local);
    return buffer;
}

void ActivityLog::append(const std::string& username, const std::string& action,
                         const std::string& world) {
    nlohmann::json entries = loadEntries(storage_);
    entries.push_back({{"username", username}, {"action", action}, {"time", clock_()}, {"world", world}});
    const std::string text = entries.dump(4);
    storage_.put(kLogName, Bytes(text.begin(), text.end()));
}

std::optional<LogEntry> ActivityLog::lastFor(const std::string& world) {
    const nlohmann::json entries = loadEntries(storage_);
    for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
        if (it->is_object() && it->value("world", "") == world) {
            return LogEntry{it->value("username", ""), it->value("action", ""),
                            it->value("time", ""), world};
        }
    }
    return std::nullopt;
}

ActivityLogObserver::ActivityLogObserver(ActivityLog& log, std::ostream& errors)
    : log_(log), errors_(errors) {}

void ActivityLogObserver::onEvent(const Event& event) {
    const char* action = nullptr;
    if (event.type == EventType::Uploaded) {
        action = "upload";
    } else if (event.type == EventType::Downloaded) {
        action = "download";
    } else {
        return;
    }
    try {
        log_.append(event.user, action, event.world);
    } catch (const std::exception& e) {
        errors_ << "Warning: could not write the activity log: " << e.what() << "\n";
    }
}

ConsoleObserver::ConsoleObserver(std::ostream& out) : out_(out) {}

void ConsoleObserver::onEvent(const Event& event) {
    out_ << "[" << event.world << "] " << event.message << "\n";
}

}  // namespace vsm
