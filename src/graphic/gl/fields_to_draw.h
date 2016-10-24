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

#include "base/vector.h"
#include "logic/map_objects/tribes/road_textures.h"
#include "logic/player.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

// Helper struct that contains the data needed for drawing all fields. All
// methods are inlined for performance reasons.
class FieldsToDraw {
public:
	struct Field {
		Widelands::Coords geometric_coords;  // geometric coordinates (i.e. map coordinates that can
		                                     // be out of bounds).
		Widelands::FCoords fcoords;  // The normalized coords and the field this is refering to.
		Vector2f gl_position;        // GL Position of this field.

		// Surface pixel this will be plotted on.
		Vector2f surface_pixel;

		// Rendertarget pixel this will be plotted on. This is only different by
		// the Rendertarget::get_rect().origin() of the view window.
		Vector2f rendertarget_pixel;
		Vector2f texture_coords;  // Texture coordinates.
		float brightness;         // brightness of the pixel

		// The next values are not necessarily the true data of this field, but
		// what the player should see. For example in fog of war we always draw
		// what we saw last.
		uint8_t roads;  // Bitmask of roads to render, see logic/roadtype.h.
		bool is_border;
		Widelands::Vision vision;
		Widelands::Player* owner;  // can be nullptr.

		// Index of neighbors in this 'FieldsToDraw'. -1 if this neighbor is not
		// contained.
		int ln_index;
		int rn_index;
		int trn_index;
		int bln_index;
		int brn_index;

// NOCOM(#codereview): We tend to use numeric_limits to mark values as invalid.
// Should we be consistent here, and preferably with a constexpr for it?
// This can be done in a follow-up branch in any case.
		inline bool all_neighbors_valid() const {
			return ln_index >= 0 && rn_index >= 0 && trn_index >= 0 && bln_index >= 0 &&
			       brn_index >= 0;
		}
	};

	FieldsToDraw() {
	}

	// Resize this fields to draw for reuse.
	void reset(int minfx, int maxfx, int minfy, int maxfy) {
		min_fx_ = minfx;
		max_fx_ = maxfx;
		min_fy_ = minfy;
		max_fy_ = maxfy;
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
