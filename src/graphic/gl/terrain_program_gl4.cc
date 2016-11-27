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

#include "graphic/gl/terrain_program_gl4.h"

#include "logic/editor_game_base.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/map.h"
#include "graphic/game_renderer_gl4.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"
#include "graphic/image_io.h"
#include "io/filesystem/layered_filesystem.h"
#include "wui/mapviewpixelfunctions.h"

using namespace Widelands;

/**
 * This is the back-end of the GL4 rendering path.
 *
 * TODO(nha): upload based on dirtiness and/or "rolling" updates of N% per frame
 *
 * TODO(nha): terrain knowledge is actually per-player
 *
 * Per-field data is uploaded directly into integer-valued textures that span
 * the entire map, and vertex shaders do most of the heavy lifting. The
 * following textures are used:
 *
 * TerrainBaseGl4: data that is independent of player perspective, GL_RGBA8UI:
 *  R = r-triangle texture index
 *  G = d-triangle texture index
 *  B = height
 *  A = brightness
 * This information is constant during the game but can be modified in the
 * editor.
 *
 * TerrainPlayerPerspectiveGl4: data that depends on the player (or omniscient)
 * perspective:
 *  - Variable information (GL_R8UI):
 *     R = player-perspective-dependent brightness modulation
 *    This information is re-uploaded every frame.
 *
 *  - Semi-permanent information (GL_RG8UI):
 *     R = field ownership
 *     B = road/flag/building data
 *    This information is re-uploaded every frame when a minimap is shown.
 *
 * Terrain is rendered in patches of a fixed structure, and many patches are
 * rendered in one call via instancing. Per-instance data is processed in a
 * vertex shader.
 *
 * Each patch consists of the triangles associated to a WxH "rectangle" of
 * fields, where the top-left field must be at an even y-coordinate, and H is
 * even, e.g. a 2x4-patch:
 *
 *       (0,0)
 *           O-------O-------*
 *          / \     / \     /
 *         /   \   /   \   /
 *        /     \ /     \ /
 *       *-------O-------O-------*
 *              / \     / \     /
 *             /   \   /   \   /
 *            /     \ /     \ /
 *           O-------O-------*
 *          / \     / \     /
 *         /   \   /   \   /
 *        /     \ /     \ /
 *       *-------O-------O-------*
 *              / \     / \     /
 *             /   \   /   \   /
 *            /     \ /     \ /
 *           *-------*-------*
 *
 * OpenGL vertices of triangles are not shared; this allows separate textures
 * and dithering in a single pass.
 *
 * Road rendering is also handled here. Roads are rendered as two triangles per
 * segment. Only per-road data is uploaded; the vertex shader sources data
 * from the per-road buffer based on the vertex ID, and an index buffer
 * (element array buffer in OpenGL terms) is used to share two vertices between
 * the triangles that make up each segment.
 */

TerrainBaseGl4::GlobalMap TerrainBaseGl4::global_map_;

std::shared_ptr<TerrainBaseGl4>
TerrainBaseGl4::get(const EditorGameBase& egbase) {
	auto it = global_map_.find(&egbase);
	if (it != global_map_.end())
		return it->second.lock();

	std::shared_ptr<TerrainBaseGl4> instance(new TerrainBaseGl4(egbase));
	global_map_[&egbase] = instance;
	return instance;
}

