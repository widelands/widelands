/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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
#include "base/random.h"
#include "network/internet_gaming.h"
#include "network/internet_gaming_protocol.h"
#include "ui_basic/messagebox.h"
#include "wlapplication_options.h"
#include "wui/login_box.h"

FullscreenMenuMultiPlayer::FullscreenMenuMultiPlayer()
   : FullscreenMenuMainMenu(),

     // Title
     title(this,
           0,
           0,
           0,
           0,
           _("Choose game type"),
           UI::Align::kCenter,
           g_style_manager->font_style(UI::FontStyle::kFsMenuTitle)),

     // Buttons
     metaserver(
        &vbox_, "metaserver", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Online Game")),
     lan(&vbox_, "lan", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("LAN / Direct IP")),
     showloginbox(
        &vbox_, "lan", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Online Game Settings")),
     back(&vbox_, "back", 0, 0, butw_, buth_, UI::ButtonStyle::kFsMenuMenu, _("Back")) {
	metaserver.sigclicked.connect([this]() { internet_login(); });

	lan.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kLan);
	});

	showloginbox.sigclicked.connect([this]() { show_internet_login(); });

	back.sigclicked.connect([this]() {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kBack);
	});

	title.set_font_scale(scale_factor());

	vbox_.add(&metaserver, UI::Box::Resizing::kFullSize);
	vbox_.add(&showloginbox, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add(&lan, UI::Box::Resizing::kFullSize);
	vbox_.add_inf_space();
	vbox_.add_inf_space();
	vbox_.add_inf_space();
	vbox_.add(&back, UI::Box::Resizing::kFullSize);

	auto_log_ = false;
	layout();
}

/// called if the user is not registered
void FullscreenMenuMultiPlayer::show_internet_login() {
	LoginBox lb(*this);
	if (lb.run<UI::Panel::Returncodes>() == UI::Panel::Returncodes::kOk && auto_log_) {
		auto_log_ = false;
		internet_login();
	}
}

/**
 * Called if "Online Game" button was pressed.
 *
 * IF no nickname or a nickname with invalid characters is set, the Online Game Settings
 * are opened.
 *
 * IF at least a name is set, all data is read from the config file
 *
 * This fullscreen menu ends it's modality.
 */
void FullscreenMenuMultiPlayer::internet_login() {
	nickname_ = get_config_string("nickname", "");
	password_ = get_config_string("password_sha1", "no_password_set");
	register_ = get_config_bool("registered", false);

	// Checks can be done directly in editbox' by using valid_username().
	// This is just to be on the safe side, in case the user changed the password in the config file.
	if (!InternetGaming::ref().valid_username(nickname_)) {
		auto_log_ = true;
		show_internet_login();
		return;
	}

	// Try to connect to the metaserver
	const std::string& meta = get_config_string("metaserver", INTERNET_GAMING_METASERVER);
	uint32_t port = get_config_natural("metaserverport", kInternetGamingPort);
	const std::string& auth = register_ ? password_ : get_config_string("uuid", "");
	assert(!auth.empty() || !register_);
	InternetGaming::ref().login(nickname_, auth, register_, meta, port);

	// Check whether metaserver send some data
	if (InternetGaming::ref().logged_in()) {
		end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kMetaserver);
	} else {
		// something went wrong -> show the error message
		ChatMessage msg = InternetGaming::ref().get_messages().back();
		UI::WLMessageBox wmb(this, _("Error!"), msg.msg, UI::WLMessageBox::MBoxType::kOk);
		wmb.run<UI::Panel::Returncodes>();

		// Reset InternetGaming and passwort and show internet login again
		InternetGaming::ref().reset();
		set_config_string("password_sha1", "no_password_set");
		auto_log_ = true;
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

	metaserver.set_desired_size(butw_, buth_);
	showloginbox.set_desired_size(butw_, buth_);
	lan.set_desired_size(butw_, buth_);
	back.set_desired_size(butw_, buth_);

	vbox_.set_pos(Vector2i(box_x_, box_y_));
	vbox_.set_inner_spacing(padding_);
	vbox_.set_size(butw_, get_h() - vbox_.get_y() - 3 * title_y_);
}
