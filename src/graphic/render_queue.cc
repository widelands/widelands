/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "graphic/render_queue.h"

#include <algorithm>

#include "base/rect.h"
#include "base/wexception.h"
#include "graphic/gl/blit_program.h"
#include "graphic/gl/dither_program.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/gl/fill_rect_program.h"
#include "graphic/gl/grid_program.h"
#include "graphic/gl/road_program.h"
#include "graphic/gl/terrain_program.h"
#include "graphic/gl/workarea_program.h"

namespace {

constexpr int kMaximumZValue = std::numeric_limits<uint16_t>::max();

// Maps [0, kMaximumZValue] linearly to [1., -1.] for use in vertex shaders.
inline float to_opengl_z(const int z) {
	return -(2.f * z) / kMaximumZValue + 1.f;
}

// The key defines in which order we render things.
//
// For opaque objects, render order makes no difference in the final image, but
//   - we batch up by program to have maximal batching.
//   - and we want to render frontmost objects first, so that we do not render
//     any pixel more than once.
static_assert(RenderQueue::Program::kHighestProgramId <= 8,
              "Need to change sorting keys.");  // 4 bits.

uint64_t
make_key_opaque(const uint64_t program_id, const uint64_t z_value, const uint64_t extra_value) {
	assert(program_id < RenderQueue::Program::kHighestProgramId);
	assert(z_value < std::numeric_limits<uint16_t>::max());

	// TODO(sirver): As a higher priority for sorting then z value, texture
	// could be used here. This allows for more batching of GL calls, but in my
	// tests hardly made a difference for Widelands..
	uint64_t sort_z_value = std::numeric_limits<uint16_t>::max() - z_value;
	// IIII0000 EEEEEEEE EEEEEEEE EEEEEEEE EEEEEEEE ZZZZZZZZ ZZZZZZZZ
	return (program_id << 60) | (extra_value << 16) | (sort_z_value);
}

// For blended objects, we need to render furthest away objects first, and we
// do not update the z-buffer. This guarantees that the image is correct.
//   - if z value is the same, we order by program second to have potential batching.
uint64_t
make_key_blended(const uint64_t program_id, const uint64_t z_value, const uint64_t extra_value) {
	assert(program_id < RenderQueue::Program::kHighestProgramId);
	assert(z_value < std::numeric_limits<uint16_t>::max());

	// Sort opaque objects increasing, alpha objects decreasing in order.
	// ZZZZZZZZ ZZZZZZZZ IIII0000 EEEEEEEE EEEEEEEE EEEEEEEE EEEEEEEE
	return (z_value << 40) | (program_id << 36) | extra_value;
}

inline void from_item(const RenderQueue::Item& item, FillRectProgram::Arguments* args) {
	args->color = item.rect_arguments.color;
	args->destination_rect = item.rect_arguments.destination_rect;
}

inline void from_item(const RenderQueue::Item& item, BlitProgram::Arguments* args) {
	args->texture = item.blit_arguments.texture;
	args->blend = item.blit_arguments.blend;
	args->mask = item.blit_arguments.mask;
	args->blit_mode = item.blit_arguments.mode;
	args->destination_rect = item.blit_arguments.destination_rect;
}

inline void from_item(const RenderQueue::Item& item, DrawLineProgram::Arguments* args) {
	args->vertices = item.line_arguments.vertices;
}

// Batches up as many items from 'items' that have the same 'program_id'.
// Increases 'index' and returns an argument vector that can directly be passed
// to the individual program.
template <typename T>
std::vector<T> batch_up(const RenderQueue::Program program_id,
                        const std::vector<RenderQueue::Item>& items,
                        size_t* index) {
	std::vector<T> all_args;
	while (*index < items.size()) {
		const RenderQueue::Item& current_item = items.at(*index);
		if (current_item.program_id != program_id) {
			break;
		}
		all_args.emplace_back();
		T& args = all_args.back();
		args.z_value = current_item.z_value;
		args.blend_mode = current_item.blend_mode;
		from_item(current_item, &args);
		++(*index);
	}
	return all_args;
}

// Calls glScissor for the given 'rect' and enables GL_SCISSOR_TEST at
// creation. Disables GL_SCISSOR_TEST at desctruction again.
class ScopedScissor {
public:
	explicit ScopedScissor(const Rectf& rect);
	~ScopedScissor();

private:
	DISALLOW_COPY_AND_ASSIGN(ScopedScissor);
};

ScopedScissor::ScopedScissor(const Rectf& rect) {
	glScissor(rect.x, rect.y, rect.w, rect.h);
	glEnable(GL_SCISSOR_TEST);
}

ScopedScissor::~ScopedScissor() {
	glDisable(GL_SCISSOR_TEST);
}

}  // namespace

RenderQueue::RenderQueue()
   : next_z_(1),
     terrain_program_(new TerrainProgram()),
     dither_program_(new DitherProgram()),
     workarea_program_(new WorkareaProgram()),
     grid_program_(new GridProgram()),
     road_program_(new RoadProgram()) {
}

// static
RenderQueue& RenderQueue::instance() {
	static RenderQueue render_queue;
	return render_queue;
}