TerrainBaseGl4::TerrainBaseGl4(const EditorGameBase& egbase)
  : egbase_(egbase), uploads_(GL_PIXEL_UNPACK_BUFFER) {
	glGenTextures(1, &fields_texture_);

	auto& gl = Gl::State::instance();
	gl.bind(GL_TEXTURE0, fields_texture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	do_update();
	upload_road_textures();
}

TerrainBaseGl4::~TerrainBaseGl4() {
	glDeleteTextures(1, &fields_texture_);

	global_map_.erase(&egbase_);
}

void TerrainBaseGl4::update() {
	if (fields_base_version_ != egbase().map().get_fields_base_version())
		do_update();
}

void TerrainBaseGl4::do_update() {
	auto& gl = Gl::State::instance();
	const Map& map = egbase().map();
	auto stream = uploads_.stream(uint(map.get_width()) * map.get_height());
	MapIndex max_index = map.max_index();

	for (MapIndex i = 0; i < max_index; ++i) {
		const Field& f = map[i];
		stream.emplace_back(f.terrain_r(), f.terrain_d(), f.get_height(), f.get_brightness());
	}

	GLintptr offset = stream.unmap();
	uploads_.bind();

	gl.bind(GL_TEXTURE0, fields_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, map.get_width(), map.get_height(), 0,
				 GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, reinterpret_cast<void*>(offset));

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	fields_base_version_ = map.get_fields_base_version();
}

TerrainBaseGl4::PerRoadTextureData::PerRoadTextureData(const FloatRect& rect)
  : x(rect.x), y(rect.y), w(rect.w), h(rect.h) {
}

void TerrainBaseGl4::upload_road_textures() {
	std::vector<PerRoadTextureData> roads;
	std::map<const TribeDescr*, unsigned> tribe_map;
	PlayerNumber const nr_players = egbase().map().get_nrplayers();

	player_roads_.resize(nr_players + 1);

	iterate_players_existing_const(p, nr_players, egbase(), player) {
		const TribeDescr& tribe = player->tribe();
		auto it = tribe_map.find(&tribe);
		if (it != tribe_map.end()) {
			player_roads_[p] = player_roads_[it->second];
		} else {
			const auto& normal_textures = tribe.road_textures().get_normal_textures();
			player_roads_[p].normal_roads = roads.size();
			player_roads_[p].num_normal_roads = normal_textures.size();
			for (const Image* image : normal_textures) {
				const BlitData& blit_data = image->blit_data();
				roads.emplace_back(to_gl_texture(blit_data));
				road_texture_object_ = blit_data.texture_id;
			}

			const auto& busy_textures = tribe.road_textures().get_busy_textures();
			player_roads_[p].busy_roads = roads.size();
			player_roads_[p].num_busy_roads = busy_textures.size();
			for (const Image* image : busy_textures)
				roads.emplace_back(to_gl_texture(image->blit_data()));

			tribe_map[&tribe] = p;
		}
	}

	road_textures_.bind();
	road_textures_.update(roads);
}

unsigned TerrainBaseGl4::road_texture_idx(PlayerNumber owner,
                                          RoadType road_type,
                                          const Coords& coords,
                                          WalkingDir direction) const {
	const PlayerRoads& roads = player_roads_[owner];
	unsigned base, count;

	if (road_type == RoadType::kNormal) {
		base = roads.normal_roads;
		count = roads.num_normal_roads;
	} else {
		base = roads.busy_roads;
		count = roads.num_busy_roads;
	}

	return base + unsigned(coords.x + coords.y + direction) % count;
}

TerrainPlayerPerspectiveGl4::GlobalMap TerrainPlayerPerspectiveGl4::global_map_;

std::shared_ptr<TerrainPlayerPerspectiveGl4>
TerrainPlayerPerspectiveGl4::get(const Widelands::EditorGameBase& egbase,
                                 const Widelands::Player* player) {
	GlobalKey key(&egbase, player);
	auto it = global_map_.find(key);
	if (it != global_map_.end())
		return it->second.lock();

	std::shared_ptr<TerrainPlayerPerspectiveGl4> instance(
		new TerrainPlayerPerspectiveGl4(egbase, player));
	global_map_[key] = instance;
	return instance;
}

TerrainPlayerPerspectiveGl4::TerrainPlayerPerspectiveGl4(const Widelands::EditorGameBase& egbase,
                                                         const Widelands::Player* player)
  : egbase_(egbase), player_(player), uploads_(GL_PIXEL_UNPACK_BUFFER) {
	glGenTextures(1, &brightness_texture_);
}

TerrainPlayerPerspectiveGl4::~TerrainPlayerPerspectiveGl4() {
	if (brightness_texture_)
		glDeleteTextures(1, &brightness_texture_);
}

void TerrainPlayerPerspectiveGl4::update() {
	auto& gl = Gl::State::instance();
	bool see_all = !player_ || player_->see_all();

	// TODO(nha): update only what is necessary

	if (see_all) {
		auto stream = uploads_.stream(1);
		stream.emplace_back(255);
		GLintptr offset = stream.unmap();
		uploads_.bind();

		gl.bind(GL_TEXTURE0, brightness_texture_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE,
		             reinterpret_cast<void*>(offset));
	} else {
		const Map& map = egbase().map();
		uint32_t gametime = egbase().get_gametime();
		MapIndex max_index = map.max_index();
		auto stream = uploads_.stream(max_index);
		uint8_t* data = stream.add(max_index);

		for (MapIndex i = 0; i < max_index; ++i) {
			const Player::Field& pf = player_->fields()[i];
			if (pf.vision == 0) {
				*data++ = 0;
			} else if (pf.vision == 1) {
				static const uint32_t kDecayTimeInMs = 20000;
				const Duration time_ago = gametime - pf.time_node_last_unseen;
				if (time_ago < kDecayTimeInMs) {
					*data++ = 255 * (2 * kDecayTimeInMs - time_ago) / (2 * kDecayTimeInMs);
				} else {
					*data++ = 128;
				}
			} else {
				*data++ = 255;
			}
		}

		GLintptr offset = stream.unmap();
		uploads_.bind();

		gl.bind(GL_TEXTURE0, brightness_texture_);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, map.get_width(), map.get_height(), 0,
		             GL_RED, GL_UNSIGNED_BYTE, reinterpret_cast<void*>(offset));
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

TerrainProgramGl4::Terrain::Terrain()
  : terrain_data(GL_UNIFORM_BUFFER), instance_data(GL_ARRAY_BUFFER) {
	// Initialize program.
	gl_program.build_vp_fp({"terrain_gl4", "terrain_common_gl4"}, {"terrain_gl4"});

	in_vertex_coordinate = glGetAttribLocation(gl_program.object(), "in_vertex_coordinate");
	in_patch_coordinate = glGetAttribLocation(gl_program.object(), "in_patch_coordinate");

	u_position_scale = glGetUniformLocation(gl_program.object(), "u_position_scale");
	u_position_offset = glGetUniformLocation(gl_program.object(), "u_position_offset");
	u_z_value = glGetUniformLocation(gl_program.object(), "u_z_value");
	u_texture_dimensions = glGetUniformLocation(gl_program.object(), "u_texture_dimensions");

	u_terrain_base = glGetUniformLocation(gl_program.object(), "u_terrain_base");
	u_player_brightness = glGetUniformLocation(gl_program.object(), "u_player_brightness");
	u_terrain_texture = glGetUniformLocation(gl_program.object(), "u_terrain_texture");
	u_dither_texture = glGetUniformLocation(gl_program.object(), "u_dither_texture");

	block_terrains_idx = glGetUniformBlockIndex(gl_program.object(), "block_terrains");
}

TerrainProgramGl4::Terrain::~Terrain() {
}

TerrainProgramGl4::Roads::Roads()
  : road_data(GL_SHADER_STORAGE_BUFFER) {
	num_index_roads = 0;

	// Initialize program.
	gl_program.build_vp_fp({"road_gl4", "terrain_common_gl4"}, {"road"});

	u_position_scale = glGetUniformLocation(gl_program.object(), "u_position_scale");
	u_position_offset = glGetUniformLocation(gl_program.object(), "u_position_offset");
	u_z_value = glGetUniformLocation(gl_program.object(), "u_z_value");

	u_terrain_base = glGetUniformLocation(gl_program.object(), "u_terrain_base");
	u_player_brightness = glGetUniformLocation(gl_program.object(), "u_player_brightness");
	u_texture = glGetUniformLocation(gl_program.object(), "u_texture");

	block_textures_idx = glGetUniformBlockIndex(gl_program.object(), "block_textures");
}

TerrainProgramGl4::Roads::~Roads() {
}

TerrainProgramGl4::TerrainProgramGl4() {
	// Initialize vertex buffer (every instance/path has the same structure).
	init_vertex_data();

	// Load mask texture for dithering.
	terrain_.dither_mask.reset(new Texture(load_image_as_sdl_surface("world/pics/edge.png", g_fs), true));

	Gl::State::instance().bind(GL_TEXTURE0, terrain_.dither_mask->blit_data().texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(GL_CLAMP_TO_EDGE));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(GL_CLAMP_TO_EDGE));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(GL_LINEAR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(GL_LINEAR));
}

TerrainProgramGl4::~TerrainProgramGl4() {
}

bool TerrainProgramGl4::supported() {
	// TODO(nha): proper implementation
	// GLSL >= 1.30
	// GL_ARB_uniform_buffer_object
	// GL_ARB_separate_shader_objects
	// GL_ARB_shader_storage_buffer_object
	return true;
}

void TerrainProgramGl4::draw(const TerrainGl4Arguments* args,
                             uint32_t gametime,
                             float z_value) {
	auto& gl = Gl::State::instance();

	// First, draw the terrain.
	glUseProgram(terrain_.gl_program.object());

	// Coordinate transform from map coordinates to GL coordinates.
	float scale_x = 2.0 / args->surface_width;
	float scale_y = -2.0 / args->surface_height;
	float offset_x = args->surface_offset.x * scale_x - 1.0;
	float offset_y = args->surface_offset.y * scale_y + 1.0;

	// Texture size
	const BlitData& blit_data = args->terrain->egbase().world().terrains().get(0).get_texture(0).blit_data();
	const FloatRect texture_coordinates = to_gl_texture(blit_data);

	// Prepare uniforms.
	upload_terrain_data(args, gametime);

	glUniform2f(terrain_.u_position_scale, scale_x, scale_y);
	glUniform2f(terrain_.u_position_offset, offset_x, offset_y);
	glUniform1f(terrain_.u_z_value, z_value);
	glUniform2f(terrain_.u_texture_dimensions, texture_coordinates.w, texture_coordinates.h);

	// Prepare textures & sampler uniforms.
	glUniform1i(terrain_.u_terrain_base, 0);
	gl.bind(GL_TEXTURE0, args->terrain->texture());

	glUniform1i(terrain_.u_player_brightness, 1);
	gl.bind(GL_TEXTURE1, args->perspective->player_brightness_texture());

	glUniform1i(terrain_.u_terrain_texture, 2);
	gl.bind(GL_TEXTURE2, blit_data.texture_id);

	glUniform1i(terrain_.u_dither_texture, 3);
	gl.bind(GL_TEXTURE3, terrain_.dither_mask->blit_data().texture_id);

	// Setup vertex and instance attribute data.
	gl.enable_vertex_attrib_array(
	   {terrain_.in_vertex_coordinate, terrain_.in_patch_coordinate});

	unsigned num_instances = upload_instance_data(args);

	terrain_.vertex_data.bind();
	glVertexAttribIPointer(terrain_.in_vertex_coordinate, 4, GL_INT, sizeof(PerVertexData), nullptr);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6 * kPatchWidth * kPatchHeight, num_instances);

	glVertexBindingDivisor(terrain_.in_patch_coordinate, 0);
}

