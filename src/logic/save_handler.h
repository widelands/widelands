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
	SaveHandler() : m_last_saved_realtime(0), m_initialized(false), m_allow_saving(true),
		m_save_requested(false), m_save_filename(""), autosave_filename_("wl_autosave") {}
	void think(Widelands::Game &);
	std::string create_file_name(const std::string& dir, const std::string& filename) const;
	bool save_game
		(Widelands::Game   &,
		 const std::string & filename,
		 std::string       * error = nullptr);

	static std::string get_base_dir() {return "save";}
	const std::string get_cur_filename() {return m_current_filename;}
	void set_current_filename(const std::string& filename) {m_current_filename = filename;}
	void set_autosave_filename(const std::string& filename) {autosave_filename_ = filename;}
	void set_allow_saving(bool t) {m_allow_saving = t;}
	bool get_allow_saving() {return m_allow_saving;}
	void request_save(const std::string& filename = "")
	{
		m_save_requested = true;
		m_save_filename = filename;
	}

private:
	uint32_t m_last_saved_realtime;
	bool m_initialized;
	bool m_allow_saving;
	bool m_save_requested;
	std::string m_save_filename;
	std::string m_current_filename;
	std::string autosave_filename_;

	void initialize(uint32_t gametime);
};

#endif  // end of include guard: WL_LOGIC_SAVE_HANDLER_H
