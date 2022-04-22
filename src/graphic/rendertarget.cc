/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#include "graphic/rendertarget.h"

#include "graphic/align.h"
#include "graphic/animation/animation.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/surface.h"

/**
 * Build a render target for the given surface.
 */
RenderTarget::RenderTarget(Surface* surf) : surface_(surf) {
	reset();
}

/**
 * Sets an arbitrary drawing window.
 */
void RenderTarget::set_window(const Recti& rc, const Vector2i& ofs) {
	rect_ = rc;
	offset_ = ofs;

	// safeguards clipping against the bitmap itself

	if (rect_.x < 0) {
		offset_.x += rect_.x;
		rect_.w = std::max<int32_t>(rect_.w + rect_.x, 0);
		rect_.x = 0;
	}

	if (rect_.x + rect_.w > surface_->width()) {
		rect_.w = std::max<int32_t>(surface_->width() - rect_.x, 0);
	}

	if (rect_.y < 0) {
		offset_.y += rect_.y;
		rect_.h = std::max<int32_t>(rect_.h + rect_.y, 0);
		rect_.y = 0;
	}

	if (rect_.y + rect_.h > surface_->height()) {
		rect_.h = std::max<int32_t>(surface_->height() - rect_.y, 0);
	}
}

/**
 * Builds a subwindow. rc is relative to the current drawing window. The
 * subwindow will be clipped appropriately.
 *
 * The previous window state is returned in previous and prevofs.
 *
 * Returns false if the subwindow is invisible. In that case, the window state
 * is not changed at all. Otherwise, the function returns true.
 */
bool RenderTarget::enter_window(const Recti& rc, Recti* previous, Vector2i* prevofs) {
	Rectf newrect_f = rc.cast<float>();
	if (clip(newrect_f)) {
		if (previous != nullptr) {
			*previous = rect_;
		}
		if (prevofs != nullptr) {
			*prevofs = offset_;
		}

		const Recti newrect = newrect_f.cast<int>();
		// Apply the changes
		offset_ = rc.origin() - (newrect.origin() - rect_.origin() - offset_);
		rect_ = newrect;

		return true;
	}
	return false;
}

/**
 * Returns the true size of the render target (ignoring the window settings).
 */
int32_t RenderTarget::width() const {
	return surface_->width();
}

/**
 * Returns the true size of the render target (ignoring the window settings).
 */
int32_t RenderTarget::height() const {
	return surface_->height();
}

/**
 * This functions draws a line in the target
 */
void RenderTarget::draw_line_strip(const std::vector<Vector2f>& points,
                                   const RGBColor& color,
                                   float line_width) {
	std::vector<Vector2f> adjusted_points;
	adjusted_points.reserve(points.size());
	for (const auto& p : points) {
		adjusted_points.emplace_back(p.x + offset_.x + rect_.x, p.y + offset_.y + rect_.y);
	}
	surface_->draw_line_strip(adjusted_points, color, line_width);
}

/**
 * Clip against window and pass those primitives along to the bitmap.
 */
void RenderTarget::draw_rect(const Recti& rect, const RGBColor& clr) {
	Rectf r(rect.cast<float>());
	if (clip(r)) {
		::draw_rect(r, clr, surface_);
	}
}

void RenderTarget::fill_rect(const Recti& rect, const RGBAColor& clr, BlendMode blend_mode) {
	Rectf r(rect.cast<float>());
	if (clip(r)) {
		surface_->fill_rect(r, clr, blend_mode);
	}
}

void RenderTarget::brighten_rect(const Recti& rect, int32_t factor) {
	Rectf r(rect.cast<float>());
	if (clip(r)) {
		surface_->brighten_rect(r, factor);
	}
}

/**
 * Blits a Image on another Surface
 *
 * This blit function copies the pixels to the destination surface.
 */
void RenderTarget::blit(const Vector2i& dst,
                        const Image* image,
                        BlendMode blend_mode,
                        UI::Align align) {
	assert(image != nullptr);
	Vector2i destination_point(dst);
	UI::correct_for_align(align, image->width(), &destination_point);

	Rectf source_rect(0.f, 0.f, image->width(), image->height());
	Rectf destination_rect(destination_point.x, destination_point.y, source_rect.w, source_rect.h);

	if (to_surface_geometry(&destination_rect, &source_rect)) {
		// I seem to remember seeing 1. a lot in blitting calls.
		constexpr float kFullyOpaque = 1.f;
		surface_->blit(destination_rect, *image, source_rect, kFullyOpaque, blend_mode);
	}
}

