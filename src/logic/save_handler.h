/*
 * Copyright (C) 2002-2009, 2015 by the Widelands Development Team
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

#ifndef WL_LOGIC_SAVE_HANDLER_H
#define WL_LOGIC_SAVE_HANDLER_H

#include <cstring>
#include <string>

#include <stdint.h>

namespace Widelands {class Game;}

// default autosave interval in minutes
#define DEFAULT_AUTOSAVE_INTERVAL 15

class SaveHandler {
public:
	SaveHandler() : last_saved_realtime_(0), initialized_(false), allow_saving_(true),
		save_requested_(false), save_filename_(""), autosave_filename_("wl_autosave") {}
	void think(Widelands::Game &);
	std::string create_file_name(const std::string& dir, const std::string& filename) const;
	bool save_game
		(Widelands::Game   &,
		 const std::string & filename,
		 std::string       * error = nullptr);

	static std::string get_base_dir() {return "save";}
	const std::string get_cur_filename() {return current_filename_;}
	void set_current_filename(const std::string& filename) {current_filename_ = filename;}
	void set_autosave_filename(const std::string& filename) {autosave_filename_ = filename;}
	void set_allow_saving(bool t) {allow_saving_ = t;}
	bool get_allow_saving() {return allow_saving_;}
	void request_save(const std::string& filename = "")
	{
		save_requested_ = true;
		save_filename_ = filename;
	}

private:
	uint32_t last_saved_realtime_;
	bool initialized_;
	bool allow_saving_;
	bool save_requested_;
	std::string save_filename_;
	std::string current_filename_;
	std::string autosave_filename_;

	void initialize(uint32_t gametime);
};

#endif  // end of include guard: WL_LOGIC_SAVE_HANDLER_H
