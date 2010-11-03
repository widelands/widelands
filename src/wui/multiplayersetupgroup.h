/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef MULTIPLAYERSETUPGROUP_H
#define MULTIPLAYERSETUPGROUP_H

#include "constants.h"
#include "ui_basic/box.h"
#include "ui_basic/panel.h"
#include "ui_basic/textarea.h"

#include <string>

#define MAXCLIENTS 64

struct GameSettingsProvider;
struct MultiPlayerSetupGroupOptions;
struct MultiPlayerClientGroup;
struct MultiPlayerPlayerGroup;

/**
 * struct MultiPlayerSetupGroup
 *
 * A MultiPlayerSetupGroup holds information and provides the UI to set the
 * clients, computers and closed players.
 *
 */
struct MultiPlayerSetupGroup : public UI::Panel {
	MultiPlayerSetupGroup
		(UI::Panel * parent,
		 int32_t x, int32_t y, int32_t w, int32_t h,
		 GameSettingsProvider * settings,
		 uint32_t butw, uint32_t buth,
		 std::string const & fname = UI_FONT_NAME,
		 uint32_t fsize = UI_FONT_SIZE_SMALL);
	~MultiPlayerSetupGroup();

	void refresh();

private:
	GameSettingsProvider   * const s;
	std::vector<MultiPlayerClientGroup *> c;
	std::vector<MultiPlayerPlayerGroup *> p;
	UI::Box                  clientbox, playerbox;
	std::vector<UI::Textarea *> labels;

	uint32_t    m_buth, m_fsize;
	std::string m_fname;

	std::map<std::string, PictureID> m_tribepics;
	std::map<std::string, std::string> m_tribenames;
};


#endif