void TerrainProgramGl4::draw_roads(const TerrainGl4Arguments* args,
                                   float z_value) {
	auto& gl = Gl::State::instance();

	// Coordinate transform from map coordinates to GL coordinates.
	float scale_x = 2.0 / args->surface_width;
	float scale_y = -2.0 / args->surface_height;
	float offset_x = args->surface_offset.x * scale_x - 1.0;
	float offset_y = args->surface_offset.y * scale_y + 1.0;

	glUseProgram(roads_.gl_program.object());

	setup_road_index_buffer(args->roads.size());

	// Prepare uniforms.
	glUniform2f(roads_.u_position_scale, scale_x, scale_y);
	glUniform2f(roads_.u_position_offset, offset_x, offset_y);
	glUniform1f(roads_.u_z_value, z_value);

	// Prepare textures & sampler uniforms.
	// Texture0&1 are already set correctly.
	glUniform1i(roads_.u_terrain_base, 0);
	glUniform1i(roads_.u_player_brightness, 1);

	glUniform1i(roads_.u_texture, 2);
	gl.bind(GL_TEXTURE2, args->terrain->road_texture_object());

	glBindBufferBase(GL_UNIFORM_BUFFER, roads_.block_textures_idx,
	                 args->terrain->road_textures_buffer_object());

	upload_road_data(args);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, roads_.gl_index_buffer.object());
	glDrawElements(GL_TRIANGLES, 6 * args->roads.size(), GL_UNSIGNED_SHORT, nullptr);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void TerrainProgramGl4::init_vertex_data() {
	std::vector<PerVertexData> vertices;

	for (int y = 0; y < int(kPatchHeight); ++y) {
		for (int x = 0; x < int(kPatchWidth); ++x) {
			int blx = (y & 1) ? x : (x - 1);

			// Down triangle.
			vertices.emplace_back(x,       y,     x, y, false, 2);
			vertices.emplace_back(blx,     y + 1, x, y, false, 0);
			vertices.emplace_back(blx + 1, y + 1, x, y, false, 1);

			// Right triangle.
			vertices.emplace_back(x,       y,     x, y, true, 1);
			vertices.emplace_back(blx + 1, y + 1, x, y, true, 2);
			vertices.emplace_back(x + 1,   y,     x, y, true, 0);
		}
	}

	assert(vertices.size() == 6 * kPatchWidth * kPatchHeight);

	terrain_.vertex_data.bind();
	terrain_.vertex_data.update(vertices);
}

