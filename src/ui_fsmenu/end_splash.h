/*
 * Copyright (C) 2024 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_END_SPLASH_H
#define WL_UI_FSMENU_END_SPLASH_H

#include <cstdint>
#include <string>
#include <vector>

struct EndSplashOption {
	int32_t time;
	std::string name;
	std::string tooltip;

	// Special values for time
	enum : int32_t {
		// The time values are saved to the config, so they are better "human friendly"

		// all negative numbers are the same as kHard
		kHard = -1,  // End splash screen as soon as possible, without fading
		kSoft = 0,   // Fade to main menu as soon as possible

		// Intermediate values mean maximum number of seconds to wait after loading is completed,
		// or until the end of the intro music if it finishes earlier, before fading to main menu

		// Wait until intro music ends, then fade to main menu
		kWaitIntroMusic = 999,

		// Always wait for user action (mouse click or keypress) before fading to main menu,
		// play menu music after intro music ends
		kUserMenuMusic = 1110,

		// Always wait for user action before fading to main menu, no music after intro music ends
		kUserSilent = 1111,
		// greater values are the same as kUserSilent

		kDefault = kWaitIntroMusic
	};
};

extern const std::vector<EndSplashOption> kEndSplashOptionEntries;

#endif  // end of include guard: WL_UI_FSMENU_END_SPLASH_H
