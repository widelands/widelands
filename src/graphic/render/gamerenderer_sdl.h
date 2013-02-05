/*
 * Copyright (C) 2010-2013 by the Widelands Development Team
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

#ifndef WIDELANDS_GAMERENDERER_SDL_H
#define WIDELANDS_GAMERENDERER_SDL_H

#include "gamerenderer.h"

struct Texture;
struct Vertex;

/**
 * Software-rendering implementation of @ref GameRenderer.
 */
struct GameRendererSDL : GameRenderer {
	void rendermap
		(RenderTarget & dst,
		 Widelands::Editor_Game_Base const &       egbase,
		 Widelands::Player           const &       player,
		 Point                                     viewofs);
	void rendermap
		(RenderTarget & dst,
		 Widelands::Editor_Game_Base const & egbase,
		 Point                               viewofs);

private:
	/**
	 * Helper function to draw two terrain triangles. This is called from the
	 * rendermap() functions.
	 */
	void draw_field
		(RenderTarget & dst,
		 Vertex  const &  f_vert,
		 Vertex  const &  r_vert,
		 Vertex  const & bl_vert,
		 Vertex  const & br_vert,
		 uint8_t         roads,
		 Texture const & tr_d_texture,
		 Texture const &  l_r_texture,
		 Texture const &  f_d_texture,
		 Texture const &  f_r_texture);
};

#endif // WIDELANDS_GAMERENDERER_SDL_H
