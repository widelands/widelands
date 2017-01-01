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

#include "graphic/game_renderer_gl4.h"
#include "graphic/gl/coordinate_conversion.h"
#include "graphic/gl/utils.h"
#include "graphic/image_io.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/editor_game_base.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/world/terrain_description.h"
#include "logic/map_objects/world/world.h"
#include "profile/profile.h"
#include "wui/mapviewpixelfunctions.h"

using namespace Widelands;

/**
 * This is the back-end of the GL4 rendering path.
 *
 * Per-field data is uploaded directly into integer-valued textures that span
 * the entire map, and vertex shaders do most of the heavy lifting. The
 * following textures are used:
 *
 * Fields texture: data that is usually constant throughout a game, GL_RGBA8UI:
 *  R = r-triangle texture index
 *  G = d-triangle texture index
 *  B = height
 *  A = brightness
 * This information can be modified in the editor and in scenarios. Note that
 * the triangle textures depend on the player perspective.
 *
 * Player brightness texture:
 *  R = player-perspective-dependent brightness modulation
 * This information is re-uploaded every frame.
 *
 * Semi-permanent information (GL_R8UI):
 *  R = bits 0..5: field ownership (player number)
 *      bits 6..7: road/flag/building data:
 *                 0: nothing, 1: road, 2: flag, 3: building
 * This information is only needed for the minimap, and re-uploaded every frame
 * when it is shown.
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

TerrainInformationGl4::GlobalMap TerrainInformationGl4::global_map_;

std::shared_ptr<TerrainInformationGl4>
TerrainInformationGl4::get(const Widelands::EditorGameBase& egbase,
                           const Widelands::Player* player) {
	GlobalKey key(&egbase, player);
	auto it = global_map_.find(key);
	if (it != global_map_.end())
		return it->second.lock();

	std::shared_ptr<TerrainInformationGl4> instance(
		new TerrainInformationGl4(egbase, player));
	global_map_[key] = instance;
	return instance;
}

TerrainInformationGl4::TerrainInformationGl4(const Widelands::EditorGameBase& egbase,
                                             const Widelands::Player* player)
  : egbase_(egbase), player_(player), uploads_(GL_PIXEL_UNPACK_BUFFER) {
	glGenTextures(1, &brightness_texture_);
	glGenTextures(1, &fields_texture_);
	glGenTextures(1, &minimap_texture_);

	const Map& map = egbase.map();
	auto& gl = Gl::State::instance();
	gl.bind(GL_TEXTURE0, fields_texture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	gl.bind(GL_TEXTURE0, brightness_texture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, map.get_width(), map.get_height(), 0,
	             GL_RED, GL_UNSIGNED_BYTE, NULL);
	brightness_see_all_ = false;

	gl.bind(GL_TEXTURE0, minimap_texture_);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, map.get_width(), map.get_height(), 0,
	             GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);

	fields_update();
	upload_road_textures();
	upload_constant_textures();

	updated_minimap_ = false;
	need_update_minimap_ = false;
	minimap_update_next_ = 0;
}

TerrainInformationGl4::~TerrainInformationGl4() {
	if (brightness_texture_)
		glDeleteTextures(1, &brightness_texture_);

	if (fields_texture_)
		glDeleteTextures(1, &fields_texture_);

	if (minimap_texture_)
		glDeleteTextures(1, &minimap_texture_);

	if (terrain_color_texture_)
		glDeleteTextures(1, &terrain_color_texture_);

	global_map_.erase(GlobalKey(&egbase_, player_));
}

/// Add @p rect to @p rects, merging it with any overlapping or touching
/// pre-existing rects (where merging means that the rectangles are replaced by
/// a single rectangle that contains their union). The order of rectangles in
/// @p rects is not preserved.
///
/// Rectangles are interpreted as half-open.
static void add_rect(std::vector<Recti>& rects, const Recti& rect) {
	Recti new_rect = rect;

	for (size_t i = 0; i < rects.size(); ++i) {
		// Merge rectangles even if they only touch instead of fully overlapping.
		// The rationale is that reducing the number of uploads is often a
		// benefit even when the total size of uploads becomes larger.
		if (new_rect.x + new_rect.w < rects[i].x ||
		    rects[i].x + rects[i].w < new_rect.x ||
		    new_rect.y + new_rect.h < rects[i].y ||
		    rects[i].y + rects[i].h < new_rect.y)
			continue;

		rects[i] = rects.back();
		rects.pop_back();
		i--;
	}

	rects.push_back(new_rect);
}

void TerrainInformationGl4::update(int minfx, int maxfx, int minfy, int maxfy) {
	const Map& map = egbase().map();
	int width = map.get_width();
	int height = map.get_height();

	auto normalize = [](int& min, int& max, int size) {
		while (min < 0) {
			min += size;
			max += size;
		}
		while (min >= size) {
			min -= size;
			max -= size;
		}
	};

	normalize(minfx, maxfx, width);
	normalize(minfy, maxfy, height);

	// Ensure proper row alignment during texture uploads.
	minfx = (minfx / 4) * 4;
	maxfx = ((maxfx + 4) / 4) * 4 - 1;

	auto add = [&](int startx, int endx) {
		add_rect(update_, Recti(startx, minfy, endx - startx, std::min(maxfy + 1, height) - minfy));
		if (maxfy >= height)
			add_rect(update_, Recti(startx, 0, endx - startx, std::min(maxfy + 1 - height, height)));
	};

	add(minfx, std::min(maxfx + 1, width));
	if (maxfx >= width)
		add(0, std::min(maxfx + 1 - width, width));
}

void TerrainInformationGl4::update_minimap() {
	need_update_minimap_ = true;
}

void TerrainInformationGl4::do_prepare_frame() {
	const Map& map = egbase().map();

	upload_terrain_data();

	if (need_update_minimap_) {
		if (!updated_minimap_) {
			// Need a full update when the minimap is drawn for the first
			// time.
			update_.clear();
			update_.emplace_back(0, 0, map.get_width(), map.get_height());
		} else {
			// For the minimap, we want to do rolling texture updates of
			// stripes that cover the whole width or height of the map,
			// depending on which is smaller. For consistency and simplicity,
			// expand all other dirty rectangles to full strips as well.
			//
			// Furthermore, use stripes of a size that is a multiple of a small
			// power of two, since that likely has bandwidth benefits due to
			// how textures are laid out in memory. This also avoids confusion
			// due to pixel (un)packing row alignments.
			unsigned width = map.get_width();
			unsigned height = map.get_height();
			bool horiz = width <= height;
			std::vector<std::pair<unsigned, unsigned>> stripes;

			// Massage existing stripes, effectively a form of insertion sort.
			stripes.reserve(update_.size() + 1);
			for (size_t i = 0; i < update_.size(); ++i) {
				unsigned min = horiz ? update_[i].y : update_[i].x;
				unsigned max = min + (horiz ? update_[i].h : update_[i].w);

				min = (min / 8) * 8;
				max = ((max + 7) / 8) * 8;

				size_t j;
				for (j = 0; j < stripes.size(); ++j) {
					if (max < stripes[j].first) {
						stripes.insert(stripes.begin() + j, std::make_pair(min, max));
						break;
					}

					if (min <= stripes[j].second) {
						stripes[j].first = std::min(stripes[j].first, min);
						stripes[j].second = std::max(stripes[j].second, max);

						size_t k;
						for (k = j + 1; k < stripes.size(); ++k) {
							if (stripes[j].second < stripes[k].first)
								break;

							stripes[j].second = std::max(stripes[j].second, stripes[k].second);
						}

						stripes.erase(stripes.begin() + j + 1, stripes.begin() + k);
						break;
					}
				}
				if (j >= stripes.size())
					stripes.emplace_back(min, max);
			}

			if (stripes.empty() || stripes[0].first != 0 ||
			    stripes[0].second < (horiz ? height : width)) {
				// Add a stripe (or expand an existing one) for the rolling minimap
				// update.
				if (minimap_update_next_ >= (horiz ? height : width))
					minimap_update_next_ = 0;

				unsigned min = minimap_update_next_;
				size_t j;
				for (j = 0; j < stripes.size(); ++j) {
					unsigned max = min + 8;
					if (max < stripes[j].first) {
						stripes.insert(stripes.begin() + j, std::make_pair(min, max));
						break;
					}

					if (min <= stripes[j].second) {
						if (min < stripes[j].first) {
							assert(max == stripes[j].first); // due to multiples of 8
							stripes[j].first = min;
							break;
						}

						min = minimap_update_next_ = stripes[j].second;
						if (min >= (horiz ? height : width)) {
							min = 0;
							j = 0;
							continue;
						}
						max = std::min(min + 8, horiz ? height : width);
						stripes[j].second = max;

						size_t k;
						for (k = j + 1; k < stripes.size(); ++k) {
							if (stripes[j].second < stripes[k].first)
								break;

							stripes[j].second = std::max(stripes[j].second, stripes[k].second);
						}
						stripes.erase(stripes.begin() + j + 1, stripes.begin() + k);
						break;
					}
				}
				if (j >= stripes.size())
					stripes.emplace_back(min, min + 8);

				minimap_update_next_ += 8;
			}

			// Convert stripes back to update rectangles.
			update_.resize(stripes.size());
			for (size_t i = 0; i < stripes.size(); ++i) {
				if (horiz) {
					update_[i].x = 0;
					update_[i].w = width;
					update_[i].y = stripes[i].first;
					update_[i].h = stripes[i].second - stripes[i].first;
				} else {
					update_[i].y = 0;
					update_[i].h = height;
					update_[i].x = stripes[i].first;
					update_[i].w = stripes[i].second - stripes[i].first;
				}
			}
		}
	}

	// Fields data updates are guarded by version numbers instead of
	// rectangles.
	if (fields_base_version_ != map.get_fields_base_version() ||
	    (player() && terrain_vision_version_ != player()->get_terrain_vision_version()))
		fields_update();

	brightness_update();

	if (need_update_minimap_)
		do_update_minimap();

	update_.clear();
	updated_minimap_ = need_update_minimap_;
	need_update_minimap_ = false;
}

void TerrainInformationGl4::prepare_frame() {
	for (auto& entries : global_map_) {
		std::shared_ptr<TerrainInformationGl4> ti = entries.second.lock();

		ti->do_prepare_frame();
	}
}

void TerrainInformationGl4::do_update_minimap() {
	// Re-upload minimap data.
	auto& gl = Gl::State::instance();
	const Map& map = egbase().map();
	unsigned width = map.get_width();
	std::vector<uint8_t> data;
	const bool see_all = !player() || player()->see_all();

	auto detail_bits = [&](const Widelands::BaseImmovable* imm) -> uint8_t {
		if (imm) {
			Widelands::MapObjectType type = imm->descr().type();
			if (type == Widelands::MapObjectType::ROAD)
				return 1u << 6;
			if (type == Widelands::MapObjectType::FLAG)
				return 2u << 6;
			if (type >= Widelands::MapObjectType::BUILDING)
				return 3u << 6;
		}
		return 0;
	};

	gl.bind(GL_TEXTURE0, minimap_texture_);

	for (const Recti& rect : update_) {
		data.resize(rect.w * rect.h);
		if (see_all) {
			unsigned i = 0;
			for (unsigned y = 0; y < unsigned(rect.h); ++y) {
				unsigned idx = (rect.y + y) * width + rect.x;
				for (unsigned x = 0; x < unsigned(rect.w); ++x, ++i, ++idx) {
					const Field& f = map[idx];
					data[i] = f.get_owned_by();
					data[i] |= detail_bits(f.get_immovable());
				}
			}
		} else {
			unsigned i = 0;
			for (unsigned y = 0; y < unsigned(rect.h); ++y) {
				unsigned idx = (rect.y + y) * width + rect.x;
				for (unsigned x = 0; x < unsigned(rect.w); ++x, ++i, ++idx) {
					const Player::Field& pf = player()->fields()[idx];
					data[i] = pf.owner;

					if (pf.vision >= 2) {
						const Field& f = map[idx];
						data[i] |= detail_bits(f.get_immovable());
					}
				}
			}
		}

		glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.w, rect.h,
		                GL_RED_INTEGER, GL_UNSIGNED_BYTE, data.data());
	}
}

void TerrainInformationGl4::fields_update() {
	auto& gl = Gl::State::instance();
	const Map& map = egbase().map();
	auto stream = uploads_.stream(sizeof(PerFieldData) * uint(map.get_width()) * map.get_height());
	PerFieldData* fd =
		reinterpret_cast<PerFieldData*>
			(stream.add(sizeof(PerFieldData) * uint(map.get_width()) * map.get_height()));
	MapIndex max_index = map.max_index();
	const bool see_all = !player() || player()->see_all();

	if (see_all) {
		for (MapIndex i = 0; i < max_index; ++i) {
			const Field& f = map[i];
			fd[i].terrain_r = f.terrain_r();
			fd[i].terrain_d = f.terrain_d();
			fd[i].height = f.get_height();
			fd[i].brightness = f.get_brightness();
		}
	} else {
		const Player::Field* player_fields = player()->fields();

		for (MapIndex i = 0; i < max_index; ++i) {
			const Field& f = map[i];
			const Player::Field& pf = player_fields[i];
			fd[i].terrain_r = pf.terrains.r;
			fd[i].terrain_d = pf.terrains.d;
			fd[i].height = f.get_height();
			fd[i].brightness = f.get_brightness();
		}
	}

	GLintptr offset = stream.unmap();
	uploads_.bind();

	gl.bind(GL_TEXTURE0, fields_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, map.get_width(), map.get_height(), 0,
				 GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, reinterpret_cast<void*>(offset));

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	fields_base_version_ = map.get_fields_base_version();
	if (player())
		terrain_vision_version_ = player()->get_terrain_vision_version();
}

TerrainInformationGl4::PerRoadTextureData::PerRoadTextureData(const Rectf& rect)
  : x(rect.x), y(rect.y), w(rect.w), h(rect.h) {
}

void TerrainInformationGl4::upload_road_textures() {
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

unsigned TerrainInformationGl4::road_texture_idx(PlayerNumber owner,
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

// Upload the per-terrain texture data. This is done on every draw call because
// it depends on the gametime.
void TerrainInformationGl4::upload_terrain_data() {
	uint32_t gametime = egbase().get_gametime();
	const auto& terrains = egbase().world().terrains();
	std::vector<PerTerrainData> data;

	data.resize(terrains.size());

	for (unsigned i = 0; i < terrains.size(); ++i) {
		PerTerrainData& terrain = data[i];
		const TerrainDescription& descr = terrains.get(i);
		terrain.offset =
			to_gl_texture(descr.get_texture(gametime).blit_data()).origin();
		terrain.dither_layer = descr.dither_layer();
	}

	terrain_data_.bind();
	terrain_data_.update(data);
}

void TerrainInformationGl4::brightness_update() {
	auto& gl = Gl::State::instance();
	bool see_all = !player_ || player_->see_all();

	gl.bind(GL_TEXTURE0, brightness_texture_);

	if (see_all) {
		if (!brightness_see_all_) {
			// Pixel unpacking has a per-row alignment of 4 bytes. Usually this
			// is not a problem for us, because maps' widths are always multiples
			// of 4, but in this particular case, OpenGL implementations disagree
			// about whether the alignment should be considered for the bounds
			// check in glTexImage2D. If we only allocate 1 byte, some
			// implementations flag a GL_INVALID_OPERATION.
			static const uint8_t data[4] = {255, 255, 255, 255};

			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 1, 1, 0, GL_RED, GL_UNSIGNED_BYTE,
			             data);
			brightness_see_all_ = true;
		}
	} else {
		const Map& map = egbase().map();
		int width = map.get_width();
		int height = map.get_height();
		uint32_t gametime = egbase().get_gametime();
		std::vector<uint8_t> data;

		if (brightness_see_all_) {
			// Resize the texture when switching between see-all and not-see-all.
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED,
			             GL_UNSIGNED_BYTE, NULL);
			brightness_see_all_ = false;
		}

		for (const Recti& rect : update_) {
			data.resize(rect.w * rect.h);

			unsigned dst = 0;
			for (unsigned y = 0; y < unsigned(rect.h); ++y) {
				unsigned src = (rect.y + y) * width + rect.x;
				for (unsigned x = 0; x < unsigned(rect.w); ++x, ++src, ++dst) {
					const Player::Field& pf = player_->fields()[src];
					if (pf.vision == 0) {
						data[dst] = 0;
					} else if (pf.vision == 1) {
						static const uint32_t kDecayTimeInMs = 20000;
						const Duration time_ago = gametime - pf.time_node_last_unseen;
						if (time_ago < kDecayTimeInMs) {
							data[dst] = 255 * (2 * kDecayTimeInMs - time_ago) / (2 * kDecayTimeInMs);
						} else {
							data[dst] = 128;
						}
					} else {
						data[dst] = 255;
					}
				}
			}

			glTexSubImage2D(GL_TEXTURE_2D, 0, rect.x, rect.y, rect.w, rect.h,
			                GL_RED, GL_UNSIGNED_BYTE, &data[0]);
		}
	}
}

void TerrainInformationGl4::upload_constant_textures() {
	auto& gl = Gl::State::instance();
	const auto& terrains = egbase().world().terrains();
	std::vector<RGBColor> colors;

	for (Widelands::DescriptionIndex i = 0; i < terrains.size(); ++i)
		colors.push_back(terrains.get(i).get_minimap_color(0));

	glGenTextures(1, &terrain_color_texture_);

	gl.bind(GL_TEXTURE0, terrain_color_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, colors.size(), 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
	             colors.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	colors.resize(kMaxPlayers);
	for (int i = 1; i <= kMaxPlayers; ++i) {
		const Widelands::Player* player = egbase().get_player(i);
		if (player)
			colors[i - 1] = player->get_playercolor();
	}

	glGenTextures(1, &player_color_texture_);

	gl.bind(GL_TEXTURE0, player_color_texture_);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, colors.size(), 1, 0, GL_RGB, GL_UNSIGNED_BYTE,
	             colors.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

TerrainProgramGl4::Terrain::Terrain()
  : instance_data(GL_ARRAY_BUFFER) {
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

TerrainProgramGl4::MiniMap::MiniMap()
  : vertex_data(GL_ARRAY_BUFFER) {
	gl_program.build_vp_fp({"minimap_gl4"}, {"minimap_gl4"});

	in_position = glGetAttribLocation(gl_program.object(), "in_position");
	in_field = glGetAttribLocation(gl_program.object(), "in_field");

	u_layer_terrain = glGetUniformLocation(gl_program.object(), "u_layer_terrain");
	u_layer_owner = glGetUniformLocation(gl_program.object(), "u_layer_owner");
	u_layer_details = glGetUniformLocation(gl_program.object(), "u_layer_details");

	u_frame_topleft = glGetUniformLocation(gl_program.object(), "u_frame_topleft");
	u_frame_bottomright = glGetUniformLocation(gl_program.object(), "u_frame_bottomright");

	u_terrain_base = glGetUniformLocation(gl_program.object(), "u_terrain_base");
	u_player_brightness = glGetUniformLocation(gl_program.object(), "u_player_brightness");
	u_minimap_extra = glGetUniformLocation(gl_program.object(), "u_minimap_extra");
	u_terrain_color = glGetUniformLocation(gl_program.object(), "u_terrain_color");
	u_player_color = glGetUniformLocation(gl_program.object(), "u_player_color");
}

TerrainProgramGl4::MiniMap::~MiniMap() {
}

TerrainProgramGl4::TerrainProgramGl4() {
	log("Using GL4 terrain rendering path\n");

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
	const auto& caps = Gl::State::instance().capabilities();

	if (caps.glsl_version < 130)
		return false;

	if (!caps.ARB_separate_shader_objects ||
	    !caps.ARB_shader_storage_buffer_object ||
	    !caps.ARB_uniform_buffer_object)
		return false;

	return !g_options.pull_section("global").get_bool("disable_gl4", false);
}

void TerrainProgramGl4::draw(const TerrainGl4Arguments* args,
                             uint32_t gametime,
                             float z_value) {
	auto& gl = Gl::State::instance();

	assert(gametime == args->terrain->egbase().get_gametime());

	// First, draw the terrain.
	glUseProgram(terrain_.gl_program.object());

	// Coordinate transform from map coordinates to GL coordinates.
	float scale_x = 2.0 / args->surface_width * args->scale;
	float scale_y = -2.0 / args->surface_height * args->scale;
	float offset_x = args->surface_offset.x * scale_x - 1.0;
	float offset_y = args->surface_offset.y * scale_y + 1.0;

	// Texture size
	const BlitData& blit_data = args->terrain->egbase().world().terrains().get(0).get_texture(0).blit_data();
	const Rectf texture_coordinates = to_gl_texture(blit_data);

	// Prepare uniforms.
	glBindBufferBase(GL_UNIFORM_BUFFER, terrain_.block_terrains_idx,
	                 args->terrain->terrain_data_buffer_object());

	glUniform2f(terrain_.u_position_scale, scale_x, scale_y);
	glUniform2f(terrain_.u_position_offset, offset_x, offset_y);
	glUniform1f(terrain_.u_z_value, z_value);
	glUniform2f(terrain_.u_texture_dimensions, texture_coordinates.w, texture_coordinates.h);

	// Prepare textures & sampler uniforms.
	glUniform1i(terrain_.u_terrain_base, 0);
	gl.bind(GL_TEXTURE0, args->terrain->fields_texture());

	glUniform1i(terrain_.u_player_brightness, 1);
	gl.bind(GL_TEXTURE1, args->terrain->player_brightness_texture());

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

void TerrainProgramGl4::draw_minimap(const TerrainGl4Arguments* args,
                                     float z_value) {
	auto& gl = Gl::State::instance();
	const Widelands::Map& map = args->terrain->egbase().map();
	float width = map.get_width();
	float height = map.get_height();

	glUseProgram(minimap_.gl_program.object());

	// Prepare minimap setting uniforms
	glUniform1i(minimap_.u_layer_terrain, (args->minimap_layers & MiniMapLayer::Terrain) ? 1 : 0);
	glUniform1i(minimap_.u_layer_owner, (args->minimap_layers & MiniMapLayer::Owner) ? 1 : 0);

	uint details = 0;
	if (args->minimap_layers & MiniMapLayer::Road)
		details |= 1;
	if (args->minimap_layers & MiniMapLayer::Flag)
		details |= 2;
	if (args->minimap_layers & MiniMapLayer::Building)
		details |= 4;

	glUniform1ui(minimap_.u_layer_details, details);

	// Prepare textures & sampler uniforms.
	glUniform1i(minimap_.u_terrain_base, 0);
	gl.bind(GL_TEXTURE0, args->terrain->fields_texture());

	glUniform1i(minimap_.u_player_brightness, 1);
	gl.bind(GL_TEXTURE1, args->terrain->player_brightness_texture());

	glUniform1i(minimap_.u_minimap_extra, 2);
	gl.bind(GL_TEXTURE2, args->terrain->minimap_texture());

	glUniform1i(minimap_.u_terrain_color, 3);
	gl.bind(GL_TEXTURE3, args->terrain->terrain_color_texture());

	glUniform1i(minimap_.u_player_color, 4);
	gl.bind(GL_TEXTURE4, args->terrain->player_color_texture());

	glUniform2f(minimap_.u_frame_topleft, (args->minfx + 0.001) / width, (args->minfy + 0.001) / height);
	glUniform2f(minimap_.u_frame_bottomright, (args->maxfx - 0.001) / width, (args->maxfy - 0.001) / height);

	// Compute coordinates and upload vertex data.
	if (args->minimap_layers & MiniMapLayer::Zoom2) {
		width *= 2;
		height *= 2;
	}

	float left = args->surface_offset.x;
	float right = left + width;
	float top = args->surface_offset.y;
	float bottom = top + height;

	pixel_to_gl_renderbuffer(args->surface_width, args->surface_height, &left, &top);
	pixel_to_gl_renderbuffer(args->surface_width, args->surface_height, &right, &bottom);

	float tx = args->minimap_tl_fx * (1.0 / map.get_width());
	float ty = args->minimap_tl_fy * (1.0 / map.get_height());

	auto stream = minimap_.vertex_data.stream(4);
	MiniMap::VertexData* v = stream.add(4);

	v[0].x = left;
	v[0].y = top;
	v[0].z = z_value;
	v[0].tx = tx;
	v[0].ty = ty;

	v[1].x = left;
	v[1].y = bottom;
	v[1].z = z_value;
	v[1].tx = tx;
	v[1].ty = ty + 1.0;

	v[2].x = right;
	v[2].y = top;
	v[2].z = z_value;
	v[2].tx = tx + 1.0;
	v[2].ty = ty;

	v[3].x = right;
	v[3].y = bottom;
	v[3].z = z_value;
	v[3].tx = tx + 1.0;
	v[3].ty = ty + 1.0;

	GLintptr offset = stream.unmap();

	gl.enable_vertex_attrib_array({minimap_.in_position, minimap_.in_field});

	minimap_.vertex_data.bind();
	Gl::vertex_attrib_pointer(minimap_.in_position, 3, sizeof(MiniMap::VertexData), offset);
	Gl::vertex_attrib_pointer(minimap_.in_field, 2, sizeof(MiniMap::VertexData),
	                          offset + offsetof(MiniMap::VertexData, tx));

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void TerrainProgramGl4::draw_roads(const TerrainGl4Arguments* args,
                                   float z_value) {
	auto& gl = Gl::State::instance();

	// Coordinate transform from map coordinates to GL coordinates.
	float scale_x = 2.0 / args->surface_width * args->scale;
	float scale_y = -2.0 / args->surface_height * args->scale;
	float offset_x = args->surface_offset.x * scale_x - 1.0;
	float offset_y = args->surface_offset.y * scale_y + 1.0;

	glUseProgram(roads_.gl_program.object());

	setup_road_index_buffer(args->roads.size());

	// Prepare uniforms.
	glUniform2f(roads_.u_position_scale, scale_x, scale_y);
	glUniform2f(roads_.u_position_offset, offset_x, offset_y);
	glUniform1f(roads_.u_z_value, z_value);

	// Prepare textures & sampler uniforms.
	glUniform1i(roads_.u_terrain_base, 0);
	gl.bind(GL_TEXTURE0, args->terrain->fields_texture());

	glUniform1i(roads_.u_player_brightness, 1);
	gl.bind(GL_TEXTURE1, args->terrain->player_brightness_texture());

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
			Vector2i(road.coord.x, road.coord.y), road.direction,
			args->terrain->road_texture_idx(
				road.owner, RoadType(road.type), road.coord, WalkingDir(road.direction)));
	}

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, roads_.road_data.object(),
	                  stream.unmap(), args->roads.size() * sizeof(PerRoadData));
}
