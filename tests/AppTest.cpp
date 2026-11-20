#include <gtest/gtest.h>

#include <iostream>
#include <sstream>

#include "TempDir.h"
#include "vsm/App.h"
#include "vsm/CliArgs.h"
#include "vsm/FileUtil.h"
#include "vsm/SaveManagerFacade.h"
#include "vsm/GzipCompressor.h"
#include "vsm/LocalFolderStorage.h"

using vsm::Bytes;

// ------------------------------------------------------------- argument parsing

TEST(CliArgsTest, NoArgumentsMeansHelp) {
    EXPECT_EQ(vsm::parseArgs({}).command, vsm::CliCommand::Help);
}

TEST(CliArgsTest, ParsesCommandAndOptions) {
    const auto args = vsm::parseArgs({"upload", "--save-dir", "/s", "--local-storage", "/c",
                                      "--user", "adri", "--world", "W", "--config", "x.json"});
    EXPECT_EQ(args.command, vsm::CliCommand::Upload);
    EXPECT_EQ(args.saveDir, "/s");
    EXPECT_EQ(args.localStorage, "/c");
    EXPECT_EQ(args.user, "adri");
    EXPECT_EQ(args.world, "W");
    EXPECT_EQ(args.configPath, "x.json");
    EXPECT_TRUE(args.configGiven);
}

TEST(CliArgsTest, RejectsUnknownCommandOptionAndMissingValue) {
    EXPECT_THROW(vsm::parseArgs({"explode"}), std::invalid_argument);
    EXPECT_THROW(vsm::parseArgs({"upload", "--nope", "x"}), std::invalid_argument);
    EXPECT_THROW(vsm::parseArgs({"upload", "--world"}), std::invalid_argument);
}

// ----------------------------------------------------------------- facade

TEST(SaveManagerFacadeTest, UploadStatusDownloadUndoFlow) {
    TempDir saves, cloud;
    vsm::LocalFolderStorage storage(cloud.path());
    vsm::GzipCompressor gzip;
    vsm::EventBus bus;
    vsm::ActivityLog log(storage);
    vsm::ActivityLogObserver logObserver(log, std::cerr);
    bus.subscribe(logObserver);
    vsm::SaveManagerFacade facade(
        vsm::CommandContext{storage, gzip, bus, vsm::Settings{"adri", "Arena19"}, saves.path()}, log);

    EXPECT_NE(facade.status().find("No recorded activity"), std::string::npos);

    vsm::writeFile(saves.path() / "Arena19.db", {1, 2, 3});
    facade.upload();
    EXPECT_NE(facade.status().find("adri (uploaded)"), std::string::npos);

    vsm::writeFile(saves.path() / "Arena19.db", {4, 4});
    facade.download();
    EXPECT_EQ(vsm::readFile(saves.path() / "Arena19.db"), (Bytes{1, 2, 3}));
    EXPECT_NE(facade.status().find("(downloaded)"), std::string::npos);

    EXPECT_TRUE(facade.undoLast());
    EXPECT_EQ(vsm::readFile(saves.path() / "Arena19.db"), (Bytes{4, 4}));
}

// ----------------------------------------------------------- whole application

namespace {

int run(const std::vector<std::string>& args, std::string* outText = nullptr,
        std::string* errText = nullptr) {
    std::ostringstream out, err;
    const int code = vsm::runMain(args, out, err);
    if (outText) *outText = out.str();
    if (errText) *errText = err.str();
    return code;
}

}  // namespace

TEST(AppTest, HelpPrintsUsage) {
    std::string out;
    EXPECT_EQ(run({"help"}, &out), 0);
    EXPECT_NE(out.find("Usage: vsm"), std::string::npos);
}

TEST(AppTest, UnknownCommandFails) {
    std::string err;
    EXPECT_EQ(run({"explode"}, nullptr, &err), 1);
    EXPECT_NE(err.find("Unknown command"), std::string::npos);
}

TEST(AppTest, UploadThenDownloadOnAnotherMachineWithLocalStorage) {
    TempDir cloud, saves1, saves2;
    vsm::writeFile(saves1.path() / "Arena19.db", {10, 20, 30});
    vsm::writeFile(saves1.path() / "Arena19.fwl", {1});

    const std::string c = cloud.path().string();
    EXPECT_EQ(run({"upload", "--local-storage", c, "--save-dir", saves1.path().string(),
                   "--user", "adri", "--world", "Arena19"}), 0);

    std::string out;
    EXPECT_EQ(run({"download", "--local-storage", c, "--save-dir", saves2.path().string(),
                   "--user", "bela", "--world", "Arena19"}, &out), 0);
    EXPECT_EQ(vsm::readFile(saves2.path() / "Arena19.db"), (Bytes{10, 20, 30}));

    EXPECT_EQ(run({"status", "--local-storage", c, "--world", "Arena19"}, &out), 0);
    EXPECT_NE(out.find("bela (downloaded)"), std::string::npos);
}

TEST(AppTest, UploadWithoutUserFails) {
    TempDir cloud, saves;
    std::string err;
    EXPECT_EQ(run({"upload", "--local-storage", cloud.path().string(), "--save-dir",
                   saves.path().string(), "--config", "/definitely/missing.json"}, nullptr, &err), 1);
    EXPECT_FALSE(err.empty());
}

TEST(AppTest, UploadOfMissingWorldReportsError) {
    TempDir cloud, saves;
    std::string err;
    EXPECT_EQ(run({"upload", "--local-storage", cloud.path().string(), "--save-dir",
                   saves.path().string(), "--user", "adri", "--world", "Nope"}, nullptr, &err), 1);
    EXPECT_NE(err.find("No save files"), std::string::npos);
}
