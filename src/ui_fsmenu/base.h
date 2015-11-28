/*
 * Copyright (C) 2002, 2006, 2008-2009 by the Widelands Development Team
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
#include <memory>

#include "graphic/text_layout.h"
#include "ui_basic/panel.h"

namespace UI {
struct Font;
struct TextStyle;
}

class Image;

/**
 * This class is the base class for a fullscreen menu.
 * A fullscreen menu is a menu which takes the full screen; it has the size
 * MENU_XRES and MENU_YRES and is a modal UI Element
 */
class FullscreenMenuBase : public UI::Panel {
public:
	enum class MenuTarget {
		kBack = static_cast<int>(UI::Panel::Returncodes::kBack),
		kOk = static_cast<int>(UI::Panel::Returncodes::kOk),

		// Options
		kRestart,

		// Main menu
		kTutorial,
		kSinglePlayer,
		kMultiplayer,
		kReplay,
		kEditor,
		kOptions,
		kReadme,
		kLicense,
		kAuthors,
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

	FullscreenMenuBase(char const * bgpic);
	virtual ~FullscreenMenuBase();

	void draw(RenderTarget &) override;

	///\return the size for texts fitting to current resolution
	uint32_t fs_small();
	uint32_t fs_big();

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

protected:
	virtual void clicked_back();
	virtual void clicked_ok();

private:
	std::string background_image_;
};


#endif  // end of include guard: WL_UI_FSMENU_BASE_H
