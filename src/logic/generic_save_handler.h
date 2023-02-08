/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_GENERIC_SAVE_HANDLER_H
#define WL_LOGIC_GENERIC_SAVE_HANDLER_H

#include <cstdint>
#include <functional>

#include "io/filesystem/filesystem.h"

/**
 * Class that provides handling all errors for generic file saving.
 * It stores error codes and error messages.
 * It can also generate an overview message aimed at a human user.
 *
 * The saving routine (what actually writes data to a file system)
 * must be provided.
 */
// TODO(Arty): Possibly make it more generic, allowing to provide options
// whether to overwrite files, whether to make backups, maybe allow
// to provide a naming scheme, etc.
class GenericSaveHandler {
public:
	// error constants; also usable as bit masks
	enum class Error : uint32_t {
		kNone = 0,
		kSuccess = 0,
		kCreatingDirFailed = 1,
		kBackupFailed = 2,
		kDeletingBackupFailed = 4,
		kSavingDataFailed = 8,
		kCorruptFileLeft = 16,
		kRestoringBackupFailed = 32,
		kUnexpectedError = 64,
		kAllErrors = 127
	};

	explicit GenericSaveHandler(std::function<void(FileSystem&)>
	                               do_save,  // function that actually saves data to the filesystem
	                            const std::string& complete_filename,
	                            FileSystem::Type type)
	   : do_save_(do_save),
	     complete_filename_(complete_filename),
	     dir_(FileSystem::fs_dirname(complete_filename)),
	     filename_(FileSystem::fs_filename(complete_filename.c_str())),
	     type_(type),
	     error_(static_cast<Error>(1132)) {
	}

	/**
	 * Tries to save a file.
	 *
	 * If the file already exists, it will be overwritten but a temporary backup
	 * is made, which will be restored if saving fails and deleted otherwise.
	 *
	 * Catches ALL errors.
	 *
	 * Error messages for all errors are written to the log but also stored.
	 * Stores and returns an error code (bit mask of all occurred errors).
	 */
	Error save();

	// returns the stored error code (of the last saving operation)
	Error error() {
		return error_;
	}

	// Returns the combination of error_messages (of occurred errors)
	// specified by a bit mask.
	std::string error_message(Error error_mask = Error::kAllErrors);

	// Generates a localized formatted message describing the result of
	// the last saving attempt.
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

	Error error_;

	static constexpr uint32_t maxErrors_ = 7;
	static_assert((1ul << maxErrors_) == static_cast<uint32_t>(Error::kAllErrors) + 1,
	              "value of maxErrors_ doesn't match!");
	std::string error_msg_[maxErrors_];

	// Returns the lowest array index of the an error.
	// Intended for use with single errors to get their array index.
	uint32_t get_index(Error);

	void clear();

	// Finds a suitable backup filename and tries to rename a file.
	// Stores an errorcode and error message (if applicable).
	void make_backup();

	// Saves a file. Assumes file doesn't exist yet.
	// Stores an errorcode and error message (if applicable).
	void save_file();
};

inline constexpr GenericSaveHandler::Error operator|(GenericSaveHandler::Error e1,
                                                     GenericSaveHandler::Error e2) {
	return static_cast<GenericSaveHandler::Error>(static_cast<uint32_t>(e1) |
	                                              static_cast<uint32_t>(e2));
}

inline constexpr GenericSaveHandler::Error operator&(GenericSaveHandler::Error e1,
                                                     GenericSaveHandler::Error e2) {
	return static_cast<GenericSaveHandler::Error>(static_cast<uint32_t>(e1) &
	                                              static_cast<uint32_t>(e2));
}

inline GenericSaveHandler::Error& operator|=(GenericSaveHandler::Error& e1,
                                             GenericSaveHandler::Error e2) {
	return e1 = e1 | e2;
}

inline GenericSaveHandler::Error& operator&=(GenericSaveHandler::Error& e1,
                                             GenericSaveHandler::Error e2) {
	return e1 = e1 & e2;
}

#endif  // end of include guard: WL_LOGIC_GENERIC_SAVE_HANDLER_H
