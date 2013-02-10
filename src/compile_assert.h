/*
 * Copyright (C) 2006 by the Widelands Development Team
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

#ifndef compile_assert
// gnu needs to append a unique number to the assert, otherwise it complains
// about this shadowing the last assert. This is not too easy, so we use the
// line number - collisions are not that probable anymore.
// The concatenation magic is from: http://stackoverflow.com/a/1597129/200945
#define TOKENPASTE(x, y) x ## y
#define TOKENPASTE2(x, y) TOKENPASTE(x, y)
#define compile_assert(x) typedef bool TOKENPASTE2(COMPILE_ASSERT_, __LINE__)[(x) ? 1 : -1]
#endif
