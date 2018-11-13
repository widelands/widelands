/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/generic_save_handler.h"

#include <memory>
#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/time_string.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"

void GenericSaveHandler::clear_errors() {
	error_ = kSuccess;
	for (uint32_t errbit = 0; errbit < maxErrors; errbit++) {
		error_msg_[errbit].clear();
	}
	backup_filename_.clear();
	return;
}

void GenericSaveHandler::make_backup() {
	std::string backup_filename_base =
	   dir_ + g_fs->file_separator() + timestring() + "_" + filename_;
	backup_filename_ = backup_filename_base + kTempBackupExtension;

	// If a file with that name already exists, then try some name modifications.
	if (g_fs->file_exists(backup_filename_))
	{
		int suffix;
		for (suffix = 0; suffix <= 9; suffix++)
		{
			backup_filename_ = backup_filename_base + "-" + std::to_string(suffix)
			   + kTempBackupExtension;
			if (!g_fs->file_exists(backup_filename_)) {
			  break;
			}
		}
		if (suffix > 9) {
			error_ |= kBackupFailed;
			error_msg_[bitBackupFailed] =
			   (boost::format("GenericSaveHandler::make_backup: %s: for all "
			   "considered filenames a file already existed (last filename tried "
			   "was %s)\n") % complete_filename_.c_str() % backup_filename_).str();
			log("%s", error_msg_[bitBackupFailed].c_str());
			return;
		}
	}

	// Try to rename file.
	try {
		g_fs->fs_rename(complete_filename_, backup_filename_);
	} catch (const FileError& e) {
			error_ |= kBackupFailed;
			error_msg_[bitBackupFailed] = (boost::format("GenericSaveHandler::make_backup: file %s "
			   "could not be renamed: %s\n") % complete_filename_.c_str() % backup_filename_).str();
			log("%s", error_msg_[bitBackupFailed].c_str());
		return;
	}

	return;
}

void GenericSaveHandler::save_file() {
	// Write data to file/dir.
	try {
		std::unique_ptr<FileSystem>
		   fs(g_fs->create_sub_file_system(complete_filename_, type_));
		do_save_(*fs);
	} catch (const std::exception& e) {
		error_ |= kSavingDataFailed;
		error_msg_[bitSavingDataFailed] =
		   (boost::format("GenericSaveHandler::save_file: data could not be "
		   "written to file %s: %s\n") % complete_filename_.c_str() % e.what()
		   ).str();
		log("%s", error_msg_[bitSavingDataFailed].c_str());
	}

	if (error_ & kSavingDataFailed) {
		// Delete remnants of the failed save attempt.
		if (g_fs->file_exists(complete_filename_)) {
			try {
				g_fs->fs_unlink(complete_filename_);
			} catch (const FileError& e) {
				error_ |= kCorruptFileLeft;
				error_msg_[bitCorruptFileLeft] =
				   (boost::format("GenericSaveHandler::save_file: possibly corrupt "
				   "file %s could not be deleted: %s\n") % complete_filename_.c_str()
					 % e.what()).str();
				log("%s", error_msg_[bitCorruptFileLeft].c_str());
			}
		}
	}
	return;
}

