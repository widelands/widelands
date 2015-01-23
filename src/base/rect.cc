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

#include "base/rect.h"

#include <algorithm>
#include <vector>

#include "base/log.h"

// This implements the Paper:
// An Optimal Rectangle-Intersection Algorithm Using Linear Arrays Only by Frank Devai1, Laszlo Neumann.
//
// I mirrored it here: http://www.widelands.org/~sirver/wl/141229_devail_rects.pdf.
//
// The code uses the nomenclature used in the Paper up to some differences. We
// did not use intervals, but instead use the rectangles directly.

namespace  {

// A left or right edge of one of the rectangles. Sort in ascending x order.
struct VerticalEdge {
	VerticalEdge(const int init_corresponding_rect_in_H, const float init_value)
	   : corresponding_rect_in_H(init_corresponding_rect_in_H), value(init_value) {
	}

	inline bool operator<(const VerticalEdge& other) const {
		return value < other.value;
	}

	int corresponding_rect_in_H;
	float value;
};

struct RectWrapper {
	RectWrapper(const size_t init_original_index, const FloatRect* init_r)
	   : original_index(init_original_index), r(init_r) {
	}

	inline float left() const {
		return r->x;
	}

	inline float right() const {
		return r->x + r->w;
	}

	inline float top() const {
		return r->y + r->h;
	}

	inline float bottom() const {
		return r->y;
	}

	// Sorts by the lower edge of the contained rectangle.
	inline bool operator<(const RectWrapper& other) const {
		return top() < other.top();
	}

	size_t original_index;
	const FloatRect* r;
};

inline void report_pair(size_t a, size_t b, OverlappingRects* result) {
	if (a > b) {
		std::swap(a, b);
	}
	(*result)[a].push_back(b);
}

void stab(const std::vector<RectWrapper>& H,
          const std::vector<size_t>& A,
          const std::vector<size_t>& B,
          OverlappingRects* result) {
	auto i = A.begin();
	auto j = B.begin();
	while (i != A.end() && j != B.end()) {
		if (H[*i].top() < H[*j].top()) {
			auto k = j;
			while (k != B.end() && H[*k].top() < H[*i].bottom()) {
				report_pair(H[*i].original_index, H[*k].original_index, result);
				++k;
			}
			++i;
		} else {
			auto k = i;
			while (k != A.end() && H[*k].top() < H[*j].bottom()) {
				report_pair(H[*j].original_index, H[*k].original_index, result);
				++k;
			}
			++j;
		}
	}
}

void detect(const std::vector<VerticalEdge>& V,
            size_t low_index,
            size_t high_index,
            const std::vector<RectWrapper>& H,
            OverlappingRects* result) {
	if (high_index - low_index < 2) {
		return;
	}

	const float min_value = V[low_index].value;
	const float max_value = V[high_index - 1].value;

	const size_t mid_index = (high_index + low_index) / 2;

	std::vector<size_t> S11, S22, S12, S21;
	for (size_t i = low_index; i < mid_index; ++i) {
		const int k = V[i].corresponding_rect_in_H;
		// If the right edge is over the highest value, we span V2, otherwise
		// we don't.
		if (H[k].right() > max_value) {
			S12.push_back(k);
		} else {
			S11.push_back(k);
		}
	}

	for (size_t i = mid_index; i < high_index; ++i) {
		const int k = V[i].corresponding_rect_in_H;
		// If the left edge is lower than the lowest value, we do span V1
		// otherwise we don't.
		if (H[k].left() < min_value) {
			S21.push_back(k);
		} else {
			S22.push_back(k);
		}
	}

	std::sort(S11.begin(), S11.end());
	S11.erase(std::unique(S11.begin(), S11.end()), S11.end());
	std::sort(S12.begin(), S12.end());
	S12.erase(std::unique(S12.begin(), S12.end()), S12.end());
	std::sort(S22.begin(), S22.end());
	S22.erase(std::unique(S22.begin(), S22.end()), S22.end());
	std::sort(S21.begin(), S21.end());
	S21.erase(std::unique(S21.begin(), S21.end()), S21.end());

	stab(H, S12, S22, result);
	stab(H, S21, S11, result);
	stab(H, S12, S21, result);

	detect(V, low_index, mid_index, H, result);
	detect(V, mid_index, high_index, H, result);
}

}  // namespace

OverlappingRects find_overlapping_rectangles(const std::vector<FloatRect>& rectangles) {
	std::vector<RectWrapper> H;
	H.reserve(rectangles.size());
	for (size_t i = 0; i < rectangles.size(); ++i) {
		H.emplace_back(i, &rectangles[i]);
	}
	std::sort(H.begin(), H.end());

	std::vector<VerticalEdge> V;
	for (size_t i = 0; i < H.size(); ++i) {
		const FloatRect& rect = *H[i].r;
		V.emplace_back(i, rect.x);
		V.emplace_back(i, rect.x + rect.w);
	}
	std::sort(V.begin(), V.end());

	OverlappingRects rv;
	rv.resize(rectangles.size());
	detect(V, 0, V.size(), H, &rv);

	for (auto& vec : rv) {
		log("#sirver vec.size(): %d\n", vec.size());
		std::sort(vec.begin(), vec.end());
		vec.erase(std::unique(vec.begin(), vec.end()), vec.end());
	}
	return rv;
}
