/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "ui_fsmenu/login_box.h"

#include "base/i18n.h"
#include "graphic/font_handler.h"
#include "network/internet_gaming.h"
#include "network/internet_gaming_protocol.h"
#include "third_party/sha1/sha1.h"
#include "ui_basic/button.h"
#include "ui_basic/messagebox.h"
#include "ui_fsmenu/main.h"
#include "wlapplication_options.h"

namespace FsMenu {

constexpr int8_t kMargin = 6;

LoginBox::LoginBox(MainMenu& parent, UI::UniqueWindow::Registry& r)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kFsMenu, "login_box", &r, 500, 290, _("Online Game Settings")),
     fsmm_(parent),
     main_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     hbox_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     buttons_box_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     vbox1_(&hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     vbox2_(&hbox_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     b_login_(&buttons_box_, "login", 0, 0, 200, 28, UI::ButtonStyle::kFsMenuPrimary, _("Save")),
     b_cancel_(
        &buttons_box_, "cancel", 0, 0, 200, 28, UI::ButtonStyle::kFsMenuSecondary, _("Cancel")),
     eb_nickname_(&vbox2_, 0, 0, 330, UI::PanelStyle::kFsMenu),
     eb_password_(&vbox2_, 0, 0, 330, UI::PanelStyle::kFsMenu),
     cb_register_(
        &main_box_, UI::PanelStyle::kFsMenu, Vector2i(0, 0), _("Log in to a registered account.")),
     ta_nickname_(&vbox1_, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuLabel, _("Nickname:")),
     ta_password_(&vbox1_, UI::PanelStyle::kFsMenu, UI::FontStyle::kFsMenuLabel, _("Password:")),
     register_account_(&main_box_,
                       0,
                       0,
                       0,
                       180,
                       UI::PanelStyle::kFsMenu,
                       format(_("In order to use a registered "
                                "account, you need an account on the Widelands website. "
                                "Please log in at %s and set an online "
                                "gaming password on your profile page."),
                              "\n\nhttps://widelands.org/accounts/register/\n\n")) {
	vbox1_.add_space(kMargin);
	vbox1_.add(&ta_nickname_, UI::Box::Resizing::kExpandBoth);
	vbox1_.add_space(kMargin);
	vbox1_.add(&ta_password_, UI::Box::Resizing::kExpandBoth);
	vbox1_.add_space(kMargin);

	vbox2_.add_space(kMargin);
	vbox2_.add(&eb_nickname_, UI::Box::Resizing::kExpandBoth);
	vbox2_.add_space(kMargin);
	vbox2_.add(&eb_password_, UI::Box::Resizing::kExpandBoth);
	vbox2_.add_space(kMargin);

	hbox_.add_space(kMargin);
	hbox_.add(&vbox1_, UI::Box::Resizing::kExpandBoth);
	hbox_.add_space(kMargin);
	hbox_.add(&vbox2_, UI::Box::Resizing::kExpandBoth);
	hbox_.add_space(kMargin);

	buttons_box_.add_space(kMargin);
	buttons_box_.add(
	   UI::g_fh->fontset()->is_rtl() ? &b_login_ : &b_cancel_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kMargin);
	buttons_box_.add(
	   UI::g_fh->fontset()->is_rtl() ? &b_cancel_ : &b_login_, UI::Box::Resizing::kExpandBoth);
	buttons_box_.add_space(kMargin);

	main_box_.add_space(kMargin);
	main_box_.add(&hbox_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kMargin);
	main_box_.add(&cb_register_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(kMargin);
	main_box_.add(&register_account_, UI::Box::Resizing::kFullSize);
	main_box_.add(&buttons_box_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(kMargin);

	set_center_panel(&main_box_);
	center_to_parent();

	b_login_.sigclicked.connect([this]() { clicked_ok(); });
	b_cancel_.sigclicked.connect([this]() { clicked_back(); });
	eb_nickname_.changed.connect([this]() { change_playername(); });
	cb_register_.clickedto.connect([this](bool) { clicked_register(); });

	eb_nickname_.set_text(get_config_string("nickname", _("nobody")));
	cb_register_.set_state(get_config_bool("registered", false));
	eb_password_.set_password(true);

	if (registered()) {
		eb_password_.set_text(get_config_string("password_sha1", ""));
		b_login_.set_enabled(false);
	} else {
		eb_password_.set_can_focus(false);
		ta_password_.set_style(UI::FontStyle::kDisabled);
	}

	eb_nickname_.focus();

	eb_nickname_.cancel.connect([this]() { clicked_back(); });
	eb_password_.cancel.connect([this]() { clicked_back(); });
	eb_nickname_.ok.connect([this]() { clicked_ok(); });
	eb_password_.ok.connect([this]() { clicked_ok(); });

	initialization_complete();
}

/// think function of the UI (main loop)
void LoginBox::think() {
	verify_input();
}

/**
 * called, if "login" is pressed.
 */
void LoginBox::clicked_ok() {
	if (cb_register_.get_state()) {
		if (check_password()) {
			set_config_string("nickname", eb_nickname_.text());
			set_config_bool("registered", true);
			fsmm_.internet_login_callback();
			die();
		}
	} else {
		set_config_string("nickname", eb_nickname_.text());
		set_config_bool("registered", false);
		set_config_string("password_sha1", "");
		fsmm_.internet_login_callback();
		die();
	}
}

/// Called if "cancel" was pressed
void LoginBox::clicked_back() {
	die();
}

/// Called when nickname was changed
void LoginBox::change_playername() {
	cb_register_.set_state(false);
	eb_password_.set_can_focus(false);
	eb_password_.set_text("");
}

bool LoginBox::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			clicked_ok();
			return true;
		case SDLK_ESCAPE:
			clicked_back();
			return true;
		default:
			break;  // not handled
		}
	}
	return UI::UniqueWindow::handle_key(down, code);
}

void LoginBox::clicked_register() {
	if (cb_register_.get_state()) {
		ta_password_.set_style(UI::FontStyle::kDisabled);
		eb_password_.set_can_focus(false);
		eb_password_.set_text("");
	} else {
		ta_password_.set_style(UI::FontStyle::kFsMenuLabel);
		eb_password_.set_can_focus(true);
		eb_password_.focus();
	}
}

void LoginBox::verify_input() {
	// Check if all neccessary input fields are valid
	b_login_.set_enabled(true);
	eb_nickname_.set_tooltip("");
	eb_password_.set_tooltip("");
	eb_nickname_.set_warning(false);

	if (eb_nickname_.text().empty()) {
		eb_nickname_.set_warning(true);
		eb_nickname_.set_tooltip(_("Please enter a nickname!"));
		b_login_.set_enabled(false);
	} else if (!InternetGaming::ref().valid_username(eb_nickname_.text())) {
		eb_nickname_.set_warning(true);
		eb_nickname_.set_tooltip(_("Enter a valid nickname. This value may contain only "
		                           "English letters, numbers, and @ . + - _ characters."));
		b_login_.set_enabled(false);
	}

	if (eb_password_.text().empty() && cb_register_.get_state()) {
		eb_password_.set_tooltip(_("Please enter your password!"));
		b_login_.set_enabled(false);
	}

	if (eb_password_.has_focus() && eb_password_.text() == get_config_string("password_sha1", "")) {
		eb_password_.set_text("");
	}

	if (cb_register_.get_state() && eb_password_.text() == get_config_string("password_sha1", "")) {
		b_login_.set_enabled(false);
	}
}

/// Check password against metaserver
bool LoginBox::check_password() {
	// Try to connect to the metaserver
	const std::string& meta = get_config_string("metaserver", INTERNET_GAMING_METASERVER);
	uint32_t port = get_config_natural("metaserverport", kInternetGamingPort);
	std::string password = crypto::sha1(eb_password_.text());

	if (!InternetGaming::ref().check_password(get_nickname(), password, meta, port)) {
		// something went wrong -> show the error message
		// idealy it is about the wrong password
		ChatMessage msg = InternetGaming::ref().get_messages().back();
		UI::WLMessageBox wmb(
		   this, UI::WindowStyle::kFsMenu, _("Error!"), msg.msg, UI::WLMessageBox::MBoxType::kOk);
		wmb.run<UI::Panel::Returncodes>();
		eb_password_.set_text("");
		eb_password_.focus();
		return false;
	}
	// NOTE: The password is only stored (in memory and on disk) and transmitted (over the network to
	// the metaserver) as cryptographic hash. This does NOT mean that the password is stored securely
	// on the local disk. While the password should be secure while transmitted to the metaserver
	// (no-one can use the transmitted data to log in as the user) this is not the case for local
	// storage. The stored hash of the password makes it hard to look at the configuration file and
	// figure out the plaintext password to, e.g., log in on the forum. However, the stored hash can
	// be copied to another system and used to log in as the user on the metaserver. Further note:
	// SHA-1 is considered broken and shouldn't be used anymore. But since the passwords on the
	// server are protected by SHA-1 we have to use it here, too
	set_config_string("password_sha1", password);
	InternetGaming::ref().logout();
	return true;
}

}  // namespace FsMenu
