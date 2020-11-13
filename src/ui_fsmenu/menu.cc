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

#include "ui_fsmenu/menu.h"

#include "base/i18n.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {
/*
==============================================================================

Base

==============================================================================
*/

/**
 * Initialize a pre-game menu
 */
BaseMenu::BaseMenu(MenuCapsule& window, const std::string& title)
   : UI::Panel(&window, UI::PanelStyle::kFsMenu, 0, 0, 0, 0),
     horizontal_padding_box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     vertical_padding_box_(
        &horizontal_padding_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     main_box_(&vertical_padding_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     header_box_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     standard_height_(get_h() * 9 / 200),
     capsule_(window) {
	horizontal_padding_box_.add_space(10 * kPadding);
	horizontal_padding_box_.add(&vertical_padding_box_, UI::Box::Resizing::kExpandBoth);
	horizontal_padding_box_.add_space(10 * kPadding);
	vertical_padding_box_.add_space(10 * kPadding);
	vertical_padding_box_.add(&main_box_, UI::Box::Resizing::kExpandBoth);
	vertical_padding_box_.add_space(10 * kPadding);

	main_box_.add(&header_box_, UI::Box::Resizing::kFullSize);

	capsule_.add(*this, title);
}

BaseMenu::~BaseMenu() {
}

void BaseMenu::return_to_main_menu() {
	capsule_.menu().set_labels();
	capsule_.clear_content();
}

void BaseMenu::layout() {
	UI::Panel::layout();
	horizontal_padding_box_.set_size(get_inner_w(), get_inner_h());
	vertical_padding_box_.set_max_size(
	   horizontal_padding_box_.get_w() - 2 * 10 * kPadding, horizontal_padding_box_.get_h());
	standard_height_ = get_inner_h() * 9 / 200;
}

TwoColumnsMenu::TwoColumnsMenu(MenuCapsule& fsmm,
                               const std::string& title,
                               double right_column_width_factor)
   : BaseMenu(fsmm, title),
     content_box_(&main_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     left_column_box_(&content_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     right_column_box_(&content_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     right_column_width_(get_inner_w() * right_column_width_factor),
     right_column_width_factor_(right_column_width_factor) {

	main_box_.add(&content_box_, UI::Box::Resizing::kExpandBoth);
	content_box_.add(&left_column_box_, UI::Box::Resizing::kExpandBoth);
	content_box_.add_space(5 * kPadding);
	content_box_.add(&right_column_box_, UI::Box::Resizing::kFullSize);
}
TwoColumnsMenu::~TwoColumnsMenu() {
}

void TwoColumnsMenu::layout() {
	BaseMenu::layout();

	content_box_.set_max_size(main_box_.get_w(), main_box_.get_h() - header_box_.get_h());
	right_column_width_ = get_inner_w() * right_column_width_factor_;

	right_column_box_.set_max_size(right_column_width_, 0);
}

TwoColumnsBasicNavigationMenu::TwoColumnsBasicNavigationMenu(MenuCapsule& fsmm,
                                                             const std::string& title,
                                                             double right_column_width_factor)
   : TwoColumnsMenu(fsmm, title, right_column_width_factor),
     right_column_content_box_(
        &right_column_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, 1 * kPadding),
     button_box_(
        &right_column_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, 0, 0, 1 * kPadding),
     back_(&button_box_, "back", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Back")) {

	right_column_box_.add(&right_column_content_box_, UI::Box::Resizing::kExpandBoth);
	right_column_box_.add_space(5 * kPadding);
	right_column_box_.add(&button_box_, UI::Box::Resizing::kFullSize);
	button_box_.add(&back_, UI::Box::Resizing::kFillSpace);

	back_.sigclicked.connect([this]() { clicked_back(); });
}

TwoColumnsBasicNavigationMenu::~TwoColumnsBasicNavigationMenu() {
}

void TwoColumnsBasicNavigationMenu::layout() {
	TwoColumnsMenu::layout();
	back_.set_desired_size(right_column_width_, standard_height_);
}

bool TwoColumnsBasicNavigationMenu::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_ESCAPE:
			clicked_back();
			return true;
		default:
			break;  // not handled
		}
	}
	return TwoColumnsMenu::handle_key(down, code);
}

void TwoColumnsBasicNavigationMenu::clicked_back() {
	die();
}

TwoColumnsFullNavigationMenu::TwoColumnsFullNavigationMenu(MenuCapsule& fsmm,
                                                           const std::string& title,
                                                           double right_column_width_factor)
   : TwoColumnsBasicNavigationMenu(fsmm, title, right_column_width_factor),

     ok_(&button_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK")) {

	button_box_.add(&ok_, UI::Box::Resizing::kFillSpace);
	ok_.sigclicked.connect([this]() { clicked_ok(); });
}

TwoColumnsFullNavigationMenu::~TwoColumnsFullNavigationMenu() {
}

void TwoColumnsFullNavigationMenu::layout() {
	TwoColumnsBasicNavigationMenu::layout();
	back_.set_desired_size((right_column_width_ / 2) - (kPadding / 2), standard_height_);
	ok_.set_desired_size((right_column_width_ / 2) - (kPadding / 2), standard_height_);
}

bool TwoColumnsFullNavigationMenu::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			clicked_ok();
			return true;
		default:
			break;  // not handled
		}
	}
	return TwoColumnsBasicNavigationMenu::handle_key(down, code);
}

MenuCapsule::MenuCapsule(MainMenu& fsmm)
   : UI::Window(&fsmm,
                UI::WindowStyle::kFsMenu,
                "menu",
                fsmm.calc_desired_window_x(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_y(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuDefault),
                ""),
     fsmm_(fsmm),
     should_die_(false) {
	set_visible(false);
	do_not_layout_on_resolution_change();
}

void MenuCapsule::layout() {
	UI::Window::layout();
	if (!is_minimal()) {
		for (auto& pair : visible_menus_) {
			pair.first->set_size(get_inner_w(), get_inner_h());
		}
	}
}

bool MenuCapsule::handle_key(bool down, SDL_Keysym sym) {
	UI::Window::handle_key(down, sym);
	// consume all events to prevent triggering hotkeys by accident
	return true;
}

void MenuCapsule::die() {
	// Never delete us! Only hide the window and its content from the user.
	// Don't do this immediately though because the caller may assume that
	// we stay around for the rest of the current frame.
	should_die_ = true;
}

void MenuCapsule::think() {
	if (should_die_) {
		clear_content();
		should_die_ = false;
	} else {
		UI::Window::think();
	}
}

void MenuCapsule::add(BaseMenu& menu, const std::string& title) {
	if (!visible_menus_.empty()) {
		visible_menus_.back().first->set_visible(false);
	}
	visible_menus_.push_back(std::make_pair(&menu, title));
	set_title(title);
	menu.set_visible(true);
	set_visible(true);
	layout();
	menu.focus();
}

void MenuCapsule::clear_content() {
	set_visible(false);
	set_title("");
	while (!visible_menus_.empty()) {
		delete visible_menus_.back().first;
		visible_menus_.pop_back();
	}
}

void MenuCapsule::on_death(UI::Panel* p) {
	if (visible_menus_.empty() || p != visible_menus_.back().first) {
		return;
	}
	visible_menus_.pop_back();
	if (visible_menus_.empty()) {
		set_visible(false);
		set_title("");
	} else {
		visible_menus_.back().first->set_visible(true);
		set_title(visible_menus_.back().second);
		visible_menus_.back().first->reactivated();
	}
}

}  // namespace FsMenu
