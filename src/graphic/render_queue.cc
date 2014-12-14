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

#include "base/log.h"
#include "base/wexception.h"
#include "graphic/gl/blit_program.h"
#include "graphic/gl/dither_program.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/gl/fill_rect_program.h"
#include "graphic/gl/road_program.h"
#include "graphic/gl/terrain_program.h"

int RenderQueue::z;

// static
RenderQueue& RenderQueue::instance() {
	static RenderQueue render_queue;
	return render_queue;
}

RenderQueue::RenderQueue()
   : terrain_program_(new TerrainProgram()),
     dither_program_(new DitherProgram()),
     road_program_(new RoadProgram()) {
}

void RenderQueue::enqueue(const Item& item) {
	items_.emplace_back(item);
}

void RenderQueue::draw() {
	for (const Item& item : items_) {
		switch (item.program) {
		case Program::BLIT:
			VanillaBlitProgram::instance().draw(item.vanilla_blit_arguments.destination_rect,
			                                    item.vanilla_blit_arguments.source_rect,
			                                    item.vanilla_blit_arguments.texture,
			                                    item.vanilla_blit_arguments.opacity,
			                                    item.blend_mode);
			break;

		case Program::BLIT_MONOCHROME:
			MonochromeBlitProgram::instance().draw(
			   item.monochrome_blit_arguments.destination_rect,
			   item.monochrome_blit_arguments.source_rect,
			   item.monochrome_blit_arguments.texture,
			   item.monochrome_blit_arguments.blend);
			break;

		case Program::BLIT_BLENDED:
			BlendedBlitProgram::instance().draw(
			   item.blended_blit_arguments.destination_rect,
			   item.blended_blit_arguments.source_rect,
			   item.blended_blit_arguments.texture,
			   item.blended_blit_arguments.mask,
			   item.blended_blit_arguments.blend);
			break;

		case Program::LINE:
			DrawLineProgram::instance().draw(
			   item.line_arguments.destination_rect.x,
			   item.line_arguments.destination_rect.y,
			   item.line_arguments.destination_rect.x + item.line_arguments.destination_rect.w,
			   item.line_arguments.destination_rect.y + item.line_arguments.destination_rect.h,
			   item.line_arguments.color,
			   item.line_arguments.line_width);
			break;


		case Program::TERRAIN:
			terrain_program_->draw(item.terrain_arguments.gametime,
			                       *item.terrain_arguments.terrains,
			                       *item.terrain_arguments.fields_to_draw);
			dither_program_->draw(item.terrain_arguments.gametime,
			                      *item.terrain_arguments.terrains,
			                      *item.terrain_arguments.fields_to_draw);
			road_program_->draw(
			   *item.terrain_arguments.screen, *item.terrain_arguments.fields_to_draw);
			delete item.terrain_arguments.fields_to_draw;
			break;

		case Program::RECT:
			FillRectProgram::instance().draw(
			   item.rect_arguments.destination_rect, item.rect_arguments.color, item.blend_mode);
			break;

		default:
			throw wexception("Unknown item.program: %d", item.program);
		}
	}
	items_.clear();
	z = 1;
}

// NOCOM(#sirver): do something here.
