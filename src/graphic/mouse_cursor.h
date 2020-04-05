/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_MOUSE_CURSOR_H
#define WL_GRAPHIC_MOUSE_CURSOR_H

#include <SDL.h>

#include "base/vector.h"

class RenderTarget;
class Image;

class MouseCursor {
public:
	MouseCursor();
	~MouseCursor();

	void initialize(bool init_use_sdl);
	void set_use_sdl(bool init_use_sdl);
	bool is_using_sdl() const;
	void change_cursor(bool is_pressed);
	void draw(RenderTarget& rt, Vector2i position);

private:
	bool use_sdl_ = false;
	bool was_pressed_ = false;

	const Image* default_cursor_ = nullptr;
	const Image* default_cursor_click_ = nullptr;

	SDL_Surface* default_cursor_sdl_surface_ = nullptr;
	SDL_Surface* default_cursor_click_sdl_surface_ = nullptr;
	SDL_Cursor* default_cursor_sdl_ = nullptr;
	SDL_Cursor* default_cursor_click_sdl_ = nullptr;
};

extern MouseCursor* g_mouse_cursor;

#endif  // end of include guard: WL_GRAPHIC_MOUSE_CURSOR_H
