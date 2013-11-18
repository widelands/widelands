/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_MAIN_H
#define FULLSCREEN_MENU_MAIN_H

#include "ui_fsmenu/base.h"
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"

/**
 * This runs the main menu. There, you can select
 * between different playmodes, exit and so on.
*/
struct Fullscreen_Menu_Main : public Fullscreen_Menu_Base {
	Fullscreen_Menu_Main();
	enum {
		mm_summary,
		mm_playtutorial,
		mm_singleplayer,
		mm_multiplayer,
		mm_replay,
		mm_editor,
		mm_options,
		mm_readme,
		mm_license,
		mm_exit
	};
private:
	uint32_t                                    m_butx;
	uint32_t                                    m_butw;
	uint32_t                                    m_buth;
	std::string                                 wlcr;
	UI::Button                     playtutorial;
	UI::Button                     singleplayer;
	UI::Button                     multiplayer;
	UI::Button                     replay;
	UI::Button                     editor;
	UI::Button                     options;
	UI::Button                     readme;
	UI::Button                     license;
	UI::Button                     exit;
	UI::Textarea                                version;
	UI::Textarea                                copyright;
	UI::Textarea                                gpl;
};

#endif
