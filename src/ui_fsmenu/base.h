/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_BASE_H
#define WL_UI_FSMENU_BASE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "graphic/align.h"
#include "ui_basic/fullscreen_window.h"

/**
 * This class is the base class for a fullscreen menu.
 * A fullscreen menu is a menu which takes the full screen; it has the size
 * MENU_XRES and MENU_YRES and is a modal UI Element
 */
class FullscreenMenuBase : public UI::FullscreenWindow {
public:
	enum class MenuTarget {
		kBack = static_cast<int>(UI::Panel::Returncodes::kBack),
		kOk = static_cast<int>(UI::Panel::Returncodes::kOk),

		// Options
		kApplyOptions,

		// Main menu
		kTutorial,
		kSinglePlayer,
		kMultiplayer,
		kReplay,
		kEditor,
		kOptions,
		kAbout,
		kExit,

		// Single player
		kNewGame,
		kCampaign,
		kLoadGame,

		// Multiplayer
		kMetaserver,
		kLan,

		// Launch game
		kNormalGame,
		kScenarioGame,
		kMultiPlayerSavegame,
		kHostgame,
		kJoingame
	};

	/// A full screen main menu outside of the game/editor itself.
	FullscreenMenuBase();
	virtual ~FullscreenMenuBase();

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

protected:
	virtual void clicked_back();
	virtual void clicked_ok();
};

#endif  // end of include guard: WL_UI_FSMENU_BASE_H
