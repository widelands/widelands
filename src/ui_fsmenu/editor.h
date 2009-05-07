/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef FULLSCREEN_MENU_EDITOR_H
#define FULLSCREEN_MENU_EDITOR_H

#include "base.h"

#include "ui_basic/button.h"
#include "ui_basic/textarea.h"

/**
 * Fullscreen Menu for Editor.
 * Here you select what game you want to play.
 */
struct Fullscreen_Menu_Editor : public Fullscreen_Menu_Base {
	Fullscreen_Menu_Editor();

	enum {Back = dying_code, New_Map, Load_Map};

private:
	uint32_t                                      m_butw;
	uint32_t                                      m_buth;
	uint32_t                                      m_butx;
	UI::Textarea                                  title;
	UI::Callback_IDButton<Fullscreen_Menu_Editor, int32_t> new_map;
	UI::Callback_IDButton<Fullscreen_Menu_Editor, int32_t> load_map;
	UI::Callback_IDButton<Fullscreen_Menu_Editor, int32_t> back;
};

#endif
