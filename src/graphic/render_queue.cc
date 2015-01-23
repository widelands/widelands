/*
 * Copyright (C) 2006-2014 by the Widelands Development Team
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

#include "graphic/render_queue.h"

#include <algorithm>
#include <limits>

#include "base/log.h"
#include "base/rect.h"
#include "base/wexception.h"
#include "graphic/gl/blit_program.h"
#include "graphic/gl/dither_program.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/gl/fill_rect_program.h"
#include "graphic/gl/road_program.h"
#include "graphic/gl/terrain_program.h"

namespace {

constexpr int kMaximumZValue = std::numeric_limits<uint16_t>::max();
constexpr float kOpenGlZDelta = -2.f / kMaximumZValue;

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
uint64_t make_key_opaque(uint64_t program_id, int z_value, uint64_t texture) {
	assert(program_id < std::numeric_limits<uint16_t>::max());
	assert(0 <= z_value && z_value < std::numeric_limits<uint16_t>::max());

	// NOCOM(#sirver): add program sorting - texture for example, so that batching them works.
	uint64_t sort_z_value = std::numeric_limits<uint16_t>::max() - z_value;
	return (program_id << 48) | (texture << 32) | sort_z_value;
}

// For blended objects, we need to render furthest away objects first, and we
// do not update the z-buffer. This guarantees that the image is correct.
//   - if z value is the same, we order by program second to have potential batching.
uint64_t make_key_blended(uint64_t program_id, int z_value, uint64_t texture) {
	assert(program_id < std::numeric_limits<uint16_t>::max());
	assert(0 <= z_value && z_value < std::numeric_limits<uint16_t>::max());

	// Sort opaque objects increasing, alpha objects decreasing in order.
	uint64_t sort_z_value = z_value;
	return (sort_z_value << 48) | (program_id << 32) | texture;
	// return (program_id << 48) | (texture << 32) | sort_z_value;
}

}  // namespace

RenderQueue::RenderQueue()
   : next_z(1),
     terrain_program_(new TerrainProgram()),
     dither_program_(new DitherProgram()),
     road_program_(new RoadProgram()) {
}

// static
RenderQueue& RenderQueue::instance() {
	static RenderQueue render_queue;
	return render_queue;
}

// NOCOM(#sirver): take individual parameters?
void RenderQueue::enqueue(const Item& given_item) {
	Item* item;
	int texture = 0;
	switch (given_item.program_id) {
		case BLIT:
			texture = given_item.vanilla_blit_arguments.texture;
			break;
		case BLIT_MONOCHROME:
			texture = given_item.monochrome_blit_arguments.texture;
			break;
		case BLIT_BLENDED:
			texture = given_item.blended_blit_arguments.texture;
			break;
	}

	if (given_item.blend_mode == BlendMode::Copy) {
		opaque_items_.emplace_back(given_item);
		item = &opaque_items_.back();
		item->z_value = to_opengl_z(next_z);
		item->key = make_key_opaque(static_cast<uint64_t>(item->program_id), next_z, texture);
	} else {
		blended_items_.emplace_back(given_item);
		item = &blended_items_.back();
		item->z_value = to_opengl_z(next_z);
		item->key = make_key_blended(static_cast<uint64_t>(item->program_id), next_z, texture);
	}

	// Add more than 1 since some items have multiple programs that all need a
	// separate z buffer.
	next_z += 3;
}

// NOCOM(#sirver): document that this draws everything in this frame.
void RenderQueue::draw() {
	if (next_z >= kMaximumZValue) {
		throw wexception("Too many drawn layers. Ran out of z-values.");
	}

	glDisable(GL_BLEND);

	// log("#sirver Drawing Opaque stuff: %ld.\n", opaque_items_.size());
	std::sort(opaque_items_.begin(), opaque_items_.end());
	draw_items(opaque_items_);
	opaque_items_.clear();

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	// log("#sirver Drawing blended stuff: %ld.\n", blended_items_.size());
	std::sort(blended_items_.begin(), blended_items_.end());

	// NOCOM(#sirver): not really needed, templatize original method?
	// std::vector<FloatRect> destination_rects;
	// for (const auto& item : blended_items_) {
		// destination_rects.emplace_back(item.destination_rect);
	// }
	// log("#sirver destination_rects.size(): %d\n", destination_rects.size());
	// const OverlappingRects overlapping_rects = find_overlapping_rectangles(destination_rects);
	// for (int i = 0; i < overlapping_rects.size(); ++i) {
		// if (overlapping_rects[i].empty()) {
			// continue;
		// }
		// log("#sirver   %d: ", i);
		// for (const auto& other : overlapping_rects[i]) {
			// log("%d ", other);
		// }
		// log("\n");
	// }

	draw_items(blended_items_);
	blended_items_.clear();

	glDepthMask(GL_TRUE);

	next_z = 1;
}


// NOCOM(#sirver): make static too.
inline void from_item(const RenderQueue::Item& item, VanillaBlitProgram::Arguments* args) {
	args->source_rect = item.vanilla_blit_arguments.source_rect;
	args->texture = item.vanilla_blit_arguments.texture;
	args->opacity = item.vanilla_blit_arguments.opacity;
}

inline void from_item(const RenderQueue::Item& item, MonochromeBlitProgram::Arguments* args) {
	args->source_rect = item.monochrome_blit_arguments.source_rect;
	args->texture = item.monochrome_blit_arguments.texture;
	args->blend = item.monochrome_blit_arguments.blend;
}

inline void from_item(const RenderQueue::Item& item, FillRectProgram::Arguments* args) {
	args->color = item.rect_arguments.color;
}

inline void from_item(const RenderQueue::Item& item, BlendedBlitProgram::Arguments* args) {
	args->texture = item.blended_blit_arguments.texture;
	args->source_rect = item.blended_blit_arguments.source_rect;
	args->blend = item.blended_blit_arguments.blend;
	args->texture_mask = item.blended_blit_arguments.texture_mask;
	args->mask_source_rect = item.blended_blit_arguments.mask_source_rect;
}

inline void from_item(const RenderQueue::Item& item, DrawLineProgram::Arguments* args) {
	args->color = item.line_arguments.color;
}

// NOCOM(#sirver): make static
template <typename T>
std::vector<T> batch_up(const RenderQueue::Program program_id,
                        const std::vector<RenderQueue::Item>& items,
                        size_t* i) {
	std::vector<T> all_args;
	while (*i < items.size()) {
		const RenderQueue::Item& current_item = items.at(*i);
		if (current_item.program_id != program_id) {
			break;
		}
		all_args.emplace_back();
		T& args = all_args.back();
		args.destination_rect = current_item.destination_rect;
		args.z_value = current_item.z_value;
		args.blend_mode = current_item.blend_mode;
		from_item(current_item, &args);
		++(*i);
	}
	// log("#sirver   Batched: %lu items for program_id: %d\n", all_args.size(), program_id);
	return all_args;
}

void RenderQueue::draw_items(const std::vector<Item>& items) {
	size_t i = 0;
	while (i < items.size()) {
		const Item& item = items[i];
		switch (item.program_id) {
			// NOCOM(#sirver): horrible code duplication.
		case Program::BLIT:
			// NOCOM(#sirver): if a ID is moved into this program_id, I would not need to pass redundant
			// information here.
			VanillaBlitProgram::instance().draw(
			   batch_up<VanillaBlitProgram::Arguments>(Program::BLIT, items, &i));
		 break;

		case Program::BLIT_MONOCHROME:
			MonochromeBlitProgram::instance().draw(
			   batch_up<MonochromeBlitProgram::Arguments>(Program::BLIT_MONOCHROME, items, &i));
			break;

		case Program::BLIT_BLENDED:
			BlendedBlitProgram::instance().draw(
			   batch_up<BlendedBlitProgram::Arguments>(Program::BLIT_BLENDED, items, &i));
			break;

		case Program::LINE:
			DrawLineProgram::instance().draw(
			   batch_up<DrawLineProgram::Arguments>(Program::LINE, items, &i));
			break;

		case Program::RECT:
			FillRectProgram::instance().draw(
			   batch_up<FillRectProgram::Arguments>(Program::RECT, items, &i));
			break;

		case Program::TERRAIN:
			glScissor(item.destination_rect.x,
			          item.destination_rect.y,
			          item.destination_rect.w,
			          item.destination_rect.h);
			glEnable(GL_SCISSOR_TEST);

			terrain_program_->draw(item.terrain_arguments.gametime,
			                       *item.terrain_arguments.terrains,
			                       *item.terrain_arguments.fields_to_draw,
										  item.z_value);
			// NOCOM(#sirver): not pretty. Instead put the other two in the blending buckte.
			glEnable(GL_BLEND);

			dither_program_->draw(item.terrain_arguments.gametime,
			                      *item.terrain_arguments.terrains,
			                      *item.terrain_arguments.fields_to_draw,
			                      item.z_value + kOpenGlZDelta);
			road_program_->draw(*item.terrain_arguments.screen,
			                    *item.terrain_arguments.fields_to_draw,
			                    item.z_value + 2 * kOpenGlZDelta);
			delete item.terrain_arguments.fields_to_draw;
			glDisable(GL_BLEND);
			glDisable(GL_SCISSOR_TEST);
			++i;
			break;


		default:
			throw wexception("Unknown item.program_id: %d", item.program_id);
		}
	}
}
