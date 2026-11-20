#include <gtest/gtest.h>

#include <map>

#include "TempDir.h"
#include "vsm/Commands.h"
#include "vsm/FileUtil.h"
#include "vsm/GzipCompressor.h"
#include "vsm/LocalFolderStorage.h"

namespace {

class RecordingObserver : public vsm::IObserver {
public:
    std::vector<vsm::EventType> types;
    void onEvent(const vsm::Event& event) override { types.push_back(event.type); }
};

/** Everything a command test needs, wired with a local folder as the "cloud". */
struct Fixture {
    TempDir saves;
    TempDir cloud;
    vsm::LocalFolderStorage storage{cloud.path()};
    vsm::GzipCompressor gzip;
    vsm::EventBus bus;
    RecordingObserver observer;

    Fixture() { bus.subscribe(observer); }

    vsm::CommandContext context() {
        return vsm::CommandContext{storage, gzip, bus, vsm::Settings{"adri", "Arena19"}, saves.path()};
    }
    void writeLocal(const std::string& file, const vsm::Bytes& data) {
        vsm::writeFile(saves.path() / file, data);
    }
    vsm::Bytes readLocal(const std::string& file) { return vsm::readFile(saves.path() / file); }
    bool localExists(const std::string& file) { return std::filesystem::exists(saves.path() / file); }
};

}  // namespace

TEST(UploadCommandTest, UploadsCompressedFilesAndPublishesEvent) {
    Fixture f;
    f.writeLocal("Arena19.db", {1, 1, 1, 1});
    f.writeLocal("Arena19.fwl", {2, 2});

    vsm::UploadCommand cmd(f.context());
    cmd.execute();

    ASSERT_TRUE(f.storage.get("Arena19_db.gz").has_value());
    ASSERT_TRUE(f.storage.get("Arena19_fwl.gz").has_value());
    EXPECT_EQ(f.gzip.decompress(*f.storage.get("Arena19_db.gz")), (vsm::Bytes{1, 1, 1, 1}));
    EXPECT_EQ(f.observer.types, (std::vector<vsm::EventType>{vsm::EventType::Uploaded}));
}

TEST(UploadCommandTest, ThrowsWhenNoLocalFilesExist) {
    Fixture f;
    vsm::UploadCommand cmd(f.context());
    EXPECT_THROW(cmd.execute(), std::runtime_error);
    EXPECT_TRUE(f.observer.types.empty());
}

TEST(UploadCommandTest, UndoIsNotSupported) {
    Fixture f;
    vsm::UploadCommand cmd(f.context());
    EXPECT_FALSE(cmd.undo());
}

TEST(UploadCommandTest, RejectsUnsafeWorldName) {
    Fixture f;
    auto ctx = f.context();
    ctx.settings.worldName = "../evil";
    EXPECT_THROW(vsm::UploadCommand(ctx).execute(), std::runtime_error);
}

TEST(DownloadCommandTest, ReplacesLocalFilesAndBacksUpOldOnes) {
    Fixture f;
    f.storage.put("Arena19_db.gz", f.gzip.compress({9, 9, 9}));
    f.writeLocal("Arena19.db", {1, 2, 3});

    vsm::DownloadCommand cmd(f.context());
    cmd.execute();

    EXPECT_EQ(f.readLocal("Arena19.db"), (vsm::Bytes{9, 9, 9}));
    EXPECT_TRUE(std::filesystem::exists(f.saves.path() / "vsm_backup"));
    EXPECT_EQ(f.observer.types, (std::vector<vsm::EventType>{vsm::EventType::BackupCreated,
                                                             vsm::EventType::Downloaded}));
}

TEST(DownloadCommandTest, UndoRestoresPreviousAndRemovesNewFiles) {
    Fixture f;
    f.storage.put("Arena19_db.gz", f.gzip.compress({9, 9, 9}));
    f.storage.put("Arena19_fwl.gz", f.gzip.compress({7}));
    f.writeLocal("Arena19.db", {1, 2, 3});  // .fwl does not exist locally

    vsm::DownloadCommand cmd(f.context());
    cmd.execute();
    ASSERT_TRUE(f.localExists("Arena19.fwl"));

    EXPECT_TRUE(cmd.undo());

    EXPECT_EQ(f.readLocal("Arena19.db"), (vsm::Bytes{1, 2, 3}));
    EXPECT_FALSE(f.localExists("Arena19.fwl"));
    EXPECT_FALSE(cmd.undo());  // second undo does nothing
}

TEST(DownloadCommandTest, UndoBeforeExecuteReturnsFalse) {
    Fixture f;
    vsm::DownloadCommand cmd(f.context());
    EXPECT_FALSE(cmd.undo());
}

TEST(DownloadCommandTest, MissingWorldThrowsAndKeepsLocalFiles) {
    Fixture f;
    f.writeLocal("Arena19.db", {1, 2, 3});
    vsm::DownloadCommand cmd(f.context());
    EXPECT_THROW(cmd.execute(), std::runtime_error);
    EXPECT_EQ(f.readLocal("Arena19.db"), (vsm::Bytes{1, 2, 3}));
}

TEST(DownloadCommandTest, CorruptDataThrowsAndKeepsLocalFiles) {
    Fixture f;
    f.storage.put("Arena19_db.gz", {1, 2, 3});
    f.writeLocal("Arena19.db", {5});
    vsm::DownloadCommand cmd(f.context());
    EXPECT_THROW(cmd.execute(), std::runtime_error);
    EXPECT_EQ(f.readLocal("Arena19.db"), (vsm::Bytes{5}));
}

TEST(RestoreLatestBackupCommandTest, RestoresFilesFromNewestBackup) {
    Fixture f;
    f.storage.put("Arena19_db.gz", f.gzip.compress({9}));
    f.writeLocal("Arena19.db", {1});
    vsm::DownloadCommand(f.context()).execute();
    ASSERT_EQ(f.readLocal("Arena19.db"), (vsm::Bytes{9}));

    vsm::RestoreLatestBackupCommand restore(f.context());
    restore.execute();

    EXPECT_EQ(f.readLocal("Arena19.db"), (vsm::Bytes{1}));
    EXPECT_EQ(f.observer.types.back(), vsm::EventType::Restored);
}

TEST(RestoreLatestBackupCommandTest, ThrowsWithoutAnyBackup) {
    Fixture f;
    vsm::RestoreLatestBackupCommand restore(f.context());
    EXPECT_THROW(restore.execute(), std::runtime_error);
}

TEST(RemoteNameTest, MatchesPythonVersionNaming) {
    EXPECT_EQ(vsm::remoteName("Arena19", ".db"), "Arena19_db.gz");
    EXPECT_EQ(vsm::remoteName("Arena19", ".fwl"), "Arena19_fwl.gz");
}
