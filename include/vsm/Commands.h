#pragma once

/**
 * @file Commands.h
 * @brief The user actions as objects (Command pattern).
 */

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "vsm/Common.h"
#include "vsm/Events.h"
#include "vsm/ICompressor.h"
#include "vsm/IStorage.h"
#include "vsm/Settings.h"

namespace vsm {

/**
 * @brief Everything a command needs to do its work.
 *
 * All members are references or plain values that are injected from outside
 * (Dependency Injection), so commands can be tested with fake storages.
 */
struct CommandContext {
    IStorage& storage;                  ///< Where the compressed saves are kept.
    ICompressor& compressor;            ///< Compression algorithm.
    EventBus& bus;                      ///< Where progress events are published.
    Settings settings;                  ///< User and world name.
    std::filesystem::path saveDir;      ///< Local Valheim save directory.
};

/**
 * @brief Returns the storage object name of a save file.
 *
 * @param world World name, for example "Arena19".
 * @param extension File extension including the dot, ".db" or ".fwl".
 * @return For example "Arena19_db.gz" (same naming as the Python version).
 */
std::string remoteName(const std::string& world, const std::string& extension);

/** @brief The extensions of the files that make up a Valheim world. */
const std::vector<std::string>& saveExtensions();

/**
 * @brief Interface of an executable, possibly undoable action.
 *
 * Because an action is an object, it can be stored, logged and undone.
 */
class ICommand {
public:
    virtual ~ICommand() = default;

    /**
     * @brief Performs the action.
     * @throws std::runtime_error if it fails.
     */
    virtual void execute() = 0;

    /**
     * @brief Reverts the effects of execute().
     * @return true if something was undone; false if undo is not possible.
     */
    virtual bool undo() = 0;

    /** @brief Short name of the command, for messages. */
    virtual std::string name() const = 0;
};

/**
 * @brief Compresses the local world files and uploads them to the storage.
 *
 * Upload cannot be undone (the previous remote version is overwritten), so
 * undo() returns false.
 */
class UploadCommand : public ICommand {
public:
    /** @brief Creates the command. @param context The shared context. */
    explicit UploadCommand(CommandContext context);

    /** @copydoc ICommand::execute */
    void execute() override;
    /** @copydoc ICommand::undo */
    bool undo() override;
    /** @copydoc ICommand::name */
    std::string name() const override { return "upload"; }

private:
    CommandContext ctx_;
};

/**
 * @brief Downloads the world from the storage and replaces the local files.
 *
 * Existing local files are copied to `<saveDir>/vsm_backup/<timestamp>/`
 * first, so undo() (or RestoreLatestBackupCommand) can bring them back.
 */
class DownloadCommand : public ICommand {
public:
    /** @brief Creates the command. @param context The shared context. */
    explicit DownloadCommand(CommandContext context);

    /** @copydoc ICommand::execute */
    void execute() override;
    /** @copydoc ICommand::undo */
    bool undo() override;
    /** @copydoc ICommand::name */
    std::string name() const override { return "download"; }

private:
    CommandContext ctx_;
    /** @brief True between a successful execute() and undo(). */
    bool executed_ = false;
    /** @brief Directory holding the backup made by execute(). */
    std::filesystem::path backupDir_;
    /** @brief Extensions that were backed up (existed before download). */
    std::vector<std::string> backedUp_;
    /** @brief Extensions that did not exist before and were newly created. */
    std::vector<std::string> created_;
};

/**
 * @brief Restores the newest backup made by a download (works across runs).
 */
class RestoreLatestBackupCommand : public ICommand {
public:
    /** @brief Creates the command. @param context The shared context. */
    explicit RestoreLatestBackupCommand(CommandContext context);

    /** @copydoc ICommand::execute */
    void execute() override;
    /** @copydoc ICommand::undo */
    bool undo() override;
    /** @copydoc ICommand::name */
    std::string name() const override { return "restore"; }

private:
    CommandContext ctx_;
};

}  // namespace vsm