void RenderTarget::blit_monochrome(const Vector2i& dst,
                                   const Image* image,
                                   const RGBAColor& blend_mode,
                                   UI::Align align) {
	Vector2i destination_point(dst);
	UI::correct_for_align(align, image->width(), &destination_point);

	Rectf source_rect(0.f, 0.f, image->width(), image->height());
	Rectf destination_rect(destination_point.x, destination_point.y, source_rect.w, source_rect.h);

	if (to_surface_geometry(&destination_rect, &source_rect)) {
		surface_->blit_monochrome(destination_rect, *image, source_rect, blend_mode);
	}
}

/**
 * Like \ref blit, but use only a sub-rectangle of the source image.
 */
void RenderTarget::blitrect(const Vector2i& dst,
                            const Image* image,
                            const Recti& rectangle,
                            BlendMode blend_mode) {
	assert(0 <= rectangle.x);
	assert(0 <= rectangle.y);

	// We want to use the given srcrc, but we must make sure that we are not
	// blitting outside of the boundaries of 'image'.
	Rectf source_rect(rectangle.x, rectangle.y,
	                  std::min<int32_t>(image->width() - rectangle.x, rectangle.w),
	                  std::min<int32_t>(image->height() - rectangle.y, rectangle.h));
	Rectf destination_rect(dst.x, dst.y, source_rect.w, source_rect.h);

	if (to_surface_geometry(&destination_rect, &source_rect)) {
		surface_->blit(destination_rect, *image, source_rect, 1., blend_mode);
	}
}

void RenderTarget::blitrect_scale(Rectf destination_rect,
                                  const Image* image,
                                  Recti source_rect_i,
                                  const float opacity,
                                  const BlendMode blend_mode) {
	Rectf source_rect = source_rect_i.cast<float>();
	if (to_surface_geometry(&destination_rect, &source_rect)) {
		surface_->blit(destination_rect, *image, source_rect, opacity, blend_mode);
	}
}

void RenderTarget::blitrect_scale_monochrome(Rectf destination_rect,
                                             const Image* image,
                                             Recti source_rect_i,
                                             const RGBAColor& blend) {
	Rectf source_rect = source_rect_i.cast<float>();
	if (to_surface_geometry(&destination_rect, &source_rect)) {
		surface_->blit_monochrome(destination_rect, *image, source_rect, blend);
	}
}

/**
 * Fill the given rectangle with the given image.
 *
 * The pixel from ofs inside image is placed at the top-left corner of
 * the filled rectangle.
 */
void RenderTarget::tile(const Recti& rect,
                        const Image* image,
                        const Vector2i& gofs,
                        BlendMode blend_mode) {
	int32_t srcw = image->width();
	int32_t srch = image->height();

	Rectf r = rect.cast<float>();
	Vector2i ofs(gofs);
	if (clip(r)) {
		if (offset_.x < 0) {
			ofs.x -= offset_.x;
		}

		if (offset_.y < 0) {
			ofs.y -= offset_.y;
		}

		// Make sure the offset is within bounds
		ofs.x = ofs.x % srcw;

		if (ofs.x < 0) {
			ofs.x += srcw;
		}

		ofs.y = ofs.y % srch;

		if (ofs.y < 0) {
			ofs.y += srch;
		}

		// Blit the image into the rectangle
		int ty = 0;

		while (ty < r.h) {
			int tx = 0;
			int32_t tofsx = ofs.x;
			Rectf srcrc;

			srcrc.y = ofs.y;
			srcrc.h = srch - ofs.y;

			if (ty + srcrc.h > r.h) {
				srcrc.h = r.h - ty;
			}

			while (tx < r.w) {
				srcrc.x = tofsx;
				srcrc.w = srcw - tofsx;

				if (tx + srcrc.w > r.w) {
					srcrc.w = r.w - tx;
				}

				const Rectf dst_rect(r.x + tx, r.y + ty, srcrc.w, srcrc.h);
				surface_->blit(dst_rect, *image, srcrc, 1., blend_mode);

				tx += srcrc.w;

				tofsx = 0;
			}

			ty += srcrc.h;
			ofs.y = 0;
		}
	}
}

