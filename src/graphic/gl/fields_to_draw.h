/*
 * Copyright (C) 2006-2015 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_GL_FIELDS_TO_DRAW_H
#define WL_GRAPHIC_GL_FIELDS_TO_DRAW_H

#include <cstddef>
#include <string>
#include <vector>

#include <stdint.h>

#include "logic/map_objects/tribes/road_textures.h"

// Helper struct that contains the data needed for drawing all fields. All
// methods are inlined for performance reasons.
class FieldsToDraw {
public:
	struct Field {
		int fx, fy;        // geometric coordinates (i.e. map coordinates that can be out of bounds).
		float gl_x, gl_y;  // GL Position of this field.
		float pixel_x, pixel_y;             // Pixel position relative to top left.
		float texture_x, texture_y;         // Texture coordinates.
		float brightness;                   // brightness of the pixel
		uint8_t ter_r, ter_d;               // Texture index of the right and down triangle.
		uint8_t roads;                      // Bitmask of roads to render, see logic/roadtype.h.
		const RoadTextures* road_textures;  // Road Textures to use for drawing.
	};

	FieldsToDraw() = default;

	// Resize this fields to draw for reuse.
	void reset(int minfx, int maxfx, int minfy, int maxfy, float zoom) {
		min_fx_ = minfx;
		max_fx_ = maxfx;
		min_fy_ = minfy;
		max_fy_ = maxfy;
		zoom_ = zoom;
		w_ = max_fx_ - min_fx_ + 1;
		h_ = max_fy_ - min_fy_ + 1;
		const size_t dimension = w_ * h_;
		if (fields_.size() != dimension) {
			fields_.resize(dimension);
		}
	}

	// Calculates the index of the given field with ('fx', 'fy') being geometric
	// coordinates in the map. Returns -1 if this field is not in the fields_to_draw.
	inline int calculate_index(int fx, int fy) const {
		uint16_t xidx = fx - min_fx_;
		if (xidx >= w_) {
			return -1;
		}
		uint16_t yidx = fy - min_fy_;
		if (yidx >= h_) {
			return -1;
		}
		return yidx * w_ + xidx;
	}

	// The number of fields to draw.
	inline size_t size() const {
		return fields_.size();
	}

	// Get the field at 'index' which must be in bound.
	inline const Field& at(const int index) const {
		return fields_.at(index);
	}

	// Returns a mutable field at 'index' which must be in bound.
	inline Field* mutable_field(const int index) {
		return &fields_[index];
	}

private:
	// Zoom factor: 2 means triangles have double the size.
	float zoom_;

	// Minimum and maximum field coordinates (geometric) to render. Can be negative.
	int min_fx_;
	int max_fx_;
	int min_fy_;
	int max_fy_;

	// Width and height in number of fields.
	int w_;
	int h_;

	std::vector<Field> fields_;
};

#endif  // end of include guard: WL_GRAPHIC_GL_FIELDS_TO_DRAW_H
