#include "wui/savegamedeleter.h"
#include "base/i18n.h"
#include "base/log.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
#include "ui_basic/messagebox.h"

#include <boost/format.hpp>
#include <libintl.h>

SavegameDeleter::SavegameDeleter(UI::Panel* parent) : parent_(parent) {
}

bool SavegameDeleter::delete_savegames(const std::vector<SavegameData>& to_be_deleted) const {
	bool do_delete = SDL_GetModState() & KMOD_CTRL;
	if (!do_delete) {
		do_delete = show_confirmation_window(to_be_deleted);
	}
	if (do_delete) {
		delete_and_count_failures(to_be_deleted);
	}
	return do_delete;
}

bool SavegameDeleter::show_confirmation_window(const std::vector<SavegameData>& selections) const {
	size_t no_selections = selections.size();
	std::string confirmation_window_header = create_header_for_confirmation_window(no_selections);
	const std::string message =
	   (boost::format("%s\n%s") % confirmation_window_header % as_filename_list(selections)).str();

	UI::WLMessageBox confirmationBox(
	   parent_->get_parent()->get_parent(),
	   no_selections == 1 ? _("Confirm Deleting File") : _("Confirm Deleting Files"), message,
	   UI::WLMessageBox::MBoxType::kOkCancel);
	return confirmationBox.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk;
}

const std::string
SavegameDeleter::create_header_for_confirmation_window(const size_t no_selections) const {
	std::string header = "";
	//	if (filetype_ == FileType::kReplay) {
	if (false) {
		header =
		   no_selections == 1 ?
		      _("Do you really want to delete this replay?") :
		      /** TRANSLATORS: Used with multiple replays, 1 replay has a
		                      separate string. DO NOT omit the placeholder in your translation. */
		      (boost::format(ngettext("Do you really want to delete this %d replay?",
		                              "Do you really want to delete these %d replays?",
		                              no_selections)) %
		       no_selections)
		         .str();
	} else {
		header = no_selections == 1 ? _("Do you really want to delete this game?") :
		                              /** TRANSLATORS: Used with multiple games, 1 game has a separate
		                                 string. DO NOT omit the placeholder in your translation. */
		            (boost::format(ngettext("Do you really want to delete this %d game?",
		                                    "Do you really want to delete these %d games?",
		                                    no_selections)) %
		             no_selections)
		               .str();
	}
	return header;
}

void SavegameDeleter::delete_and_count_failures(
   const std::vector<SavegameData>& to_be_deleted) const {
	uint32_t number_of_failed_deletes = try_to_delete(to_be_deleted);
	if (number_of_failed_deletes > 0) {
		notify_deletion_failed(to_be_deleted.size(), number_of_failed_deletes);
	}
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

void SavegameDeleter::notify_deletion_failed(const uint32_t no_to_be_deleted,
                                             const uint32_t no_failed) const {

	// Notify the player.
	const std::string caption =
	   (no_failed == 1) ? _("Error Deleting File!") : _("Error Deleting Files!");
	std::string header = "";
	//	if (filetype_ == FileType::kReplay) {
	if (false) {
		if (no_to_be_deleted == 1) {
			header = _("The replay could not be deleted.");
		} else {
			header = (boost::format(ngettext("%d replay could not be deleted.",
			                                 "%d replays could not be deleted.", no_failed)) %
			          no_failed)
			            .str();
		}
	} else {
		if (no_to_be_deleted == 1) {
			header = _("The game could not be deleted.");
		} else {
			header = (boost::format(ngettext("%d game could not be deleted.",
			                                 "%d games could not be deleted.", no_failed)) %
			          no_failed)
			            .str();
		}
	}
	std::string message =
	   (boost::format("%s\n%s") % header % "filename_list_string(failed_selections)").str();
	UI::WLMessageBox msgBox(
	   parent_->get_parent()->get_parent(), caption, message, UI::WLMessageBox::MBoxType::kOk);
	msgBox.run<UI::Panel::Returncodes>();
}
