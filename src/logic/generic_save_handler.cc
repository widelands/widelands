/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "base/i18n.h"
#include "base/log.h"
#include "base/string.h"
#include "base/time_string.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"

void GenericSaveHandler::clear() {
	error_ = Error::kNone;
	std::fill(std::begin(error_msg_), std::end(error_msg_), "");
	backup_filename_.clear();
}

uint32_t GenericSaveHandler::get_index(GenericSaveHandler::Error err) {
	if (err == Error::kNone) {
		return maxErrors_;
	}
	uint32_t error_uint = static_cast<uint32_t>(err);
	for (uint32_t index = 0; index < maxErrors_; index++) {
		if (error_uint & 1) {
			return index;
		}
		error_uint >>= 1;
	}
	return maxErrors_;
}

void GenericSaveHandler::make_backup() {
	std::string backup_filename_base =
	   dir_ + FileSystem::file_separator() + timestring() + "_" + filename_;
	backup_filename_ = backup_filename_base + kTempBackupExtension;

	// If a file with that name already exists, then try some name modifications.
	if (g_fs->file_exists(backup_filename_)) {
		int suffix;
		for (suffix = 0; suffix <= 9; suffix++) {
			backup_filename_ = backup_filename_base.append("-")
			                      .append(std::to_string(suffix))
			                      .append(kTempBackupExtension);
			if (!g_fs->file_exists(backup_filename_)) {
				break;
			}
		}
		if (suffix > 9) {
			error_ |= Error::kBackupFailed;
			uint32_t index = get_index(Error::kBackupFailed);
			error_msg_[index] =
			   format("GenericSaveHandler::make_backup: %s: for all "
			          "considered filenames a file already existed (last filename tried "
			          "was %s)\n",
			          complete_filename_.c_str(), backup_filename_);
			log_err("%s", error_msg_[index].c_str());
			return;
		}
	}

	// Try to rename file.
	try {
		g_fs->fs_rename(complete_filename_, backup_filename_);
	} catch (const FileError& e) {
		error_ |= Error::kBackupFailed;
		uint32_t index = get_index(Error::kBackupFailed);
		error_msg_[index] = format("GenericSaveHandler::make_backup: file %s "
		                           "could not be renamed to %s: %s\n",
		                           complete_filename_.c_str(), backup_filename_, e.what());
		log_err("%s", error_msg_[index].c_str());
		return;
	}
}

void GenericSaveHandler::save_file() {
	// Write data to file/dir.
	try {
		std::unique_ptr<FileSystem> fs(g_fs->create_sub_file_system(complete_filename_, type_));
		do_save_(*fs);
	} catch (const std::exception& e) {
		error_ |= Error::kSavingDataFailed;
		uint32_t index = get_index(Error::kSavingDataFailed);
		error_msg_[index] = format("GenericSaveHandler::save_file: data could not be "
		                           "written to file %s: %s\n",
		                           complete_filename_.c_str(), e.what());
		log_err("%s", error_msg_[index].c_str());
	}

	if ((error_ & Error::kSavingDataFailed) != Error::kNone) {
		// Delete remnants of the failed save attempt.
		if (g_fs->file_exists(complete_filename_)) {
			try {
				g_fs->fs_unlink(complete_filename_);
			} catch (const FileError& e) {
				error_ |= Error::kCorruptFileLeft;
				uint32_t index = get_index(Error::kCorruptFileLeft);
				error_msg_[index] = format("GenericSaveHandler::save_file: possibly corrupt "
				                           "file %s could not be deleted: %s\n",
				                           complete_filename_.c_str(), e.what());
				log_err("%s", error_msg_[index].c_str());
			}
		}
	}
}

