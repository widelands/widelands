#include "savegamedeleter.h"
#include "base/log.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
SavegameDeleter::SavegameDeleter() {
}

uint32_t SavegameDeleter::try_to_delete(const std::vector<SavegameData>& to_be_deleted) const {
	// Failed deletions aren't a serious problem, we just catch the errors
	// and keep track to notify the player.
	uint32_t failed_deletions = 0;
	bool failed;
	for (const auto& delete_me : to_be_deleted) {
		failed = false;
		const std::string& file_to_be_deleted = delete_me.filename;
		try {
			g_fs->fs_unlink(file_to_be_deleted);
		} catch (const FileError& e) {
			log("player-requested file deletion failed: %s", e.what());
			failed = true;
		}
		//		if (filetype_ == FileType::kReplay) {
		//			try {
		//				g_fs->fs_unlink(file_to_be_deleted + kSavegameExtension);
		//				// If at least one of the two relevant files of a replay are
		//				// successfully deleted then count it as success.
		//				// (From the player perspective the replay is gone.)
		//				failed = false;
		//				// If it was a multiplayer replay, also delete the synchstream. Do
		//				// it here, so it's only attempted if replay deletion was successful.
		//				if (g_fs->file_exists(file_to_be_deleted + kSyncstreamExtension)) {
		//					g_fs->fs_unlink(file_to_be_deleted + kSyncstreamExtension);
		//				}
		//			} catch (const FileError& e) {
		//				log("player-requested file deletion failed: %s", e.what());
		//			}
		//		}
		if (failed) {
			++failed_deletions;
		}
	}
	return failed_deletions;
}
