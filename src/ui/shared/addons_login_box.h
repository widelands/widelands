/*
 * Copyright (C) 2021-2025 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_UI_SHARED_ADDONS_LOGIN_BOX_H
#define WL_UI_SHARED_ADDONS_LOGIN_BOX_H

#include "ui/basic/box.h"
#include "ui/basic/button.h"
#include "ui/basic/textinput.h"
#include "ui/basic/window.h"

namespace AddOnsUI {

class AddOnsLoginBox : public UI::Window {
public:
	explicit AddOnsLoginBox(UI::Panel& parent, UI::WindowStyle style);

	const std::string& get_username() const;
	std::string get_password() const;

	bool handle_key(bool down, SDL_Keysym code) override;

	void think() override;

private:
	const std::string password_sha1_;
	UI::Box box_, hbox_, left_box_, right_box_, buttons_box_;
	UI::EditBox username_, password_;
	UI::Button ok_, cancel_, reset_;

	void ok();
	void reset();
};

}  // namespace AddOnsUI

#endif  // end of include guard: WL_UI_SHARED_ADDONS_LOGIN_BOX_H
