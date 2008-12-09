/*
 * Copyright (C) 2002-2008 by the Widelands Development Team
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

#include "fullscreen_menu_singleplayer.h"

#include "constants.h"
#include "i18n.h"

Fullscreen_Menu_SinglePlayer::Fullscreen_Menu_SinglePlayer() :
Fullscreen_Menu_Base("singleplmenu.jpg"),

// Values for alignment and size
m_xres
	(gr_x()),
m_yres
	(gr_y()),
m_butw
	(m_xres*0.35),
m_buth
	(m_yres*0.0475),
m_butx
	((m_xres-m_butw)/2),
m_fs
	(fs_small()),
m_fn
	(ui_fn()),

// Title
title
	(this,
	 m_xres/2, m_yres*0.075,
	 _("Single Player Menu"), Align_HCenter),

// Buttons
new_game
	(this,
	 m_butx, m_yres*0.24, m_butw, m_buth,
	 1,
	 &Fullscreen_Menu_SinglePlayer::end_modal, this, New_Game,
	 _("New Game"), std::string(), true, false,
	 m_fn, m_fs),
campaign
	(this,
	 m_butx, m_yres*0.305, m_butw, m_buth,
	 1,
	 &Fullscreen_Menu_SinglePlayer::end_modal, this, Campaign,
	 _("Campaigns"), std::string(), true, false,
	 m_fn, m_fs),
load_game
	(this,
	 m_butx, m_yres*0.435, m_butw, m_buth,
	 1,
	 &Fullscreen_Menu_SinglePlayer::end_modal, this, Load_Game,
	 _("Load Game"), std::string(), true, false,
	 m_fn, m_fs),
back
	(this,
	 m_butx, m_yres*0.75, m_butw, m_buth,
	 0,
	 &Fullscreen_Menu_SinglePlayer::end_modal, this, Back,
	 _("Back"), std::string(), true, false,
	 m_fn, m_fs)
{title.set_font(m_fn, fs_big(), UI_FONT_CLR_FG);}
