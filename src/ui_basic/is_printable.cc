/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "ui_basic/is_printable.h"

namespace UI {

	// NOCOM(#sirver): who uses this?
bool is_printable(SDL_Keysym k) {
	return (k.sym == SDLK_TAB) || ((k.sym >= SDLK_SPACE) && (k.sym <= SDLK_z)) ||
			 ((k.sym >= SDL_SCANCODE_KP_0) && (k.sym <= SDL_SCANCODE_KP_EQUALS));
}

}  // namespace UI
