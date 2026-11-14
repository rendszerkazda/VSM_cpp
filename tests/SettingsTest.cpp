#include <gtest/gtest.h>

#include "vsm/Settings.h"

TEST(SettingsTest, AcceptsNormalWorldNames) {
    EXPECT_TRUE(vsm::isValidName("Arena19"));
    EXPECT_TRUE(vsm::isValidName("my_world-2"));
    EXPECT_TRUE(vsm::isValidName("Dedicated World"));
}

TEST(SettingsTest, RejectsEmptyName) {
    EXPECT_FALSE(vsm::isValidName(""));
}

TEST(SettingsTest, RejectsPathTraversalAndSeparators) {
    EXPECT_FALSE(vsm::isValidName("../secret"));
    EXPECT_FALSE(vsm::isValidName("a/b"));
    EXPECT_FALSE(vsm::isValidName("a\\b"));
}

TEST(SettingsTest, RejectsShellMetaCharacters) {
    EXPECT_FALSE(vsm::isValidName("world'; rm -rf"));
    EXPECT_FALSE(vsm::isValidName("a\"b"));
    EXPECT_FALSE(vsm::isValidName("a$b"));
}