// Upload the per-terrain texture data. This is done on every draw call because
// it depends on the gametime.
//
// TODO(nha): it seems reasonable to make this part of TerrainBaseGl4, since
// all views should refer to the same gametime.
void TerrainProgramGl4::upload_terrain_data(const TerrainGl4Arguments* args,
                                            uint32_t gametime) {
	const auto& terrains = args->terrain->egbase().world().terrains();
	auto stream = terrain_.terrain_data.stream(terrains.size());

	for (unsigned i = 0; i < terrains.size(); ++i) {
		stream.emplace_back();
		PerTerrainData& terrain = stream.back();
		const TerrainDescription& descr = terrains.get(i);
		terrain.offset =
			to_gl_texture(descr.get_texture(gametime).blit_data()).origin();
		terrain.dither_layer = descr.dither_layer();
	}

	GLintptr offset = stream.unmap();
	glBindBufferRange(GL_UNIFORM_BUFFER, terrain_.block_terrains_idx,
	                  terrain_.terrain_data.object(),
	                  offset, sizeof(PerTerrainData) * terrains.size());
}

// Determine which instances/patches to draw, upload the data and set up the
// vertex attributes.
unsigned TerrainProgramGl4::upload_instance_data(const TerrainGl4Arguments* args) {
	int minfx = args->minfx;
	int minfy = args->minfy;
	int maxfx = args->maxfx;
	int maxfy = args->maxfy;
	if (minfy & 1)
		minfy--;

	int ph = (maxfy - minfy + kPatchHeight) / kPatchHeight;
	int pw = (maxfx - minfx + kPatchWidth) / kPatchWidth;
	int num_patches = pw * ph;

	auto stream = terrain_.instance_data.stream(num_patches);
	for (int py = 0; py < ph; ++py) {
		for (int px = 0; px < pw; ++px) {
			const int fx = minfx + px * kPatchWidth;
			const int fy = minfy + py * kPatchHeight;

			stream.emplace_back();
			PerInstanceData& i = stream.back();
			i.coordinate.x = fx;
			i.coordinate.y = fy;
		}
	}

	GLintptr offset = stream.unmap();

	glVertexAttribIPointer(terrain_.in_patch_coordinate, 2, GL_INT, sizeof(PerInstanceData),
	                       reinterpret_cast<void*>(offset + offsetof(PerInstanceData, coordinate)));

	glVertexBindingDivisor(terrain_.in_patch_coordinate, 1);

	return num_patches;
}

