/*
 * Copyright (C) 2010-2016 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_GAME_RENDERER_GL4_H
#define WL_GRAPHIC_GAME_RENDERER_GL4_H

#include "graphic/game_renderer.h"

class TerrainBaseGl4;
class TerrainPlayerPerspectiveGl4;

struct TerrainGl4Arguments {
	std::shared_ptr<TerrainBaseGl4> terrain;
	std::shared_ptr<TerrainPlayerPerspectiveGl4> perspective;
	Point surface_offset;
	int surface_width;
	int surface_height;
	int minfx, minfy, maxfx, maxfy;
};

/**
 * This is the front-end of the GL4 rendering path game renderer.
 */
class GameRendererGl4 : public GameRenderer {
public:
	GameRendererGl4();
	virtual ~GameRendererGl4();

	static bool supported();

	void draw(RenderTarget& dst,
	          const Widelands::EditorGameBase& egbase,
	          const Point& view_offset,
	          const Widelands::Player* player) override;

private:
	TerrainGl4Arguments args_;

	DISALLOW_COPY_AND_ASSIGN(GameRendererGl4);
};

#endif  // end of include guard: WL_GRAPHIC_GAME_RENDERER_H
