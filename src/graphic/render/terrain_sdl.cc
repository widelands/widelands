/*
 * Copyright (C) 2002-2004, 2006, 2009-2010 by the Widelands Development Team
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

#include "graphic/render/terrain_sdl.h"

/// get lambda and mu so that
/// lambda * u + mu * v = (1 0)^T with u = (u1 u2)^T and v = (v1 v2)^T
void get_horiz_linearcomb
	(int32_t const u1, int32_t const u2, int32_t const v1, int32_t const v2,
	 float & lambda, float & mu)
{
	float det;

	det = u1 * v2 - u2 * v1; //  determinant of (u v)

	lambda = v2 / det;       //  by Cramer's rule
	mu = -u2 / det;
}