uint32_t GenericSaveHandler::save() {
	try {  // everything additionally in one big try block
	       // to catch any unexpected errors
		clear_errors();

		//  Make sure that the current directory exists and is writeable.
		try {
			g_fs->ensure_directory_exists(dir_);
		} catch (const FileError& e) {
			error_ |= kCreatingDirFailed;
			error_msg_[bitCreatingDirFailed] =
			   (boost::format("GenericSaveHandler::save: directory %s could not be "
			   "created: %s\n") % dir_.c_str() % e.what()).str();
			log("%s", error_msg_[bitCreatingDirFailed].c_str());
			return error_;
		}

		// Make a backup if file already exists.
		if (g_fs->file_exists(complete_filename_)) {
			make_backup();
		}
		if (error_) {
		  return error_;
		}

		// Write data to file/dir.
		save_file();

		// Restore or delete backup if one was made.
		if (!backup_filename_.empty())
		{
			if (!error_) {
				// Delete backup.
				try {
					g_fs->fs_unlink(backup_filename_);
				} catch (const FileError& e) {
					error_ |= kDeletingBackupFailed;
					error_msg_[bitDeletingBackupFailed] =
					   (boost::format("GenericSaveHandler::save: backup file %s could "
					   "not be deleted: %s\n") % backup_filename_.c_str() % e.what()
					   ).str();
					log("%s", error_msg_[bitDeletingBackupFailed].c_str());
				}

			} else {
				if (error_ & kCorruptFileLeft) {
					error_ |= kRestoringBackupFailed;
					error_msg_[bitRestoringBackupFailed] =
					   (boost::format("GenericSaveHandler::save: file %s could not be "
					   "restored from backup %s: file still exists\n")
					   % complete_filename_.c_str() % backup_filename_.c_str()).str();
					log("%s", error_msg_[bitRestoringBackupFailed].c_str());
				}
				else {
					// Restore backup.
					try {
						g_fs->fs_rename(backup_filename_, complete_filename_);
					} catch (const FileError& e) {
						error_ |= kRestoringBackupFailed;
						error_msg_[bitRestoringBackupFailed] =
						   (boost::format("GenericSaveHandler::save: file %s could not "
						   "be restored from backup %s: %s\n") % backup_filename_.c_str()
						   % backup_filename_.c_str() % e.what()).str();
						log("%s", error_msg_[bitRestoringBackupFailed].c_str());
					}
				}
			}
		}

	} catch (const std::exception& e) {
		error_ |= kUnexpectedError;
		error_msg_[bitUnexpectedError] =
		   (boost::format("GenericSaveHandler::save: unknown error: %s\n")
		   % e.what()).str();
		log("%s", error_msg_[bitUnexpectedError].c_str());
	}

	return error_;
}

std::string GenericSaveHandler::error_message(uint32_t error_mask) {
	error_mask &= error_;
	std::string err_msg;
	for (uint32_t errind = 0; errind < maxErrors; errind++) {
		if ((error_mask >> errind) & 1) {
			err_msg += error_msg_[errind];
		}
	}
	return err_msg;
}

std::string GenericSaveHandler::localized_formatted_result_message() {
	std::string msg;

	if (error_ == kSuccess || error_ == kDeletingBackupFailed) {
		// no need to mention a failed backup deletion
		return _("File successfully saved!");
	}

	if (error_ == kCreatingDirFailed) {
		return
		   (boost::format(_("Directory ‘%s’ could not be created!"))
		   % dir_).str();
	}

	if (error_ == kBackupFailed) {
		return
		   (boost::format(_("File ‘%s’ could not be removed!"))
		   % complete_filename_.c_str()).str() + "\n"
			 + _("Try saving under a different name!");
	}

	// from here on multiple errors might have occurred
	if (error_ & kSavingDataFailed) {
		msg =
		   (boost::format(_("Error writing data to file ‘%s’!"))
		   % complete_filename_.c_str()).str();
	}

	if (error_ & kCorruptFileLeft) {
		if (!msg.empty()) {
			msg += "\n";
		}
		msg += (boost::format(_("Saved file may be corrupt!"))).str();
	}

	if (error_ & kRestoringBackupFailed) {
		if (!msg.empty()) {
			msg += "\n";
		}
		msg +=
		   (boost::format(_("File ‘%s’ could not be restored!"))
		   % complete_filename_.c_str()).str() + "\n" +
		   (boost::format(_("Backup file ‘%s’ will be available for some time."))
		   % backup_filename_.c_str()).str();
	}

	if (!backup_filename_.empty() &&
	    (error_ & kSavingDataFailed) &&
	    !(error_ & kCorruptFileLeft) &&
	    !(error_ & kRestoringBackupFailed)) {
		if (!msg.empty()) {
			msg += "\n";
		}
		msg +=
		   (boost::format(_("File ‘%s’ was restored from backup."))
		   % complete_filename_.c_str()).str();
	}

	if (error_ & kUnexpectedError) {
		if (!msg.empty()) {
			msg += "\n";
		}
		msg +=
		   (boost::format(_("An unexpected error occurred:" ))).str()
		   + "\n" + error_msg_[bitUnexpectedError];
	}

	return msg;
}
