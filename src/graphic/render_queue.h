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

#include <memory>
#include <vector>

#include <stdint.h>

#include "base/macros.h"
#include "base/rect.h"
#include "graphic/blend_mode.h"
#include "graphic/color.h"
#include "graphic/gl/blit_source.h"
#include "graphic/gl/fields_to_draw.h"
#include "logic/description_maintainer.h"
#include "logic/world/terrain_description.h"

class DitherProgram;
class RoadProgram;
class TerrainProgram;

// NOCOM(#sirver): document
class RenderQueue {
public:
	enum Program {
		TERRAIN_BASE,
		TERRAIN_DITHER,
		TERRAIN_ROAD,
		BLIT,
		BLIT_MONOCHROME,
		BLIT_BLENDED,
		RECT,
		LINE,
		HIGHEST_PROGRAM_ID,
	};

	struct VanillaBlitArguments {
		BlitSource texture;
		float opacity;
	};

	struct MonochromeBlitArguments {
		BlitSource texture;
		RGBAColor blend;
	};

	struct BlendedBlitArguments {
		BlitSource texture;
		BlitSource mask;
		RGBAColor blend;
	};

	struct RectArguments {
		RGBAColor color;
	};

	struct LineArguments {
		RGBColor color;
		uint8_t line_width;
	};

	struct TerrainArguments {
		TerrainArguments() {}

		int gametime;
		int renderbuffer_width;
		int renderbuffer_height;
		const DescriptionMaintainer<Widelands::TerrainDescription>* terrains;
		FieldsToDraw* fields_to_draw;
	};

	// NOCOM(#sirver): document and figure out.
	struct Item {
		Item() {}

		inline bool operator<(const Item& other) const {
			return key < other.key;
		}

		int program_id;
		float z_value;
		FloatRect destination_rect;
		uint64_t key;
		BlendMode blend_mode;

		union {
			VanillaBlitArguments vanilla_blit_arguments;
			MonochromeBlitArguments monochrome_blit_arguments;
			BlendedBlitArguments blended_blit_arguments;
			TerrainArguments terrain_arguments;
			RectArguments rect_arguments;
			LineArguments line_arguments;
		};
	};

	static RenderQueue& instance();

	void enqueue(const Item& item);

	void draw(int screen_width, int screen_height);

private:
	RenderQueue();

	void draw_items(const std::vector<Item>& items);

	// The z value that should be used for the next draw, so that it is on top
	// of everything before.
	int next_z_;

	std::unique_ptr<TerrainProgram> terrain_program_;
	std::unique_ptr<DitherProgram> dither_program_;
	std::unique_ptr<RoadProgram> road_program_;

	std::vector<Item> blended_items_;
	std::vector<Item> opaque_items_;

	DISALLOW_COPY_AND_ASSIGN(RenderQueue);
};


#endif  // end of include guard: WL_GRAPHIC_RENDER_QUEUE_H
