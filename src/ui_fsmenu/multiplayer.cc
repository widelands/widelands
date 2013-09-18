/*
 * Copyright (C) 2002-2010, 2012 by the Widelands Development Team
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

#include "ui_fsmenu/multiplayer.h"

#include "constants.h"
#include "i18n.h"
#include "profile/profile.h"
#include "wui/login_box.h"

Fullscreen_Menu_MultiPlayer::Fullscreen_Menu_MultiPlayer() :
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
		 _("Choose game type"), UI::Align_HCenter),

// Buttons
	metaserver
		(this, "metaserver",
		 m_butx, get_h() * 6 / 25, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("Internet game"), std::string(), true, false),
	lan
		(this, "lan",
		 m_butx, get_h() * 61 / 200, m_butw, m_buth,
		 g_gr->images().get("pics/but1.png"),
		 _("LAN / Direct IP"), std::string(), true, false),
	back
		(this, "back",
		 m_butx, get_h() * 3 / 4, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false)
{
	metaserver.sigclicked.connect(boost::bind(&Fullscreen_Menu_MultiPlayer::internetLogin, boost::ref(*this)));
	metaserver.set_font(font_small());
	lan.set_font(font_small());
	lan.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_MultiPlayer::end_modal, boost::ref(*this),
			  static_cast<int32_t>(Lan)));
	back.set_font(font_small());
	back.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_MultiPlayer::end_modal, boost::ref(*this),
			  static_cast<int32_t>(Back)));

	title.set_font(m_fn, fs_big(), UI_FONT_CLR_FG);

	Section & s = g_options.pull_section("global");
	m_auto_log = s.get_bool("auto_log", false);
	if (m_auto_log) {
		showloginbox =
			new UI::Button
				(this, "login_dialog",
				 m_butx + m_butw + m_buth / 4, get_h() * 6 / 25, m_buth, m_buth,
				 g_gr->images().get("pics/but1.png"),
				 g_gr->images().get("pics/continue.png"),
				 _("Show login dialog"), true, false);
		showloginbox->sigclicked.connect
			(boost::bind
				(&Fullscreen_Menu_MultiPlayer::showInternetLogin, boost::ref(*this)));
		showloginbox->set_font(font_small());
	}
}


/// called if the showloginbox button was pressed
void Fullscreen_Menu_MultiPlayer::showInternetLogin() {
	m_auto_log = false;
	internetLogin();
}


/**
 * Called if "Internet" button was pressed.
 *
 * IF autologin is not set, a LoginBox is shown and, if the user clicks on
 * 'login' in it's menu, the data is read from the LoginBox and saved in 'this'
 * so wlapplication can read it.
 *
 * IF autologin is set, all data is read from the config file and saved.
 * That data will be used for login to the metaserver.
 *
 * In both cases this fullscreen menu ends it's modality.
 */
void Fullscreen_Menu_MultiPlayer::internetLogin() {
	Section & s = g_options.pull_section("global");
	if (m_auto_log) {
		m_nickname = s.get_string("nickname", _("nobody"));
		m_password = s.get_string("password", "nobody");
		m_register = s.get_bool("registered", false);
		end_modal(Metaserver);
		return;
	}

	LoginBox lb(*this);
	if (lb.run()) {
		m_nickname = lb.get_nickname();
		m_password = lb.get_password();
		m_register = lb.registered();

		s.set_bool("registered", lb.registered());
		s.set_bool("auto_log", lb.set_automaticlog());

		end_modal(Metaserver);
	}
}
