/*
 * Copyright (C) 2016-2022 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_ABOUT_H
#define WL_UI_FSMENU_ABOUT_H

#include <memory>

#include "ui_basic/fileview_panel.h"
#include "ui_basic/unique_window.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {

/**
 * "Fullscreen "About" information with tabs
 */
class About : public UI::UniqueWindow {
public:
	explicit About(MainMenu&, UI::UniqueWindow::Registry&);
	~About() override {
	}

	WindowLayoutID window_layout_id() const override {
		return UI::Window::WindowLayoutID::kFsMenuAbout;
	}

	bool handle_key(bool, SDL_Keysym) override;

protected:
	void layout() override;

private:
	UI::Box box_;
	UI::FileViewPanel tabs_;
	UI::Button close_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_ABOUT_H
