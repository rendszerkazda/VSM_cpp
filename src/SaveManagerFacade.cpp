#include "vsm/SaveManagerFacade.h"

#include <exception>
#include <utility>

namespace vsm {

SaveManagerFacade::SaveManagerFacade(CommandContext context, ActivityLog& log)
    : ctx_(std::move(context)), log_(log) {}

void SaveManagerFacade::run(std::unique_ptr<ICommand> command) {
    command->execute();
    last_ = std::move(command);
}

void SaveManagerFacade::upload() {
    run(std::make_unique<UploadCommand>(ctx_));
}

void SaveManagerFacade::download() {
    run(std::make_unique<DownloadCommand>(ctx_));
}

void SaveManagerFacade::restoreBackup() {
    run(std::make_unique<RestoreLatestBackupCommand>(ctx_));
}

bool SaveManagerFacade::undoLast() {
    return last_ && last_->undo();
}

std::string SaveManagerFacade::status() {
    try {
        const auto last = log_.lastFor(ctx_.settings.worldName);
        if (!last) {
            return "No recorded activity for world '" + ctx_.settings.worldName + "'.";
        }
        return "Last activity: " + last->username + " (" +
               (last->action == "upload" ? "uploaded" : "downloaded") + ") - " + last->time;
    } catch (const std::exception& e) {
        return std::string("Could not read the activity log: ") + e.what();
    }
}

}  // namespace vsm
