/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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
#include "network/crypto.h"
#include "network/internet_gaming.h"
#include "profile/profile.h"
#include "random/random.h"
#include "ui_basic/messagebox.h"
#include "wui/login_box.h"

FullscreenMenuMultiPlayer::FullscreenMenuMultiPlayer()
   : FullscreenMenuMainMenu(),

     // Title
     title(this, 0, 0, _("Choose game type"), UI::Align::kCenter),

     // Buttons
     metaserver(
        &vbox_, "metaserver", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Internet game")),
     showloginbox(nullptr),
     lan(&vbox_, "lan", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("LAN / Direct IP")),
     back(&vbox_, "back", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Back")) {
	metaserver.sigclicked.connect(
	   boost::bind(&FullscreenMenuMultiPlayer::internet_login, boost::ref(*this)));

	lan.sigclicked.connect(
	   boost::bind(&FullscreenMenuMultiPlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kLan));

	back.sigclicked.connect(
	   boost::bind(&FullscreenMenuMultiPlayer::end_modal<FullscreenMenuBase::MenuTarget>,
	               boost::ref(*this), FullscreenMenuBase::MenuTarget::kBack));

	title.set_fontsize(fs_big());

	vbox_.add(&metaserver, UI::Box::Resizing::kFullSize);
	vbox_.add(&lan, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add(&back, UI::Box::Resizing::kFullSize);

	Section& s = g_options.pull_section("global");
	auto_log_ = s.get_bool("auto_log", false);
	if (auto_log_) {
		showloginbox =
		   new UI::Button(this, "login_dialog", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary,
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
		password_ = s.get_string("password_sha1", "nobody");
		register_ = s.get_bool("registered", false);
	} else {
		LoginBox lb(*this);
		if (lb.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk) {
			nickname_ = lb.get_nickname();
			/// NOTE: The password is only stored (in memory and on disk) and transmitted (over the
			/// network
			/// to the metaserver) as cryptographic hash. This does NOT mean that the password is
			/// stored
			/// securely on the local disk. While the password should be secure while transmitted to
			/// the
			/// metaserver (no-one can use the transmitted data to log in as the user) this is not the
			/// case
			/// for local storage. The stored hash of the password makes it hard to look at the
			/// configuration
			/// file and figure out the plaintext password to, e.g., log in on the forum. However, the
			/// stored hash can be copied to another system and used to log in as the user on the
			/// metaserver.
			// Further note: SHA-1 is considered broken and shouldn't be used anymore. But since the
			// passwords on the server are protected by SHA-1 we have to use it here, too
			password_ = crypto::sha1(lb.get_password());
			register_ = lb.registered();

			s.set_bool("registered", lb.registered());
			s.set_bool("auto_log", lb.set_automaticlog());
		} else {
			return;
		}
	}

	// Try to connect to the metaserver
	const std::string& meta = s.get_string("metaserver", INTERNET_GAMING_METASERVER.c_str());
	uint32_t port = s.get_natural("metaserverport", kInternetGamingPort);
	std::string auth = register_ ? password_ : s.get_string("uuid");
	assert(!auth.empty());
	InternetGaming::ref().login(nickname_, auth, register_, meta, port);

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
		s.set_string("password_sha1", "");
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
