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
#include "base/log.h"

void BaseMenu::printBox(UI::Box& b) {
	log_dbg("%s: (%d,%d) %dx%d", b.get_name().c_str(), b.get_x(), b.get_y(), b.get_w(), b.get_h());
} /*
 ==============================================================================

 FullscreenMenuBase

 ==============================================================================
 */

/**
 * Initialize a pre-game menu
 */
BaseMenu::BaseMenu(FullscreenMenuMain& fsmm, std::string& title)
   : UI::Window(&fsmm,
                UI::WindowStyle::kFsMenu,
                title,
                fsmm.calc_desired_window_x(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_y(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_width(UI::Window::WindowLayoutID::kFsMenuDefault),
                fsmm.calc_desired_window_height(UI::Window::WindowLayoutID::kFsMenuDefault),
                _(title)),
     horizontal_padding_box_(this, 0, 0, UI::Box::Horizontal, 0, 0, 0, "main horizontal"),
     vertical_padding_box_(
        &horizontal_padding_box_, 0, 0, UI::Box::Vertical, 0, 0, 0, "main vertical"),
     main_box_(&vertical_padding_box_, 0, 0, UI::Box::Vertical, 0, 0, 0, "main"),
     header_box_(&main_box_, 0, 0, UI::Box::Vertical, 0, 0, 0, "header") {
	horizontal_padding_box_.add_space(10 * padding);
	horizontal_padding_box_.add(&vertical_padding_box_, UI::Box::Resizing::kExpandBoth);
	horizontal_padding_box_.add_space(10 * padding);
	vertical_padding_box_.add_space(10 * padding);
	vertical_padding_box_.add(&main_box_, UI::Box::Resizing::kExpandBoth);
	vertical_padding_box_.add_space(10 * padding);

	main_box_.add(&header_box_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(10 * padding);
}

BaseMenu::~BaseMenu() {
}

void BaseMenu::layout() {
	Window::layout();
	horizontal_padding_box_.set_size(get_inner_w(), get_inner_h());
	vertical_padding_box_.set_max_size(
	   horizontal_padding_box_.get_w() - 2 * 10 * padding, horizontal_padding_box_.get_h());
	printBox(horizontal_padding_box_);
	printBox(vertical_padding_box_);
}

TwoColumnsMenu::TwoColumnsMenu(FullscreenMenuMain& fsmm,
                               std::string& title,
                               double right_column_width_factor)
   : BaseMenu(fsmm, title),
     content_box_(&main_box_, 0, 0, UI::Box::Horizontal, 0, 0, 0, "content"),
     left_column_box_(&content_box_, 0, 0, UI::Box::Vertical, 0, 0, 0, "individual"),
     right_column_box_(&content_box_, 0, 0, UI::Box::Vertical, 0, 0, 0 /*padding*/, "right"),
     right_column_width_factor_(right_column_width_factor) {

	main_box_.add(&content_box_, UI::Box::Resizing::kExpandBoth);
	content_box_.add(&left_column_box_, UI::Box::Resizing::kExpandBoth);
	content_box_.add_space(5 * padding);
	content_box_.add(&right_column_box_, UI::Box::Resizing::kFullSize);
}
TwoColumnsMenu::~TwoColumnsMenu() {
}

void TwoColumnsMenu::layout() {
	BaseMenu::layout();
	// TODO(jmoerschbach): this feels weird (and error prone), but we need to limit child box
	// manually because otherwise, children of childbox may claim to need more height than available
	// (e.g. table!) and childbox just says ok I need to grow too which results in a childbox bigger
	// than main_box... ;(

	content_box_.set_max_size(
	   main_box_.get_w(), main_box_.get_h() - 1 * 10 * padding - header_box_.get_h());

	right_column_box_.set_desired_size(get_w() * right_column_width_factor_, 0);
	printBox(main_box_);
	//	printBox(header_box_);
	printBox(content_box_);
	printBox(left_column_box_);
	printBox(right_column_box_);
}

TwoColumnsNavigationMenu::TwoColumnsNavigationMenu(FullscreenMenuMain& fsmm,
                                                   std::string title,
                                                   double right_column_width_factor)
   : TwoColumnsMenu(fsmm, title, right_column_width_factor),
     button_box_(&right_column_box_, 0, 0, UI::Box::Vertical, 0, 0, 1 * padding, "button"),
     back_(&button_box_, "back", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Back")),
     ok_(&button_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuPrimary, _("OK")) {
	button_box_.add(&ok_, UI::Box::Resizing::kFullSize);
	button_box_.add(&back_, UI::Box::Resizing::kFullSize);

	ok_.sigclicked.connect([this]() { clicked_ok(); });
	back_.sigclicked.connect([this]() { clicked_back(); });
}

TwoColumnsNavigationMenu::~TwoColumnsNavigationMenu() {
}

void TwoColumnsNavigationMenu::layout() {
	TwoColumnsMenu::layout();
	printBox(button_box_);
}

bool TwoColumnsNavigationMenu::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_KP_ENTER:
		case SDLK_RETURN:
			clicked_ok();
			return true;
		case SDLK_ESCAPE:
			clicked_back();
			return true;
		default:
			break;  // not handled
		}
	}
	return UI::Panel::handle_key(down, code);
}

void TwoColumnsNavigationMenu::clicked_back() {
	end_modal<MenuTarget>(MenuTarget::kBack);
}
void TwoColumnsNavigationMenu::clicked_ok() {
	end_modal<MenuTarget>(MenuTarget::kOk);
}
