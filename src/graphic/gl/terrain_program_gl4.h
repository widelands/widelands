/*
 * Copyright (C) 2006-2016 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_GL_TERRAIN_PROGRAM_GL4_H
#define WL_GRAPHIC_GL_TERRAIN_PROGRAM_GL4_H

#include <map>
#include <memory>
#include <unordered_map>

#include "base/point.h"
#include "graphic/gl/streaming_buffer.h"
#include "graphic/gl/utils.h"

namespace Widelands {
class EditorGameBase;
class Player;
}

struct TerrainGl4Arguments;
class Texture;

/**
 * This class maintains the permanent terrain information texture.
 */
class TerrainBaseGl4 {
public:
	~TerrainBaseGl4();

	// Get the global terrain base instance associated to the given editor/game
	// instance.
	static std::shared_ptr<TerrainBaseGl4>
	get(const Widelands::EditorGameBase& egbase);

	const Widelands::EditorGameBase& egbase() const {
		return egbase_;
	}

	GLuint texture() const {
		return fields_texture_;
	}

	// Upload updated information to texture(s) if necessary.
	void update();

private:
	TerrainBaseGl4(const Widelands::EditorGameBase& egbase);

	void do_update();

	struct PerFieldData {
		uint8_t terrain_r;
		uint8_t terrain_d;
		uint8_t height;

		// Will be interpreted as unsigned by the texel fetch in the shader.
		int8_t brightness;

		PerFieldData(uint8_t terrain_r_, uint8_t terrain_d_, uint8_t height_, int8_t brightness_)
		  : terrain_r(terrain_r_), terrain_d(terrain_d_), height(height_), brightness(brightness_) {
		}
	};

	typedef std::unordered_map<const Widelands::EditorGameBase*, std::weak_ptr<TerrainBaseGl4>> GlobalMap;

	static GlobalMap global_map_;

	const Widelands::EditorGameBase& egbase_;
	uint32_t fields_base_version_;

	// The texture containing per-field information.
	GLuint fields_texture_;

	Gl::StreamingBuffer<PerFieldData> uploads_;

	DISALLOW_COPY_AND_ASSIGN(TerrainBaseGl4);
};

/**
 * This class maintains the terrain information textures that depend on player
 * perspective.
 */
class TerrainPlayerPerspectiveGl4 {
public:
	~TerrainPlayerPerspectiveGl4();

	// Get the global instance associated to the given editor/game instance
	// and player perspective. If player is nullptr, an omniscient perspective
	// is returned.
	static std::shared_ptr<TerrainPlayerPerspectiveGl4>
	get(const Widelands::EditorGameBase& egbase,
	    const Widelands::Player* player = nullptr);

	const Widelands::EditorGameBase& egbase() const {
		return egbase_;
	}

	const Widelands::Player* player() const {
		return player_;
	}

	// Upload updated information to texture(s) if necessary.
	void update();

private:
	TerrainPlayerPerspectiveGl4(const Widelands::EditorGameBase& egbase,
	                            const Widelands::Player* player);

	typedef std::pair<const Widelands::EditorGameBase*, const Widelands::Player*> GlobalKey;
	typedef std::map<GlobalKey, std::weak_ptr<TerrainPlayerPerspectiveGl4>> GlobalMap;

	static GlobalMap global_map_;

	const Widelands::EditorGameBase& egbase_;
	const Widelands::Player* player_;

	DISALLOW_COPY_AND_ASSIGN(TerrainPlayerPerspectiveGl4);
};

class TerrainProgramGl4 {
public:
	// The patch height must be even. The patch size is chosen so that the
	// number of vertices per patch is close to a power of two.
	static constexpr unsigned kPatchWidth = 5;
	static constexpr unsigned kPatchHeight = 4;

public:
	TerrainProgramGl4();
	~TerrainProgramGl4();

	static bool supported();

	// Draws the terrain.
	void draw(const TerrainGl4Arguments* args,
	          uint32_t gametime,
	          float z_value);

private:
	void init_vertex_data();
	void upload_terrain_data(const TerrainGl4Arguments* args, uint32_t gametime);
	unsigned upload_instance_data(const TerrainGl4Arguments* args);

	struct PerTerrainData {
		FloatPoint offset;
		int dither_layer;
		float padding[1];
	};
	static_assert(sizeof(PerTerrainData) == 16, "incorrect padding");

	struct PerInstanceData {
		Point coordinate;
		FloatPoint basepix;
	};
	static_assert(sizeof(PerInstanceData) == 16, "incorrect padding");

	struct PerVertexData {
		Point vertex_coordinate;
		Point triangle_coordinate;

		PerVertexData(int vx, int vy, int tx, int ty, bool r, uint dither_vid)
		  : vertex_coordinate(vx, vy), triangle_coordinate((tx << 3) | (dither_vid << 1) | r, ty) {
		}
	};
	static_assert(sizeof(PerVertexData) == 16, "incorrect padding");

	// The program used for drawing the terrain.
	Gl::Program gl_program_;

	// Uniform buffer with per-terrain information.
	Gl::StreamingBuffer<PerTerrainData> terrain_data_;

	// Per-instance/patch data.
	Gl::StreamingBuffer<PerInstanceData> instance_data_;

	// Per-vertex data.
	Gl::Buffer<PerVertexData> vertex_data_;

	std::unique_ptr<Texture> dither_mask_;

	// Vertex attributes.
	GLint in_vertex_coordinate_;
	GLint in_patch_coordinate_;
	GLint in_patch_basepix_;

	// Uniforms.
	GLint u_position_scale_;
	GLint u_position_offset_;
	GLint u_z_value_;
	GLint u_texture_dimensions_;

	GLint u_terrain_base_;
	GLint u_terrain_texture_;
	GLint u_dither_texture_;

	// Uniform block.
	GLint block_terrains_idx_;

	DISALLOW_COPY_AND_ASSIGN(TerrainProgramGl4);
};

#endif  // end of include guard: WL_GRAPHIC_GL_TERRAIN_PROGRAM_GL4_H
