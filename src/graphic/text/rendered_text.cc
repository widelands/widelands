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

#include "graphic/text_layout.h"
#include "graphic/text/rendered_text.h"

namespace UI {


RenderedRect::RenderedRect(Vector2i init_point, const Image* init_image)
	: point_(init_point), image_(init_image) {
}

bool RenderedRect::operator==(const RenderedRect& other) const {
	return point_ == Vector2i(other.get_x(), other.get_y());
}
const Image* RenderedRect::image() const {
	return image_.get();
}

int RenderedRect::get_x() const {
	return point_.x;
}

int RenderedRect::get_y() const {
	return point_.y;
}

int RenderedRect::width() const {
	return image_->width();
}
int RenderedRect::height() const {
	return image_->height();
}


int RenderedText::width() const {
	int result = 0;
	for (const auto& rect : texts) {
		result = std::max(result, rect->get_x() + rect->width());
	}
	return result;
}
int RenderedText::height() const {
	int result = 0;
	for (const auto& rect : texts) {
		result = std::max(result, rect->get_y() + rect->height());
	}
	return result;
}

void RenderedText::draw(RenderTarget& dst,
					const Vector2i& position,
					Recti region, Align align) const {

	Vector2i aligned_pos(position.x, position.y); // un-const the position
	UI::correct_for_align(align, region.w, &aligned_pos);
	for (const auto& rect : texts) {
		// NOCOM implement when we have actual data
		//bool contains_origin = region.contains(rect->point);
		//bool contains_opposite = region.contains(Vector2i(rect->point.x + rect->image->width(), rect->image->height()));
		//if (contains_origin && contains_opposite) {
		Vector2f blit_point(aligned_pos.x + rect->get_x(),  aligned_pos.y + rect->get_y());
		dst.blitrect(blit_point, rect->image(), region);
		//}
	}

}

void RenderedText::draw(RenderTarget& dst, const Vector2i& position, UI::Align align) const {
	draw(dst, position, Recti(0, 0, width(), height()), align);
}

}  // namespace UI
