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
#include "base/wexception.h"
#include "graphic/gl/blit_program.h"
#include "graphic/gl/dither_program.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/gl/fill_rect_program.h"
#include "graphic/gl/road_program.h"
#include "graphic/gl/terrain_program.h"

namespace {

// Maps [0, max_z] linearly to [1., -1.] for use in vertex shaders.
inline float to_opengl_z(int z, int max_z) {
	return -(2.f * z) / max_z + 1.f;
}

// The key defines in which order we render things.
//
// For opaque objects, render order makes no difference in the final image, but
//   - we batch up by program to have maximal batching.
//   - and we want to render frontmost objects first, so that we do not render
//     any pixel more than once.
uint64_t make_key_opaque(uint64_t program, int z_value) {
	assert(program < std::numeric_limits<uint16_t>::max());
	assert(0 <= z_value && z_value < std::numeric_limits<uint16_t>::max());

	// NOCOM(#sirver): add program sorting - texture for example, so that batching them works.
	uint64_t sort_z_value = std::numeric_limits<uint16_t>::max() - z_value;
	return (program << 48) | (sort_z_value << 32);
}

// For blended objects, we need to render furthest away objects first, and we
// do not update the z-buffer. This guarantees that the image is correct.
//   - if z value is the same, we order by program second to have potential batching.
uint64_t make_key_blended(uint64_t program, int z_value) {
	assert(program < std::numeric_limits<uint16_t>::max());
	assert(0 <= z_value && z_value < std::numeric_limits<uint16_t>::max());

	// Sort opaque objects increasing, alpha objects decreasing in order.
	uint64_t sort_z_value = z_value;
	return (sort_z_value << 48) | (program << 32);
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
	if (given_item.blend_mode == BlendMode::Copy) {
		opaque_items_.emplace_back(given_item);
		item = &opaque_items_.back();
		item->z = next_z;
		item->key = make_key_opaque(static_cast<uint64_t>(item->program), item->z);
	} else {
		blended_items_.emplace_back(given_item);
		item = &blended_items_.back();
		item->z = next_z;
		item->key = make_key_blended(static_cast<uint64_t>(item->program), item->z);
	}

	// Add more than 1 since some items have multiple programs that all need a
	// separate z buffer.
	next_z += 3;
}

// NOCOM(#sirver): document that this draws everything in this frame.
void RenderQueue::draw() {
	// NOCOM(#sirver): pull out constant
	assert(next_z < 65536);

	glDisable(GL_BLEND);

	log("#sirver Drawing Opaque stuff: %d.\n", opaque_items_.size());
	std::sort(opaque_items_.begin(), opaque_items_.end());
	draw_items(opaque_items_);
	opaque_items_.clear();

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);

	log("#sirver Drawing blended stuff: %d.\n", blended_items_.size());
	std::sort(blended_items_.begin(), blended_items_.end());
	draw_items(blended_items_);
	blended_items_.clear();

	glDepthMask(GL_TRUE);

	next_z = 1;
}

void RenderQueue::draw_items(const std::vector<Item>& items) {
	size_t i = 0;
	while (i < items.size()) {
		const Item& item = items[i];

		log("#sirver    program: %d, item.z: %d %f, key: %llx\n", item.program, item.z, to_opengl_z(item.z, next_z), item.key);
		switch (item.program) {
		case Program::BLIT:
			VanillaBlitProgram::instance().draw(item.destination_rect,
			                                    item.vanilla_blit_arguments.source_rect,
			                                    to_opengl_z(item.z, next_z),
			                                    item.vanilla_blit_arguments.texture,
			                                    item.vanilla_blit_arguments.opacity,
			                                    item.blend_mode);
			++i;
			break;

		case Program::BLIT_MONOCHROME:
			MonochromeBlitProgram::instance().draw(item.destination_rect,
			                                       item.monochrome_blit_arguments.source_rect,
			                                       to_opengl_z(item.z, next_z),
			                                       item.monochrome_blit_arguments.texture,
			                                       item.monochrome_blit_arguments.blend);
			++i;
			break;

		case Program::BLIT_BLENDED:
			BlendedBlitProgram::instance().draw(item.destination_rect,
			                                    item.blended_blit_arguments.source_rect,
			                                    to_opengl_z(item.z, next_z),
			                                    item.blended_blit_arguments.texture,
			                                    item.blended_blit_arguments.mask,
			                                    item.blended_blit_arguments.blend);
			++i;
			break;

		case Program::LINE:
			DrawLineProgram::instance().draw(item.destination_rect.x,
			                                 item.destination_rect.y,
			                                 item.destination_rect.x + item.destination_rect.w,
			                                 item.destination_rect.y + item.destination_rect.h,
			                                 to_opengl_z(item.z, next_z),
			                                 item.line_arguments.color,
			                                 item.line_arguments.line_width);
			++i;
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
			                       to_opengl_z(item.z, next_z));
			// NOCOM(#sirver): not pretty. Instead put the other two in the blending buckte.
			glEnable(GL_BLEND);

			dither_program_->draw(item.terrain_arguments.gametime,
			                      *item.terrain_arguments.terrains,
			                      *item.terrain_arguments.fields_to_draw,
			                      to_opengl_z(item.z + 1, next_z));
			road_program_->draw(*item.terrain_arguments.screen,
			                    *item.terrain_arguments.fields_to_draw,
			                    to_opengl_z(item.z + 2, next_z));
			delete item.terrain_arguments.fields_to_draw;
			glDisable(GL_BLEND);
			glDisable(GL_SCISSOR_TEST);
			++i;
			break;

		case Program::RECT: {
			std::vector<FillRectProgram::Arguments> args;
			while (i < items.size()) {
				const Item& current_item = items[i];
				if (current_item.program != item.program) {
					break;
				}
				args.emplace_back(FillRectProgram::Arguments{current_item.destination_rect,
				                                             to_opengl_z(current_item.z, next_z),
				                                             current_item.rect_arguments.color,
				                                             current_item.blend_mode});
				++i;
			}
			log("#sirver   Batched: args.size(): %d\n", args.size());
			FillRectProgram::instance().draw(args);
		} break;

		default:
			throw wexception("Unknown item.program: %d", item.program);
		}
	}
}
