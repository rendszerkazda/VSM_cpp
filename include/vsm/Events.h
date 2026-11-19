#pragma once

/**
 * @file Events.h
 * @brief Event system (Observer pattern).
 */

#include <string>
#include <vector>

namespace vsm {

/** @brief The kinds of things that can happen during a save operation. */
enum class EventType {
    Uploaded,       ///< Save files were uploaded to the storage.
    Downloaded,     ///< Save files were downloaded from the storage.
    BackupCreated,  ///< Local files were backed up before being overwritten.
    Restored        ///< A local backup was restored.
};

/** @brief Describes one thing that happened. */
struct Event {
    /** @brief What happened. */
    EventType type;
    /** @brief World the event belongs to. */
    std::string world;
    /** @brief User who caused the event. */
    std::string user;
    /** @brief Human readable details. */
    std::string message;
};

/**
 * @brief Interface of an object that wants to be notified about events.
 *
 * The publisher (a command) does not know who listens: the console printer
 * and the cloud activity log are just two observers. Following the Hollywood
 * principle: "don't call us, we'll call you".
 */
class IObserver {
public:
    virtual ~IObserver() = default;

    /**
     * @brief Called for every published event.
     * @param event The event.
     */
    virtual void onEvent(const Event& event) = 0;
};

/**
 * @brief The subject of the Observer pattern: keeps observers and notifies them.
 */
class EventBus {
public:
    /**
     * @brief Registers an observer (it must outlive the bus or be unsubscribed).
     * @param observer The observer to add; adding it twice has no effect.
     */
    void subscribe(IObserver& observer);

    /**
     * @brief Removes an observer.
     * @param observer The observer to remove; unknown observers are ignored.
     */
    void unsubscribe(IObserver& observer);

    /**
     * @brief Sends an event to all registered observers, in subscription order.
     * @param event The event to publish.
     */
    void publish(const Event& event) const;

private:
    /** @brief Registered observers (not owned). */
    std::vector<IObserver*> observers_;
};

}  // namespace vsm
