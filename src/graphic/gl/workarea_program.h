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

#ifndef WL_GRAPHIC_GL_WORKAREA_PROGRAM_H
#define WL_GRAPHIC_GL_WORKAREA_PROGRAM_H

#include <memory>

#include "base/vector.h"
#include "graphic/gl/fields_to_draw.h"
#include "graphic/gl/utils.h"
#include "logic/map_objects/tribes/workarea_info.h"

class WorkareaProgram {
public:
	// Compiles the program. Throws on errors.
	WorkareaProgram();

	// Draws the workarea overlay.
	void draw(uint32_t texture_id, Workareas, const FieldsToDraw&, float z, Vector2f rendertarget);

private:
	struct PerVertexData {
		float gl_x;
		float gl_y;
		float overlay_r;
		float overlay_g;
		float overlay_b;
		float overlay_a;
	};
	static_assert(sizeof(PerVertexData) == 24, "Wrong padding.");

	void gl_draw(int gl_texture, float z_value);

	// Adds a vertex to the end of vertices with data from 'field' in order to apply the specified
	// 'overlay' and, if desired, at the specified offset.
	void add_vertex(const FieldsToDraw::Field& field,
	                RGBAColor overlay,
	                std::vector<PerVertexData>*,
	                Vector2f offset = Vector2f::zero(),
	                Vector2f viewport = Vector2f::zero());

	// The program used for drawing the workarea overlay.
	Gl::Program gl_program_;

	// The buffer that will contain 'vertices_' for rendering.
	Gl::Buffer<PerVertexData> gl_array_buffer_;

	// Attributes.
	GLint attr_position_;
	GLint attr_overlay_;

	// Uniforms.
	GLint u_z_value_;

	// Objects below are kept around to avoid memory allocations on each frame.
	// They could theoretically also be recreated.
	std::vector<PerVertexData> vertices_;
	std::vector<PerVertexData> outer_vertices_;

	// Calculating the workareas is a bit slow, so we only recalculate when we have to
	struct WorkareasCache {
		WorkareasCache(const Workareas& wa, const Widelands::FCoords f, const Vector2f v)
		   : workareas(wa), fcoords(f), surface_pixel(v) {
		}

		const Workareas workareas;
		// Viewpoint data
		const Widelands::FCoords fcoords;
		const Vector2f surface_pixel;
	};
	std::unique_ptr<WorkareasCache> cache_;

	DISALLOW_COPY_AND_ASSIGN(WorkareaProgram);
};

#endif  // end of include guard: WL_GRAPHIC_GL_WORKAREA_PROGRAM_H
