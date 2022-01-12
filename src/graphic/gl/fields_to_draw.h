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

#ifndef WL_GRAPHIC_GL_FIELDS_TO_DRAW_H
#define WL_GRAPHIC_GL_FIELDS_TO_DRAW_H

#include "base/vector.h"
#include "graphic/rendertarget.h"
#include "graphic/road_segments.h"
#include "logic/editor_game_base.h"
#include "logic/vision.h"
#include "logic/widelands_geometry.h"

// Helper struct that contains the data needed for drawing all fields.
class FieldsToDraw {
public:
	static constexpr int kInvalidIndex = std::numeric_limits<int>::min();

	struct Field {
		Widelands::Coords geometric_coords;  // geometric coordinates (i.e. map coordinates that can
		                                     // be out of bounds).
		Widelands::FCoords fcoords;  // The normalized coords and the field this is refering to.
		Vector2f gl_position = Vector2f::zero();  // GL Position of this field.

		// Surface pixel this will be plotted on.
		Vector2f surface_pixel = Vector2f::zero();

		// Rendertarget pixel this will be plotted on. This is only different by
		// the Rendertarget::get_rect().origin() of the view window.
		Vector2f rendertarget_pixel = Vector2f::zero();
		Vector2f texture_coords = Vector2f::zero();  // Texture coordinates.
		float brightness;                            // brightness of the pixel

		// The next values are not necessarily the true data of this field, but
		// what the player should see. For example in fog of war we always draw
		// what we saw last.
		Widelands::RoadSegment road_e;
		Widelands::RoadSegment road_sw;
		Widelands::RoadSegment road_se;
		bool is_border;
		Widelands::VisibleState seeing;
		Widelands::Player* owner;  // can be nullptr.

		// Index of neighbors in this 'FieldsToDraw'. INVALID_INDEX if this
		// neighbor is not contained.
		int ln_index;
		int rn_index;
		int trn_index;
		int bln_index;
		int brn_index;

		inline bool all_neighbors_valid() const {
			return ln_index != kInvalidIndex && rn_index != kInvalidIndex &&
			       trn_index != kInvalidIndex && bln_index != kInvalidIndex &&
			       brn_index != kInvalidIndex;
		}
	};

	// Reinitialize for the given view parameters.
	void reset(const Widelands::EditorGameBase& egbase,
	           const Vector2f& viewpoint,
	           const float zoom,
	           RenderTarget* dst);

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

	// Calculates the index of the given field with ('fx', 'fy') being geometric
	// coordinates in the map. Returns INVALID_INDEX if this field is not in the
	// fields_to_draw.
	inline int calculate_index(int fx, int fy) const {
		if (fx < min_fx_ || fx > max_fx_ || fy < min_fy_ || fy > max_fy_) {
			return kInvalidIndex;
		}
		return (fy - min_fy_) * w_ + (fx - min_fx_);
	}

private:
	// Minimum and maximum field coordinates (geometric) to render. Can be negative.
	int min_fx_ = 0;
	int max_fx_ = 0;
	int min_fy_ = 0;
	int max_fy_ = 0;

	// Width and height in number of fields.
	int w_ = 0;
	int h_ = 0;

	std::vector<Field> fields_;
};

#endif  // end of include guard: WL_GRAPHIC_GL_FIELDS_TO_DRAW_H
