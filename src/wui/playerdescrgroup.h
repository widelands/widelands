/*
 * Copyright (C) 2002, 2006, 2008, 2010 by the Widelands Development Team
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

#ifndef PLAYERDESCRGROUP_H
#define PLAYERDESCRGROUP_H

#include <map>
#include <string>

#include "constants.h"
#include "ui_basic/panel.h"

namespace UI {
struct Font;
}

struct GameSettingsProvider;
struct PlayerDescriptionGroupImpl;

/** struct PlayerDescriptionGroup
 *
 * - checkbox to enable/disable player
 * - button to switch between: Human, Remote, AI
 */
struct PlayerDescriptionGroup : public UI::Panel {
	PlayerDescriptionGroup
		(UI::Panel * parent,
		 int32_t x, int32_t y, int32_t w, int32_t h,
		 GameSettingsProvider * settings,
		 uint32_t plnum,
		 UI::Font * font);
	~PlayerDescriptionGroup();

	void refresh();

private:
	void enable_player(bool);
	void toggle_playertype();
	void toggle_playertribe();
	void toggle_playerinit();
	void toggle_playerteam();

	PlayerDescriptionGroupImpl * d;
	std::map<std::string, std::string> m_tribenames;
};


#endif
