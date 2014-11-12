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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "ui_fsmenu/main.h"

#include <boost/format.hpp>

#include "base/i18n.h"
#include "build_info.h"
#include "graphic/graphic.h"

FullscreenMenuMain::FullscreenMenuMain() :
	FullscreenMenuMainMenu("mainmenu.jpg"),

// Buttons
	playtutorial
		(this, "play_tutorial",
		 m_butx, get_h() * 42 / 200, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Play Tutorial"), "", true, false),
	singleplayer
		(this, "single_player",
		 m_butx, get_y_from_preceding(playtutorial) + m_buth, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Single Player"), "", true, false),
	multiplayer
		(this, "multi_player",
		 m_butx, get_y_from_preceding(singleplayer) + m_padding, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Multiplayer"), "", true, false),
	replay
		(this, "replay",
		 m_butx, get_y_from_preceding(multiplayer) + m_padding, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Watch Replay"), "", true, false),
	editor
		(this, "editor",
		 m_butx, get_y_from_preceding(replay) + m_padding, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Editor"), "", true, false),
	options
		(this, "options",
		 m_butx, get_y_from_preceding(editor) + m_buth, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Options"), "", true, false),
	readme
		(this, "readme",
		 m_butx, get_y_from_preceding(options) + m_buth, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("View Readme"), "", true, false),
	license
		(this, "license",
		 m_butx, get_y_from_preceding(readme) + m_padding, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("License"), "", true, false),
	authors
		(this, "authors",
		 m_butx, get_y_from_preceding(license) + m_padding, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Authors"), "", true, false),
	exit
		(this, "exit",
		 m_butx, get_y_from_preceding(authors) + m_buth, m_butw, m_buth,
		 g_gr->images().get(m_button_background),
		 _("Exit Widelands"), "", true, false),

// Textlabels
	version
		(this,
		 get_w(), get_h(),
		 /** TRANSLATORS: %1$s = version string, %2%s = "Debug" or "Release" */
		 (boost::format(_("Version %1$s (%2$s)")) % build_id().c_str() % build_type().c_str()).str(),
		 UI::Align_BottomRight),
	copyright
		(this,
		 0, get_h() - 0.5 * m_buth,
		 /** TRANSLATORS: Placeholders are the copyright years */
		 (boost::format(_("(C) %1%-%2% by the Widelands Development Team"))
		  % kWidelandsCopyrightStart % kWidelandsCopyrightEnd).str(),
		 UI::Align_BottomLeft),
	gpl
		(this,
		 0, get_h(),
		 _("Licensed under the GNU General Public License V2.0"),
		 UI::Align_BottomLeft)
{
	playtutorial.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kTutorial)));
	singleplayer.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kSinglePlayer)));
	multiplayer.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kMultiplayer)));
	replay.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kReplay)));
	editor.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kEditor)));
	options.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kOptions)));
	readme.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kReadme)));
	license.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kLicense)));
	authors.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kAuthors)));
	exit.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMain::end_modal, boost::ref(*this),
			  static_cast<int32_t>(MenuTarget::kExit)));
}
