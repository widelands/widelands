/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#include "i18n.h"
#include "profile/profile.h"
#include "ui_basic/button.h"
#include "ui_basic/messagebox.h"

LoginBox::LoginBox(Panel & parent)
:
Window(&parent, "login_box", 0, 0, 500, 210, _("Metaserver login"))
{
	center_to_parent();

	ta_nickname = new UI::Textarea(this, 10, 5, _("Nickname:"));
	eb_nickname =
		new UI::EditBox
			(this, 150, 5, 330, 20,
			 g_gr->images().get("pics/but2.png"), UI::Align_Left);

	ta_password = new UI::Textarea(this, 10, 40, _("Password:"));
	eb_password =
		new UI::EditBox
			(this, 150, 40, 330, 20,
			 g_gr->images().get("pics/but2.png"), UI::Align_Left);

	pwd_warning =
		new UI::Multiline_Textarea
			(this, 10, 65, 505, 50,
			 _("WARNING: Password will be shown and saved readable!"),
			 UI::Align_Left);

	cb_register = new UI::Checkbox(this, Point(10, 110));
	ta_register =
		new UI::Textarea(this, 40, 110, _("Log in to a registered account"));

	cb_auto_log = new UI::Checkbox(this, Point(10, 135));
	ta_auto_log = new UI::Textarea
		(this, 40, 135,
		 _("Automatically use this login information from now on."));

	UI::Button * loginbtn = new UI::Button
		(this, "login",
		 (get_inner_w() / 2 - 200) / 2, 175, 200, 20,
		 g_gr->images().get("pics/but0.png"),
		 _("Login"));
	loginbtn->sigclicked.connect(boost::bind(&LoginBox::pressedLogin, boost::ref(*this)));
	UI::Button * cancelbtn = new UI::Button
		(this, "cancel",
		 (get_inner_w() / 2 - 200) / 2 + get_inner_w() / 2, 175, 200, 20,
		 g_gr->images().get("pics/but1.png"),
		 _("Cancel"));
	cancelbtn->sigclicked.connect(boost::bind(&LoginBox::pressedCancel, boost::ref(*this)));

	Section & s = g_options.pull_section("global");
	eb_nickname->setText(s.get_string("nickname", _("nobody")));
	eb_password->setText(s.get_string("password", ""));
	cb_register->set_state(s.get_bool("registered", false));
}


/// called, if "login" is pressed
void LoginBox::pressedLogin()
{
	// Check if all needed input fields are valid
	if (eb_nickname->text().empty()) {
		UI::WLMessageBox mb
			(this, _("Empty Nickname"), _("Please enter a nickname!"),
			 UI::WLMessageBox::OK);
		mb.run();
		return;
	}
	if (eb_nickname->text().find(' ') <= eb_nickname->text().size()) {
		UI::WLMessageBox mb
			(this, _("Space in Nickname"),
			 _("Sorry, but spaces are not allowed in nicknames!"),
			 UI::WLMessageBox::OK);
		mb.run();
		return;
	}
	if (eb_password->text().empty() && cb_register->get_state()) {
		UI::WLMessageBox mb
			(this, _("Empty Password"), _("Please enter your password!"),
			 UI::WLMessageBox::OK);
		mb.run();
		return;
	}
	end_modal(1);
}


/// Called if "cancel" was pressed
void LoginBox::pressedCancel()
{
	end_modal(0);
}
