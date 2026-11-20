#include "vsm/Commands.h"

#include <algorithm>
#include <ctime>
#include <stdexcept>
#include <utility>

#include "vsm/FileUtil.h"

namespace vsm {

namespace fs = std::filesystem;

namespace {

const char* const kBackupFolder = "vsm_backup";

fs::path localFile(const CommandContext& ctx, const std::string& ext) {
    return ctx.saveDir / (ctx.settings.worldName + ext);
}

std::string timestampForFolder() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &local);
    return buffer;
}

void requireValidSettings(const CommandContext& ctx) {
    if (!isValidName(ctx.settings.worldName)) {
        throw std::runtime_error("Invalid world name: '" + ctx.settings.worldName + "'");
    }
}

void copyOverwrite(const fs::path& from, const fs::path& to) {
    fs::create_directories(to.parent_path());
    fs::copy_file(from, to, fs::copy_options::overwrite_existing);
}

}  // namespace

std::string remoteName(const std::string& world, const std::string& extension) {
    return world + "_" + extension.substr(1) + ".gz";
}

const std::vector<std::string>& saveExtensions() {
    static const std::vector<std::string> extensions = {".db", ".fwl"};
    return extensions;
}

// ------------------------------------------------------------------ Upload

UploadCommand::UploadCommand(CommandContext context) : ctx_(std::move(context)) {}

void UploadCommand::execute() {
    requireValidSettings(ctx_);

    int uploaded = 0;
    for (const auto& ext : saveExtensions()) {
        const fs::path path = localFile(ctx_, ext);
        if (!fs::exists(path)) {
            continue;
        }
        const Bytes raw = readFile(path);
        ctx_.storage.put(remoteName(ctx_.settings.worldName, ext), ctx_.compressor.compress(raw));
        ++uploaded;
    }
    if (uploaded == 0) {
        throw std::runtime_error("No save files for world '" + ctx_.settings.worldName +
                                 "' found in " + ctx_.saveDir.string());
    }
    ctx_.bus.publish({EventType::Uploaded, ctx_.settings.worldName, ctx_.settings.username,
                      "Uploaded " + std::to_string(uploaded) + " file(s)."});
}

bool UploadCommand::undo() {
    return false;
}

// ---------------------------------------------------------------- Download

DownloadCommand::DownloadCommand(CommandContext context) : ctx_(std::move(context)) {}

void DownloadCommand::execute() {
    requireValidSettings(ctx_);

    // 1. Fetch and decompress everything first, so a failure changes nothing locally.
    std::map<std::string, Bytes> downloaded;
    for (const auto& ext : saveExtensions()) {
        const auto data = ctx_.storage.get(remoteName(ctx_.settings.worldName, ext));
        if (data) {
            downloaded[ext] = ctx_.compressor.decompress(*data);
        }
    }
    if (downloaded.empty()) {
        throw std::runtime_error("World '" + ctx_.settings.worldName + "' was not found in the storage.");
    }

    // 2. Back up the local files that are about to be overwritten.
    backupDir_ = ctx_.saveDir / kBackupFolder / timestampForFolder();
    for (int suffix = 1; fs::exists(backupDir_); ++suffix) {
        backupDir_ = ctx_.saveDir / kBackupFolder / (timestampForFolder() + "_" + std::to_string(suffix));
    }
    backedUp_.clear();
    created_.clear();
    for (const auto& item : downloaded) {
        const fs::path local = localFile(ctx_, item.first);
        if (fs::exists(local)) {
            copyOverwrite(local, backupDir_ / local.filename());
            backedUp_.push_back(item.first);
        } else {
            created_.push_back(item.first);
        }
    }
    if (!backedUp_.empty()) {
        ctx_.bus.publish({EventType::BackupCreated, ctx_.settings.worldName, ctx_.settings.username,
                          "Backup of local files saved to " + backupDir_.string()});
    }

    // 3. Replace the local files.
    for (const auto& item : downloaded) {
        writeFile(localFile(ctx_, item.first), item.second);
    }
    executed_ = true;
    ctx_.bus.publish({EventType::Downloaded, ctx_.settings.worldName, ctx_.settings.username,
                      "Downloaded " + std::to_string(downloaded.size()) + " file(s)."});
}

bool DownloadCommand::undo() {
    if (!executed_) {
        return false;
    }
    for (const auto& ext : backedUp_) {
        const fs::path local = localFile(ctx_, ext);
        copyOverwrite(backupDir_ / local.filename(), local);
    }
    for (const auto& ext : created_) {
        std::error_code ec;
        fs::remove(localFile(ctx_, ext), ec);
    }
    executed_ = false;
    ctx_.bus.publish({EventType::Restored, ctx_.settings.worldName, ctx_.settings.username,
                      "Download undone, previous local files restored."});
    return true;
}

// ----------------------------------------------------------------- Restore

RestoreLatestBackupCommand::RestoreLatestBackupCommand(CommandContext context)
    : ctx_(std::move(context)) {}

void RestoreLatestBackupCommand::execute() {
    requireValidSettings(ctx_);

    const fs::path root = ctx_.saveDir / kBackupFolder;
    std::vector<fs::path> backups;
    if (fs::exists(root)) {
        for (const auto& entry : fs::directory_iterator(root)) {
            if (entry.is_directory()) {
                backups.push_back(entry.path());
            }
        }
    }
    std::sort(backups.begin(), backups.end());

    // Newest backup (folder names sort chronologically) that contains this world.
    for (auto it = backups.rbegin(); it != backups.rend(); ++it) {
        int restored = 0;
        for (const auto& ext : saveExtensions()) {
            const fs::path file = *it / (ctx_.settings.worldName + ext);
            if (fs::exists(file)) {
                copyOverwrite(file, localFile(ctx_, ext));
                ++restored;
            }
        }
        if (restored > 0) {
            ctx_.bus.publish({EventType::Restored, ctx_.settings.worldName, ctx_.settings.username,
                              "Restored " + std::to_string(restored) + " file(s) from " +
                                  it->string()});
            return;
        }
    }
    throw std::runtime_error("No backup found for world '" + ctx_.settings.worldName + "'.");
}

bool RestoreLatestBackupCommand::undo() {
    return false;
}

}  // namespace vsm
