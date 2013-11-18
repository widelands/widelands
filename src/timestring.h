/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include <stdint.h>

/// Get a string representation conforming to ISO 8601 of the current time (in
/// seconds since the Epoch). The return value points to a statically allocated
/// string which might be overwritten by subsequent calls.
char * timestring();

/// Get a string representation of the game time
/// as hhh:mm:ss. If Time represents more than
/// 999 hours, it wraps around
char * gametimestring(uint32_t gametime);
