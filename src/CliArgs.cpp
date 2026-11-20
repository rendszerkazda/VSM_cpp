#include "vsm/CliArgs.h"

#include <stdexcept>

namespace vsm {

CliArgs parseArgs(const std::vector<std::string>& args) {
    CliArgs result;
    if (args.empty()) {
        return result;  // Help
    }

    const std::string& cmd = args[0];
    if (cmd == "upload") {
        result.command = CliCommand::Upload;
    } else if (cmd == "download") {
        result.command = CliCommand::Download;
    } else if (cmd == "status") {
        result.command = CliCommand::Status;
    } else if (cmd == "restore") {
        result.command = CliCommand::Restore;
    } else if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        return result;
    } else {
        throw std::invalid_argument("Unknown command: " + cmd);
    }

    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string& option = args[i];
        std::string* target = nullptr;
        if (option == "--config") {
            target = &result.configPath;
            result.configGiven = true;
        } else if (option == "--save-dir") {
            target = &result.saveDir;
        } else if (option == "--local-storage") {
            target = &result.localStorage;
        } else if (option == "--user") {
            target = &result.user;
        } else if (option == "--world") {
            target = &result.world;
        } else if (option == "--help" || option == "-h") {
            result.command = CliCommand::Help;
            return result;
        } else {
            throw std::invalid_argument("Unknown option: " + option);
        }
        if (i + 1 >= args.size()) {
            throw std::invalid_argument("Option " + option + " needs a value");
        }
        *target = args[++i];
    }
    return result;
}

std::string usageText() {
    return
        "Valheim Save Manager (vsm)\n"
        "\n"
        "Usage: vsm <command> [options]\n"
        "\n"
        "Commands:\n"
        "  upload     Compress the local world and upload it\n"
        "  download   Download the world (local files are backed up first)\n"
        "  status     Show who touched the world last\n"
        "  restore    Restore the newest local backup\n"
        "  help       Show this text\n"
        "\n"
        "Options:\n"
        "  --config <file>        Config file (default: config.json)\n"
        "  --save-dir <dir>       Valheim worlds folder (default: the Windows location)\n"
        "  --local-storage <dir>  Use a folder instead of Supabase as the storage\n"
        "  --user <name>          Player name (overrides config.json)\n"
        "  --world <name>         World name (overrides config.json)\n"
        "\n"
        "The Supabase key can be given in the VSM_SUPABASE_KEY environment variable.\n";
}

}  // namespace vsm
