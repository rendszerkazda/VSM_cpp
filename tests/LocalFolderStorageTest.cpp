#include <gtest/gtest.h>

#include "TempDir.h"
#include "vsm/LocalFolderStorage.h"

using vsm::Bytes;

TEST(LocalFolderStorageTest, PutThenGetReturnsSameBytes) {
    TempDir dir;
    vsm::LocalFolderStorage storage(dir.path());
    const Bytes data = {1, 2, 3, 0, 255};

    storage.put("Arena19_db.gz", data);

    const auto result = storage.get("Arena19_db.gz");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(*result, data);
}

TEST(LocalFolderStorageTest, GetOfMissingObjectReturnsNullopt) {
    TempDir dir;
    vsm::LocalFolderStorage storage(dir.path());
    EXPECT_FALSE(storage.get("nothing.gz").has_value());
}

TEST(LocalFolderStorageTest, PutOverwritesExistingObject) {
    TempDir dir;
    vsm::LocalFolderStorage storage(dir.path());
    storage.put("a.gz", {1});
    storage.put("a.gz", {2, 2});
    EXPECT_EQ(*storage.get("a.gz"), (Bytes{2, 2}));
}

TEST(LocalFolderStorageTest, CreatesMissingRootDirectory) {
    TempDir dir;
    vsm::LocalFolderStorage storage(dir.path() / "nested" / "root");
    storage.put("a.gz", {7});
    EXPECT_TRUE(storage.get("a.gz").has_value());
}

TEST(LocalFolderStorageTest, RejectsUnsafeNames) {
    TempDir dir;
    vsm::LocalFolderStorage storage(dir.path());
    EXPECT_THROW(storage.put("../evil", {1}), std::runtime_error);
    EXPECT_THROW(storage.get("a/b"), std::runtime_error);
}
