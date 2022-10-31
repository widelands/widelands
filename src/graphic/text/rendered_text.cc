/*
 * Copyright (C) 2017-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "graphic/text/rendered_text.h"

#include <cassert>
#include <memory>

#include "graphic/graphic.h"
#include "graphic/hyperlink.h"

namespace UI {
// RenderedRect
RenderedRect::RenderedRect(const Recti& init_rect,
                           std::shared_ptr<const Image> init_image,
                           bool visited,
                           const RGBColor& color,
                           bool is_background_color_set,
                           DrawMode init_mode,
                           const TextClickTarget* click_target)
   : rect_(init_rect),
     transient_image_(std::move(init_image)),
     permanent_image_(nullptr),
     visited_(visited),
     background_color_(color),
     is_background_color_set_(is_background_color_set),
     mode_(init_mode),
     click_target_(click_target) {
}
RenderedRect::RenderedRect(const Recti& init_rect,
                           const Image* init_image,
                           bool visited,
                           const RGBColor& color,
                           bool is_background_color_set,
                           DrawMode init_mode,
                           const TextClickTarget* click_target)
   : rect_(init_rect),
     transient_image_(nullptr),
     permanent_image_(init_image),
     visited_(visited),
     background_color_(color),
     is_background_color_set_(is_background_color_set),
     mode_(init_mode),
     click_target_(click_target) {
}

RenderedRect::RenderedRect(const Recti& init_rect,
                           const Image* init_image,
                           const TextClickTarget* click_target)
   : RenderedRect(
        init_rect, init_image, false, RGBColor(0, 0, 0), false, DrawMode::kTile, click_target) {
}
RenderedRect::RenderedRect(const Recti& init_rect,
                           const RGBColor& color,
                           const TextClickTarget* click_target)
   : RenderedRect(init_rect, nullptr, false, color, true, DrawMode::kTile, click_target) {
}
RenderedRect::RenderedRect(const std::shared_ptr<const Image>& init_image,
                           const TextClickTarget* click_target)
   : RenderedRect(Recti(0, 0, init_image->width(), init_image->height()),
                  init_image,
                  false,
                  RGBColor(0, 0, 0),
                  false,
                  DrawMode::kBlit,
                  click_target) {
}
RenderedRect::RenderedRect(const Image* init_image, const TextClickTarget* click_target)
   : RenderedRect(Recti(0, 0, init_image->width(), init_image->height()),
                  init_image,
                  false,
                  RGBColor(0, 0, 0),
                  false,
                  DrawMode::kBlit,
                  click_target) {
}

const Image* RenderedRect::image() const {
	assert(permanent_image_ == nullptr || transient_image_ == nullptr);
	return permanent_image_ == nullptr ? transient_image_.get() : permanent_image_;
}

const Recti& RenderedRect::rect() const {
	return rect_;
}

int RenderedRect::x() const {
	return rect_.x;
}

int RenderedRect::y() const {
	return rect_.y;
}

int RenderedRect::width() const {
	return rect_.w;
}
int RenderedRect::height() const {
	return rect_.h;
}

bool RenderedRect::handle_mousepress(int32_t x, int32_t y) const {
	return click_target_->handle_mousepress(x - rect_.x, y - rect_.y);
}
const std::string* RenderedRect::get_tooltip(int32_t x, int32_t y) const {
	return click_target_->get_tooltip(x - rect_.x, y - rect_.y);
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
RenderedText::RenderedText() : memory_tree_root_(nullptr) {
}
RenderedText::~RenderedText() {
	delete memory_tree_root_;
}

void RenderedText::set_memory_tree_root(TextClickTarget* t) {
	assert(t != nullptr);
	assert(memory_tree_root_ == nullptr);
	memory_tree_root_ = t;
}

int RenderedText::width() const {
	int result = 0;
	for (const auto& rect : rects) {
		result = std::max(result, rect->x() + rect->width());
	}
	return result;
}
int RenderedText::height() const {
	int result = 0;
	for (const auto& rect : rects) {
		result = std::max(result, rect->y() + rect->height());
	}
	return result;
}

bool RenderedText::handle_mousepress(int32_t x, int32_t y) const {
	for (const auto& r : rects) {
		if (r->rect().contains(Vector2i(x, y)) && r->handle_mousepress(x, y)) {
			return true;
		}
	}
	return false;
}
const std::string* RenderedText::get_tooltip(int32_t x, int32_t y) const {
	for (const auto& r : rects) {
		if (r->rect().contains(Vector2i(x, y))) {
			const std::string* tt = r->get_tooltip(x, y);
			if (tt != nullptr && !tt->empty()) {
				return tt;
			}
		}
	}
	return nullptr;
}

void RenderedText::draw(RenderTarget& dst,
                        const Vector2i& position,
                        const Recti& region,
                        Align align,
                        CropMode cropmode) const {

	// Un-const the position and adjust for alignment according to region width
	Vector2i aligned_pos(position.x, position.y);

	// For cropping images that don't fit
	int offset_x = 0;
	if (cropmode == CropMode::kSelf) {
		UI::correct_for_align(align, width(), &aligned_pos);
		if (align != UI::Align::kLeft) {
			for (const auto& rect : rects) {
				offset_x = std::min(region.w - rect->width(), offset_x);
			}
			if (align == UI::Align::kCenter) {
				offset_x /= 2;
			}
		}
	} else {
		aligned_pos.x -= region.x;
		aligned_pos.y -= region.y;
		UI::correct_for_align(align, region.w, &aligned_pos);
	}

	// Blit the rects
	for (const auto& rect : rects) {
		blit_rect(dst, offset_x, aligned_pos, *rect, region, align, cropmode);
	}
}

void RenderedText::blit_rect(RenderTarget& dst,
                             int offset_x,
                             const Vector2i& aligned_position,
                             const RenderedRect& rect,
                             const Recti& region,
                             Align align,
                             CropMode cropmode) const {
	const Vector2i blit_point(aligned_position.x + rect.x(), aligned_position.y + rect.y());

	// Draw Solid background Color
	if (rect.has_background_color()) {
		const int maximum_size = g_gr->max_texture_size_for_font_rendering();
		const int tile_width = std::min(maximum_size, rect.width());
		const int tile_height = std::min(maximum_size, rect.height());
		for (int tile_x = blit_point.x; tile_x + tile_width <= blit_point.x + rect.width();
		     tile_x += tile_width) {
			for (int tile_y = blit_point.y; tile_y + tile_height <= blit_point.y + rect.height();
			     tile_y += tile_height) {
				dst.fill_rect(Recti(tile_x, tile_y, tile_width, tile_height), rect.background_color());
			}
		}
	}

	if (rect.image() != nullptr) {
		switch (rect.mode()) {
		// Draw a foreground texture
		case RenderedRect::DrawMode::kBlit: {
			switch (cropmode) {
			case CropMode::kRenderTarget:
				// dst will handle any cropping
				dst.blit(blit_point, rect.image());
				break;
			case CropMode::kSelf:
				blit_cropped(dst, offset_x, aligned_position, blit_point, rect, region, align);
			}
		} break;
		// Draw a background image (tiling)
		case RenderedRect::DrawMode::kTile:
			dst.tile(Recti(blit_point, rect.width(), rect.height()), rect.image(), Vector2i::zero());
			break;
		}
	}
}

void RenderedText::draw(RenderTarget& dst, const Vector2i& position, UI::Align align) const {
	draw(dst, position, Recti(0, 0, width(), height()), align);
}

// Crop horizontally if it doesn't fit
void RenderedText::blit_cropped(RenderTarget& dst,
                                int offset_x,
                                const Vector2i& position,
                                const Vector2i& blit_point,
                                const RenderedRect& rect,
                                const Recti& region,
                                Align align) const {

	int blit_width = rect.width();
	int cropped_left = 0;
	if (align != UI::Align::kLeft) {
		if (rect.x() + rect.width() + offset_x <= region.x) {
			// Falls off the left-hand side
			return;
		}
		if (rect.x() + offset_x < 0) {
			// Needs cropping
			blit_width = rect.width() + offset_x + rect.x() - region.x;
			cropped_left = rect.width() - blit_width;
		}
	}

	if (align != UI::Align::kRight) {
		if (rect.x() + rect.width() - offset_x > region.w - region.x) {
			blit_width = region.w - rect.x() - offset_x;
		}
	}

	// Don't blit tiny or negative width
	if (blit_width < 3) {
		return;
	}

	dst.blitrect(
	   Vector2i(cropped_left > 0 ?
                  position.x + region.x - (align == UI::Align::kRight ? region.w : region.w / 2) :
                  blit_point.x,
	            blit_point.y),
	   rect.image(), Recti(cropped_left > 0 ? cropped_left : 0, region.y, blit_width, region.h));
}

}  // namespace UI
