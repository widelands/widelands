/*
 * Copyright (C) 2021-2022 by the Widelands Development Team
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

#ifndef WL_WLAPPLICATION_MOUSEWHEEL_OPTIONS_H
#define WL_WLAPPLICATION_MOUSEWHEEL_OPTIONS_H

#include <SDL_keyboard.h>

#include "base/vector.h"

/*
 * Mousewheel options
 */

// Config file options
enum class MousewheelOptionID : uint16_t {
	k_Begin = 0,

	kUIChangeValueInvertX = k_Begin,
	kUIChangeValueInvertY,
	kUITabInvertX,
	kUITabInvertY,
	kMapZoomMod,
	kMapZoomX,
	kMapZoomY,
	kMapZoomInvertX,
	kMapZoomInvertY,
	kMapScrollMod,
	kMapScroll,
	kGameSpeedMod,
	kGameSpeedX,
	kGameSpeedY,
	kEditorToolsizeMod,
	kEditorToolsizeX,
	kEditorToolsizeY,

	k_End = kEditorToolsizeY,

	// This is used in update_mousewheel_settings() during the initialisation
	// of WLApplication before SDL_Init() and the actual detection afterwards,
	// so it cannot be stored in WLApplication
	kInvertedXDetected,

	// And this is the user override if we get the detection wrong
	// After k_End, so this will not be reset with the rest in the options dialog
	kOverrideInvertedX,

	kAlwaysOn,
	kDisabled,
	kNoMod
};

void set_mousewheel_option_bool(MousewheelOptionID, bool);
bool get_mousewheel_option_bool(MousewheelOptionID);

void set_mousewheel_keymod(MousewheelOptionID, uint16_t);
uint16_t get_mousewheel_keymod(MousewheelOptionID);

// Map config options to handlers
enum class MousewheelHandlerConfigID : uint16_t {
	k_Begin = 0,

	kChangeValue = k_Begin,
	kTabBar,
	kZoom,
	kMapScroll,
	kGameSpeed,
	kEditorToolsize,
	kScrollbarVertical,  // Only vertical is used right now.
	                     // Horizontal will have to be configurable, because we can't determine
	                     // whether the hardware has 2D scrolling capability.

	k_End = kScrollbarVertical
};

// Read settings and apply them to handlers
void update_mousewheel_settings();

// Write all default values to config file
void reset_mousewheel_settings();

// Primary handler function
int32_t get_mousewheel_change(MousewheelHandlerConfigID handler_id,
                              int32_t x,
                              int32_t y,
                              uint16_t modstate);

// Handler function for 2D scrolling
Vector2i get_mousewheel_change_2D(MousewheelHandlerConfigID handler_id,
                                  int32_t x,
                                  int32_t y,
                                  uint16_t modstate);

#endif  // end of include guard: WL_WLAPPLICATION_MOUSEWHEEL_OPTIONS_H
