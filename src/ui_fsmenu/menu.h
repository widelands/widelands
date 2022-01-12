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

#ifndef WL_UI_FSMENU_MENU_H
#define WL_UI_FSMENU_MENU_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/window.h"

namespace FsMenu {

class MainMenu;
class MenuCapsule;

constexpr int kPadding = 4;
constexpr double kDefaultColumnWidthFactor = 1.0 / 3;

/**
 * Base for a menu. Takes care of padding. Use this to freely implement your own layout by adding
 * your content into header_box and main_box
 */
class BaseMenu : public UI::Panel {
public:
	BaseMenu(MenuCapsule&, const std::string& title);

	MenuCapsule& get_capsule() {
		return capsule_;
	}

	virtual void reactivated() {
	}

private:
	UI::Box horizontal_padding_box_, vertical_padding_box_;

protected:
	void layout() override;

	// Hides the parent window, deleting this panel immediately.
	// The main menu's Continue buttons are updated in case a new savegame or map was created.
	void return_to_main_menu();

	UI::Box main_box_, header_box_;
	uint32_t standard_height_;
	MenuCapsule& capsule_;
};

/**
 * A Menu which provides a two column layout below the header_box_. Add your content to
 * TwoColumnsMenu::left_column_box_ and TwoColumnsMenu::right_column_box_
 */
class TwoColumnsMenu : public BaseMenu {
public:
	TwoColumnsMenu(MenuCapsule&,
	               const std::string& title,
	               double right_column_width_factor = kDefaultColumnWidthFactor);

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
	TwoColumnsBasicNavigationMenu(MenuCapsule&,
	                              const std::string& title,
	                              double right_column_width_factor = kDefaultColumnWidthFactor);

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
	TwoColumnsFullNavigationMenu(MenuCapsule&,
	                             const std::string& title,
	                             double right_column_width_factor = kDefaultColumnWidthFactor);
	~TwoColumnsFullNavigationMenu() override;

protected:
	void layout() override;
	virtual void clicked_ok() = 0;
	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

	UI::Button ok_;
};

/**
 * Singleton class owned by the MainMenu. A MenuCapsule can hold one or more BaseMenus which
 * are stacked on top of each other so that only the top of the stack is visible to the user.
 */
class MenuCapsule : public UI::Window {
public:
	explicit MenuCapsule(MainMenu&);
	~MenuCapsule() override = default;

	UI::Window::WindowLayoutID window_layout_id() const override {
		return UI::Window::WindowLayoutID::kFsMenuDefault;
	}

	MainMenu& menu() const {
		return fsmm_;
	}

	// Remove and free all panels
	void clear_content();

	void layout() override;
	void think() override;
	void die() override;
	void on_death(UI::Panel*) override;
	bool handle_key(bool down, SDL_Keysym) override;

	void draw(RenderTarget&) override;

	// Should be called only by BaseMenu ctor
	void add(BaseMenu&, const std::string& title);

private:
	struct Entry {
		std::string title;

		BaseMenu* panel;
		UI::Button* button;
		UI::Panel* icon;
		UI::Panel* spacer1;
		UI::Panel* spacer2;

		void cleanup(bool all);
	};
	UI::Box box_;
	std::vector<Entry> visible_menus_;
	MainMenu& fsmm_;
	bool should_die_;
};

}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_MENU_H
