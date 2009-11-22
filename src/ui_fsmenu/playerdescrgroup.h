/*
 * Copyright (C) 2002, 2006, 2008 by the Widelands Development Team
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

#ifndef PLAYERDESCRGROUP_H
#define PLAYERDESCRGROUP_H

#include "constants.h"
#include "ui_basic/panel.h"

#include <string>

struct GameSettingsProvider;

struct PlayerDescriptionGroupImpl;

/** class PlayerDescriptionGroup
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
		 std::string const & fname = UI_FONT_NAME,
		 uint32_t fsize = UI_FONT_SIZE_SMALL);
	~PlayerDescriptionGroup();

	void refresh();
	void enable_pdg       (bool enable = true);
	void show_tribe_button(bool show   = true);

private:
	void enable_player(bool);
	void ready_player(bool);
	void toggle_playertype();
	void toggle_playertribe();
	void toggle_playerinit();

	PlayerDescriptionGroupImpl * d;
};


#endif
