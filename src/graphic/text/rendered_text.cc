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

#include "graphic/text/rendered_text.h"

#include <memory>

#include "graphic/graphic.h"
#include "graphic/text_layout.h"

namespace UI {
// RenderedRect
RenderedRect::RenderedRect(const Recti& init_rect,
                           const Image* init_image,
                           bool visited,
                           const RGBColor& color,
                           bool is_background_color_set,
                           DrawMode init_mode)
   : rect_(init_rect),
     image_(init_image),
     visited_(visited),
     background_color_(color),
     is_background_color_set_(is_background_color_set),
     mode_(init_mode) {
}

RenderedRect::RenderedRect(const Recti& init_rect, const Image* init_image)
   : RenderedRect(init_rect, init_image, false, RGBColor(0, 0, 0), false, DrawMode::kTile) {
}
RenderedRect::RenderedRect(const Recti& init_rect, const RGBColor& color)
   : RenderedRect(init_rect, nullptr, false, color, true, DrawMode::kTile) {
}
RenderedRect::RenderedRect(const Image* init_image)
   : RenderedRect(Recti(0, 0, init_image->width(), init_image->height()),
                  init_image,
                  false,
                  RGBColor(0, 0, 0),
                  false,
                  DrawMode::kBlit) {
}

const Image* RenderedRect::image() const {
	return image_;
}

int RenderedRect::get_x() const {
	return rect_.x;
}

int RenderedRect::get_y() const {
	return rect_.y;
}

int RenderedRect::width() const {
	return rect_.w;
}
int RenderedRect::height() const {
	return rect_.h;
}

void RenderedRect::set_origin(const Vector2i& new_origin) {
	rect_.x = new_origin.x;
	rect_.y = new_origin.y;
}
void RenderedRect::set_visited() {
	visited_ = true;
}
bool RenderedRect::was_visited() const {
	return visited_;
}

bool RenderedRect::has_background_color() const {
	return is_background_color_set_;
}
const RGBColor& RenderedRect::background_color() const {
	return background_color_;
}

RenderedRect::DrawMode RenderedRect::mode() const {
	return mode_;
}

// RenderedText
int RenderedText::width() const {
	int result = 0;
	for (const auto& rect : rects) {
		result = std::max(result, rect->get_x() + rect->width());
	}
	return result;
}
int RenderedText::height() const {
	int result = 0;
	for (const auto& rect : rects) {
		result = std::max(result, rect->get_y() + rect->height());
	}
	return result;
}

void RenderedText::draw(RenderTarget& dst,
                        const Vector2i& position,
                        const Recti& region,
                        Align align) const {

	// Un-const the position and adjust for the region's origin point
	Vector2i aligned_pos(position.x - region.x, position.y - region.y);
	UI::correct_for_align(align, region.w, &aligned_pos);
	for (const auto& rect : rects) {
		Vector2i blit_point(aligned_pos.x + rect->get_x(), aligned_pos.y + rect->get_y());

		// Draw Solid background Color
		if (rect->has_background_color()) {
#ifndef NDEBUG
			const int maximum_size = kMinimumSizeForTextures;
#else
			const int maximum_size = g_gr->max_texture_size();
#endif
			// NOCOM Replace 'test' with 'maximum_size' when testing is done.
			const int test = 4;
			const int tile_width = std::min(test, rect->width());
			const int tile_height = std::min(test, rect->height());
			for (int tile_x = blit_point.x; tile_x + tile_width <= blit_point.x + rect->width();
			     tile_x += tile_width) {
				for (int tile_y = blit_point.y; tile_y + tile_height <= blit_point.y + rect->height();
				     tile_y += tile_height) {
					dst.fill_rect(
					   Rectf(tile_x, tile_y, tile_width, tile_height), rect->background_color());
				}
			}
		}

		if (rect->image()) {
			switch (rect->mode()) {
			// Draw a foreground texture
			case RenderedRect::DrawMode::kBlit:
				dst.blit(blit_point, rect->image());
				break;
			// Draw a background image (tiling)
			case RenderedRect::DrawMode::kTile:
				dst.tile(
				   Recti(blit_point, rect->width(), rect->height()), rect->image(), Vector2i(0, 0));
				break;
			}
		}
		// TODO(GunChleoc): Remove this line when testing is done.
		// dst.draw_rect(Rectf(blit_point.x, blit_point.y, rect->width(), rect->height()),
		// RGBColor(100, 100, 100));
	}
}

void RenderedText::draw(RenderTarget& dst, const Vector2i& position, UI::Align align) const {
	draw(dst, position, Recti(0, 0, width(), height()), align);
}

// For testing purposes only. Needs to mirror the draw function.
std::unique_ptr<Texture> RenderedText::as_texture() const {
	std::unique_ptr<Texture> texture(new Texture(width(), height()));
	for (const auto& rect : rects) {
		const Rectf dest(rect->get_x(), rect->get_y(), rect->width(), rect->height());

		// Draw Solid background Color
		if (rect->has_background_color()) {
			texture->fill_rect(dest, rect->background_color());
		}

		if (rect->image()) {
			switch (rect->mode()) {
			// Draw a foreground texture
			case RenderedRect::DrawMode::kBlit:
				texture->blit(dest, *rect->image(), dest, 1., BlendMode::Copy);
				break;
			// Draw a background image (tiling)
			// TODO(GunChleoc): Support tiling here
			case RenderedRect::DrawMode::kTile:
				break;
			}
		}
	}
	return texture;
}

}  // namespace UI
