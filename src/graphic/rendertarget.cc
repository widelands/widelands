/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#include "graphic/rendertarget.h"

#include "base/macros.h"
#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "graphic/surface.h"

/**
 * Build a render target for the given surface.
 */
RenderTarget::RenderTarget(Surface* surf)
{
	m_surface = surf;
	reset();
}

/**
 * Sets an arbitrary drawing window.
 */
void RenderTarget::set_window(const Rect& rc, const Point& ofs)
{
	m_rect = rc;
	m_offset = ofs;

	// safeguards clipping against the bitmap itself

	if (m_rect.x < 0) {
		m_offset.x += m_rect.x;
		m_rect.w = std::max<int32_t>(m_rect.w + m_rect.x, 0);
		m_rect.x = 0;
	}

	if (m_rect.x + m_rect.w > m_surface->width())
		m_rect.w =
			std::max<int32_t>(m_surface->width() - m_rect.x, 0);

	if (m_rect.y < 0) {
		m_offset.y += m_rect.y;
		m_rect.h = std::max<int32_t>(m_rect.h + m_rect.y, 0);
		m_rect.y = 0;
	}

	if (m_rect.y + m_rect.h > m_surface->height())
		m_rect.h =
			std::max<int32_t>(m_surface->height() - m_rect.y, 0);
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
bool RenderTarget::enter_window
	(const Rect& rc, Rect* previous, Point* prevofs)
{
	Rect newrect = rc;

	if (clip(newrect)) {
		if (previous)
			*previous = m_rect;
		if (prevofs)
			*prevofs = m_offset;

		// Apply the changes
		m_offset = rc.origin() - (newrect.origin() - m_rect.origin() - m_offset);
		m_rect = newrect;

		return true;
	} else return false;
}

/**
 * Returns the true size of the render target (ignoring the window settings).
 */
int32_t RenderTarget::width() const
{
	return m_surface->width();
}

/**
 * Returns the true size of the render target (ignoring the window settings).
 */
int32_t RenderTarget::height() const
{
	return m_surface->height();
}

/**
 * This functions draws a line in the target
 */
void RenderTarget::draw_line_strip(const std::vector<FloatPoint>& points,
                                   const RGBColor& color,
                                   float line_width,
                                   const LineDrawMode& line_draw_mode) {
	std::vector<FloatPoint> adjusted_points;
	adjusted_points.reserve(points.size());
	for (const auto& p : points) {
		adjusted_points.emplace_back(p.x + m_offset.x + m_rect.x, p.y + m_offset.y + m_rect.y);
	}
	m_surface->draw_line_strip(adjusted_points, color, line_width, line_draw_mode);
}

/**
 * Clip against window and pass those primitives along to the bitmap.
 */
void RenderTarget::draw_rect(const Rect& rect, const RGBColor& clr)
{
	Rect r(rect);
	if (clip(r)) {
		::draw_rect(r, clr, m_surface);
	}
}

void RenderTarget::fill_rect(const Rect& rect, const RGBAColor& clr)
{
	Rect r(rect);
	if (clip(r))
		m_surface->fill_rect(r, clr);
}

void RenderTarget::brighten_rect(const Rect& rect, int32_t factor)
{
	Rect r(rect);
	if (clip(r))
		m_surface->brighten_rect(r, factor);
}

/**
 * Blits a Image on another Surface
 *
 * This blit function copies the pixels to the destination surface.
 */
void RenderTarget::blit(const Point& dst, const Image* image, BlendMode blend_mode, UI::Align align)
{
	Point destination_point(dst);
	UI::correct_for_align(align, image->width(), image->height(), &destination_point);

	Rect source_rect(Point(0, 0), image->width(), image->height());
	Rect destination_rect(destination_point.x, destination_point.y, source_rect.w, source_rect.h);

	if (to_surface_geometry(&destination_rect, &source_rect)) {
		m_surface->blit(destination_rect, *image, source_rect, 1., blend_mode);
	}
}

void RenderTarget::blit_monochrome(const Point& dst,
									  const Image* image,
									  const RGBAColor& blend_mode, UI::Align align) {
	Point destination_point(dst);
	UI::correct_for_align(align, image->width(), image->height(), &destination_point);

	Rect source_rect(Point(0, 0), image->width(), image->height());
	Rect destination_rect(destination_point.x, destination_point.y, source_rect.w, source_rect.h);

	if (to_surface_geometry(&destination_rect, &source_rect)) {
		m_surface->blit_monochrome(destination_rect, *image, source_rect, blend_mode);
	}
}

/**
 * Like \ref blit, but use only a sub-rectangle of the source image.
 */
void RenderTarget::blitrect
	(const Point& dst, const Image* image, const Rect& gsrcrc, BlendMode blend_mode)
{
	assert(0 <= gsrcrc.x);
	assert(0 <= gsrcrc.y);

	// We want to use the given srcrc, but we must make sure that we are not
	// blitting outside of the boundaries of 'image'.
	Rect source_rect(gsrcrc.x,
	           gsrcrc.y,
	           std::min<int32_t>(image->width() - gsrcrc.x, gsrcrc.w),
	           std::min<int32_t>(image->height() - gsrcrc.y, gsrcrc.h));
	Rect destination_rect(dst.x, dst.y, source_rect.w, source_rect.h);

	if (to_surface_geometry(&destination_rect, &source_rect)) {
		m_surface->blit(destination_rect, *image, source_rect, 1., blend_mode);
	}
}

void RenderTarget::blitrect_scale(Rect destination_rect,
                                  const Image* image,
                                  Rect source_rect,
                                  const float opacity,
                                  const BlendMode blend_mode) {
	if (to_surface_geometry(&destination_rect, &source_rect)) {
		m_surface->blit(destination_rect, *image, source_rect, opacity, blend_mode);
	}
}

void RenderTarget::blitrect_scale_monochrome(Rect destination_rect,
                                       const Image* image,
                                       Rect source_rect,
													const RGBAColor& blend) {
	if (to_surface_geometry(&destination_rect, &source_rect)) {
		m_surface->blit_monochrome(destination_rect, *image, source_rect, blend);
	}
}

/**
 * Fill the given rectangle with the given image.
 *
 * The pixel from ofs inside image is placed at the top-left corner of
 * the filled rectangle.
 */
void RenderTarget::tile(const Rect& rect, const Image* image, const Point& gofs, BlendMode blend_mode)
{
	int32_t srcw = image->width();
	int32_t srch = image->height();

	Rect r(rect);
	Point ofs(gofs);
	if (clip(r)) {
		if (m_offset.x < 0)
			ofs.x -= m_offset.x;

		if (m_offset.y < 0)
			ofs.y -= m_offset.y;

		// Make sure the offset is within bounds
		ofs.x = ofs.x % srcw;

		if (ofs.x < 0)
			ofs.x += srcw;

		ofs.y = ofs.y % srch;

		if (ofs.y < 0)
			ofs.y += srch;

		// Blit the image into the rectangle
		int ty = 0;

		while (ty < r.h) {
			int tx = 0;
			int32_t tofsx = ofs.x;
			Rect srcrc;

			srcrc.y = ofs.y;
			srcrc.h = srch - ofs.y;

			if (ty + srcrc.h > r.h)
				srcrc.h = r.h - ty;

			while (tx < r.w) {
				srcrc.x = tofsx;
				srcrc.w = srcw - tofsx;

				if (tx + srcrc.w > r.w)
					srcrc.w = r.w - tx;

				const Rect dst_rect(r.x + tx, r.y + ty, srcrc.w, srcrc.h);
				m_surface->blit(dst_rect, *image, srcrc, 1., blend_mode);

				tx += srcrc.w;

				tofsx = 0;
			}

			ty += srcrc.h;
			ofs.y = 0;
		}
	}
}

/**
 * Draws a frame of an animation at the given location
 * Plays sound effect that is registered with this frame (the SoundHandler
 * decides if the fx really does get played)
 *
 * \param dstx, dsty the on-screen location of the animation hot spot
 * \param animation the animation ID
 * \param time the time, in milliseconds, in the animation
 * \param player the player this object belongs to, for player colour
 * purposes. May be 0 (for example, for world objects).
 */
// TODO(unknown): Correctly calculate the stereo position for sound effects
// TODO(unknown): The chosen semantics of animation sound effects is problematic:
// What if the game runs very slowly or very quickly?
void RenderTarget::blit_animation(const Point& dst, uint32_t animation, uint32_t time) {
	const Animation& anim = g_gr->animations().get_animation(animation);
	do_blit_animation(dst, anim, time, nullptr, Rect(Point(0, 0), anim.width(), anim.height()));
}

void RenderTarget::blit_animation(const Point& dst,
                                  uint32_t animation,
                                  uint32_t time,
                                  const RGBColor& player_color) {
	const Animation& anim = g_gr->animations().get_animation(animation);
	do_blit_animation(dst, anim, time, &player_color, Rect(Point(0, 0), anim.width(), anim.height()));
}

void RenderTarget::blit_animation(const Point& dst,
                                  uint32_t animation,
                                  uint32_t time,
                                  const RGBColor& player_color,
                                  const Rect& source_rect) {
	do_blit_animation(
	   dst, g_gr->animations().get_animation(animation), time, &player_color, source_rect);
}

void RenderTarget::do_blit_animation(const Point& dst,
                                     const Animation& animation,
                                     uint32_t time,
                                     const RGBColor* player_color,
                                     const Rect& source_rect) {
	Rect destination_rect(dst.x - animation.hotspot().x + source_rect.x,
	                      dst.y - animation.hotspot().y + source_rect.y, source_rect.w,
	                      source_rect.h);
	Rect srcrc(source_rect);
	if (to_surface_geometry(&destination_rect, &srcrc)) {
		animation.blit(time, destination_rect.origin(), srcrc, player_color, m_surface);
	}

	// Look if there is a sound effect registered for this frame and trigger the
	// effect (see SoundHandler::stereo_position).
	// TODO(sirver): Playing a sound effect in here is rather silly. What if
	// this animation is used in the menus?
	animation.trigger_soundfx(time, 128);
}

/**
 * Called every time before the render target is handed out by the Graphic
 * implementation to start in a neutral state.
 */
void RenderTarget::reset()
{
	m_rect.x = m_rect.y = 0;
	m_rect.w = m_surface->width();
	m_rect.h = m_surface->height();

	m_offset.x = m_offset.y = 0;
}

/**
 * Offsets r by m_offset and clips r against m_rect.
 *
 * If true is returned, r a valid rectangle that can be used.
 * If false is returned, r may not be used and may be partially modified.
 */
bool RenderTarget::clip(Rect & r) const
{
	r.x += m_offset.x;
	r.y += m_offset.y;

	if (r.x < 0) {
		if (r.w <= -r.x)
			return false;

		r.w += r.x;

		r.x = 0;
	}

	if (r.x + r.w > m_rect.w) {
		if (m_rect.w <= r.x)
			return false;
		r.w = m_rect.w - r.x;
	}

	if (r.y < 0) {
		if (r.h <= -r.y)
			return false;
		r.h += r.y;
		r.y = 0;
	}

	if (r.y + r.h > m_rect.h) {
		if (m_rect.h <= r.y)
			return false;
		r.h = m_rect.h - r.y;
	}

	r.x += m_rect.x;
	r.y += m_rect.y;

	return r.w && r.h;
}

/**
 * Clip against window and source bitmap, returns false if blitting is
 * unnecessary because image is not inside the target surface.
 */
bool RenderTarget::to_surface_geometry(Rect* destination_rect, Rect* source_rect) const
{
	assert(0 <= source_rect->x);
	assert(0 <= source_rect->y);
	destination_rect->x += m_offset.x;
	destination_rect->y += m_offset.y;

	// We have to clip the target rect against our own drawing area. If we make
	// changes to any side of our rectangle, we have to change the source rect
	// too. But since the source_rectangle might have a different size than the
	// destination_rect, we do this by making the proportional change.

	// Clipping, from the left.
	if (destination_rect->x < 0) {
		if (destination_rect->w <= -destination_rect->x) {
			return false;
		}
		// Adding 0.5 is a cheap way of turning integer truncation into a rounded value.
		const int source_rect_pixel_change =
		   0.5 + -static_cast<double>(destination_rect->x) / destination_rect->w * source_rect->w;
		source_rect->x += source_rect_pixel_change;
		source_rect->w -= source_rect_pixel_change;
		destination_rect->w += destination_rect->x;
		destination_rect->x = 0;
	}

	// Clipping, from the right.
	if (destination_rect->x + destination_rect->w > m_rect.w) {
		if (m_rect.w <= destination_rect->x) {
			return false;
		}
		const int new_destination_w = m_rect.w - destination_rect->x;
		// Adding 0.5 is a cheap way of turning integer truncation into a rounded value.
		source_rect->w =
		   0.5 + static_cast<double>(new_destination_w) / destination_rect->w * source_rect->w;
		destination_rect->w = new_destination_w;
	}

	// Clipping, from the top.
	if (destination_rect->y < 0) {
		if (destination_rect->h <= -destination_rect->y) {
			return false;
		}
		// Adding 0.5 is a cheap way of turning integer truncation into a rounded value.
		const int source_rect_pixel_change = 0.5 +
		   -static_cast<double>(destination_rect->y) / destination_rect->h * source_rect->h;
		source_rect->y += source_rect_pixel_change;
		source_rect->h -= source_rect_pixel_change;
		destination_rect->h += destination_rect->y;
		destination_rect->y = 0;
	}

	// Clipping, from the bottom.
	if (destination_rect->y + destination_rect->h > m_rect.h) {
		if (m_rect.h <= destination_rect->y) {
			return false;
		}
		const int new_destination_h = m_rect.h - destination_rect->y;
		// Adding 0.5 is a cheap way of turning integer truncation into a rounded value.
		source_rect->h =
		   0.5 + static_cast<double>(new_destination_h) / destination_rect->h * source_rect->h;
		destination_rect->h = new_destination_h;
	}

	destination_rect->x += m_rect.x;
	destination_rect->y += m_rect.y;
	return true;
}
