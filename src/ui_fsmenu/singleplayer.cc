/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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

#include "singleplayer.h"

#include "constants.h"
#include "i18n.h"

Fullscreen_Menu_SinglePlayer::Fullscreen_Menu_SinglePlayer() :
Fullscreen_Menu_Base("singleplmenu.jpg"),

// Values for alignment and size
	m_butw (m_xres * 7 / 20),
	m_buth (m_yres * 19 / 400),
	m_butx ((m_xres - m_butw) / 2),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Title
	title
		(this,
		 m_xres / 2, m_yres * 3 / 40,
		 _("Single Player Menu"), UI::Align_HCenter),

// Buttons
	new_game
		(this, "new_game",
		 m_butx, m_yres * 6 / 25, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 boost::bind
			(&Fullscreen_Menu_SinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(New_Game)),
		 _("New Game"), std::string(), true, false,
		 m_fn, m_fs),
	campaign
		(this, "campaigns",
		 m_butx, m_yres * 61 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 boost::bind
			(&Fullscreen_Menu_SinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(Campaign)),
		 _("Campaigns"), std::string(), true, false,
		 m_fn, m_fs),
	load_game
		(this, "load_game",
		 m_butx, m_yres * 87 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 boost::bind
			(&Fullscreen_Menu_SinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(Load_Game)),
		 _("Load Game"), std::string(), true, false,
		 m_fn, m_fs),
	back
		(this, "back",
		 m_butx, m_yres * 3 / 4, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 boost::bind(&Fullscreen_Menu_SinglePlayer::end_modal, boost::ref(*this), static_cast<int32_t>(Back)),
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs)
{
	title.set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
}
