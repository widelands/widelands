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

#ifndef WL_GRAPHIC_MOUSE_CURSOR_H
#define WL_GRAPHIC_MOUSE_CURSOR_H

#include <SDL_mouse.h>

#include "base/vector.h"

class RenderTarget;
class Image;

/**
 * This class manages drawing the mouse cursor.
 * It can be used in two modes:
 * In the SDL mode the cursor is set using SDL calls and is rendered by the OS.
 * Most of the time it offers better performance.
 * If the SDL mode is disabled the cursor is rendered by Widelands.
 * This is be useful e.g. if we want the cursor to be present on screenshots.
 */
class MouseCursor {
public:
	MouseCursor() = default;
	~MouseCursor();

	// Call initialize before first use
	void initialize(bool init_use_sdl = true);

	// Enable/disable SDL mode
	void set_use_sdl(bool init_use_sdl);
	bool is_using_sdl() const;

	// Switch between "normal" and "pressed" cursors
	void change_cursor(bool is_pressed);

	// Hide/show the cursor
	void set_visible(bool visible);
	bool is_visible() const;

	// Render the cursor (does nothing in SDL mode)
	void draw(RenderTarget& rt, Vector2i position);

private:
	bool use_sdl_ = false;
	bool was_pressed_ = false;
	bool visible_ = true;

	// Used when SDL mode is disabled
	const Image* default_cursor_ = nullptr;
	const Image* default_cursor_click_ = nullptr;

	// Used in SDL mode
	SDL_Surface* default_cursor_sdl_surface_ = nullptr;
	SDL_Surface* default_cursor_click_sdl_surface_ = nullptr;
	SDL_Cursor* default_cursor_sdl_ = nullptr;
	SDL_Cursor* default_cursor_click_sdl_ = nullptr;
};

extern MouseCursor* g_mouse_cursor;

#endif  // end of include guard: WL_GRAPHIC_MOUSE_CURSOR_H
