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

#ifndef WL_GRAPHIC_RENDER_QUEUE_H
#define WL_GRAPHIC_RENDER_QUEUE_H

#include <vector>

#include <stdint.h>

#include "base/macros.h"
#include "base/rect.h"
#include "graphic/blend_mode.h"

// NOCOM(#sirver): this should be a base class. Figure out how to make it widelands agnostic.

// NOCOM(#sirver): document
class RenderQueue {
public:
	static int z; // NOCOM(#sirver): ugly.

	enum class Program {
		BLIT
	};

	// NOCOM(#sirver): maybe BlendMode::REMOVE?

	struct VanillaBlitArguments {
		FloatRect destination_rect;
		FloatRect source_rect;
		int texture;
		float opacity;
	};

	// NOCOM(#sirver): document and figure out.
	// // NOCOM(#sirver): maybe combine to a single 64 bit value?
	struct Item {
		Item() {}

		Program program;
		uint16_t z;
		BlendMode blend_mode;
		union {
			VanillaBlitArguments vanilla_blit_arguments;
		};
	};

	static RenderQueue& instance();

	void enqueue(const Item& item);

	void draw();

private:
	RenderQueue() = default;

	std::vector<Item> items_;

	DISALLOW_COPY_AND_ASSIGN(RenderQueue);
};


#endif  // end of include guard: WL_GRAPHIC_RENDER_QUEUE_H
