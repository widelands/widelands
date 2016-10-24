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

#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"
#include "graphic/game_renderer_gl4.h"
#include "graphic/image_io.h"
#include "io/filesystem/layered_filesystem.h"
#include "wui/mapviewpixelfunctions.h"

using namespace Widelands;

/**
 * This is the back-end of the GL4 rendering path.
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
 * TODO(nha): upload based on dirtiness and/or "rolling" updates of N% per frame
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
  : egbase_(egbase), player_(player) {
}

TerrainPlayerPerspectiveGl4::~TerrainPlayerPerspectiveGl4() {
}

void TerrainPlayerPerspectiveGl4::update() {
	// TODO
}

TerrainProgramGl4::TerrainProgramGl4()
  : terrain_data_(GL_UNIFORM_BUFFER), instance_data_(GL_ARRAY_BUFFER) {
	// Initialize program.
	gl_program_.build("terrain_gl4");

	in_vertex_coordinate_ = glGetAttribLocation(gl_program_.object(), "in_vertex_coordinate");
	in_patch_coordinate_ = glGetAttribLocation(gl_program_.object(), "in_patch_coordinate");
	in_patch_basepix_ = glGetAttribLocation(gl_program_.object(), "in_patch_basepix");

	u_position_scale_ = glGetUniformLocation(gl_program_.object(), "u_position_scale");
	u_position_offset_ = glGetUniformLocation(gl_program_.object(), "u_position_offset");
	u_z_value_ = glGetUniformLocation(gl_program_.object(), "u_z_value");
	u_texture_dimensions_ = glGetUniformLocation(gl_program_.object(), "u_texture_dimensions");

	u_terrain_base_ = glGetUniformLocation(gl_program_.object(), "u_terrain_base");
	u_terrain_texture_ = glGetUniformLocation(gl_program_.object(), "u_terrain_texture");
	u_dither_texture_ = glGetUniformLocation(gl_program_.object(), "u_dither_texture");

	block_terrains_idx_ = glGetUniformBlockIndex(gl_program_.object(), "block_terrains");

	// Initialize vertex buffer (every instance/path has the same structure).
	init_vertex_data();

	// Load mask texture for dithering.
	dither_mask_.reset(new Texture(load_image_as_sdl_surface("world/pics/edge.png", g_fs), true));

	Gl::State::instance().bind(GL_TEXTURE0, dither_mask_->blit_data().texture_id);
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
	return true;
}

void TerrainProgramGl4::draw(const TerrainGl4Arguments* args,
                             uint32_t gametime,
                             float z_value) {
	auto& gl = Gl::State::instance();

	glUseProgram(gl_program_.object());

	// Coordinate transform from map coordinates to GL coordinates.
	float scale_x = 2.0 / args->surface_width;
	float scale_y = -2.0 / args->surface_height;
	float offset_x = args->surface_offset.x * scale_x - 1.0;
	float offset_y = args->surface_offset.y * scale_y + 1.0;
// 			f.gl_x = f.pixel_x = x + surface_offset.x;
// 			f.gl_y = f.pixel_y = y + surface_offset.y - fcoords.field->get_height() * kHeightFactor;
// inline void pixel_to_gl_renderbuffer(const int width, const int height, float* x, float* y) {
// 	*x = (*x / width) * 2.0f - 1.0f;
// 	*y = 1.0f - (*y / height) * 2.0f;
// }

	// Texture size
	const BlitData& blit_data = args->terrain->egbase().world().terrains().get(0).get_texture(0).blit_data();
	const FloatRect texture_coordinates = to_gl_texture(blit_data);

	// Prepare uniforms.
	upload_terrain_data(args, gametime);

	glUniform2f(u_position_scale_, scale_x, scale_y);
	glUniform2f(u_position_offset_, offset_x, offset_y);
	glUniform1f(u_z_value_, z_value);
	glUniform2f(u_texture_dimensions_, texture_coordinates.w, texture_coordinates.h);

	// Prepare textures & sampler uniforms.
	glUniform1i(u_terrain_base_, 0);
	gl.bind(GL_TEXTURE0, args->terrain->texture());

	glUniform1i(u_terrain_texture_, 1);
	gl.bind(GL_TEXTURE1, blit_data.texture_id);

	glUniform1i(u_dither_texture_, 2);
	gl.bind(GL_TEXTURE2, dither_mask_->blit_data().texture_id);

	// Setup vertex and instance attribute data.
	gl.enable_vertex_attrib_array(
	   {in_vertex_coordinate_, in_patch_coordinate_, in_patch_basepix_});

	unsigned num_instances = upload_instance_data(args);

	vertex_data_.bind();
	glVertexAttribIPointer(in_vertex_coordinate_, 4, GL_INT, sizeof(PerVertexData), nullptr);

	glDrawArraysInstanced(GL_TRIANGLES, 0, 6 * kPatchWidth * kPatchHeight, num_instances);

	glVertexBindingDivisor(in_patch_coordinate_, 0);
	glVertexBindingDivisor(in_patch_basepix_, 0);
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

	vertex_data_.bind();
	vertex_data_.update(vertices);
}

// Upload the per-terrain texture data. This is done on every draw call because
// it depends on the gametime.
//
// TODO(nha): it seems reasonable to make this part of TerrainBaseGl4, since
// all views should refer to the same gametime.
void TerrainProgramGl4::upload_terrain_data(const TerrainGl4Arguments* args,
                                            uint32_t gametime) {
	const auto& terrains = args->terrain->egbase().world().terrains();
	auto stream = terrain_data_.stream(terrains.size());

	for (unsigned i = 0; i < terrains.size(); ++i) {
		stream.emplace_back();
		PerTerrainData& terrain = stream.back();
		const TerrainDescription& descr = terrains.get(i);
		terrain.offset =
			to_gl_texture(descr.get_texture(gametime).blit_data()).origin();
		terrain.dither_layer = descr.dither_layer();
	}

	GLintptr offset = stream.unmap();
	glBindBufferRange(GL_UNIFORM_BUFFER, block_terrains_idx_, terrain_data_.object(),
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

	auto stream = instance_data_.stream(num_patches);
	for (int py = 0; py < ph; ++py) {
		for (int px = 0; px < pw; ++px) {
			const int fx = minfx + px * kPatchWidth;
			const int fy = minfy + py * kPatchHeight;

			stream.emplace_back();
			PerInstanceData& i = stream.back();
			i.coordinate.x = fx;
			i.coordinate.y = fy;

			int x, y;
			MapviewPixelFunctions::get_basepix(Coords(fx, fy), x, y);
			i.basepix.x = x;
			i.basepix.y = y;
		}
	}

	GLintptr offset = stream.unmap();

	glVertexAttribIPointer(in_patch_coordinate_, 2, GL_INT, sizeof(PerInstanceData),
	                       reinterpret_cast<void*>(offset + offsetof(PerInstanceData, coordinate)));
	glVertexAttribPointer(in_patch_basepix_, 2, GL_FLOAT, GL_FALSE, sizeof(PerInstanceData),
	                      reinterpret_cast<void*>(offset + offsetof(PerInstanceData, basepix)));

	glVertexBindingDivisor(in_patch_coordinate_, 1);
	glVertexBindingDivisor(in_patch_basepix_, 1);

	return num_patches;
}
