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

#ifndef WIDELANDS_GAMERENDERER_GL_H
#define WIDELANDS_GAMERENDERER_GL_H

#include <memory>
#include <vector>

#include "graphic/render/gamerenderer.h"
#include "logic/widelands.h"
#include "rect.h"

namespace Widelands {
struct Coords;
struct FCoords;
struct World;
}

class GLSurface;
class GLSurfaceTexture;

/**
 * OpenGL implementation of @ref GameRenderer.
 */
class GameRendererGL : public GameRenderer {
public:
	GameRendererGL();
	virtual ~GameRendererGL();

private:
	struct basevertex {
		float x;
		float y;
		float tcx;
		float tcy;
		uint8_t color[4];
		uint32_t pad[3];
	};

	struct dithervertex {
		float x;
		float y;
		float tcx;
		float tcy;
		float edgex;
		float edgey;
		uint8_t color[4];
		uint32_t pad[1];
	};

	const GLSurfaceTexture * get_dither_edge_texture(const Widelands::World & world);

	void draw() override;
	void prepare_terrain_base();
	void collect_terrain_base(bool onlyscan);
	void count_terrain_base(Widelands::Terrain_Index ter);
	void add_terrain_base_triangle
		(Widelands::Terrain_Index ter,
		 const Widelands::Coords & p1, const Widelands::Coords & p2, const Widelands::Coords & p3);
	void draw_terrain_base();
	void prepare_terrain_dither();
	void collect_terrain_dither(bool onlyscan);
	void add_terrain_dither_triangle
		(bool onlyscan, Widelands::Terrain_Index ter,
		 const Widelands::Coords & edge1, const Widelands::Coords & edge2,
		 const Widelands::Coords & opposite);
	void draw_terrain_dither();
	void prepare_roads();
	void draw_roads();

	uint32_t patch_index(const Widelands::Coords & f) const;
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

	Rect m_patch_size;
	std::unique_ptr<basevertex[]> m_patch_vertices;
	uint32_t m_patch_vertices_size;
	std::unique_ptr<uint16_t[]> m_patch_indices;
	uint32_t m_patch_indices_size;
	std::vector<uint32_t> m_patch_indices_indexs;
	std::vector<uint32_t> m_terrain_freq;
	std::vector<uint32_t> m_terrain_freq_cum;
	std::unique_ptr<dithervertex[]> m_edge_vertices;
	uint32_t m_edge_vertices_size;
	std::vector<uint32_t> m_terrain_edge_freq;
	std::vector<uint32_t> m_terrain_edge_freq_cum;
	std::vector<uint32_t> m_terrain_edge_indexs;

	uint32_t m_road_freq[2];
	std::unique_ptr<basevertex[]> m_road_vertices;
	uint32_t m_road_vertices_size;
	/*@}*/
};

#endif // WIDELANDS_GAMERENDERER_GL_H
