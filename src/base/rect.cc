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
#include <set>
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
		return r->y;
	}

	inline float bottom() const {
		return r->y + r->h;
	}

	// Sorts by the lower edge of the contained rectangle.
	inline bool operator<(const RectWrapper& other) const {
		return top() < other.top();
	}

	size_t original_index;
	const FloatRect* r;
};

using ResultType = std::set<std::pair<size_t, size_t>>;

void report_pair(const size_t a, const size_t b, ResultType* result) {
	result->insert(std::make_pair(std::min(a, b), std::max(a, b)));
}

void stab(const std::vector<RectWrapper>& H,
          const std::vector<size_t>& A,
          const std::vector<size_t>& B,
          ResultType* result) {
	size_t i = 0;
	size_t j = 0;
	while (i < A.size() && j < B.size()) {
		if (H[A[i]].top() < H[B[j]].top()) {
			size_t k = j;
			while (k < B.size() && H[B[k]].top() < H[A[i]].bottom()) {
				report_pair(H[A[i]].original_index, H[B[k]].original_index, result);
				++k;
			}
			++i;
		} else {
			size_t k = i;
			while (k < A.size() && H[A[k]].top() < H[B[j]].bottom()) {
				report_pair(H[B[j]].original_index, H[A[k]].original_index, result);
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
            ResultType* result) {
	if (high_index - low_index < 2) {
		return;
	}

	std::vector<int> count1(H.size());
	std::vector<int> count2(H.size());


	const float min_value = V[low_index].value;
	const float max_value = V[high_index - 1].value;

	const size_t mid_index = (high_index + low_index) / 2;
	for (size_t i = low_index; i < mid_index; ++i) {
		++count1[V[i].corresponding_rect_in_H];
	}
	for (size_t i = mid_index; i < high_index; ++i) {
		++count2[V[i].corresponding_rect_in_H];
	}

	std::vector<size_t> S11, S22, S12, S21;
	for (size_t i = 0; i < H.size(); ++i) {
		if (count1[i] == 0 && count2[i] == 0) {
			continue;
		}

		if (count1[i] == 2 && count2[i] == 0) {
			S11.push_back(i);
		} else if (count1[i] == 0 && count2[i] == 2) {
			S22.push_back(i);
		} else if (count1[i] == 1 && count2[i] == 0) {
			// If the right edge is over the highest value, we span V2, otherwise
			// we don't.
			if (H[i].right() > max_value) {
				S12.push_back(i);
			} else {
				S11.push_back(i);
			}
		} else if (count1[i] == 0 && count2[i] == 1) {
			// If the left edge is lower than the lowest value, we do span V1
			// otherwise we don't.
			if (H[i].left() < min_value) {
				S21.push_back(i);
			} else {
				S22.push_back(i);
			}
		}
	}

	stab(H, S12, S22, result);
	stab(H, S21, S11, result);
	stab(H, S12, S21, result);

	detect(V, low_index, (low_index + high_index) / 2, H, result);
	detect(V, (low_index + high_index) / 2, high_index, H, result);
}

}  // namespace

std::vector<std::vector<int>> report(const std::vector<FloatRect>& rectangles) {
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

	ResultType result;
	detect(V, 0, V.size(), H, &result);

	std::vector<std::vector<int>> rv;
	rv.resize(rectangles.size());

	for (const auto& p : result) {
		rv[p.first].push_back(p.second);
	}
	return rv;
}
