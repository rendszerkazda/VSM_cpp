#include "vsm/App.h"

#include <exception>
#include <filesystem>
#include <memory>
#include <stdexcept>

#include "vsm/ActivityLog.h"
#include "vsm/Config.h"
#include "vsm/CurlProcessHttpClient.h"
#include "vsm/GzipCompressor.h"
#include "vsm/LocalFolderStorage.h"
#include "vsm/PathProvider.h"
#include "vsm/SaveManagerFacade.h"
#include "vsm/SupabaseStorage.h"

namespace vsm {

namespace {

/** @brief Factory: picks the storage backend requested on the command line. */
std::unique_ptr<IStorage> makeStorage(const CliArgs& args, const Config& config,
                                      IHttpClient& http) {
    if (!args.localStorage.empty()) {
        return std::make_unique<LocalFolderStorage>(args.localStorage);
    }
    return std::make_unique<SupabaseStorage>(http, config.supabaseUrl(), config.supabaseKey(),
                                             config.bucket());
}

}  // namespace

int runApp(const CliArgs& args, std::ostream& out, std::ostream& err) {
    if (args.command == CliCommand::Help) {
        out << usageText();
        return 0;
    }

    try {
        // --- configuration (the only place the Config singleton is used)
        Config& config = Config::instance();
        config.reset();
        if (std::filesystem::exists(args.configPath)) {
            config.loadFromFile(args.configPath);
        } else if (args.configGiven) {
            throw std::runtime_error("Config file not found: " + args.configPath);
        }
        config.applyEnvironment();

        Settings settings = config.settings();
        if (!args.user.empty()) {
            settings.username = args.user;
        }
        if (!args.world.empty()) {
            settings.worldName = args.world;
        }
        const bool needsUser = args.command == CliCommand::Upload || args.command == CliCommand::Download;
        if (needsUser && !isValidName(settings.username)) {
            throw std::runtime_error("A valid user name is required (config.json \"username\" or --user).");
        }
        if (!isValidName(settings.worldName)) {
            throw std::runtime_error("A valid world name is required (config.json \"world_name\" or --world).");
        }

        // --- dependencies
        CurlProcessHttpClient http;
        std::unique_ptr<IStorage> storage = makeStorage(args, config, http);
        GzipCompressor compressor;
        const std::string saveDirText = args.saveDir.empty() ? config.saveDir() : args.saveDir;
        // "status" only reads the shared log, so it does not need a local save folder.
        const std::filesystem::path saveDir =
            args.command == CliCommand::Status ? std::filesystem::path()
                                               : makePathProvider(saveDirText)->saveDir();

        EventBus bus;
        ConsoleObserver console(out);
        ActivityLog log(*storage);
        ActivityLogObserver logObserver(log, err);
        bus.subscribe(console);
        bus.subscribe(logObserver);

        SaveManagerFacade facade(CommandContext{*storage, compressor, bus, settings, saveDir}, log);

        // --- run
        switch (args.command) {
            case CliCommand::Upload:
                facade.upload();
                break;
            case CliCommand::Download:
                facade.download();
                break;
            case CliCommand::Restore:
                facade.restoreBackup();
                break;
            case CliCommand::Status:
                out << facade.status() << "\n";
                break;
            case CliCommand::Help:
                break;
        }
        return 0;
    } catch (const std::exception& e) {
        err << "Error: " << e.what() << "\n";
        return 1;
    }
}

int runMain(const std::vector<std::string>& argv, std::ostream& out, std::ostream& err) {
    try {
        return runApp(parseArgs(argv), out, err);
    } catch (const std::invalid_argument& e) {
        err << "Error: " << e.what() << "\n\n" << usageText();
        return 1;
    }
}

}  // namespace vsm
