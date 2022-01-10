/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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
#include "base/string.h"
#include "graphic/image_cache.h"
#include "ui_basic/icon.h"
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
		case SDLK_RETURN:
			clicked_ok();
			return true;
		default:
			break;  // not handled
		}
	}
	return TwoColumnsBasicNavigationMenu::handle_key(down, code);
}

void MenuCapsule::Entry::cleanup(const bool all) {
	assert(button);
	assert(icon);
	assert(spacer1);
	assert(spacer2);
	assert(panel);

	button->die();
	button = nullptr;
	icon->die();
	icon = nullptr;
	spacer1->die();
	spacer1 = nullptr;
	spacer2->die();
	spacer2 = nullptr;

	if (all) {
		panel->die();
		panel = nullptr;
	}
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
     box_(this, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal),
     fsmm_(fsmm),
     should_die_(false) {
	set_visible(false);
	do_not_layout_on_resolution_change();
}

void MenuCapsule::layout() {
	UI::Window::layout();
	if (!is_minimal()) {
		int dw, dh;
		box_.get_desired_size(&dw, &dh);
		box_.set_size(dw, dh);

		for (Entry& e : visible_menus_) {
			e.panel->set_pos(Vector2i(0, dh + kPadding));
			e.panel->set_size(get_inner_w(), get_inner_h() - dh - kPadding);
		}
	}
}

void MenuCapsule::draw(RenderTarget& r) {
	UI::Window::draw(r);
	if (!is_minimal()) {
		r.brighten_rect(Recti(0, 0, get_inner_w(), box_.get_h()), -30);
	}
}

bool MenuCapsule::handle_key(bool down, SDL_Keysym sym) {
	// Make sure all keys are forwarded to the content pane
	if (!visible_menus_.empty()) {
		if (visible_menus_.back().panel->handle_key(down, sym)) {
			return true;
		}
	}
	return UI::Window::handle_key(down, sym);
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

		if (visible_menus_.empty()) {
			return;
		}
		for (Entry& e : visible_menus_) {
			e.button->set_perm_pressed(false);
		}
		visible_menus_.back().button->set_perm_pressed(true);
	}
}

void MenuCapsule::add(BaseMenu& menu, const std::string& title) {
	if (!visible_menus_.empty()) {
		visible_menus_.back().panel->set_visible(false);
	}

	UI::Button* button = new UI::Button(&box_, title, 0, 0, 0, 0, UI::ButtonStyle::kFsMenuMenu,
	                                    title, format(_("Back to ‘%s’"), title));
	UI::Panel* spacer1 = new UI::Panel(&box_, UI::PanelStyle::kFsMenu, 0, 0, kPadding, kPadding);
	UI::Panel* spacer2 = new UI::Panel(&box_, UI::PanelStyle::kFsMenu, 0, 0, kPadding, kPadding);
	UI::Panel* icon;
	if (visible_menus_.empty()) {
		UI::Button* b = new UI::Button(&box_, title, 0, 0, 30, 30, UI::ButtonStyle::kFsMenuMenu,
		                               g_image_cache->get("images/ui_basic/list_first_entry.png"),
		                               _("Close window"), UI::Button::VisualState::kFlat);
		b->sigclicked.connect([this]() { die(); });
		icon = b;
		spacer1->set_visible(false);
	} else {
		icon = new UI::Icon(
		   &box_, UI::PanelStyle::kFsMenu, g_image_cache->get("images/ui_basic/scrollbar_right.png"));
	}

	visible_menus_.push_back(Entry{title, &menu, button, icon, spacer1, spacer2});

	box_.add(spacer1, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	box_.add(icon, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	box_.add(spacer2, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	box_.add(button, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	button->sigclicked.connect([this, &menu]() {
		for (;;) {
			assert(!visible_menus_.empty());
			if (visible_menus_.back().panel == &menu) {
				visible_menus_.back().panel->set_visible(true);
				set_title(visible_menus_.back().title);
				visible_menus_.back().panel->reactivated();
				return;
			}
			visible_menus_.back().cleanup(true);
			visible_menus_.pop_back();
		}
	});

	set_title(title);
	menu.set_visible(true);
	set_visible(true);

	layout();
	menu.focus();
	initialization_complete();
}

void MenuCapsule::clear_content() {
	set_visible(false);
	set_title("");
	while (!visible_menus_.empty()) {
		visible_menus_.back().cleanup(true);
		visible_menus_.pop_back();
	}
}

void MenuCapsule::on_death(UI::Panel* p) {
	if (visible_menus_.empty() || p != visible_menus_.back().panel) {
		return;
	}
	visible_menus_.back().cleanup(false);
	visible_menus_.pop_back();
	if (visible_menus_.empty()) {
		set_visible(false);
		set_title("");
	} else {
		visible_menus_.back().panel->set_visible(true);
		set_title(visible_menus_.back().title);
		visible_menus_.back().panel->reactivated();
	}
}

}  // namespace FsMenu
