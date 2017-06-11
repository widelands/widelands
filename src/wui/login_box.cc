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

#include "wui/login_box.h"

#include "base/i18n.h"
#include "graphic/font_handler1.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/messagebox.h"

LoginBox::LoginBox(Panel& parent)
   : Window(&parent, "login_box", 0, 0, 500, 220, _("Metaserver login")) {
	center_to_parent();

	int32_t margin = 10;

	ta_nickname = new UI::Textarea(this, margin, margin, _("Nickname:"));
	ta_password = new UI::Textarea(this, margin, 40, _("Password:"));
	eb_nickname = new UI::EditBox(
	   this, 150, margin, 330, 20, 2, Panel::Style::kWui);
	eb_password =
	   new UI::EditBox(this, 150, 40, 330, 20, 2, Panel::Style::kWui);

	pwd_warning = new UI::MultilineTextarea(
	   this, margin, 65, 505, 50, UI::Panel::Style::kWui, _("WARNING: Password will be shown and saved readable!"));

	cb_register = new UI::Checkbox(this, Vector2i(margin, 110), _("Log in to a registered account"),
	                               "", get_inner_w() - 2 * margin);
	cb_auto_log = new UI::Checkbox(this, Vector2i(margin, 135),
	                               _("Automatically use this login information from now on."), "",
	                               get_inner_w() - 2 * margin);

	UI::Button* loginbtn =
	   new UI::Button(this, "login", UI::g_fh1->fontset()->is_rtl() ?
	                                    (get_inner_w() / 2 - 200) / 2 :
	                                    (get_inner_w() / 2 - 200) / 2 + get_inner_w() / 2,
	                  get_inner_h() - 20 - margin, 200, 20,
	                  UI::Button::Style::kWuiPrimary, _("Login"));
	loginbtn->sigclicked.connect(boost::bind(&LoginBox::clicked_ok, boost::ref(*this)));
	UI::Button* cancelbtn = new UI::Button(
	   this, "cancel",
	   UI::g_fh1->fontset()->is_rtl() ? (get_inner_w() / 2 - 200) / 2 + get_inner_w() / 2 :
	                                    (get_inner_w() / 2 - 200) / 2,
	   loginbtn->get_y(), 200, 20, UI::Button::Style::kWuiSecondary, _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&LoginBox::clicked_back, boost::ref(*this)));

	Section& s = g_options.pull_section("global");
	eb_nickname->set_text(s.get_string("nickname", _("nobody")));
	eb_password->set_text(s.get_string("password", ""));
	cb_register->set_state(s.get_bool("registered", false));
	eb_nickname->focus();
}

/**
 * called, if "login" is pressed.
 */
void LoginBox::clicked_ok() {
	// Check if all needed input fields are valid
	if (eb_nickname->text().empty()) {
		UI::WLMessageBox mb(
		   this, _("Empty Nickname"), _("Please enter a nickname!"), UI::WLMessageBox::MBoxType::kOk);
		mb.run<UI::Panel::Returncodes>();
		return;
	}
	if (eb_nickname->text().find(' ') <= eb_nickname->text().size()) {
		UI::WLMessageBox mb(this, _("Space in Nickname"),
		                    _("Sorry, but spaces are not allowed in nicknames!"),
		                    UI::WLMessageBox::MBoxType::kOk);
		mb.run<UI::Panel::Returncodes>();
		return;
	}
	if (eb_password->text().empty() && cb_register->get_state()) {
		UI::WLMessageBox mb(this, _("Empty Password"), _("Please enter your password!"),
		                    UI::WLMessageBox::MBoxType::kOk);
		mb.run<UI::Panel::Returncodes>();
		return;
	}
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
}

/// Called if "cancel" was pressed
void LoginBox::clicked_back() {
	end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
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