void RenderTarget::blit_animation(const Vector2f& dst,
                                  const Widelands::Coords& coords,
                                  const float scale,
                                  uint32_t animation_id,
                                  const Time& time,
                                  const RGBColor* player_color,
                                  const float opacity,
                                  const int percent_from_bottom) {
	const Animation& animation = g_animation_manager->get_animation(animation_id);
	assert(percent_from_bottom <= 100);
	if (percent_from_bottom > 0) {
		// Scaling for zoom and animation image size, then fit screen edges.
		Rectf srcrc = animation.source_rectangle(percent_from_bottom, scale);
		Rectf dstrc = animation.destination_rectangle(dst, srcrc, scale);
		if (to_surface_geometry(&dstrc, &srcrc)) {
			animation.blit(time.get(), coords, srcrc, dstrc, player_color, surface_, scale, opacity);
		}
	}
}

/**
 * Called every time before the render target is handed out by the Graphic
 * implementation to start in a neutral state.
 */
void RenderTarget::reset() {
	rect_.x = rect_.y = 0;
	rect_.w = surface_->width();
	rect_.h = surface_->height();

	offset_.x = offset_.y = 0;
}

/**
 * Offsets r by offset_ and clips r against rect_.
 *
 * If true is returned, r a valid rectangle that can be used.
 * If false is returned, r may not be used and may be partially modified.
 */
bool RenderTarget::clip(Rectf& r) const {
	r.x += offset_.x;
	r.y += offset_.y;

	if (r.x < 0) {
		if (r.w <= -r.x) {
			return false;
		}

		r.w += r.x;

		r.x = 0;
	}

	if (r.x + r.w > rect_.w) {
		if (rect_.w <= r.x) {
			return false;
		}
		r.w = rect_.w - r.x;
	}

	if (r.y < 0) {
		if (r.h <= -r.y) {
			return false;
		}
		r.h += r.y;
		r.y = 0;
	}

	if (r.y + r.h > rect_.h) {
		if (rect_.h <= r.y) {
			return false;
		}
		r.h = rect_.h - r.y;
	}

	r.x += rect_.x;
	r.y += rect_.y;

	return (r.w != 0.0f) && (r.h != 0.0f);
}

/**
 * Clip against window and source bitmap, returns false if blitting is
 * unnecessary because image is not inside the target surface.
 */
bool RenderTarget::to_surface_geometry(Rectf* destination_rect, Rectf* source_rect) const {
	assert(0 <= source_rect->x);
	assert(0 <= source_rect->y);
	destination_rect->x += offset_.x;
	destination_rect->y += offset_.y;

	// We have to clip the target rect against our own drawing area. If we make
	// changes to any side of our rectangle, we have to change the source rect
	// too. But since the source_rectangle might have a different size than the
	// destination_rect, we do this by making the proportional change.

	// Clipping, from the left.
	if (destination_rect->x < 0.f) {
		if (destination_rect->w <= -destination_rect->x) {
			return false;
		}
		const float source_rect_pixel_change =
		   -destination_rect->x / destination_rect->w * source_rect->w;
		source_rect->x += source_rect_pixel_change;
		source_rect->w -= source_rect_pixel_change;
		destination_rect->w += destination_rect->x;
		destination_rect->x = 0.f;
	}

	// Clipping, from the right.
	if (destination_rect->x + destination_rect->w > rect_.w) {
		if (rect_.w <= destination_rect->x) {
			return false;
		}
		const float new_destination_w = rect_.w - destination_rect->x;
		source_rect->w = new_destination_w / destination_rect->w * source_rect->w;
		destination_rect->w = new_destination_w;
	}

	// Clipping, from the top.
	if (destination_rect->y < 0.f) {
		if (destination_rect->h <= -destination_rect->y) {
			return false;
		}
		const float source_rect_pixel_change =
		   -destination_rect->y / destination_rect->h * source_rect->h;
		source_rect->y += source_rect_pixel_change;
		source_rect->h -= source_rect_pixel_change;
		destination_rect->h += destination_rect->y;
		destination_rect->y = 0.f;
	}

	// Clipping, from the bottom.
	if (destination_rect->y + destination_rect->h > rect_.h) {
		if (rect_.h <= destination_rect->y) {
			return false;
		}
		const float new_destination_h = rect_.h - destination_rect->y;
		source_rect->h = new_destination_h / destination_rect->h * source_rect->h;
		destination_rect->h = new_destination_h;
	}
	destination_rect->x += rect_.x;
	destination_rect->y += rect_.y;
	return true;
}
