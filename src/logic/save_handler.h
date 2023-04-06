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

#ifndef WL_LOGIC_SAVE_HANDLER_H
#define WL_LOGIC_SAVE_HANDLER_H

#include <cstdint>
#include <optional>

#include "io/filesystem/filesystem.h"
#include "logic/filesystem_constants.h"

namespace Widelands {
class Game;
}  // namespace Widelands

/**
 * Takes care of manual or autosave via think().
 *
 * Note that this handler is used for replays via the ReplayWriter, too.
 */
class SaveHandler {
public:
	SaveHandler() = default;

	void think(Widelands::Game&);
	[[nodiscard]] std::string create_file_name(const std::string& dir,
	                                           const std::string& filename) const;

	// Saves the game, overwrites file, handles errors
	bool save_game(Widelands::Game&,
	               const std::string& filename,
	               std::optional<FileSystem::Type> fstype,
	               std::string* error_str = nullptr);

	const std::string get_cur_filename() {
		return current_filename_;
	}
	void set_current_filename(const std::string& filename) {
		current_filename_ = filename;
	}
	void set_autosave_filename(const std::string& filename) {
		autosave_filename_ = filename;
	}
	// Used by lua only
	void set_allow_saving(bool t) {
		allow_saving_ = t;
	}
	// Used by lua only
	[[nodiscard]] bool get_allow_saving() const {
		return allow_saving_;
	}
	// Used by lua only
	void request_save(const std::string& filename = "") {
		save_requested_ = true;
		save_filename_ = filename;
	}

	[[nodiscard]] uint32_t last_save_time() const {
		return last_save_realtime_;
	}

private:
	uint32_t next_save_realtime_{0U};
	uint32_t last_save_realtime_{0U};
	bool initialized_{false};
	bool allow_saving_{true};
	bool save_requested_{false};
	bool saving_next_tick_{false};
	std::string save_filename_;
	std::string current_filename_;
	std::string autosave_filename_{kAutosavePrefix};

	FileSystem::Type fs_type_{FileSystem::ZIP};
	int32_t autosave_interval_in_ms_{kDefaultAutosaveInterval * 60 * 1000};
	int32_t number_of_rolls_{5};  // For rolling file update

	void initialize(uint32_t realtime);
	bool roll_save_files(const std::string& filename, std::string* error) const;
	bool check_next_tick(Widelands::Game& game, uint32_t realtime) const;
};

#endif  // end of include guard: WL_LOGIC_SAVE_HANDLER_H
