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
	Edge(const int init_rectangle_index, const float init_value, const FloatRect& init_rect)
	   : rectangle_index(init_rectangle_index), value(init_value), rect(&init_rect) {
	}

	inline bool operator<(const Edge& other) const {
		return value < other.value;
	}

	int rectangle_index;
	float value;
	const FloatRect* rect;
};

inline int low(const FloatRect& r) {
	return r.y;
}

inline int high(const FloatRect& r) {
	return r.y + r.h;
}

struct SortByLow {
	inline bool operator()(const FloatRect& r1, const FloatRect& r2) {
		return low(r1) < low(r2);
	}
};

static std::set<std::pair<int, int>> pairs;

void report_pair(const int a, const int b) {
	pairs.insert(std::make_pair(std::min(a, b), std::max(a, b)));
}

void stab(const std::vector<FloatRect>& H, const std::vector<int>& A, const std::vector<int>& B) {
	size_t i = 0;
	size_t j = 0;
	while (i < A.size() && j < B.size()) {
		if (low(H[A[i]]) < low(H[B[j]])) {
			size_t k = j;
			while (k < B.size() && low(H[B[k]]) < high(H[A[i]])) {
				report_pair(A[i], B[k]);
				++k;
			}
			++i;
		} else {
			size_t k = i;
			while (k < A.size() && low(H[A[k]]) < high(H[B[j]])) {
				report_pair(B[j], A[k]);
				++k;
			}
			++j;
		}
	}
}

void detect(const std::vector<Edge>& V, int low, int high, const std::vector<FloatRect>& H, const std::string indent) {
	log("#sirver %slow: %d,high: %d\n", indent.c_str(), low, high);
	if (high - low <= 1) {
		return;
	}

	std::vector<int> count1(H.size());
	std::vector<int> count2(H.size());

	// NOCOM(#sirver): what should H1 and H2 contain?
	std::vector<FloatRect> H1, H2;
	for (int i = low; i < (low + high) / 2; ++i) {
		++count1[V[i].rectangle_index];
	}
	for (int i = (low + high) / 2; i < high; ++i) {
		++count2[V[i].rectangle_index];
	}

	// NOCOM(#sirver): index into H
	std::vector<int> S11, S22, S12, S21;
	for (size_t i = 0; i < H.size(); ++i) {
		if (count1[i] > 0) {
			H1.push_back(H[i]);
		}
		if (count2[i] > 0) {
			H2.push_back(H[i]);
		}

		if (count1[i] == 2 && count2[i] == 0) { // Only in V2, not spanning V2
			S11.push_back(i);
		} else if (count1[i] == 1 && count2[i] == 0) { // Only in V1, spanning V2.
			S12.push_back(i);
		} else if (count1[i] == 0 && count2[i] == 2) { // Only in V2, not spanning V1.
			S22.push_back(i);
		} else if (count1[i] == 0 && count2[i] == 1) { // Only in V2, spanning V1.
			S21.push_back(i);
		}
	}

	stab(H, S12, S22);
	stab(H, S21, S11);
	stab(H, S12, S21);

	detect(V, low, (low + high) / 2, H, indent + " ");
	detect(V, (low + high) / 2, high, H, indent + " ");
}

}  // namespace

void report(std::vector<FloatRect> H) {
	std::sort(H.begin(), H.end(), SortByLow());

	std::vector<Edge> V;
	for (size_t i = 0; i < H.size(); ++i) {
		const FloatRect& rect = H[i];
		V.emplace_back(i, rect.x, rect);
		V.emplace_back(i, rect.x + rect.w, rect);
	}

	std::sort(V.begin(), V.end());

	detect(V, 0, V.size(), H, "");

	for (const auto& p : pairs) {
		const FloatRect& r1 = H[p.first];
		const FloatRect& r2 = H[p.second];
		log("#sirver r1.x: %f,r1.y: %f, r2.x: %f, r2.y: %f\n", r1.x, r1.y, r2.x, r2.y);
	}
}