void RenderQueue::enqueue(const Item& given_item) {
	Item* item;
	uint32_t extra_value = 0;

	switch (given_item.program_id) {
	case Program::kBlit:
		extra_value = given_item.blit_arguments.texture.texture_id;
		break;

	case Program::kLine:
	case Program::kRect:
	case Program::kTerrainBase:
	case Program::kTerrainDither:
	case Program::kTerrainWorkarea:
	case Program::kTerrainGrid:
	case Program::kTerrainRoad:
		/* all fallthroughs intended */
		break;

	default:
		throw wexception("Unknown given_item.program_id: %d", given_item.program_id);
	}

	if (given_item.blend_mode == BlendMode::Copy) {
		opaque_items_.emplace_back(given_item);
		item = &opaque_items_.back();
		item->z_value = to_opengl_z(next_z_);
		item->key = make_key_opaque(static_cast<uint64_t>(item->program_id), next_z_, extra_value);
	} else {
		blended_items_.emplace_back(given_item);
		item = &blended_items_.back();
		item->z_value = to_opengl_z(next_z_);
		item->key = make_key_blended(static_cast<uint64_t>(item->program_id), next_z_, extra_value);
	}
	++next_z_;
}

void RenderQueue::clear() {
	opaque_items_.clear();
	blended_items_.clear();
}

void RenderQueue::draw(const int screen_width, const int screen_height) {
	// TODO(sirver): If next_z >= kMaximumZValue here, we ran out of z-layers to
	// correctly order the drawing of our objects (see
	// https://bugs.launchpad.net/widelands/+bug/1658593). This is non-critical,
	// but will look strange. We used to crash here in this case, but since it
	// can happen on large zoom and huge screen resolution (> 3440 x 1400), we
	// do not crash anymore. The linked bug contains a discussion how to fix the
	// issue properly, but it was too much work to address at the time.

	Gl::State::instance().bind_framebuffer(0, 0);
	glViewport(0, 0, screen_width, screen_height);

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glDisable(GL_BLEND);

	std::sort(opaque_items_.begin(), opaque_items_.end());
	draw_items(opaque_items_);
	opaque_items_.clear();

	glEnable(GL_BLEND);

	std::sort(blended_items_.begin(), blended_items_.end());
	draw_items(blended_items_);
	blended_items_.clear();

	glDisable(GL_DEPTH_TEST);
	next_z_ = 1;
}

void RenderQueue::draw_items(const std::vector<Item>& items) {
	size_t i = 0;
	while (i < items.size()) {
		const Item& item = items[i];
		switch (item.program_id) {
		case Program::kBlit:
			BlitProgram::instance().draw(batch_up<BlitProgram::Arguments>(Program::kBlit, items, &i));
			break;

		case Program::kLine:
			DrawLineProgram::instance().draw(
			   batch_up<DrawLineProgram::Arguments>(Program::kLine, items, &i));
			break;

		case Program::kRect:
			FillRectProgram::instance().draw(
			   batch_up<FillRectProgram::Arguments>(Program::kRect, items, &i));
			break;

		case Program::kTerrainBase: {
			ScopedScissor scoped_scissor(item.terrain_arguments.destination_rect);
			terrain_program_->draw(item.terrain_arguments.gametime, *item.terrain_arguments.terrains,
			                       *item.terrain_arguments.fields_to_draw, item.z_value,
			                       item.terrain_arguments.player);
			++i;
		} break;

		case Program::kTerrainDither: {
			ScopedScissor scoped_scissor(item.terrain_arguments.destination_rect);
			dither_program_->draw(item.terrain_arguments.gametime, *item.terrain_arguments.terrains,
			                      *item.terrain_arguments.fields_to_draw, item.z_value,
			                      item.terrain_arguments.player);
			++i;
		} break;

		case Program::kTerrainWorkarea: {
			ScopedScissor scoped_scissor(item.terrain_arguments.destination_rect);
			workarea_program_->draw(
			   item.terrain_arguments.terrains->get(0).get_texture(0).blit_data().texture_id,
			   item.terrain_arguments.workareas, *item.terrain_arguments.fields_to_draw, item.z_value,
			   Vector2f(item.terrain_arguments.renderbuffer_width,
			            item.terrain_arguments.renderbuffer_height));
			++i;
		} break;

		case Program::kTerrainGrid: {
			ScopedScissor scoped_scissor(item.terrain_arguments.destination_rect);
			grid_program_->draw(
			   item.terrain_arguments.terrains->get(0).get_texture(0).blit_data().texture_id,
			   *item.terrain_arguments.fields_to_draw, item.z_value);
			++i;
		} break;

		case Program::kTerrainRoad: {
			ScopedScissor scoped_scissor(item.terrain_arguments.destination_rect);
			road_program_->draw(
			   item.terrain_arguments.renderbuffer_width, item.terrain_arguments.renderbuffer_height,
			   *item.terrain_arguments.fields_to_draw, item.terrain_arguments.scale, item.z_value);
			++i;
		} break;

		default:
			throw wexception("Unknown item.program_id: %d", item.program_id);
		}
	}
}
