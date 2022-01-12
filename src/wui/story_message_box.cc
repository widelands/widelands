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

#include "wui/story_message_box.h"

#include <SDL_mouse.h>

#include "logic/game_controller.h"
#include "logic/save_handler.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "wui/interactive_player.h"

namespace {
constexpr int kPadding = 4;
}  // namespace

StoryMessageBox::StoryMessageBox(Widelands::Game* game,
                                 const Widelands::Coords coords,
                                 const std::string& title,
                                 const std::string& body,
                                 int32_t const x,
                                 int32_t const y,
                                 uint32_t const w,
                                 uint32_t const h,
                                 bool allow_next_scenario)
   : UI::Window(game->get_ipl(), UI::WindowStyle::kWui, "story_message_box", x, y, w, h, title),
     main_box_(this, UI::PanelStyle::kWui, kPadding, kPadding, UI::Box::Vertical, 0, 0, kPadding),
     button_box_(
        &main_box_, UI::PanelStyle::kWui, kPadding, kPadding, UI::Box::Horizontal, 0, 0, kPadding),
     textarea_(&main_box_, 0, 0, 100, 50, UI::PanelStyle::kWui, body),
     ok_(&button_box_,
         "ok",
         0,
         0,
         120,
         0,
         allow_next_scenario ? UI::ButtonStyle::kWuiSecondary : UI::ButtonStyle::kWuiPrimary,
         _("OK")),
     next_scenario_(&button_box_,
                    "next",
                    0,
                    0,
                    120,
                    0,
                    UI::ButtonStyle::kWuiPrimary,
                    _("Next Mission"),
                    _("Start the next mission now")),
     main_menu_(&button_box_,
                "mainmenu",
                0,
                0,
                120,
                0,
                allow_next_scenario && game->list_of_scenarios().size() < 2 ?
                   UI::ButtonStyle::kWuiPrimary :
                   UI::ButtonStyle::kWuiSecondary,
                _("Main Menu"),
                _("Return to the main menu")),
     desired_speed_(game->game_controller()->desired_speed()),
     game_(game) {

	// Pause the game
	game_->game_controller()->set_desired_speed(0);
	game_->save_handler().set_allow_saving(false);

	// Adjust map view
	if (coords != Widelands::Coords::null()) {
		game_->get_ipl()->map_view()->scroll_to_field(coords, MapView::Transition::Jump);
	}

	// Add and configure the panels
	main_box_.set_size(get_inner_w() - 3 * kPadding, get_inner_h() - 2 * kPadding);

	main_box_.add(&textarea_, UI::Box::Resizing::kExpandBoth);
	main_box_.add(&button_box_, UI::Box::Resizing::kFullSize);

	button_box_.add_inf_space();
	button_box_.add(&ok_);
	button_box_.add_inf_space();

	if (allow_next_scenario) {  // End of game
		button_box_.add(&main_menu_);
		button_box_.add_inf_space();
		if (game->list_of_scenarios().size() > 1) {  // Next scenario can be started
			button_box_.add(&next_scenario_);
			button_box_.add_inf_space();
			next_scenario_.sigclicked.connect([this]() { clicked_next_scenario(); });
		} else {  // Last scenario in the campaign
			next_scenario_.set_visible(false);
		}
		main_menu_.sigclicked.connect([this]() { clicked_main_menu(); });
		ok_.set_tooltip(_("Continue playing this mission"));
	} else {  // Normal message box, which has only the OK button
		next_scenario_.set_visible(false);
		main_menu_.set_visible(false);
	}

	ok_.sigclicked.connect([this]() { clicked_ok(); });

	if (x == -1 && y == -1) {
		center_to_parent();
	}
	move_inside_parent();
	textarea_.focus();

	if (!is_modal()) {
		resume_game();
	}

	initialization_complete();
}

void StoryMessageBox::clicked_ok() {
	set_visible(false);
	set_thinks(false);

	if (is_modal()) {
		resume_game();
		end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kOk);
	} else {
		die();
	}
}

void StoryMessageBox::resume_game() {
	// Manually force the game to reevaluate its current state, especially time information.
	game_->game_controller()->think();
	// Now get the game running again.
	game_->game_controller()->set_desired_speed(desired_speed_);
	game_->save_handler().set_allow_saving(true);
}

void StoryMessageBox::clicked_next_scenario() {
	assert(game_->list_of_scenarios().size() > 1);
	auto it = game_->list_of_scenarios().begin();
	++it;
	game_->set_next_game_to_load(*it);

	clicked_main_menu();
}
void StoryMessageBox::clicked_main_menu() {
	clicked_ok();
	get_topmost_forefather().end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
}

bool StoryMessageBox::handle_mousepress(const uint8_t btn, int32_t mx, int32_t my) {
	if (btn == SDL_BUTTON_RIGHT) {
		return true;
	}

	return UI::Window::handle_mousepress(btn, mx, my);
}

bool StoryMessageBox::handle_key(bool down, SDL_Keysym code) {
	if (down) {
		switch (code.sym) {
		case SDLK_RETURN:
			if (next_scenario_.is_visible()) {
				clicked_next_scenario();
			} else if (main_menu_.is_visible()) {
				clicked_main_menu();
			} else {
				clicked_ok();
			}
			return true;
		case SDLK_ESCAPE:
			clicked_ok();
			return true;
		default:
			break;  // not handled
		}
	}
	return UI::Window::handle_key(down, code);
}
