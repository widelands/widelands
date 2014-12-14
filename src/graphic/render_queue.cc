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

int RenderQueue::z;

// static
RenderQueue& RenderQueue::instance() {
	static RenderQueue render_queue;
	return render_queue;
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

		default:
			throw wexception("Unknown item.program: %d", item.program);
		}
	}
	items_.clear();
	z = 1;
}

// NOCOM(#sirver): do something here.
