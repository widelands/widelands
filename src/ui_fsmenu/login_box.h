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

#ifndef WL_UI_FSMENU_LOGIN_BOX_H
#define WL_UI_FSMENU_LOGIN_BOX_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/editbox.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/textarea.h"
#include "ui_basic/unique_window.h"

namespace FsMenu {

class MainMenu;

struct LoginBox : public UI::UniqueWindow {
	explicit LoginBox(MainMenu&, UI::UniqueWindow::Registry&);

	void think() override;

	std::string get_nickname() {
		return eb_nickname_.text();
	}
	std::string get_password() {
		return eb_password_.text();
	}
	bool registered() {
		return cb_register_.get_state();
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

	MainMenu& fsmm_;

	UI::Box main_box_, hbox_, buttons_box_, vbox1_, vbox2_;
	UI::Button b_login_, b_cancel_;
	UI::EditBox eb_nickname_, eb_password_;
	UI::Checkbox cb_register_;
	UI::Textarea ta_nickname_, ta_password_;
	UI::MultilineTextarea register_account_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_LOGIN_BOX_H
