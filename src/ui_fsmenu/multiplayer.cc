/*
 * Copyright (C) 2002-2010 by the Widelands Development Team
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

#include "multiplayer.h"

#if HAVE_GGZ

#include "constants.h"
#include "i18n.h"
#include "profile/profile.h"
#include "wui/login_box.h"

Fullscreen_Menu_MultiPlayer::Fullscreen_Menu_MultiPlayer() :
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
		 _("Choose game type"), UI::Align_HCenter),

// Buttons
	metaserver
		(this,
		 m_butx, m_yres * 6 / 25, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Fullscreen_Menu_MultiPlayer::ggzLogin, *this,
		 _("Internet game"), std::string(), true, false,
		 m_fn, m_fs),
	lan
		(this,
		 m_butx, m_yres * 61 / 200, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &Fullscreen_Menu_MultiPlayer::end_modal, *this, Lan,
		 _("LAN / Direct IP"), std::string(), true, false,
		 m_fn, m_fs),
	back
		(this,
		 m_butx, m_yres * 3 / 4, m_butw, m_buth,
		 g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &Fullscreen_Menu_MultiPlayer::end_modal, *this, Back,
		 _("Back"), std::string(), true, false,
		 m_fn, m_fs)
{
	title.set_font(m_fn, fs_big(), UI_FONT_CLR_FG);

	Section & s = g_options.pull_section("global");
	m_auto_log = s.get_bool("auto_log", false);
	if (m_auto_log)
		showloginbox =
			new UI::Callback_Button<Fullscreen_Menu_MultiPlayer>
				(this,
				 m_butx + m_butw + m_buth / 4, m_yres * 6 / 25, m_buth, m_buth,
				 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
				 g_gr->get_picture(PicMod_UI, "pics/continue.png"),
				 &Fullscreen_Menu_MultiPlayer::showGGZLogin, *this,
				 _("Show login dialog"), true, false,
				 m_fn, m_fs);
}


/// called if the showloginbox button was pressed
void Fullscreen_Menu_MultiPlayer::showGGZLogin() {
	m_auto_log = false;
	ggzLogin();
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
void Fullscreen_Menu_MultiPlayer::ggzLogin() {
	Section & s = g_options.pull_section("global");
	if (m_auto_log) {
		m_nickname = s.get_string("nickname", _("nobody"));
		m_password = s.get_string("password", "nobody");
		m_email    = s.get_string("emailadd", "nobody@nobody.nob");
		m_register = false;
		end_modal(Metaserver);
		return;
	}

	LoginBox lb(*this);
	if (lb.run()) {
		m_nickname = lb.get_nickname();
		m_password = lb.get_password();
		m_email    = lb.get_email();
		m_register = lb.new_registration();

		s.set_bool("auto_log", lb.set_automaticlog());

		end_modal(Metaserver);
	}
}

#endif // if HAVE_GGZ
