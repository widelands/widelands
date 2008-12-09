/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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
m_xres
	(gr_x()),
m_yres
	(gr_y()),
m_butx
	(m_xres*0.325),
m_butw
	(m_xres*0.35),
m_buth
	(m_yres*0.0475),
m_fs
	(fs_small()),
m_fn
	(ui_fn()),

// Buttons
singleplayer
	(this,
	 m_butx, m_yres*0.24, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, this, mm_singleplayer,
	 _("Single Player"), std::string(), true, false,
	 m_fn, m_fs),
multiplayer
	(this,
	 m_butx, m_yres*0.305, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, this, mm_multiplayer,
	 _("Multi Player"), std::string(), true, false,
	 m_fn, m_fs),
replay
	(this,
	 m_butx, m_yres*0.37, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, this, mm_replay,
	 _("Watch Replay"), std::string(), true, false,
	 m_fn, m_fs),
options
	(this,
	 m_butx, m_yres*0.435, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, this, mm_options,
	 _("Options"), std::string(), true, false,
	 m_fn, m_fs),
editor
	(this,
	 m_butx, m_yres*0.5, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, this, mm_editor,
	 _("Editor"), std::string(), true, false,
	 m_fn, m_fs),
readme
	(this,
	 m_butx, m_yres*0.565, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, this, mm_readme,
	 _("View Readme"), std::string(), true, false,
	 m_fn, m_fs),
license
	(this,
	 m_butx, m_yres*0.63, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, this, mm_license,
	 _("License"), std::string(), true, false,
	 m_fn, m_fs),
exit
	(this,
	 m_butx, m_yres*0.75, m_butw, m_buth,
	 3,
	 &Fullscreen_Menu_Main::end_modal, this, mm_exit,
	 _("Exit Game"), std::string(), true, false,
	 m_fn, m_fs),

// Textlabels
version
	(this,
	 m_xres - 10, m_yres - 24,
	 std::string(_("Version ")) + build_id(), Align_Right),
copyright
	(this,
	 10, m_yres - 24,
	 _("(C) 2002-2008 by the Widelands Development Team"), Align_TopLeft)
{
	version.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	copyright.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
}
