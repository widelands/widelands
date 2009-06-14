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

#include "editor.h"

#include "constants.h"
#include "i18n.h"

Fullscreen_Menu_Editor::Fullscreen_Menu_Editor() :
Fullscreen_Menu_Base("singleplmenu.jpg"),

// Values for alignment and size
m_butw
	(m_xres * 7 / 20),
m_buth
	(m_yres * 19 / 400),
m_butx
	((m_xres - m_butw) / 2),

// Title
title(this, m_xres / 2, m_yres * 3 / 40, _("Editor Menu"), Align_HCenter),

// Buttons
	new_map
		(this,
		 m_butx, m_yres * 6 / 25, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Fullscreen_Menu_Editor::end_modal, *this, New_Map,
		 _("New Map"), std::string(), true, false,
		 ui_fn(), fs_small()),
	load_map
		(this,
		 m_butx, m_yres * 61 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Fullscreen_Menu_Editor::end_modal, *this, Load_Map,
		 _("Load Map"), std::string(), true, false,
		 ui_fn(), fs_small()),
	back
		(this,
		 m_butx, m_yres * 3 / 4, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Fullscreen_Menu_Editor::end_modal, *this, Back,
		 _("Back"), std::string(), true, false,
		 ui_fn(), fs_small())
{
	title.set_font(ui_fn(), fs_big(), UI_FONT_CLR_FG);
}
