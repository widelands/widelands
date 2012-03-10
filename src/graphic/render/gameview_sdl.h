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

#ifndef GAMEVIEW_SDL_H_
#define GAMEVIEW_SDL_H_

#include "gameview.h"

class GameViewSDL : public GameView
{
public:
	GameViewSDL(RenderTarget & rt):
		GameView(rt) {}
	~GameViewSDL() {}

protected:
	/**
	 * Helper function to draw two terrain triangles. This is called from the
	 * rendermap() functions.
	 */
	void draw_field
		(Rect          & subwin,
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

#endif /* GAMEVIEW_SDL_H_ */
