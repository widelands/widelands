/*
 * Copyright (C) 2011-2013 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_GL_GAME_RENDERER_H
#define WL_GRAPHIC_GL_GAME_RENDERER_H

#include <memory>
#include <vector>

#include "base/rect.h"
#include "graphic/game_renderer.h"
#include "graphic/gl/utils.h"
#include "logic/widelands.h"

namespace Widelands {
struct Coords;
struct FCoords;
class World;
}

class GLSurface;
class GLSurfaceTexture;

class TerrainProgram;
class DitherProgram;

/**
 * OpenGL implementation of @ref GameRenderer.
 */
class GlGameRenderer : public GameRenderer {
public:
	GlGameRenderer();
	virtual ~GlGameRenderer();

private:
	static std::unique_ptr<TerrainProgram> terrain_program_;
	static std::unique_ptr<DitherProgram> dither_program_;

	void draw_terrain_triangles();

	struct BaseVertex {
		float x;
		float y;
		float tcx;
		float tcy;
		uint8_t color[4];
		uint32_t pad[3];
	};

	struct DitherVertex {
		float x;
		float y;
		float tcx;
		float tcy;
		float edgex;
		float edgey;
		uint8_t color[4];
		uint32_t pad[1];
	};

	void draw() override;
	void prepare_roads();
	void draw_roads();

	uint8_t field_brightness(const Widelands::FCoords & coords) const;
	uint8_t field_roads(const Widelands::FCoords & coords) const;
	template<typename vertex>
	void compute_basevertex(const Widelands::Coords & coords, vertex & vtx) const;

	/**
	 * The following variables are only valid during rendering.
	 */
	/*@{*/
	GLSurface * m_surface;

	/// Bounding rectangle inside the destination surface
	Rect m_rect;

	/// Translation from map pixel coordinates to surface pixel coordinates
	/// (relative to the top-left corner of the surface, @b not relative
	/// to the bounding rectangle)
	Point m_surface_offset;

	uint32_t m_road_freq[2];
	std::unique_ptr<BaseVertex[]> m_road_vertices;
	uint32_t m_road_vertices_size;
	/*@}*/
};

#endif  // end of include guard: WL_GRAPHIC_GL_GAME_RENDERER_H
