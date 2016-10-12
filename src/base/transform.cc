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

#include "base/transform.h"

Transform2f::Transform2f(const FloatPoint& translation, const float zoom)
   : translation_(translation), zoom_(zoom) {
}

Transform2f Transform2f::identity() {
	return Transform2f(FloatPoint(0.f, 0.f), 1.f);
}

Transform2f Transform2f::from_zoom(const float zoom) {
	return Transform2f(FloatPoint(0.f, 0.f), zoom);
}

Transform2f Transform2f::from_translation(const FloatPoint& translation) {
	return Transform2f(translation, 1.f);
}

Transform2f Transform2f::chain(const Transform2f& other) const {
	return Transform2f(other.translation_ * zoom_ + translation_, zoom_ * other.zoom_);
}

FloatPoint Transform2f::apply(const FloatPoint& v) const {
	return FloatPoint(v.x * zoom_ + translation_.x, v.y * zoom_ + translation_.y);
}

Transform2f Transform2f::inverse() const {
	return Transform2f(-translation_ / zoom_, 1 / zoom_);
}

