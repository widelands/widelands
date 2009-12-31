/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#include "login_box.h"
#include "i18n.h"
#include "ui_basic/button.h"
#include "ui_basic/messagebox.h"
#include "profile/profile.h"

namespace UI {

LoginBox::LoginBox(Panel * parent)
:
Window(parent, 0, 0, 500, 200, _("Metaserver login"))
{
	center_to_parent();

	ta_nickname = new Textarea(this, 10, 5, _("Nickname:"));
	eb_nickname = new EditBox
		(this, 150, 5, 330, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"), 0, Align_Left);

	ta_emailadd = new Textarea(this, 10, 40, _("email (optional):"));
	eb_emailadd = new EditBox
		 (this, 150, 40, 330, 20,
		  g_gr->get_picture(PicMod_UI, "pics/but2.png"), 0, Align_Left);

	pwd_warning = new Textarea
		(this, 250, 85,
		 _("WARNING: Password will be shown and saved readable!"),
		 Align_Center);

	ta_password = new Textarea(this, 10, 110, _("Password:"));
	eb_password = new EditBox
		(this, 150, 110, 330, 20,
		 g_gr->get_picture(PicMod_UI, "pics/but2.png"), 0, Align_Left);

	cb_register = new Checkbox(this, Point(150, 135));
	ta_register = new Textarea(this, 180, 130, _("Register new account"));

	new Callback_Button<LoginBox>
		(this,
		 (get_inner_w() / 2 - 200) / 2, 170, 200, 20,
		  g_gr->get_picture(PicMod_UI, "pics/but0.png"),
		 &LoginBox::pressedLogin, *this,
		 _("Login"));
	new Callback_Button<LoginBox>
		(this,
		 (get_inner_w() / 2 - 200) / 2 + get_inner_w() / 2, 170, 200, 20,
		  g_gr->get_picture(PicMod_UI, "pics/but1.png"),
		 &LoginBox::pressedCancel, *this,
		 _("Cancel"));

	Section & s = g_options.pull_section("global");
	eb_nickname->setText(s.get_string("nickname", _("nobody")));
	eb_emailadd->setText(s.get_string("emailadd", ""));
	eb_password->setText(s.get_string("password", ""));
}

void LoginBox::pressedLogin()
{
	if (eb_nickname->text().empty()) {
		WLMessageBox mb
			(this, _("Empty Nickname"), _("Please enter a nickname!"),
			 WLMessageBox::OK);
		mb.run();
		return;
	}
	if (eb_nickname->text().find(' ') <= eb_nickname->text().size()) {
		WLMessageBox mb
			(this, _("Space in Nickname"),
			 _("Sorry, but spaces are not allowed in nicknames!"),
			 WLMessageBox::OK);
		mb.run();
		return;
	}
	if (eb_password->text().empty()) {
		WLMessageBox mb
			(this, _("Empty Password"), _("Please enter a password!"),
			 WLMessageBox::OK);
		mb.run();
		return;
	}
	end_modal(1);
}

void LoginBox::pressedCancel()
{
	end_modal(0);
}

}
