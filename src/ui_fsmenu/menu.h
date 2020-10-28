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

#ifndef WL_UI_FSMENU_MENU_H
#define WL_UI_FSMENU_MENU_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"

constexpr int padding = 4;

class BaseMenu : public UI::Window {
public:
	BaseMenu(FullscreenMenuMain&, const std::string& name, const std::string& title);
	~BaseMenu() override;

	WindowLayoutID window_layout_id() const override {
		return UI::Window::WindowLayoutID::kFsMenuDefault;
	}

private:
	UI::Box horizontal_padding_box_, vertical_padding_box_;

protected:
	void printBox(UI::Box& b);
	void layout() override;

	UI::Box main_box_, header_box_;
};

class TwoColumnsMenu : public BaseMenu {
public:
	TwoColumnsMenu(FullscreenMenuMain&,
	               const std::string& name,
	               const std::string& title,
	               double right_column_width_factor = 1.0 / 3);
	~TwoColumnsMenu() override;

protected:
	void layout() override;

	UI::Box content_box_, left_column_box_, right_column_box_;

private:
	/// right column width = get_w * factor. rest goes to left column (minus some padding)
	double right_column_width_factor_;
};

class TwoColumnsNavigationMenu : public TwoColumnsMenu {
public:
	TwoColumnsNavigationMenu(FullscreenMenuMain&,
	                         const std::string& name,
	                         const std::string& title,
	                         double right_column_width_factor = 1.0 / 3);
	~TwoColumnsNavigationMenu() override;

protected:
	void layout() override;
	virtual void clicked_back();
	virtual void clicked_ok();
	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

	UI::Box right_column_content_box_, button_box_;

	UI::Button back_;
	UI::Button ok_;
};

#endif  // end of include guard: WL_UI_FSMENU_MENU_H
