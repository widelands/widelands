/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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
     title(this, 0, 0, _("Choose game type"), UI::Align::kTopCenter),

     // Buttons
     metaserver(&vbox_,
                "metaserver",
                0, 0, butw_, buth_,
                g_gr->images().get(button_background_),
                _("Internet game")),
     showloginbox(nullptr),
     lan(&vbox_,
         "lan",
         0, 0, butw_, buth_,
         g_gr->images().get(button_background_),
         _("LAN / Direct IP")),
     back(&vbox_, "back", 0, 0, butw_, buth_, g_gr->images().get(button_background_), _("Back")) {
	metaserver.sigclicked.connect(
	   boost::bind(&FullscreenMenuMultiPlayer::internet_login, boost::ref(*this)));

	lan.sigclicked.connect(
	   boost::bind(&FullscreenMenuMultiPlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kLan));

	back.sigclicked.connect(
	   boost::bind(&FullscreenMenuMultiPlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kBack));

	title.set_fontsize(fs_big());

	vbox_.add(&metaserver, UI::HAlign::kHCenter, true);
	vbox_.add(&lan, UI::HAlign::kHCenter, true);
	vbox_.add_inf_space();
	vbox_.add(&back, UI::HAlign::kHCenter, true);

	Section& s = g_options.pull_section("global");
	auto_log_ = s.get_bool("auto_log", false);
	if (auto_log_) {
		showloginbox = new UI::Button(
		   this, "login_dialog", 0, 0, 0, 0, g_gr->images().get("images/ui_basic/but1.png"),
		   g_gr->images().get("images/ui_basic/continue.png"), _("Show login dialog"));
		showloginbox->sigclicked.connect(
		   boost::bind(&FullscreenMenuMultiPlayer::show_internet_login, boost::ref(*this)));
	}
	layout();
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

void FullscreenMenuMultiPlayer::layout() {
	title.set_size(get_w(), title.get_h());
	FullscreenMenuMainMenu::layout();

	title.set_pos(Vector2i(0, title_y_));

	metaserver.set_size(butw_, buth_);
	if (showloginbox) {
		showloginbox->set_pos(Vector2i(box_x_ + butw_ + padding_ / 2, box_y_));
		showloginbox->set_size(buth_, buth_);
	}
	metaserver.set_desired_size(butw_, buth_);
	lan.set_desired_size(butw_, buth_);
	back.set_desired_size(butw_, buth_);

	vbox_.set_pos(Vector2i(box_x_, box_y_));
	vbox_.set_inner_spacing(padding_);
	vbox_.set_size(butw_, get_h() - vbox_.get_y() - 3 * title_y_);
}
