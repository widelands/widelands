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

template<typename F>
class FieldsToDrawRef;

struct FieldToDrawBase {
	Widelands::FCoords fcoords;  // The normalized coords and the field this is refering to.
	Vector2f rendertarget_pixel;

	// The next values are not necessarily the true data of this field, but
	// what the player should see. For example in fog of war we always draw
	// what we saw last.
	//
	// Note: we put the roads member variable here for better alignment even
	// though it isn't needed in the Gl4 path.
	Widelands::Player* owner;  // can be nullptr.
	uint8_t roads;  // Bitmask of roads to render, see logic/roadtype.h.
	bool is_border;
	Widelands::Vision vision;
};

struct FieldToDrawGl2 : FieldToDrawBase {
	Vector2f gl_position;        // GL Position of this field.

	// Surface pixel this will be plotted on.
	Vector2f surface_pixel;

	// Rendertarget pixel this will be plotted on. This is only different by
	// the Rendertarget::get_rect().origin() of the view window.
	Vector2f texture_coords;  // Texture coordinates.
	float brightness;         // brightness of the pixel
};

// Helper struct that contains the data needed for drawing all fields. All
// methods are inlined for performance reasons.
class FieldsToDrawImplBase {
public:
	static constexpr int kInvalidIndex = std::numeric_limits<int>::min();

	FieldsToDrawImplBase() {
	}

	// Resize this fields to draw for reuse.
	void reset(int minfx, int maxfx, int minfy, int maxfy) {
		min_fx_ = minfx;
		max_fx_ = maxfx;
		min_fy_ = minfy;
		max_fy_ = maxfy;
		w_ = max_fx_ - min_fx_ + 1;
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

protected:
	// Minimum and maximum field coordinates (geometric) to render. Can be negative.
	int min_fx_;
	int max_fx_;
	int min_fy_;
	int max_fy_;

	// Width in number of fields.
	int w_;
};

// Helper struct that contains the data needed for drawing all fields. All
// methods are inlined for performance reasons.
template<typename Field_>
class FieldsToDraw : public FieldsToDrawImplBase {
public:
	using Field = Field_;

	FieldsToDraw() {
	}

	// Resize this fields to draw for reuse.
	void reset(int minfx, int maxfx, int minfy, int maxfy) {
		FieldsToDrawImplBase::reset(minfx, maxfx, minfy, maxfy);

		int h = max_fy_ - min_fy_ + 1;
		const size_t dimension = FieldsToDrawImplBase::get_w() * h;
		if (fields_.size() != dimension) {
			fields_.resize(dimension);
		}
	}

	// The number of fields to draw.
	inline size_t size() const {
		return fields_.size();
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
	template<typename F>
	friend class FieldsToDrawRef;

	const Field* get_fields() const {
		return &fields_[0];
	}

	intptr_t get_fields_stride() const {
		return sizeof(fields_[0]);
	}

	std::vector<Field> fields_;
};

template<typename Field_>
class FieldsToDrawRef : public FieldsToDrawImplBase {
public:
	using Field = Field_;

	template<typename FTD>
	FieldsToDrawRef(const FTD& fields_to_draw) {
		reset(fields_to_draw.min_fx(), fields_to_draw.max_fx(),
		      fields_to_draw.min_fy(), fields_to_draw.max_fy());

		// First assign to type Field* for type-checking, before reinterpreting
		// to the char* that will be used for pointer arithmetic.
		const Field* fields = fields_to_draw.get_fields();

		fields_ = reinterpret_cast<const char*>(fields);
		stride_ = fields_to_draw.get_fields_stride();
	}

	const Field& operator[](int index) const {
		const char* base = fields_ + index * stride_;
		return *reinterpret_cast<const Field*>(base);
	}

	FieldsToDrawCursor<const FieldsToDrawRef> cursor() const;

private:
	const char* fields_;
	intptr_t stride_;
};

using FieldsToDrawBase = FieldsToDraw<FieldToDrawBase>;
using FieldsToDrawGl2 = FieldsToDraw<FieldToDrawGl2>;
using FieldsToDrawRefBase = FieldsToDrawRef<FieldToDrawBase>;

// For iteration over fields.
//
// Template for const-correctness.
template<typename FTD>
class FieldsToDrawCursor {
public:
	using Field = typename FTD::Field;

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

	// Return current geometric coordinates (i.e. map coordinates that can
	// be out of bounds).
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
		assert(valid());
		return (geometric_coords_.y > fields_.min_fy() &&
		        geometric_coords_.y < fields_.max_fy() &&
		        geometric_coords_.x > fields_.min_fx() &&
		        geometric_coords_.x < fields_.max_fx());
	}

private:
	template<typename F>
	void type_check(const FieldsToDraw<F>&) {}
	template<typename F>
	void type_check(const FieldsToDrawRef<F>&) {}

	FTD& fields_;

	Widelands::Coords geometric_coords_;
	int geometric_tblx_shift_; // top/bottom left neighbor geometric x-coordinate offset
	int index_;
};

template<typename F>
inline FieldsToDrawCursor<FieldsToDraw<F>> FieldsToDraw<F>::cursor() {
	return {*this};
}

template<typename F>
inline FieldsToDrawCursor<const FieldsToDraw<F>> FieldsToDraw<F>::cursor() const {
	return {*this};
}

template<typename F>
inline FieldsToDrawCursor<const FieldsToDrawRef<F>> FieldsToDrawRef<F>::cursor() const {
	return {*this};
}

#endif  // end of include guard: WL_GRAPHIC_GL_FIELDS_TO_DRAW_H
