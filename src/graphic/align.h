/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef ALIGN_H
#define ALIGN_H

#include "point.h"

namespace UI {

enum Align {
	Align_Left         =  0,
	Align_HCenter      =  1,
	Align_Right        =  2,
	Align_Horizontal   =  3,

	Align_Top          =  0,
	Align_VCenter      =  4,
	Align_Bottom       =  8,
	Align_Vertical     = 12,

	Align_TopLeft      =  0,
	Align_CenterLeft   = Align_VCenter,
	Align_BottomLeft   = Align_Bottom,

	Align_TopCenter    = Align_HCenter,
	Align_Center       = Align_HCenter|Align_VCenter,
	Align_BottomCenter = Align_HCenter|Align_Bottom,

	Align_TopRight     = Align_Right,
	Align_CenterRight  = Align_Right|Align_VCenter,

	Align_BottomRight  = Align_Right|Align_Bottom,
};

void correct_for_align(Align, uint32_t w, uint32_t h, Point* pt);

}
#endif
