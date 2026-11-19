#pragma once

/**
 * @file ActivityLog.h
 * @brief Shared activity log ("who uploaded/downloaded what and when").
 */

#include <functional>
#include <optional>
#include <ostream>
#include <string>

#include "vsm/Events.h"
#include "vsm/IStorage.h"

namespace vsm {

/** @brief One line of the activity log. */
struct LogEntry {
    std::string username;  ///< Who did it.
    std::string action;    ///< "upload" or "download".
    std::string time;      ///< Local time, "YYYY-MM-DD HH:MM:SS".
    std::string world;     ///< World name.
};

/**
 * @brief Reads and appends entries of the "log.json" object in a storage.
 *
 * The format is the same JSON array that the earlier Python version used, so
 * old and new versions can share a log.
 */
class ActivityLog {
public:
    /** @brief Function that returns the current time as text (replaceable in tests). */
    using Clock = std::function<std::string()>;

    /**
     * @brief Creates the log.
     * @param storage Storage that holds log.json (must outlive this object).
     * @param clock Time source; defaults to the system clock.
     */
    explicit ActivityLog(IStorage& storage, Clock clock = defaultClock);

    /**
     * @brief Appends an entry for the given user, action and world.
     * @param username Who did it.
     * @param action "upload" or "download".
     * @param world World name.
     */
    void append(const std::string& username, const std::string& action, const std::string& world);

    /**
     * @brief Finds the most recent entry of a world.
     * @param world World name.
     * @return The last entry, or std::nullopt if there is none.
     */
    std::optional<LogEntry> lastFor(const std::string& world);

    /** @brief Returns the current system time as "YYYY-MM-DD HH:MM:SS". */
    static std::string defaultClock();

private:
    IStorage& storage_;
    Clock clock_;
};

/**
 * @brief Observer that records uploads and downloads in the ActivityLog.
 */
class ActivityLogObserver : public IObserver {
public:
    /**
     * @brief Creates the observer.
     * @param log The log to write into.
     * @param errors Stream where logging failures are reported; a failing log
     *        must never abort a successful upload or download.
     */
    ActivityLogObserver(ActivityLog& log, std::ostream& errors);

    /** @copydoc IObserver::onEvent */
    void onEvent(const Event& event) override;

private:
    ActivityLog& log_;
    std::ostream& errors_;
};

/**
 * @brief Observer that prints every event as a line of text.
 */
class ConsoleObserver : public IObserver {
public:
    /** @brief Creates the observer writing to the given stream. */
    explicit ConsoleObserver(std::ostream& out);

    /** @copydoc IObserver::onEvent */
    void onEvent(const Event& event) override;

private:
    std::ostream& out_;
};

}  // namespace vsm
