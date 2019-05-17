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

#include "wui/login_box.h"

#include "base/i18n.h"
#include "graphic/font_handler.h"
#include "network/internet_gaming.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/messagebox.h"

LoginBox::LoginBox(Panel& parent)
   : Window(&parent, "login_box", 0, 0, 500, 280, _("Online game Settings")) {
	center_to_parent();

	int32_t margin = 10;

	ta_nickname = new UI::Textarea(this, margin, margin, _("Nickname:"));
	ta_password = new UI::Textarea(this, margin, 70, _("Password:"));
	eb_nickname = new UI::EditBox(this, 150, margin, 330, 20, 2, UI::PanelStyle::kWui);
	eb_password = new UI::EditBox(this, 150, 70, 330, 20, 2, UI::PanelStyle::kWui);

	cb_register = new UI::Checkbox(this, Vector2i(margin, 40), _("Log in to a registered account."),
	                               "", get_inner_w() - 2 * margin);

	register_account = new UI::MultilineTextarea(this, margin, 105, 470, 140, UI::PanelStyle::kWui,
			(boost::format(_("In order to use a registered "
				"account, you need an account on the widelands website. "
				"Please log in at %s and set an online "
				"gaming password on your profile page."))
				% "\n\nhttps://widelands.org/accounts/register/\n\n").str());

	loginbtn = new UI::Button(
	   this, "login",
	   UI::g_fh->fontset()->is_rtl() ? (get_inner_w() / 2 - 200) / 2 :
	                                   (get_inner_w() / 2 - 200) / 2 + get_inner_w() / 2,
	   get_inner_h() - 20 - margin, 200, 20, UI::ButtonStyle::kWuiPrimary, _("Save"));

	cancelbtn = new UI::Button(
	   this, "cancel",
	   UI::g_fh->fontset()->is_rtl() ? (get_inner_w() / 2 - 200) / 2 + get_inner_w() / 2 :
	                                   (get_inner_w() / 2 - 200) / 2,
	   loginbtn->get_y(), 200, 20, UI::ButtonStyle::kWuiSecondary, _("Cancel"));

	loginbtn->sigclicked.connect(boost::bind(&LoginBox::clicked_ok, boost::ref(*this)));
	cancelbtn->sigclicked.connect(boost::bind(&LoginBox::clicked_back, boost::ref(*this)));
	eb_nickname->changed.connect(boost::bind(&LoginBox::change_playername, this));
	cb_register->clickedto.connect(boost::bind(&LoginBox::clicked_register, this));

	Section& s = g_options.pull_section("global");
	eb_nickname->set_text(s.get_string("nickname", _("nobody")));
	cb_register->set_state(s.get_bool("registered", false));
	eb_password->set_password(true);

	if (registered()) {
		eb_password->set_text(s.get_string("password_sha1", ""));
	} else {
		eb_password->set_can_focus(false);
		ta_password->set_color(UI_FONT_CLR_DISABLED);
	}

	eb_nickname->focus();

}

/// think function of the UI (main loop)
void LoginBox::think() {
	verify_input();
}

/**
 * called, if "login" is pressed.
 */
void LoginBox::clicked_ok() {
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
}

/// Called if "cancel" was pressed
void LoginBox::clicked_back() {
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

/// Calles when nickname was changed
void LoginBox::change_playername() {
	cb_register->set_state(false);
	eb_password->set_can_focus(false);
	eb_password->set_text("");
}

bool LoginBox::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
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
	return UI::Panel::handle_key(down, code);
}

void LoginBox::clicked_register() {
	if (cb_register->get_state()) {
		ta_password->set_color(UI_FONT_CLR_DISABLED);
		eb_password->set_can_focus(false);
		eb_password->set_text("");
	} else {
		ta_password->set_color(UI_FONT_CLR_FG);
		eb_password->set_can_focus(true);
	}
}

void LoginBox::verify_input() {
	// Check if all neccessary input fields are valid
	loginbtn->set_enabled(true);
	eb_nickname->set_tooltip("");
	eb_password->set_tooltip("");
	eb_nickname->set_warning(false);

	if (eb_nickname->text().empty()) {
		eb_nickname->set_warning(true);
		eb_nickname->set_tooltip(_("Please enter a nickname!"));
		loginbtn->set_enabled(false);
	} else if (!InternetGaming::ref().valid_username(eb_nickname->text())) {
			eb_nickname->set_warning(true);
			eb_nickname->set_tooltip(_("Enter a valid nickname. This value may contain only "
													  "English letters, numbers, and @ . + - _ characters."));
			loginbtn->set_enabled(false);
	}

	if (eb_password->text().empty() && cb_register->get_state()) {
		eb_password->set_tooltip(_("Please enter your password!"));
		loginbtn->set_enabled(false);
		eb_password->focus();
	}

	Section& s = g_options.pull_section("global");
	if (eb_password->has_focus() && eb_password->text() == s.get_string("password_sha1", "")) {
		eb_password->set_text("");
	}
}
