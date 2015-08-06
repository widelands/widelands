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

#include "base/i18n.h"
#include "graphic/text_constants.h"
#include "profile/profile.h"
#include "wui/login_box.h"

FullscreenMenuMultiPlayer::FullscreenMenuMultiPlayer() :
	FullscreenMenuMainMenu(),

// Title
	title
		(this,
		 get_w() / 2, m_title_y,
		 _("Choose game type"), UI::Align_HCenter),

// Buttons
	vbox(this, m_box_x, m_box_y, UI::Box::Vertical,
		  m_butw, get_h() - m_box_y, m_padding),
	metaserver
		(&vbox, "metaserver", 0, 0, m_butw, m_buth, g_gr->images().get(m_button_background),
		 _("Internet game"), "", true, false),
	lan
		(&vbox, "lan", 0, 0, m_butw, m_buth, g_gr->images().get(m_button_background),
		 _("LAN / Direct IP"), "", true, false),
	back
		(&vbox, "back", 0, 0, m_butw, m_buth, g_gr->images().get(m_button_background),
		 _("Back"), "", true, false)
{
	metaserver.sigclicked.connect(boost::bind(&FullscreenMenuMultiPlayer::internet_login, boost::ref(*this)));

	lan.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMultiPlayer::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
			  FullscreenMenuBase::MenuTarget::kLan));

	back.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuMultiPlayer::end_modal<FullscreenMenuBase::MenuTarget>, boost::ref(*this),
			  FullscreenMenuBase::MenuTarget::kBack));

	title.set_font(ui_fn(), fs_big(), UI_FONT_CLR_FG);

	vbox.add(&metaserver, UI::Box::AlignCenter);
	vbox.add(&lan, UI::Box::AlignCenter);

	// Multiple add_space calls to get the same height for the back button as in the single player menu
	vbox.add_space(m_buth);
	vbox.add_space(m_buth);
	vbox.add_space(6 * m_buth);

	vbox.add(&back, UI::Box::AlignCenter);

	vbox.set_size(m_butw, get_h() - vbox.get_y());

	Section & s = g_options.pull_section("global");
	m_auto_log = s.get_bool("auto_log", false);
	if (m_auto_log) {
		showloginbox =
			new UI::Button
				(this, "login_dialog",
				 m_box_x + m_butw + m_buth / 4, get_h() * 6 / 25, m_buth, m_buth,
				 g_gr->images().get("pics/but1.png"),
				 g_gr->images().get("pics/continue.png"),
				 _("Show login dialog"), true, false);
		showloginbox->sigclicked.connect
			(boost::bind
				(&FullscreenMenuMultiPlayer::show_internet_login, boost::ref(*this)));
	}
}


/// called if the showloginbox button was pressed
void FullscreenMenuMultiPlayer::show_internet_login() {
	m_auto_log = false;
	internet_login();
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
void FullscreenMenuMultiPlayer::internet_login() {
	Section & s = g_options.pull_section("global");
	if (m_auto_log) {
		m_nickname = s.get_string("nickname", _("nobody"));
		m_password = s.get_string("password", "nobody");
		m_register = s.get_bool("registered", false);
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kMetaserver);
		return;
	}

	LoginBox lb(*this);
	if (lb.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
		m_nickname = lb.get_nickname();
		m_password = lb.get_password();
		m_register = lb.registered();

		s.set_bool("registered", lb.registered());
		s.set_bool("auto_log", lb.set_automaticlog());

		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kMetaserver);
	}
}

void FullscreenMenuMultiPlayer::clicked_ok() {
	internet_login();
}
