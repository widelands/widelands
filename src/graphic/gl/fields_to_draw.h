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
#include <limits>
#include <string>
#include <vector>

#include <stdint.h>

#include "base/vector.h"
#include "logic/map_objects/tribes/road_textures.h"
#include "logic/player.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

template<typename FTD>
class FieldsToDrawCursor;

// Helper struct that contains the data needed for drawing all fields. All
// methods are inlined for performance reasons.
class FieldsToDraw {
public:
	static constexpr int kInvalidIndex = std::numeric_limits<int>::min();

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

	int min_fx() const {
		return min_fx_;
	}

	int max_fx() const {
		return max_fx_;
	}

	int min_fy() const {
		return min_fy_;
	}

	int max_fy() const {
		return max_fy_;
	}

	int get_w() const {
		return w_;
	}

	// Calculates the index of the given field with ('fx', 'fy') being geometric
	// coordinates in the map. Returns kInvalidIndex if this field is not in the
	// fields_to_draw.
	inline int calculate_index(int fx, int fy) const {
		uint16_t xidx = fx - min_fx_;
		if (xidx >= w_) {
			return kInvalidIndex;
		}
		uint16_t yidx = fy - min_fy_;
		if (yidx >= h_) {
			return kInvalidIndex;
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

	Field& operator[](int index) {
		return fields_[index];
	}

	const Field& operator[](int index) const {
		return fields_[index];
	}

	FieldsToDrawCursor<FieldsToDraw> cursor();
	FieldsToDrawCursor<const FieldsToDraw> cursor() const;

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

// For iteration over fields.
//
// Template for const-correctness.
template<typename FTD>
class FieldsToDrawCursor {
public:
	using Field = FieldsToDraw::Field;

	FieldsToDrawCursor(FTD& fields_to_draw)
	  : fields_(fields_to_draw) {
		type_check(fields_to_draw);

		geometric_coords_.x = fields_.min_fx();
		geometric_coords_.y = fields_.min_fy();
		geometric_tblx_shift_ = (fields_.min_fy() & 1) - 1;

		index_ = 0;
	}

	bool valid() const {
		return index_ >= 0;
	}

	void next() {
		assert(valid());

		index_++;
		geometric_coords_.x++;
		if (geometric_coords_.x > fields_.max_fx()) {
			geometric_coords_.x = fields_.min_fx();
			geometric_tblx_shift_ = -1 - geometric_tblx_shift_;
			geometric_coords_.y++;
			if (geometric_coords_.y > fields_.max_fy())
				index_ = -1;
		}
	}

	const Field& field() const {
		assert(valid());
		return fields_[index_];
	}

	Field& mutable_field() {
		assert(valid());
		return fields_[index_];
	}

	Widelands::Coords geometric_coords() const {
		assert(valid());
		return geometric_coords_;
	}

	bool tln_valid() const {
		assert(valid());
		return (geometric_coords_.y > fields_.min_fy()) &&
				(geometric_coords_.x + geometric_tblx_shift_ >= fields_.min_fx());
	}

	const Field& tln() const {
		assert(tln_valid());
		return fields_[index_ + geometric_tblx_shift_ - fields_.get_w()];
	}

	bool trn_valid() const {
		assert(valid());
		return (geometric_coords_.y > fields_.min_fy()) &&
				(geometric_coords_.x + geometric_tblx_shift_ + 1 <= fields_.max_fx());
	}

	const Field& trn() const {
		assert(trn_valid());
		return fields_[index_ + geometric_tblx_shift_ + 1 - fields_.get_w()];
	}

	bool ln_valid() const {
		assert(valid());
		return (geometric_coords_.x - 1 >= fields_.min_fx());
	}

	const Field& ln() const {
		assert(ln_valid());
		return fields_[index_ - 1];
	}

	bool rn_valid() const {
		assert(valid());
		return (geometric_coords_.x + 1 <= fields_.max_fx());
	}

	const Field& rn() const {
		assert(rn_valid());
		return fields_[index_ + 1];
	}

	bool bln_valid() const {
		assert(valid());
		return (geometric_coords_.y < fields_.max_fy()) &&
				(geometric_coords_.x + geometric_tblx_shift_ >= fields_.min_fx());
	}

	const Field& bln() const {
		assert(bln_valid());
		return fields_[index_ + geometric_tblx_shift_ + fields_.get_w()];
	}

	bool brn_valid() const {
		assert(valid());
		return (geometric_coords_.y < fields_.max_fy()) &&
				(geometric_coords_.x + geometric_tblx_shift_ + 1 <= fields_.max_fx());
	}

	const Field& brn() const {
		assert(brn_valid());
		return fields_[index_ + geometric_tblx_shift_ + 1 + fields_.get_w()];
	}

	bool all_neighbors_valid() const {
		return tln_valid() && brn_valid();
	}

private:
	void type_check(const FieldsToDraw&) {}

	FTD& fields_;

	Widelands::Coords geometric_coords_;
	int geometric_tblx_shift_; // top/bottom left neighbor geometric x-coordinate offset
	int index_;
};

inline FieldsToDrawCursor<FieldsToDraw> FieldsToDraw::cursor() {
	return {*this};
}

inline FieldsToDrawCursor<const FieldsToDraw> FieldsToDraw::cursor() const {
	return {*this};
}

#endif  // end of include guard: WL_GRAPHIC_GL_FIELDS_TO_DRAW_H
