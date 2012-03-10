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


#include "gameview_opengl.h"

#include "constants.h"
#include "economy/road.h"
#include "graphic/graphic.h"
#include "graphic/texture.h"

#include "terrain_opengl.h"

using namespace  Widelands;

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
void GameViewOpenGL::draw_field
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
	// Draw triangle right (bottom) of the field
	draw_field_opengl
		(subwin, f_vert, br_vert, r_vert, f_r_texture, f_d_texture, tr_d_texture);
	// Draw triangle bottom of the field
	draw_field_opengl
		(subwin, f_vert, bl_vert, br_vert, f_d_texture, l_r_texture, f_d_texture);

	// Draw the roads
	draw_roads_opengl(subwin, roads, f_vert, r_vert, bl_vert, br_vert);
}

void GameViewOpenGL::rendermap_init()
{
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef
		(1.0f / static_cast<GLfloat>(TEXTURE_WIDTH),
		 1.0f / static_cast<GLfloat>(TEXTURE_HEIGHT), 1);
	glDisable(GL_BLEND);

	// Use scissor test to clip the window. This takes coordinates in screen
	// coordinates (y goes from bottom to top)
	glScissor
		(m_rect.x, g_gr->get_yres() - m_rect.y - m_rect.h,
		 m_rect.w, m_rect.h);
	glEnable(GL_SCISSOR_TEST);
}

void GameViewOpenGL::rendermap_deint()
{
	glDisable(GL_SCISSOR_TEST);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
}
