#include <gtest/gtest.h>

#include <sstream>

#include "TempDir.h"
#include "vsm/ActivityLog.h"
#include "vsm/Events.h"
#include "vsm/LocalFolderStorage.h"

namespace {

class RecordingObserver : public vsm::IObserver {
public:
    std::vector<vsm::Event> events;
    void onEvent(const vsm::Event& event) override { events.push_back(event); }
};

class FailingStorage : public vsm::IStorage {
public:
    void put(const std::string&, const vsm::Bytes&) override { throw std::runtime_error("boom"); }
    std::optional<vsm::Bytes> get(const std::string&) override { return std::nullopt; }
};

vsm::Event makeEvent(vsm::EventType type) {
    return vsm::Event{type, "Arena19", "adri", "something happened"};
}

}  // namespace

TEST(EventBusTest, SubscribedObserversReceiveEventsInOrder) {
    vsm::EventBus bus;
    RecordingObserver a, b;
    bus.subscribe(a);
    bus.subscribe(b);

    bus.publish(makeEvent(vsm::EventType::Uploaded));

    ASSERT_EQ(a.events.size(), 1u);
    ASSERT_EQ(b.events.size(), 1u);
    EXPECT_EQ(a.events[0].user, "adri");
}

TEST(EventBusTest, UnsubscribedObserverGetsNothing) {
    vsm::EventBus bus;
    RecordingObserver a;
    bus.subscribe(a);
    bus.unsubscribe(a);
    bus.publish(makeEvent(vsm::EventType::Uploaded));
    EXPECT_TRUE(a.events.empty());
}

TEST(EventBusTest, SubscribingTwiceDeliversOnce) {
    vsm::EventBus bus;
    RecordingObserver a;
    bus.subscribe(a);
    bus.subscribe(a);
    bus.publish(makeEvent(vsm::EventType::Downloaded));
    EXPECT_EQ(a.events.size(), 1u);
}

TEST(ActivityLogTest, AppendAndReadLastEntryOfWorld) {
    TempDir dir;
    vsm::LocalFolderStorage storage(dir.path());
    int tick = 0;
    vsm::ActivityLog log(storage, [&tick] { return "t" + std::to_string(++tick); });

    log.append("adri", "upload", "Arena19");
    log.append("bela", "download", "Arena19");
    log.append("cili", "upload", "Other");

    const auto last = log.lastFor("Arena19");
    ASSERT_TRUE(last.has_value());
    EXPECT_EQ(last->username, "bela");
    EXPECT_EQ(last->action, "download");
    EXPECT_EQ(last->time, "t2");
    EXPECT_FALSE(log.lastFor("Unknown").has_value());
}

TEST(ActivityLogTest, CorruptLogIsTreatedAsEmpty) {
    TempDir dir;
    vsm::LocalFolderStorage storage(dir.path());
    storage.put("log.json", {'n', 'o', 't', ' ', 'j', 's', 'o', 'n'});
    vsm::ActivityLog log(storage);
    EXPECT_FALSE(log.lastFor("Arena19").has_value());
    log.append("adri", "upload", "Arena19");
    EXPECT_TRUE(log.lastFor("Arena19").has_value());
}

TEST(ActivityLogObserverTest, RecordsUploadsAndDownloadsOnly) {
    TempDir dir;
    vsm::LocalFolderStorage storage(dir.path());
    vsm::ActivityLog log(storage);
    std::ostringstream errors;
    vsm::ActivityLogObserver observer(log, errors);

    observer.onEvent(makeEvent(vsm::EventType::BackupCreated));
    EXPECT_FALSE(log.lastFor("Arena19").has_value());

    observer.onEvent(makeEvent(vsm::EventType::Uploaded));
    ASSERT_TRUE(log.lastFor("Arena19").has_value());
    EXPECT_EQ(log.lastFor("Arena19")->action, "upload");
}

TEST(ActivityLogObserverTest, FailingLogDoesNotThrow) {
    FailingStorage storage;
    vsm::ActivityLog log(storage);
    std::ostringstream errors;
    vsm::ActivityLogObserver observer(log, errors);

    EXPECT_NO_THROW(observer.onEvent(makeEvent(vsm::EventType::Uploaded)));
    EXPECT_NE(errors.str().find("boom"), std::string::npos);
}

TEST(ConsoleObserverTest, PrintsWorldAndMessage) {
    std::ostringstream out;
    vsm::ConsoleObserver observer(out);
    observer.onEvent(makeEvent(vsm::EventType::Uploaded));
    EXPECT_EQ(out.str(), "[Arena19] something happened\n");
}