void TerrainProgramGl4::setup_road_index_buffer(unsigned num_roads) {
	if (num_roads <= roads_.num_index_roads)
		return;

	if (num_roads > 65536 / 4)
		throw wexception("Too many roads for 16-bit indices");

	std::vector<uint16_t> indices;
	indices.reserve(num_roads * 6);

	for (unsigned i = 0; i < num_roads; ++i) {
		indices.push_back(4 * i);
		indices.push_back(4 * i + 1);
		indices.push_back(4 * i + 2);

		indices.push_back(4 * i + 2);
		indices.push_back(4 * i + 1);
		indices.push_back(4 * i + 3);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, roads_.gl_index_buffer.object());
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * indices.size(),
	             indices.data(), GL_STATIC_DRAW);
	roads_.num_index_roads = num_roads;
}

void TerrainProgramGl4::upload_road_data(const TerrainGl4Arguments* args) {
	assert(!args->roads.empty());

	auto stream = roads_.road_data.stream(args->roads.size());

	for (const TerrainGl4Arguments::Road& road : args->roads) {
		stream.emplace_back(
			Point(road.coord.x, road.coord.y), road.direction,
			args->terrain->road_texture_idx(
				road.owner, RoadType(road.type), road.coord, WalkingDir(road.direction)));
	}

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, roads_.road_data.object(),
	                  stream.unmap(), args->roads.size() * sizeof(PerRoadData));
}
