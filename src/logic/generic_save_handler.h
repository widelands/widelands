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

#ifndef WL_LOGIC_GENERIC_SAVE_HANDLER_H
#define WL_LOGIC_GENERIC_SAVE_HANDLER_H

#include <functional>
#include <string>

#include <stdint.h>

#include "io/filesystem/filesystem.h"

// just some constants for convenience
namespace {
	enum ErrorBitIndex : uint32_t {
		bitCreatingDirFailed = 0,
		bitBackupFailed,
		bitSavingDataFailed,
		bitCorruptFileLeft,
		bitDeletingBackupFailed,
		bitRestoringBackupFailed,
		bitUnexpectedError,
		maxErrors
	};
}

/**
 * Class that provides handling all errors for generic file saving.
 * It stores error codes and error messages.
 * It can also generate an overview message aimed at a human user.
 *
 * The saving routine (what actually writes data to a file system) must be provided.
 */
// TODO(Arty): Possibly make it more generic, allowing to provide options whether
// to overwrite, whether to make backups; maybe allow to provide a naming scheme, etc.
class GenericSaveHandler {
public:
	// error constants; usable as bit masks
	static constexpr uint32_t kSuccess = 0;
	static constexpr uint32_t kCreatingDirFailed = uint32_t(1) << bitCreatingDirFailed;
	static constexpr uint32_t kBackupFailed = uint32_t(1) << bitBackupFailed;
	static constexpr uint32_t kSavingDataFailed = uint32_t(1) << bitSavingDataFailed;
	static constexpr uint32_t kCorruptFileLeft = uint32_t(1) << bitCorruptFileLeft;
	static constexpr uint32_t kDeletingBackupFailed = uint32_t(1) << bitDeletingBackupFailed;
	static constexpr uint32_t kRestoringBackupFailed = uint32_t(1) << bitRestoringBackupFailed;
	static constexpr uint32_t kUnexpectedError = uint32_t(1) << bitUnexpectedError;
	static constexpr uint32_t kAllErrors = (1 << maxErrors) - 1;

	explicit GenericSaveHandler(
	   std::function<void(FileSystem&)> do_save, // function which actually saves data to the filesystem
	   std::string complete_filename,
	   FileSystem::Type type)
   : do_save_(do_save),
     complete_filename_(complete_filename),
     dir_(FileSystem::fs_dirname(complete_filename.c_str())),
     filename_(FileSystem::fs_filename(complete_filename.c_str())),
     type_(type),
     error_(kSuccess) {};

	/**
	 * Tries to save a file.
	 * If the file already exists, it will be overwritten but a temporary backup is made
	 * which will be restored if saving fails and deleted otherwise.
	 * Catches ALL errors.
	 * Error messages for all errors are written to the log but also stored.
	 * Stores and returns an error code (bit mask of all occurred errors).
	 */
	uint32_t save();

	// returns the stored error code (from the last saving operation)
	uint32_t error() { return error_; };

	// Returns the combination of error_messages (of occurred errors) specified by a bit mask.
	std::string error_message(uint32_t error_mask = kAllErrors);

	// Generates a localized formatted message describing
	// the result of the saving attempt.
	// Aimed to be sufficiently informative for a human user.
	std::string localized_formatted_result_message();

private:
	std::function<void(FileSystem&)> do_save_;
	std::string complete_filename_;
	std::string dir_;
	std::string filename_;
	FileSystem::Type type_;

	// Backup filename is automatically generated when saving but is also
	// stored for generating messages containing backup-related things.
	std::string backup_filename_;
	uint32_t error_;
	std::string error_msg_[maxErrors];

	void clear_errors();

	// Finds a suitable backup filename and tries to rename a file.
	// Stores an errorcode and error message (if applicable).
	void make_backup();

	// Saves a file. Assumes file doesn't exist.
	// Stores an errorcode and error message (if applicable).
	void save_file();
};


#endif  // end of include guard: WL_LOGIC_GENERIC_SAVE_HANDLER_H
