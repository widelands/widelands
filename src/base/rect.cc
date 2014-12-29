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

#include <algorithm>
#include <cassert>
#include <set>
#include <vector>

#include "base/log.h"
#include "base/rect.h"

// NOCOM(#sirver): figure this out
// NOCOM(#sirver): document that I used their nomeclature.
// An Optimal Rectangle-Intersection Algorithm Using Linear Arrays Only by Frank Devai1, Laszlo Neumann.

namespace  {

struct Edge {
	Edge(const int init_rectangle_index, const float init_value)
	   : rectangle_index(init_rectangle_index), value(init_value) {
	}

	inline bool operator<(const Edge& other) const {
		return value < other.value;
	}

	int rectangle_index;
	float value;
};

struct RectWrapper {
	RectWrapper(const size_t init_original_index, const FloatRect* init_r)
	   : original_index(init_original_index), r(init_r) {
	}

	inline float lower_x() const {
		return r->x;
	}

	inline float higher_x() const {
		return r->x + r->w;
	}

	inline float lower_y() const {
		return r->y;
	}

	inline float higher_y() const {
		return r->y + r->h;
	}

	// Sorts by the lower edge of the contained rectangle.
	inline bool operator<(const RectWrapper& other) const {
		return higher_y() < other.higher_y();
	}

	size_t original_index;
	const FloatRect* r;
};

static std::set<std::pair<int, int>> pairs;

void report_pair(const int a, const int b) {
	log("#sirver a: %d,b: %d\n", a, b);
	pairs.insert(std::make_pair(std::min(a, b), std::max(a, b)));
}

void stab(const std::vector<RectWrapper>& H, const std::vector<int>& A, const std::vector<int>& B) {
	size_t i = 0;
	size_t j = 0;
	while (i < A.size() && j < B.size()) {
		if (H[A[i]].lower_y() < H[B[j]].lower_y()) {
			size_t k = j;
			while (k < B.size() && H[B[k]].lower_y() < H[A[i]].higher_y()) {
				report_pair(H[A[i]].original_index, H[B[k]].original_index);
				++k;
			}
			++i;
		} else {
			size_t k = i;
			while (k < A.size() && H[A[k]].lower_y() < H[B[j]].higher_y()) {
				report_pair(H[B[j]].original_index, H[A[k]].original_index);
				++k;
			}
			++j;
		}
	}
}

void detect(const std::vector<Edge>& V, int lower_index, int higher_index, const std::vector<RectWrapper>& H, const std::string indent) {
	log("#sirver higher_index: %d,lower_index: %d\n", higher_index, lower_index);
	if (higher_index - lower_index < 2) {
		return;
	}

	std::vector<int> count1(H.size());
	std::vector<int> count2(H.size());

	std::vector<RectWrapper> H1, H2;
	// NOCOM(#sirver): index into H
	std::vector<int> S11, S22, S12, S21;

	const float min_value = V[lower_index].value;
	const float max_value = V[higher_index - 1].value;

	for (int i = lower_index; i < (lower_index + higher_index) / 2; ++i) {
		++count1[V[i].rectangle_index];
	}
	for (int i = (lower_index + higher_index) / 2; i < higher_index; ++i) {
		++count2[V[i].rectangle_index];
	}

	for (size_t i = 0; i < H.size(); ++i) {
		if (count1[i] == 0 && count2[i] == 0) {
			continue;
		}
		assert(count1[i] + countl2[i] <= 2);

		if (count1[i] == 2 && count2[i] == 0) {
			S11.push_back(i);
		} else if (count1[i] == 0 && count2[i] == 2) {
			S22.push_back(i);
		} else if (count1[i] == 1 && count2[i] == 1) {
				// S12.push_back(i);
				// S21.push_back(i);
			// NOCOM(#sirver): what here?
		} else if (count1[i] == 1 && count2[i] == 0) {
			// If the right edge is over the highest value, we span V2.
			if (H[i].higher_x() >= max_value) {
				S12.push_back(i);
			} else {
				S11.push_back(i);
			}
		} else if (count1[i] == 0 && count2[i] == 1) {
			// If the left edge is lower than the lowest value, we do span V1.
			if (H[i].lower_x() <= min_value) {
				S21.push_back(i);
			} else {
				S22.push_back(i);
			}
		} else {
			assert(false);
		}
	}
	log("#sirver S11: ");
	for (const auto& i : S11) { log("%d ", i); }
	log("\n");
	log("#sirver S12: ");
	for (const auto& i : S12) { log("%d ", i); }
	log("\n");
	log("#sirver S22: ");
	for (const auto& i : S22) { log("%d ", i); }
	log("\n");
	log("#sirver S21: ");
	for (const auto& i : S21) { log("%d ", i); }
	log("\n");

	stab(H, S12, S22);
	stab(H, S21, S11);
	stab(H, S12, S21);

	detect(V, lower_index, (lower_index + higher_index) / 2, H, indent + " ");
	detect(V, (lower_index + higher_index) / 2, higher_index, H, indent + " ");
}

}  // namespace

std::vector<std::vector<int>> report(const std::vector<FloatRect>& H) {
	std::vector<RectWrapper> rects;
	rects.reserve(H.size());

	for (size_t i = 0; i < H.size(); ++i) {
		rects.emplace_back(i, &H[i]);
	}

	std::sort(rects.begin(), rects.end());

	for (int i = 0; i < rects.size(); ++i) {
		log("#sirver %f %f\n", rects[i].r->x, rects[i].r->y);
	}

	std::vector<Edge> V;
	for (size_t i = 0; i < rects.size(); ++i) {
		const FloatRect& rect = *rects[i].r;
		V.emplace_back(i, rect.x);
		V.emplace_back(i, rect.x + rect.w);
	}

	std::sort(V.begin(), V.end());

	detect(V, 0, V.size(), rects, "");

	std::vector<std::vector<int>> rv;
	rv.resize(H.size());

	for (const auto& p : pairs) {
		log("#sirver intersection: %d %d\n", p.first, p.second);
		rv[p.first].push_back(p.second);
	}
	return rv;
}
