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

#ifndef WL_GRAPHIC_ALIGN_H
#define WL_GRAPHIC_ALIGN_H

#include "base/point.h"

namespace UI {

enum class Align {
	Align_Left         =  0,
	Align_HCenter      =  1,
	Align_Right        =  2,
	Align_Horizontal   =  3,

	Align_Top          =  0,
	Align_VCenter      =  4,
	Align_Bottom       =  8,
	Align_Vertical     = 12,

	Align_TopLeft      =  0,
	Align_CenterLeft   = Align::Align_VCenter,
	Align_BottomLeft   = Align::Align_Bottom,

	Align_TopCenter    = Align::Align_HCenter,
	Align_Center       = Align::Align_HCenter | Align::Align_VCenter,
	Align_BottomCenter = Align::Align_HCenter | Align::Align_Bottom,

	Align_TopRight     = Align::Align_Right,
	Align_CenterRight  = Align::Align_Right | Align::Align_VCenter,

	Align_BottomRight  = Align::Align_Right | Align::Align_Bottom,
};

inline Align operator &(Align a, Align b)
{
	 return static_cast<Align>(static_cast<int>(a) & static_cast<int>(b));
}

inline Align operator |(Align a, Align b)
{
	 return static_cast<Align>(static_cast<int>(a) | static_cast<int>(b));
}

Align mirror_alignment(Align alignment);
void correct_for_align(Align, uint32_t w, uint32_t h, Point* pt);

}
#endif  // end of include guard: WL_GRAPHIC_ALIGN_H
