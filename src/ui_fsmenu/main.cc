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

#include "main.h"

#include "build_info.h"
#include "i18n.h"

Fullscreen_Menu_Main::Fullscreen_Menu_Main() :
	Fullscreen_Menu_Base("mainmenu.jpg"),

// Values for alignment and size
	m_butx (get_w() * 13 / 40),
	m_butw (get_w() * 7 / 20),
	m_buth (get_h() * 19 / 400),
	wlcr   (WLCR),

// Buttons
	playtutorial
		(this, "play_tutorial",
		 m_butx, get_h() * 42 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but3.png"),
		 boost::bind
			 (&Fullscreen_Menu_Main::end_modal, boost::ref(*this),
			  static_cast<int32_t>(mm_playtutorial)),
		 _("Play Tutorial"), std::string(), true, false),
	singleplayer
		(this, "single_player",
		 m_butx, get_h() * 61 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but3.png"),
		 boost::bind
			 (&Fullscreen_Menu_Main::end_modal, boost::ref(*this),
			  static_cast<int32_t>(mm_singleplayer)),
		 _("Single Player"), std::string(), true, false),
	multiplayer
		(this, "multi_player",
		 m_butx, get_h() * 37 / 100, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but3.png"),
		 boost::bind
			 (&Fullscreen_Menu_Main::end_modal, boost::ref(*this),
			  static_cast<int32_t>(mm_multiplayer)),
		 _("Multi Player"), std::string(), true, false),
	replay
		(this, "replay",
		 m_butx, get_h() * 87 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but3.png"),
		 boost::bind
			 (&Fullscreen_Menu_Main::end_modal, boost::ref(*this),
			  static_cast<int32_t>(mm_replay)),
		 _("Watch Replay"), std::string(), true, false),
	editor
		(this, "editor",
		 m_butx, get_h() * 100 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but3.png"),
		 boost::bind
			 (&Fullscreen_Menu_Main::end_modal, boost::ref(*this),
			  static_cast<int32_t>(mm_editor)),
		 _("Editor"), std::string(), true, false),
	options
		(this, "options",
		 m_butx, get_h() * 119 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but3.png"),
		 boost::bind
			 (&Fullscreen_Menu_Main::end_modal, boost::ref(*this),
			  static_cast<int32_t>(mm_options)),
		 _("Options"), std::string(), true, false),
	readme
		(this, "readme",
		 m_butx, get_h() * 138 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but3.png"),
		 boost::bind
			 (&Fullscreen_Menu_Main::end_modal, boost::ref(*this),
			  static_cast<int32_t>(mm_readme)),
		 _("View Readme"), std::string(), true, false),
	license
		(this, "license",
		 m_butx, get_h() * 151 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but3.png"),
		 boost::bind
			 (&Fullscreen_Menu_Main::end_modal, boost::ref(*this),
			  static_cast<int32_t>(mm_license)),
		 _("License"), std::string(), true, false),
	exit
		(this, "exit",
		 m_butx, get_h() * 178 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but3.png"),
		 boost::bind
			 (&Fullscreen_Menu_Main::end_modal, boost::ref(*this),
			  static_cast<int32_t>(mm_exit)),
		 _("Exit Game"), std::string(), true, false),

// Textlabels
	version
		(this,
		 get_w(), get_h(),
		 _("Version ") + build_id() + '(' + build_type() + ')',
		 UI::Align_BottomRight),
	copyright
		(this,
		 0, get_h(),
		 (wlcr + _("by the Widelands Development Team")).c_str(),
		 UI::Align_BottomLeft)
{
	playtutorial.set_font(font_small());
	singleplayer.set_font(font_small());
	multiplayer.set_font(font_small());
	replay.set_font(font_small());
	editor.set_font(font_small());
	options.set_font(font_small());
	readme.set_font(font_small());
	license.set_font(font_small());
	exit.set_font(font_small());

	version.set_textstyle(ts_small());
	copyright.set_textstyle(ts_small());
}
