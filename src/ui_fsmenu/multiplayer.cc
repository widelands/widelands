/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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
#include "network/internet_gaming.h"
#include "profile/profile.h"
#include "ui_basic/messagebox.h"
#include "wui/login_box.h"

FullscreenMenuMultiPlayer::FullscreenMenuMultiPlayer()
   : FullscreenMenuMainMenu(),

     // Title
     title(this, get_w() / 2, title_y_, _("Choose game type"), UI::Align::kHCenter),

     // Buttons
     vbox(this, box_x_, box_y_, UI::Box::Vertical, butw_, get_h() - box_y_, padding_),
     metaserver(&vbox,
                "metaserver",
                0,
                0,
                butw_,
                buth_,
                g_gr->images().get(button_background_),
					 _("Internet game")),
     lan(&vbox,
         "lan",
         0,
         0,
         butw_,
         buth_,
         g_gr->images().get(button_background_),
			_("LAN / Direct IP")),
     back(&vbox,
          "back",
          0,
          0,
          butw_,
          buth_,
          g_gr->images().get(button_background_),
			 _("Back")) {
	metaserver.sigclicked.connect(
	   boost::bind(&FullscreenMenuMultiPlayer::internet_login, boost::ref(*this)));

	lan.sigclicked.connect(
	   boost::bind(&FullscreenMenuMultiPlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kLan));

	back.sigclicked.connect(
	   boost::bind(&FullscreenMenuMultiPlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kBack));

	title.set_fontsize(fs_big());

	vbox.add(&metaserver, UI::Align::kHCenter);
	vbox.add(&lan, UI::Align::kHCenter);

	// Multiple add_space calls to get the same height for the back button as in the single player
	// menu
	vbox.add_space(buth_);
	vbox.add_space(buth_);
	vbox.add_space(6 * buth_);

	vbox.add(&back, UI::Align::kHCenter);

	vbox.set_size(butw_, get_h() - vbox.get_y());

	Section& s = g_options.pull_section("global");
	auto_log_ = s.get_bool("auto_log", false);
	if (auto_log_) {
		showloginbox = new UI::Button(
		   this, "login_dialog", box_x_ + butw_ + buth_ / 4, get_h() * 6 / 25, buth_, buth_,
		   g_gr->images().get("images/ui_basic/but1.png"),
			g_gr->images().get("images/ui_basic/continue.png"), _("Show login dialog"));
		showloginbox->sigclicked.connect(
		   boost::bind(&FullscreenMenuMultiPlayer::show_internet_login, boost::ref(*this)));
	}
}

/// called if the showloginbox button was pressed
void FullscreenMenuMultiPlayer::show_internet_login() {
	auto_log_ = false;
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
	Section& s = g_options.pull_section("global");
	if (auto_log_) {
		nickname_ = s.get_string("nickname", _("nobody"));
		password_ = s.get_string("password", "nobody");
		register_ = s.get_bool("registered", false);
	} else {
		LoginBox lb(*this);
		if (lb.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
			nickname_ = lb.get_nickname();
			password_ = lb.get_password();
			register_ = lb.registered();

			s.set_bool("registered", lb.registered());
			s.set_bool("auto_log", lb.set_automaticlog());
		} else {
			return;
		}
	}

	// Try to connect to the metaserver
	const std::string& meta = s.get_string("metaserver", INTERNET_GAMING_METASERVER.c_str());
	uint32_t port = s.get_natural("metaserverport", INTERNET_GAMING_PORT);
	InternetGaming::ref().login(nickname_, password_, register_, meta, port);

	// Check whether metaserver send some data
	if (InternetGaming::ref().logged_in())
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kMetaserver);
	else {
		// something went wrong -> show the error message
		ChatMessage msg = InternetGaming::ref().get_messages().back();
		UI::WLMessageBox wmb(this, _("Error!"), msg.msg, UI::WLMessageBox::MBoxType::kOk);
		wmb.run<UI::Panel::Returncodes>();

		// Reset InternetGaming and passwort and show internet login again
		InternetGaming::ref().reset();
		s.set_string("password", "");
		show_internet_login();
	}
}

void FullscreenMenuMultiPlayer::clicked_ok() {
	internet_login();
}
