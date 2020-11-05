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

namespace FsMenu {
constexpr int kPadding = 4;
constexpr double kDefaultColumnWidthFactor = 1.0 / 3;

/**
 * Base for a menu. Takes care of padding. Use this to freely implement your own layout by adding
 * your content into header_box and main_box
 */
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
	void layout() override;

	UI::Box main_box_, header_box_;
	uint32_t standard_height_;
	FullscreenMenuMain& fsmm_;
};

/**
 * A Menu which provides a two column layout below the header_box_. Add your content to
 * TwoColumnsMenu::left_column_box_ and TwoColumnsMenu::right_column_box_
 */
class TwoColumnsMenu : public BaseMenu {
public:
	TwoColumnsMenu(FullscreenMenuMain&,
	               const std::string& name,
	               const std::string& title,
	               double right_column_width_factor = kDefaultColumnWidthFactor);
	~TwoColumnsMenu() override;

protected:
	void layout() override;

	UI::Box content_box_, left_column_box_, right_column_box_;
	int right_column_width_;

private:
	/// right column width = get_w * factor. rest goes to left column (minus some padding)
	double right_column_width_factor_;
};

/**
 * A two column menu which provides a "Back"-button at the bottom of the right column. Add your
 * content to TwoColumnsBasicNavigationMenu::left_column_box_ and
 * TwoColumnsBasicNavigationMenu::right_column_content_box_
 */
class TwoColumnsBasicNavigationMenu : public TwoColumnsMenu {
public:
	TwoColumnsBasicNavigationMenu(FullscreenMenuMain&,
	                              const std::string& name,
	                              const std::string& title,
	                              double right_column_width_factor = kDefaultColumnWidthFactor);
	~TwoColumnsBasicNavigationMenu() override;

protected:
	void layout() override;
	virtual void clicked_back();
	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

	UI::Box right_column_content_box_, button_box_;

	UI::Button back_;
};
/**
 * A two column menu which provides a "Back"-button and a "Ok"-button at the bottom of the right
 * column. Add your content to TwoColumnsFullNavigationMenu::left_column_box_ and
 * TwoColumnsFullNavigationMenu::right_column_content_box_
 */
class TwoColumnsFullNavigationMenu : public TwoColumnsBasicNavigationMenu {
public:
	TwoColumnsFullNavigationMenu(FullscreenMenuMain&,
	                             const std::string& name,
	                             const std::string& title,
	                             double right_column_width_factor = kDefaultColumnWidthFactor);
	~TwoColumnsFullNavigationMenu() override;

protected:
	void layout() override;
	virtual void clicked_ok();
	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

	UI::Button ok_;
};
}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_MENU_H
