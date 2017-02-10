/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_MAIN_H
#define WL_UI_FSMENU_MAIN_H

#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/main_menu.h"

/**
 * This runs the main menu. There, you can select
 * between different playmodes, exit and so on.
*/
class FullscreenMenuMain : public FullscreenMenuMainMenu {
public:
	FullscreenMenuMain();

protected:
	void clicked_ok() override;

private:
	void layout() override;

	UI::Button playtutorial;
	UI::Button singleplayer;
	UI::Button multiplayer;
	UI::Button replay;
	UI::Button editor;
	UI::Button options;
	UI::Button about;
	UI::Button exit;
	UI::Textarea version;
	UI::Textarea copyright;
	UI::Textarea gpl;
};

#endif  // end of include guard: WL_UI_FSMENU_MAIN_H
