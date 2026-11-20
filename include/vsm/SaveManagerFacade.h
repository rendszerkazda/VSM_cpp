#pragma once

/**
 * @file SaveManagerFacade.h
 * @brief One simple entry point to the whole save manager (Facade pattern).
 */

#include <memory>
#include <string>

#include "vsm/ActivityLog.h"
#include "vsm/Commands.h"

namespace vsm {

/**
 * @brief Hides commands, compression, storage and logging behind five methods.
 *
 * The command line front end only talks to this class and does not need to
 * know how a download is composed of fetching, decompressing, backing up and
 * logging.
 */
class SaveManagerFacade {
public:
    /**
     * @brief Creates the facade.
     * @param context Shared dependencies (storage, compressor, event bus, ...).
     * @param log Activity log used by status().
     */
    SaveManagerFacade(CommandContext context, ActivityLog& log);

    /** @brief Uploads the local world to the storage. */
    void upload();

    /** @brief Downloads the world, backing up the local files first. */
    void download();

    /** @brief Restores the newest local backup. */
    void restoreBackup();

    /**
     * @brief Undoes the last command run by this facade, if it supports undo.
     * @return true if something was undone.
     */
    bool undoLast();

    /**
     * @brief Describes the last recorded activity of the world.
     * @return A one-line, human readable status.
     */
    std::string status();

private:
    /** @brief Runs a command and remembers it for undoLast(). */
    void run(std::unique_ptr<ICommand> command);

    CommandContext ctx_;
    ActivityLog& log_;
    std::unique_ptr<ICommand> last_;
};

}  // namespace vsm
