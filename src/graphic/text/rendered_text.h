/*
 * Copyright (C) 2017 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_GRAPHIC_TEXT_RENDERED_TEXT_H
#define WL_GRAPHIC_TEXT_RENDERED_TEXT_H

#include <memory>
#include <vector>

#include "base/vector.h"
#include "graphic/image.h"

namespace UI {

struct RenderedRect {
	/*
	RenderedRect()
		: RenderedRect(Vector2i(0, 0), nullptr) {
	}
	*/
	RenderedRect(Vector2i init_point, const Image* init_image);
	~RenderedRect() {}

	bool operator==(const RenderedRect& other) const;

	const Image* image() const;

	int get_x() const;
	int get_y() const;
	int width() const;
	int height() const;

private:
	const Vector2i point_;
	std::unique_ptr<const Image> image_;
};

struct RenderedText {
	std::vector<std::unique_ptr<RenderedRect>> texts;
	// Dimensions occupied by the rendered images in pixels.
	int width() const;
	int height() const;
};

} // namespace UI

#endif  // end of include guard: WL_GRAPHIC_TEXT_RENDERED_TEXT_H
