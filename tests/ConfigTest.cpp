#include <gtest/gtest.h>

#include <cstdlib>

#include "vsm/Config.h"
#include "vsm/PathProvider.h"

namespace {

void setEnv(const char* name, const char* value) {
#ifdef _WIN32
    _putenv_s(name, value);
#else
    ::setenv(name, value, 1);
#endif
}

void unsetEnv(const char* name) {
#ifdef _WIN32
    _putenv_s(name, "");
#else
    ::unsetenv(name);
#endif
}

}  // namespace

TEST(ConfigTest, InstanceAlwaysReturnsTheSameObject) {
    EXPECT_EQ(&vsm::Config::instance(), &vsm::Config::instance());
}

TEST(ConfigTest, LoadsValuesAndDerivesUrlFromProjectId) {
    auto& cfg = vsm::Config::instance();
    cfg.loadFromText(R"({"username":"adri","world_name":"Arena19",
                         "supabase_project_id":"abc123","supabase_key":"k"})");
    EXPECT_EQ(cfg.settings().username, "adri");
    EXPECT_EQ(cfg.settings().worldName, "Arena19");
    EXPECT_EQ(cfg.supabaseUrl(), "https://abc123.supabase.co");
    EXPECT_EQ(cfg.supabaseKey(), "k");
    EXPECT_EQ(cfg.bucket(), "valheim-saves");
    cfg.reset();
}

TEST(ConfigTest, ExplicitUrlAndBucketWin) {
    auto& cfg = vsm::Config::instance();
    cfg.loadFromText(R"({"supabase_url":"http://localhost:1","bucket":"b","save_dir":"/x"})");
    EXPECT_EQ(cfg.supabaseUrl(), "http://localhost:1");
    EXPECT_EQ(cfg.bucket(), "b");
    EXPECT_EQ(cfg.saveDir(), "/x");
    cfg.reset();
}

TEST(ConfigTest, InvalidJsonThrows) {
    auto& cfg = vsm::Config::instance();
    EXPECT_THROW(cfg.loadFromText("{not json"), std::runtime_error);
    EXPECT_THROW(cfg.loadFromText("[1,2]"), std::runtime_error);
}

TEST(ConfigTest, MissingFileThrows) {
    EXPECT_THROW(vsm::Config::instance().loadFromFile("/no/such/config.json"), std::runtime_error);
}

TEST(ConfigTest, EnvironmentOverridesKey) {
    auto& cfg = vsm::Config::instance();
    cfg.loadFromText(R"({"supabase_key":"from_file"})");
    setEnv("VSM_SUPABASE_KEY", "from_env");
    cfg.applyEnvironment();
    EXPECT_EQ(cfg.supabaseKey(), "from_env");
    unsetEnv("VSM_SUPABASE_KEY");
    cfg.reset();
}

TEST(PathProviderTest, WindowsProviderBuildsValheimPath) {
    vsm::WindowsPathProvider provider("C:/Users/test");
    const auto dir = provider.saveDir().generic_string();
    EXPECT_EQ(dir, "C:/Users/test/AppData/LocalLow/IronGate/Valheim/worlds_local");
}

TEST(PathProviderTest, FixedProviderReturnsGivenDirectory) {
    vsm::FixedPathProvider provider("/tmp/saves");
    EXPECT_EQ(provider.saveDir(), std::filesystem::path("/tmp/saves"));
}

TEST(PathProviderTest, FactoryUsesOverrideWhenGiven) {
    auto provider = vsm::makePathProvider("/tmp/custom");
    EXPECT_EQ(provider->saveDir(), std::filesystem::path("/tmp/custom"));
}
