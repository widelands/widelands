/*
 * Copyright (C) 2002-2018 by the Widelands Development Team
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

#include <boost/bind.hpp>
#include <boost/format.hpp>

#include "ai/computer_player.h"
#include "ai/defaultai.h"
#include "base/i18n.h"
#include "network/gamehost.h"
#include "ui_basic/messagebox.h"
#include "wui/interactive_gamebase.h"

namespace {

constexpr int32_t width = 256;
constexpr int32_t margin = 10;
constexpr int32_t vspacing = 5;
constexpr uint32_t vgap = 3;

/**
 * Small helper class for a "Really exit game?" message box.
 */
class GameClientDisconnectedExitConfirmBox : public UI::WLMessageBox {
public:
	// TODO(GunChleoc): Arabic: Buttons need more height for Arabic
	GameClientDisconnectedExitConfirmBox(GameClientDisconnected* gcd, InteractiveGameBase* gb)
	   : UI::WLMessageBox(gcd->get_parent(),
	                      /** TRANSLATORS: Window label when "Exit game" has been pressed */
	                      _("Exit Game Confirmation"),
	                      _("Are you sure you wish to exit this game?"),
	                      MBoxType::kOkCancel),
	     igb_(gb), gcd_(gcd) {
	}

	void clicked_ok() override {
		igb_->end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	}

	void clicked_back() override {
		cancel();
		die();
		// Make parent window visible again so player can use another option
		gcd_->set_visible(true);
	}

private:
	InteractiveGameBase* igb_;
	GameClientDisconnected* gcd_;
};

} // end anonymous namespace

GameClientDisconnected::GameClientDisconnected(InteractiveGameBase* gb,
                                 UI::UniqueWindow::Registry& registry,
                                 GameHost* host)
	: UI::UniqueWindow(gb, "client_disconnected", &registry, 2 * margin + width, 0,
						/** TRANSLATORS: Window label */
						_("Client got disconnected")),
		igb_(gb),
		host_(host),
		box_(this, margin, margin, UI::Box::Vertical, width, get_h() - 2 * margin, vspacing),
		box_h_(&box_, margin, margin, UI::Box::Horizontal, width, 35, vspacing),
		text_(&box_,
			0,
			0,
			width,
			10, // automatic height
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
			_("Continue game with selected AI")),
		type_dropdown_(&box_h_,
			width - 50, // x
			0, // y
			60, // width of selection box
			800, // height of selection box, shrinks automatically
			35, // width/height of button
			/** TRANSLATORS: Dropdown tooltip to select the AI difficulty */
			_("Select AI to continue game with"),
			UI::DropdownType::kPictorial,
			UI::PanelStyle::kWui),
		exit_game_(&box_,
			"exit_game",
			0,
			0,
			width,
			35,
			UI::ButtonStyle::kWuiMenu,
			g_gr->images().get("images/wui/menus/menu_exit_game.png"),
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

	continue_.sigclicked.connect(
	   boost::bind(&GameClientDisconnected::clicked_continue, boost::ref(*this)));
	exit_game_.sigclicked.connect(
	   boost::bind(&GameClientDisconnected::clicked_exit_game, boost::ref(*this)));

	// Add all AI types
	for (const auto* impl : ComputerPlayer::get_implementations()) {
		/** TRANSLATORS: Parameter is the pretty/descriptive name of the AI that will be used
			as replacement for a disconnected player */
		type_dropdown_.add((boost::format(_("Replace with %s")) % impl->descname).str(),
							impl->name.c_str(),
							g_gr->images().get(impl->icon_filename), false,
							(boost::format(_("Replace with %s")) % impl->descname).str());
	}

	// Set default mode to normal AI
	type_dropdown_.select(DefaultAI::normal_impl.name.c_str());

	if (get_usedefaultpos()) {
		center_to_parent();
	}
}

void GameClientDisconnected::die() {

	if (host_->forced_pause()) {
		host_->end_forced_pause();
	}
	if (is_visible()) {
		// Dialog aborted, default to the old behavior and add a normal AI
		set_ai(DefaultAI::normal_impl.name);
	}
	UI::UniqueWindow::die();
}

void GameClientDisconnected::clicked_continue() {
	assert(type_dropdown_.has_selection());

	const std::string selection = type_dropdown_.get_selected();
	assert(selection != "");

	set_ai(selection);
	// Visibility works as a hint that the window was closed by a button click
	set_visible(false);

	die();
}

void GameClientDisconnected::clicked_exit_game() {
	if (SDL_GetModState() & KMOD_CTRL) {
		igb_->end_modal<UI::Panel::Returncodes>(UI::Panel::Returncodes::kBack);
	} else {
		new GameClientDisconnectedExitConfirmBox(this, igb_);
		set_visible(false);
	}
}

void GameClientDisconnected::set_ai(const std::string& ai) {
	const std::string ai_descr = ComputerPlayer::get_implementation(ai)->descname;
	for (size_t i = 0; i < host_->settings().players.size(); i++) {
		if (host_->settings().players.at(i).state != PlayerSettings::State::kOpen
				|| !igb_->game().get_player(i + 1)->get_ai().empty()) {
			continue;
		}
		host_->replace_client_with_ai(i, ai);
	}
}
