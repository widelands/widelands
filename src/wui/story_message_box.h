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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_WUI_STORY_MESSAGE_BOX_H
#define WL_WUI_STORY_MESSAGE_BOX_H

#include "logic/game.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/window.h"

/**
 * A message box window with an OK button for use in scenarios.
 * Closing this window per right-click is blocked.
 * The game will be paused for the duration that this window is shown.
 * If 'coords' != Coords::null(), jumps the map view to the specified coordinates.
 * If 'x' == 'y' == -1, the message box will be centered on screen.
 */
struct StoryMessageBox : public UI::Window {
	StoryMessageBox(Widelands::Game* game,
	                const Widelands::Coords coords,
	                const std::string& title,
	                const std::string& body,
	                int32_t x,
	                int32_t y,
	                uint32_t w,
	                uint32_t h,
	                bool allow_next_scenario);

protected:
	/// Avoid being closed by right-click.
	bool handle_mousepress(uint8_t btn, int32_t mx, int32_t my) override;

	/// Handle keypresses for the OK button.
	bool handle_key(bool down, SDL_Keysym code) override;

	void clicked_button_close() override {
		clicked_ok();
	}

private:
	/// Get the game running again and close the window.
	void clicked_ok();
	void clicked_next_scenario();
	void clicked_main_menu();
	void resume_game();

	// UI elements
	UI::Box main_box_;
	UI::Box button_box_;
	UI::MultilineTextarea textarea_;
	UI::Button ok_, next_scenario_, main_menu_;

	const uint32_t desired_speed_;  // Remember the previous game speed
	Widelands::Game* game_;         // For controlling the game speed
};

#endif  // end of include guard: WL_WUI_STORY_MESSAGE_BOX_H
