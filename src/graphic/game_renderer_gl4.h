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

#include <vector>

#include "graphic/game_renderer.h"
#include "graphic/minimap_layer.h"
#include "logic/widelands_geometry.h"

class TerrainInformationGl4;

/**
 * This structure is used for the @ref RenderQueue by terrain rendering, road
 * rendering, and minimap rendering. Each use only uses a subset of memers.
 */
struct TerrainGl4Arguments {
	struct Road {
		Widelands::Coords coord;

		// One of the RoadTypes
		uint8_t type;

		// One of the WalkingDirs
		uint8_t direction;

		// Player number
		uint8_t owner;

		Road(Widelands::Coords coord_, uint8_t type_, uint8_t direction_, uint8_t owner_)
		  : coord(coord_), type(type_), direction(direction_), owner(owner_) {
		}
	};
	static_assert(sizeof(Road) == 8, "bad alignment");

	std::shared_ptr<TerrainInformationGl4> terrain;
	float zoom;
	Vector2f surface_offset;
	int surface_width;
	int surface_height;
	int minfx, minfy, maxfx, maxfy;
	std::vector<Road> roads;

	int minimap_tl_fx, minimap_tl_fy;
	MiniMapLayer minimap_layers;
};

/**
 * This is the front-end of the GL4 rendering path game renderer.
 */
class GameRendererGl4 : public GameRenderer {
public:
	GameRendererGl4();
	virtual ~GameRendererGl4();

	static bool supported();

	void draw(const Widelands::EditorGameBase& egbase,
	          const Vector2f& view_offset,
	          const float zoom,
	          const TextToDraw draw_text,
	          const Widelands::Player* player,
	          RenderTarget* dst) override;

private:
	void scan_fields(const Vector2f& view_offset);

	TerrainGl4Arguments args_;
	FieldsToDrawBase fields_to_draw_;

	DISALLOW_COPY_AND_ASSIGN(GameRendererGl4);
};

#endif  // end of include guard: WL_GRAPHIC_GAME_RENDERER_H
