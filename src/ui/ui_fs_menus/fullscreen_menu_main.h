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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef FULLSCREEN_MENU_MAIN_H
#define FULLSCREEN_MENU_MAIN_H

#include "fullscreen_menu_base.h"

#include "ui_button.h"
#include "ui_textarea.h"

/**
 * This runs the main menu. There, you can select
 * between different playmodes, exit and so on.
*/
struct Fullscreen_Menu_Main : public Fullscreen_Menu_Base {
	Fullscreen_Menu_Main();
	enum {
		mm_singleplayer,
		mm_multiplayer,
		mm_replay,
		mm_options,
		mm_editor,
		mm_readme,
		mm_license,
		mm_exit
	};
private:
	uint32_t                                    m_butx;
	uint32_t                                    m_butw;
	uint32_t                                    m_buth;
	uint32_t                                    m_fs;
	std::string                                 m_fn;
	UI::IDButton<Fullscreen_Menu_Main, int32_t> singleplayer;
	UI::IDButton<Fullscreen_Menu_Main, int32_t> multiplayer;
	UI::IDButton<Fullscreen_Menu_Main, int32_t> replay;
	UI::IDButton<Fullscreen_Menu_Main, int32_t> options;
	UI::IDButton<Fullscreen_Menu_Main, int32_t> editor;
	UI::IDButton<Fullscreen_Menu_Main, int32_t> readme;
	UI::IDButton<Fullscreen_Menu_Main, int32_t> license;
	UI::IDButton<Fullscreen_Menu_Main, int32_t> exit;
	UI::Textarea                                version;
	UI::Textarea                                copyright;
};

#endif
