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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "ui_fsmenu/singleplayer.h"

#include "constants.h"
#include "graphic/graphic.h"
#include "i18n.h"

Fullscreen_Menu_SinglePlayer::Fullscreen_Menu_SinglePlayer() :
Fullscreen_Menu_Base("singleplmenu.jpg"),

// Values for alignment and size
	m_butw (get_w() * 7 / 20),
	m_buth (get_h() * 19 / 400),
	m_butx ((get_w() - m_butw) / 2),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Title
	title
		(this,
		 get_w() / 2, get_h() * 3 / 40,
		 _("Single Player"), UI::Align_HCenter),

// Buttons
	new_game
		(this, "new_game",
		 m_butx, get_h() * 6 / 25, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("New Game"), std::string(), true, false),
	campaign
		(this, "campaigns",
		 m_butx, get_h() * 61 / 200, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("Campaigns"), std::string(), true, false),
	load_game
		(this, "load_game",
		 m_butx, get_h() * 87 / 200, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("Load Game"), std::string(), true, false),
	back
		(this, "back",
		 m_butx, get_h() * 3 / 4, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false)
{
	new_game.sigclicked.connect
		(boost::bind
			(&Fullscreen_Menu_SinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(New_Game)));
	campaign.sigclicked.connect
		(boost::bind
			(&Fullscreen_Menu_SinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(Campaign)));
	load_game.sigclicked.connect
		(boost::bind
			(&Fullscreen_Menu_SinglePlayer::end_modal,
			 boost::ref(*this),
			 static_cast<int32_t>(Load_Game)));
	back.sigclicked.connect
		(boost::bind(&Fullscreen_Menu_SinglePlayer::end_modal, boost::ref(*this), static_cast<int32_t>(Back)));

	back.set_font(font_small());
	new_game.set_font(font_small());
	campaign.set_font(font_small());
	load_game.set_font(font_small());

	title.set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
}
