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

#include "wui/game_client_disconnected.h"

#include "ai/computer_player.h"
#include "ai/defaultai.h"
#include "base/i18n.h"
#include "network/gamehost.h"
#include "wui/game_exit_confirm_box.h"
#include "wui/interactive_gamebase.h"

namespace {

constexpr int32_t width = 256;
constexpr int32_t margin = 10;
constexpr int32_t vspacing = 5;
constexpr uint32_t vgap = 3;

}  // end anonymous namespace

GameClientDisconnected::GameClientDisconnected(InteractiveGameBase* gb,
                                               UI::UniqueWindow::Registry& registry,
                                               GameHost* host)
   : UI::UniqueWindow(gb,
                      UI::WindowStyle::kWui,
                      "client_disconnected",
                      &registry,
                      2 * margin + width,
                      0,
                      /** TRANSLATORS: Window label */
                      _("Client got disconnected")),
     igb_(gb),
     host_(host),
     box_(this,
          UI::PanelStyle::kWui,
          margin,
          margin,
          UI::Box::Vertical,
          width,
          get_h() - 2 * margin,
          vspacing),
     box_h_(&box_, UI::PanelStyle::kWui, margin, margin, UI::Box::Horizontal, width, 35, vspacing),
     text_(&box_,
           0,
           0,
           width,
           10,  // automatic height
           UI::PanelStyle::kWui,
           _("A player disconnected from the game. An automatic save game has been created. "
             "Do you want to continue playing?"),
           UI::Align::kLeft,
           UI::MultilineTextarea::ScrollMode::kNoScrolling),
     continue_(&box_h_,
               "continue_game",
               0,
               0,
               width - 35 - vspacing,
               35,
               UI::ButtonStyle::kWuiMenu,
               /** TRANSLATORS: Button text */
               _("Continue game"),
               /** TRANSLATORS: Button tooltip */
               _("Replace the disconnected player with the selected AI and continue playing")),
     type_dropdown_(&box_h_,
                    "dropdown_ai",
                    width - 50,  // x
                    0,           // y
                    60,          // width of selection box
                    16,  // maximum number of items in the selection box, shrinks automatically
                    35,  // width/height of button
                    /** TRANSLATORS: Dropdown tooltip to select the AI difficulty when a player has
                       disconnected from a game */
                    _("AI for the disconnected player"),
                    UI::DropdownType::kPictorial,
                    UI::PanelStyle::kWui,
                    UI::ButtonStyle::kWuiMenu),
     exit_game_(&box_,
                "exit_game",
                0,
                0,
                width,
                35,
                UI::ButtonStyle::kWuiMenu,
                g_image_cache->get("images/wui/menus/exit.png"),
                /** TRANSLATORS: Button tooltip */
                _("Exit Game")) {

	box_.add(&text_);
	box_.add_space(vgap);
	box_h_.add(&continue_);
	box_h_.add(&type_dropdown_);
	box_.add(&box_h_);
	box_.add(&exit_game_);
	box_.set_size(width, text_.get_h() + vgap + box_h_.get_h() + exit_game_.get_h() + 3 * vspacing);
	set_inner_size(get_inner_w(), box_.get_h() + 2 * margin);

	continue_.sigclicked.connect([this]() { clicked_continue(); });
	exit_game_.sigclicked.connect([this]() { clicked_exit_game(); });

	// Add all AI types
	for (const auto* impl : AI::ComputerPlayer::get_implementations()) {
		type_dropdown_.add(_(impl->descname), impl->name, g_image_cache->get(impl->icon_filename),
		                   false,
		                   /** TRANSLATORS: Dropdown selection. Parameter is the name of the AI that
		                      will be used as replacement for a disconnected player */
		                   format(_("Replace player with %s"), impl->descname));
	}

	// Set default mode to normal AI
	type_dropdown_.select(AI::DefaultAI::normal_impl.name);

	if (get_usedefaultpos()) {
		center_to_parent();
	}

	initialization_complete();
}

void GameClientDisconnected::die() {

	if (host_->forced_pause()) {
		host_->end_forced_pause();
	}
	if (is_visible()) {
		// Dialog aborted, default to the old behavior and add a normal AI
		set_ai(AI::DefaultAI::normal_impl.name);
	}
	UI::UniqueWindow::die();
}

void GameClientDisconnected::clicked_continue() {
	assert(type_dropdown_.has_selection());

	const std::string selection = type_dropdown_.get_selected();
	assert(!selection.empty());

	set_ai(selection);
	// Visibility works as a hint that the window was closed by a button click
	set_visible(false);

	die();
}

void GameClientDisconnected::clicked_exit_game() {
	if (SDL_GetModState() & KMOD_CTRL) {
		igb_->end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	} else {
		GameExitConfirmBox* gecb = new GameExitConfirmBox(*get_parent(), *igb_);
		gecb->cancel.connect([this, gecb]() { exit_game_aborted(gecb); });

		set_visible(false);
	}
}

void GameClientDisconnected::exit_game_aborted(Panel* dialog) {
	// Make parent window visible again so player can use another option
	set_visible(true);
	// Make panel (that is: the dialog box) invisible to avoid both being visible for a moment
	dialog->set_visible(false);
}

void GameClientDisconnected::set_ai(const std::string& ai) {
	const std::string ai_descr = AI::ComputerPlayer::get_implementation(ai)->descname;
	for (size_t i = 0; i < host_->settings().players.size(); i++) {
		if (host_->settings().players.at(i).state != PlayerSettings::State::kOpen ||
		    !igb_->game().get_player(i + 1)->get_ai().empty()) {
			continue;
		}
		host_->replace_client_with_ai(i, ai);
	}
}
