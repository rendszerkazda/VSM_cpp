#pragma once

/**
 * @file App.h
 * @brief The composition root: wires all parts together and runs a command.
 */

#include <ostream>
#include <vector>
#include <string>

#include "vsm/CliArgs.h"

namespace vsm {

/**
 * @brief Runs the program for the given parsed arguments.
 *
 * This is the only place that decides which concrete classes are used
 * (Supabase or local folder storage, gzip, Windows or fixed path...) and
 * injects them into the facade.
 *
 * @param args Parsed command line.
 * @param out Stream for normal output.
 * @param err Stream for errors and warnings.
 * @return Process exit code: 0 on success, 1 on failure.
 */
int runApp(const CliArgs& args, std::ostream& out, std::ostream& err);

/**
 * @brief Parses the raw arguments and runs the program.
 * @param argv The arguments without the program name.
 * @param out Stream for normal output.
 * @param err Stream for errors and warnings.
 * @return Process exit code.
 */
int runMain(const std::vector<std::string>& argv, std::ostream& out, std::ostream& err);

}  // namespace vsm