GenericSaveHandler::Error GenericSaveHandler::save() {
	try {  // everything additionally in one big try block
		    // to catch any unexpected errors
		clear();

		//  Make sure that the current directory exists and is writeable.
		try {
			g_fs->ensure_directory_exists(dir_);
		} catch (const FileError& e) {
			error_ |= Error::kCreatingDirFailed;
			uint32_t index = get_index(Error::kCreatingDirFailed);
			error_msg_[index] = format("GenericSaveHandler::save: directory %s could not be "
			                           "created: %s\n",
			                           dir_.c_str(), e.what());
			log_err("%s", error_msg_[index].c_str());
			return error_;
		}

		// Make a backup if file already exists.
		if (g_fs->file_exists(complete_filename_)) {
			make_backup();
		}
		if (error_ != Error::kNone) {
			return error_;
		}

		// Write data to file/dir.
		save_file();

		// Restore or delete backup if one was made.
		if (!backup_filename_.empty()) {
			if (error_ == Error::kNone) {
				// Delete backup.
				try {
					g_fs->fs_unlink(backup_filename_);
				} catch (const FileError& e) {
					error_ |= Error::kDeletingBackupFailed;
					uint32_t index = get_index(Error::kDeletingBackupFailed);
					error_msg_[index] = format("GenericSaveHandler::save: backup file %s could "
					                           "not be deleted: %s\n",
					                           backup_filename_.c_str(), e.what());
					log_err("%s", error_msg_[index].c_str());
				}

			} else {
				if ((error_ & Error::kCorruptFileLeft) != Error::kNone) {
					error_ |= Error::kRestoringBackupFailed;
					uint32_t index = get_index(Error::kRestoringBackupFailed);
					error_msg_[index] = format("GenericSaveHandler::save: file %s could not be "
					                           "restored from backup %s: file still exists\n",
					                           complete_filename_.c_str(), backup_filename_.c_str());
					log_err("%s", error_msg_[index].c_str());
				} else {
					// Restore backup.
					try {
						g_fs->fs_rename(backup_filename_, complete_filename_);
					} catch (const FileError& e) {
						error_ |= Error::kRestoringBackupFailed;
						uint32_t index = get_index(Error::kRestoringBackupFailed);
						error_msg_[index] =
						   format("GenericSaveHandler::save: file %s could not "
						          "be restored from backup %s: %s\n",
						          backup_filename_.c_str(), backup_filename_.c_str(), e.what());
						log_err("%s", error_msg_[index].c_str());
					}
				}
			}
		}

	} catch (const std::exception& e) {
		error_ |= Error::kUnexpectedError;
		uint32_t index = get_index(Error::kUnexpectedError);
		error_msg_[index] = format("GenericSaveHandler::save: unknown error: %s\n", e.what());
		log_err("%s", error_msg_[index].c_str());
	}

	return error_;
}

std::string GenericSaveHandler::error_message(GenericSaveHandler::Error error_mask) {
	uint32_t error_uint = static_cast<uint32_t>(error_mask) & static_cast<uint32_t>(error_);
	std::string err_msg;
	for (uint32_t index = 0; index < maxErrors_; index++) {
		if ((error_uint >> index) & 1) {
			err_msg += error_msg_[index];
		}
	}
	return err_msg;
}

std::string GenericSaveHandler::localized_formatted_result_message() {
	std::string msg;

	if (error_ == Error::kSuccess) {
		return _("File successfully saved!");
	}

	if (error_ == Error::kDeletingBackupFailed) {
		return std::string(_("File successfully saved!")) + "\n" +
		       format(_("Backup file ‘%s’ could not be deleted."), backup_filename_);
	}

	if (error_ == Error::kCreatingDirFailed) {
		return format(_("Directory ‘%s’ could not be created!"), dir_);
	}

	if (error_ == Error::kBackupFailed) {
		return format(_("File ‘%s’ could not be removed!"), complete_filename_) + "\n" +
		       _("Try saving under a different name!");
	}

	// from here on multiple errors might have occurred
	if ((error_ & Error::kSavingDataFailed) != Error::kNone) {
		msg = format(_("Error writing data to file ‘%s’!"), complete_filename_);
	}

	if ((error_ & Error::kCorruptFileLeft) != Error::kNone) {
		if (!msg.empty()) {
			msg += '\n';
		}
		msg += _("Saved file may be corrupt!");
	}

	if ((error_ & Error::kRestoringBackupFailed) != Error::kNone) {
		if (!msg.empty()) {
			msg += '\n';
		}
		msg += format(_("File ‘%s’ could not be restored!"), complete_filename_) + "\n" +
		       format(_("Backup file ‘%s’ will be available for some time."), backup_filename_);
	}

	if (!backup_filename_.empty() && ((error_ & Error::kSavingDataFailed) != Error::kNone) &&
	    !((error_ & Error::kCorruptFileLeft) != Error::kNone) &&
	    !((error_ & Error::kRestoringBackupFailed) != Error::kNone)) {
		if (!msg.empty()) {
			msg += '\n';
		}
		msg += format(_("File ‘%s’ was restored from backup."), complete_filename_);
	}

	if ((error_ & Error::kUnexpectedError) != Error::kNone) {
		if (!msg.empty()) {
			msg += '\n';
		}
		msg += _("An unexpected error occurred:");
		msg += '\n';
		msg += error_message(Error::kUnexpectedError);
	}

	return msg;
}
