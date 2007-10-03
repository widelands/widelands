/*
* Copyright (C) 2002-2007 by the Widelands Development Team
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
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*/

#ifndef __S__SAVE_HANDLER_H
#define __S__SAVE_HANDLER_H

#include <string>

class Game;
// default autosave interval in minutes
#define DEFAULT_AUTOSAVE_INTERVAL 15

class SaveHandler {
	int32_t m_lastSaveTime;
	bool m_initialized;

	void initialize(int32_t currenttime);

protected:

public:
	SaveHandler() : m_initialized(false) {}
	void think(Game* g, int32_t currenttime);
	std::string create_file_name(std::string dir, std::string filename);
	bool save_game (Game* g, std::string filename, std::string *error = NULL);

	static std::string get_base_dir() {return "ssave";}
};

#endif
