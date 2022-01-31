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

#include "graphic/animation/diranimations.h"

DirAnimations::DirAnimations(
   uint32_t dir1, uint32_t dir2, uint32_t dir3, uint32_t dir4, uint32_t dir5, uint32_t dir6) {
	animations_[0] = dir1;
	animations_[1] = dir2;
	animations_[2] = dir3;
	animations_[3] = dir4;
	animations_[4] = dir5;
	animations_[5] = dir6;
}
