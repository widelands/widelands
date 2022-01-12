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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef WL_GRAPHIC_RENDER_QUEUE_H
#define WL_GRAPHIC_RENDER_QUEUE_H

#include <memory>

#include "base/macros.h"
#include "base/rect.h"
#include "graphic/blend_mode.h"
#include "graphic/blit_mode.h"
#include "graphic/color.h"
#include "graphic/gl/draw_line_program.h"
#include "graphic/gl/fields_to_draw.h"
#include "logic/map_objects/description_maintainer.h"
#include "logic/map_objects/world/terrain_description.h"

class DitherProgram;
class GridProgram;
class RoadProgram;
class TerrainProgram;
class WorkareaProgram;

// The RenderQueue is a singleton implementing the concept of deferred
// rendering: Every rendering call that pretends to draw onto the screen will
// instead enqueue an item into the RenderQueue. The Graphic::refresh() will
// then setup OpenGL to render onto the screen and then call
// RenderQueue::draw() which will execute all the draw calls.
//
// The advantage of this design is that render calls can be reordered and
// batched up to avoid OpenGL state changes as much as possible. This can
// reduce the amount of OpenGL calls done in the system per frame by an order
// of magnitude if assets are properly batched up into texture atlases.
//
// Rendering is simple: first everything fully opaque is rendered front to back
// (so that no pixel is drawn twice). This allows for maximum program batching,
// as for example all opaque rectangles can be rendered in one draw call,
// ignoring z-value.
//
// In the second step, all drawing calls with (partially) transparent pixels
// are done. This has to be done strictly in z ordering (back to front), so
// that transparency works correctly. But common operations can still be
// batched - for example the blitting of houses could all be done with the same
// z value and using a common texture atlas. Then they could be drawn in one
// woosh.
//
// Non overlapping rectangles can be drawn in parallel, ignoring z-order. I
// experimented with a linear algorithm to find all overlapping rectangle
// pairs (see bzr history), but it did not buy the performance I was hoping it
// would. So I abandoned this idea again.
//
// Note: all draw calls that target a Texture are not going to the RenderQueue,
// but are still immediately executed. The RenderQueue is only used for
// rendering onto the screen.
//
// TODO(sirver): we could (even) better performance by being z-layer aware
// while drawing. For example the UI could draw non-overlapping windows and
// sibling children with the same z-value for better batching. Also for example
// build-help symbols, buildings, and flags could all be drawn with the same
// z-layer for better batching up. This would also get rid of the z-layer
// issues we are having.
class RenderQueue {
public:
	enum Program {
		kTerrainBase,
		kTerrainDither,
		kTerrainWorkarea,
		kTerrainGrid,
		kTerrainRoad,
		kBlit,
		kRect,
		kLine,
		kHighestProgramId,
	};

	struct MonochromeBlitArguments {
		BlitData texture;
		RGBAColor blend;
	};

	struct BlitArguments {
		BlitMode mode;
		BlitData texture;
		BlitData mask;
		RGBAColor blend;
		Rectf destination_rect;
	};

	struct RectArguments {
		RGBAColor color;
		Rectf destination_rect;
	};

	// TODO(sirver): these are really triangle arguments.
	struct LineArguments {
		std::vector<DrawLineProgram::PerVertexData> vertices;
	};

	struct TerrainArguments {
		// Initialize everything to make cppcheck happy.
		int gametime = 0;
		int renderbuffer_width = 0;
		int renderbuffer_height = 0;
		const Widelands::DescriptionMaintainer<Widelands::TerrainDescription>* terrains = nullptr;
		const FieldsToDraw* fields_to_draw = nullptr;
		Workareas workareas;
		float scale = 1.f;
		Rectf destination_rect = Rectf(0.f, 0.f, 0.f, 0.f);
		const Widelands::Player* player = nullptr;
	};

	// The union of all possible program arguments represents an Item that is
	// enqueued in the Queue. This is on purpose not done with OOP so that the
	// queue is more cache friendly.
	struct Item {
		inline bool operator<(const Item& other) const {
			return key < other.key;
		}

		// The program that will be used to draw this item. Also defines which
		// union type is filled in.
		int program_id;

		// The z-value in GL space that will be used for drawing.
		float z_value;

		// The key for sorting this item in the queue. It depends on the type of
		// item how this is calculated, but it will contain at least the program,
		// the z-layer, if it is opaque or transparent and program specific
		// options. After ordering the queue by this, it defines the batching.
		uint64_t key;

		// If this is opaque or, if not, which blend_mode to use.
		BlendMode blend_mode;

		// This is a logical union, i.e. only one of these members will be filled
		// with useful data. It cannot be a true union because some items are not
		// plain-old data types.
		BlitArguments blit_arguments;
		TerrainArguments terrain_arguments;
		RectArguments rect_arguments;
		LineArguments line_arguments;
	};

	static RenderQueue& instance();

	// Enqueues 'item' in the queue with a higher 'z' value than the last enqueued item.
	void enqueue(const Item& item);

	// Draws all items in the queue in an optimal ordering and as much batching
	// as possible. This will draw one complete frame onto the screen and this
	// function is the only one that actually triggers draws to the screen
	// directly.
	void draw(int screen_width, int screen_height);

	// Clear the render queue without drawing anything.
	void clear();

private:
	RenderQueue();

	void draw_items(const std::vector<Item>& items);

	// The z value that should be used for the next draw, so that it is on top
	// of everything before.
	int next_z_;

	std::unique_ptr<TerrainProgram> terrain_program_;
	std::unique_ptr<DitherProgram> dither_program_;
	std::unique_ptr<WorkareaProgram> workarea_program_;
	std::unique_ptr<GridProgram> grid_program_;
	std::unique_ptr<RoadProgram> road_program_;

	std::vector<Item> blended_items_;
	std::vector<Item> opaque_items_;

	DISALLOW_COPY_AND_ASSIGN(RenderQueue);
};

#endif  // end of include guard: WL_GRAPHIC_RENDER_QUEUE_H
