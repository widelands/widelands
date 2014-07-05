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

#ifndef WL_UI_BASIC_IS_PRINTABLE_H
#define WL_UI_BASIC_IS_PRINTABLE_H

#include <SDL.h>

namespace UI {

/**
 * Tell whether a SDL_Keysym is printable.
 *
 * \param k SDL_Keysym to be checked for printability
 *
 * \return True if k is a printable character
 *
 * \todo This is _by_far_ not complete enough
 * \todo Should be based on k.unicode (already enabled by
 * WLApplication::init_hardware()) instead of k.sym. Doing so needs to take the
 * current locale into account; perhaps there already is a fitting gettext
 * function?
 */
bool is_printable(SDL_keysym k);

}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_IS_PRINTABLE_H
