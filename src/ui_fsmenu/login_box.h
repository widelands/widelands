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

#ifndef WL_UI_FSMENU_LOGIN_BOX_H
#define WL_UI_FSMENU_LOGIN_BOX_H

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

struct LoginBox : public UI::Window {
	explicit LoginBox(UI::Panel&);

	void think() override;

	std::string get_nickname() {
		return eb_nickname->text();
	}
	std::string get_password() {
		return eb_password->text();
	}
	bool registered() {
		return cb_register->get_state();
	}

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

private:
	void change_playername();
	void clicked_back();
	void clicked_ok();
	void clicked_register();
	void verify_input();
	bool check_password();

	UI::Button* loginbtn;
	UI::Button* cancelbtn;
	UI::EditBox* eb_nickname;
	UI::EditBox* eb_password;
	UI::Checkbox* cb_register;
	UI::Textarea* ta_nickname;
	UI::Textarea* ta_password;
	UI::MultilineTextarea* register_account;
};

#endif  // end of include guard: WL_UI_FSMENU_LOGIN_BOX_H
