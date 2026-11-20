#pragma once

/**
 * @file CliArgs.h
 * @brief Command line argument parsing.
 */

#include <string>
#include <vector>

namespace vsm {

/** @brief The sub-commands of the program. */
enum class CliCommand { Help, Upload, Download, Status, Restore };

/** @brief The parsed command line. */
struct CliArgs {
    CliCommand command = CliCommand::Help;  ///< What to do.
    std::string configPath = "config.json"; ///< --config
    bool configGiven = false;               ///< True if --config was passed explicitly.
    std::string saveDir;                    ///< --save-dir (overrides the Windows default)
    std::string localStorage;               ///< --local-storage (use a folder instead of Supabase)
    std::string user;                       ///< --user (overrides config.json)
    std::string world;                      ///< --world (overrides config.json)
};

/**
 * @brief Parses the command line.
 *
 * @param args The arguments without the program name.
 * @return The parsed arguments.
 * @throws std::invalid_argument on unknown commands/options or missing values.
 */
CliArgs parseArgs(const std::vector<std::string>& args);

/** @brief Returns the usage/help text. */
std::string usageText();

}  // namespace vsm
