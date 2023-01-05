/*
 * Copyright (C) 2021-2023 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_ADDONS_PROGRESS_H
#define WL_UI_FSMENU_ADDONS_PROGRESS_H

#include "ui_basic/box.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

namespace AddOnsUI {

class ProgressIndicatorWindow : public UI::Window {
public:
	ProgressIndicatorWindow(UI::Panel* parent, UI::WindowStyle style, const std::string& title);
	~ProgressIndicatorWindow() override = default;

	void set_message_1(const std::string& msg) {
		txt1_.set_text(msg);
	}
	void set_message_2(const std::string& msg) {
		txt2_.set_text(msg);
	}
	void set_message_3(const std::string& msg) {
		txt3_.set_text(msg);
	}
	UI::ProgressBar& progressbar() {
		return progress_;
	}

private:
	UI::Panel::ModalGuard modal_;
	UI::Box box_, hbox_;
	UI::Textarea txt1_, txt2_, txt3_;
	UI::ProgressBar progress_;
};

}  // namespace AddOnsUI

#endif  // end of include guard: WL_UI_FSMENU_ADDONS_PROGRESS_H
