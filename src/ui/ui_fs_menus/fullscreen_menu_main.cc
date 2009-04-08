/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "fullscreen_menu_main.h"

#include "../../build_id.h"
#include "i18n.h"

Fullscreen_Menu_Main::Fullscreen_Menu_Main()
:
Fullscreen_Menu_Base("mainmenu.jpg"),

// Values for alignment and size
m_butx
	(m_xres * 13 / 40),
m_butw
	(m_xres * 7 / 20),
m_buth
	(m_yres * 19 / 400),
m_fs
	(fs_small()),
m_fn
	(ui_fn()),
wlcr(WLCR),

// Buttons
singleplayer
	(this,
	 m_butx, m_yres * 6 / 25, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, *this, mm_singleplayer,
	 _("Single Player"), std::string(), true, false,
	 m_fn, m_fs),
multiplayer
	(this,
	 m_butx, m_yres * 61 / 200, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, *this, mm_multiplayer,
	 _("Multi Player"), std::string(), true, false,
	 m_fn, m_fs),
replay
	(this,
	 m_butx, m_yres * 37 / 100, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, *this, mm_replay,
	 _("Watch Replay"), std::string(), true, false,
	 m_fn, m_fs),
options
	(this,
	 m_butx, m_yres * 87 / 200, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, *this, mm_options,
	 _("Options"), std::string(), true, false,
	 m_fn, m_fs),
editor
	(this,
	 m_butx, m_yres / 2, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, *this, mm_editor,
	 _("Editor"), std::string(), true, false,
	 m_fn, m_fs),
readme
	(this,
	 m_butx, m_yres * 113 / 200, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, *this, mm_readme,
	 _("View Readme"), std::string(), true, false,
	 m_fn, m_fs),
license
	(this,
	 m_butx, m_yres * 63 / 100, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, *this, mm_license,
	 _("License"), std::string(), true, false,
	 m_fn, m_fs),
exit
	(this,
	 m_butx, m_yres * 3 / 4, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, *this, mm_exit,
	 _("Exit Game"), std::string(), true, false,
	 m_fn, m_fs),

// Textlabels
version
	(this,
	 m_xres, m_yres,
	 std::string(_("Version ")) + build_id(), Align_BottomRight),
copyright
	(this,
	 0, m_yres,
	 (wlcr + _("by the Widelands Development Team")).c_str(),
	 Align_BottomLeft)
{
	version.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	copyright.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
}
