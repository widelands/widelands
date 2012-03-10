/*
 * Copyright (C) 2011-2012 by the Widelands Development Team
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

#include "gameview_sdl.h"

#include "upcast.h"

#include "economy/road.h"
#include "economy/flag.h"

#include "wui/minimap.h"
#include "wui/mapviewpixelconstants.h"

#include "logic/field.h"
#include "logic/map.h"
#include "logic/player.h"

#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/surface.h"
#include "surface_sdl.h"
#include "graphic/texture.h"

#include "wui/overlay_manager.h"

#include "terrain_sdl.h"


/**
 * Draw ground textures and roads for the given parallelogram (two triangles)
 * into the bitmap.
 *
 * Vertices:
 *   - f_vert vertex of the field
 *   - r_vert vertex right of the field
 *   - bl_vert vertex bottom left of the field
 *   - br_vert vertex bottom right of the field
 *
 * Textures:
 *   - f_r_texture Terrain of the triangle right of the field
 *   - f_d_texture Terrain of the triangle under of the field
 *   - tr_d_texture Terrain of the triangle top of the right triangle ??
 *   - l_r_texture Terrain of the triangle left of the down triangle ??
 *
 *             (tr_d)
 *
 *       (f) *------* (r)
 *          / \  r /
 *  (l_r)  /   \  /
 *        /  d  \/
 *  (bl) *------* (br)
 */
void GameViewSDL::draw_field
	(Rect          & subwin,
	 Vertex  const &  f_vert,
	 Vertex  const &  r_vert,
	 Vertex  const & bl_vert,
	 Vertex  const & br_vert,
	 uint8_t         roads,
	 Texture const & tr_d_texture,
	 Texture const &  l_r_texture,
	 Texture const &  f_d_texture,
	 Texture const &  f_r_texture)
{
	upcast(SurfaceSDL, sdlsurf, m_surface.get());
	if (sdlsurf)
	{
		sdlsurf->set_subwin(subwin);
		switch (sdlsurf->format().BytesPerPixel) {
		case 2:
			draw_field_int<Uint16>
				(*sdlsurf,
				 f_vert, r_vert, bl_vert, br_vert,
				 roads,
				 tr_d_texture, l_r_texture, f_d_texture, f_r_texture);
			break;
		case 4:
			draw_field_int<Uint32>
				(*sdlsurf,
				 f_vert, r_vert, bl_vert, br_vert,
				 roads,
				 tr_d_texture, l_r_texture, f_d_texture, f_r_texture);
			break;
		default:
			assert(false);
		}
		sdlsurf->unset_subwin();
	}
}
