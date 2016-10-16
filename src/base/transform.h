/*
 * Copyright (C) 2006-2016 by the Widelands Development Team
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

#ifndef WL_BASE_TRANSFORM_H
#define WL_BASE_TRANSFORM_H

#include "base/macros.h"
#include "base/vector.h"

// NOCOM(#sirver): rename Vector2i to Vector2 (or just use use Vector and dump one of them?).
class Transform2f {
public:
	Transform2f(const Vector2f& translation, const float zoom);

	static Transform2f identity();
	static Transform2f from_zoom(float zoom);
	static Transform2f from_translation(const Vector2f& translation);

	const Vector2f& translation() const { return translation_; }
	float zoom() const { return zoom_; }

	// Transform this point by this transform.
	Vector2f apply(const Vector2f& v) const;

	// Returns B, so that B.apply(p) == this.apply(A.apply(p)).
	Transform2f chain(const Transform2f& A) const;

	// Returns the inverse of this transform.
	Transform2f inverse() const;

private:
	Vector2f translation_;
	float zoom_;
};

#endif  // end of include guard: WL_BASE_TRANSFORM_H
