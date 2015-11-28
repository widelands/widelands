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

#include "base/log.h"
#include "base/macros.h"
#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "graphic/surface.h"
#include "logic/player.h"
#include "logic/tribes/tribe_descr.h"
#include "wui/overlay_manager.h"

using Widelands::BaseImmovable;
using Widelands::Coords;
using Widelands::FCoords;
using Widelands::Map;
using Widelands::MapObjectDescr;
using Widelands::Player;
using Widelands::TCoords;

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
void RenderTarget::draw_line(const Point& start,
                             const Point& end,
                             const RGBColor& color,
                             uint8_t line_width) {
	m_surface->draw_line(Point(start.x + m_offset.x + m_rect.x, start.y + m_offset.y + m_rect.y),
	                     Point(end.x + m_offset.x + m_rect.x, end.y + m_offset.y + m_rect.y),
	                     color,
	                     line_width);
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

	Rect srcrc(Point(0, 0), image->width(), image->height());

	if (to_surface_geometry(&destination_point, &srcrc)) {
		m_surface->blit(Rect(destination_point.x, destination_point.y, srcrc.w, srcrc.h),
		                *image,
		                srcrc,
		                1.,
		                blend_mode);
	}
}

void RenderTarget::blit_monochrome(const Point& dst,
									  const Image* image,
									  const RGBAColor& blend_mode, UI::Align align) {
	Point destination_point(dst);

	UI::correct_for_align(align, image->width(), image->height(), &destination_point);

	Rect srcrc(Point(0, 0), image->width(), image->height());

	if (to_surface_geometry(&destination_point, &srcrc)) {
		::blit_monochrome(Rect(destination_point.x, destination_point.y, srcrc.w, srcrc.h),
			  *image,
			  srcrc,
			  blend_mode,
			  m_surface);
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
	Rect srcrc(gsrcrc.x,
	           gsrcrc.y,
	           std::min<int32_t>(image->width() - gsrcrc.x, gsrcrc.w),
	           std::min<int32_t>(image->height() - gsrcrc.y, gsrcrc.h));

	Point destination_point(dst);
	if (to_surface_geometry(&destination_point, &srcrc))
		m_surface->blit(Rect(destination_point.x, destination_point.y, srcrc.w, srcrc.h),
		         *image,
		         srcrc,
		         1.,
		         blend_mode);
}

void RenderTarget::blitrect_scale(const Rect& dst,
                                  const Image* image,
                                  const Rect& source_rect,
											 const float opacity,
                                  const BlendMode blend_mode) {

	Point destination_point(dst.x, dst.y);
	Rect srcrect(source_rect);
	if (to_surface_geometry(&destination_point, &srcrect)) {
		m_surface->blit(Rect(destination_point.x, destination_point.y, dst.w, dst.h),
		                *image,
		                source_rect,
		                opacity,
		                blend_mode);
	}
}

void RenderTarget::blitrect_scale_monochrome(const Rect& destination_rect,
                                       const Image* image,
                                       const Rect& source_rect,
													const RGBAColor& blend) {
	Point destination_point(destination_rect.x, destination_rect.y);
	Rect srcrect(source_rect);
	if (to_surface_geometry(&destination_point, &srcrect)) {
		m_surface->blit_monochrome(
		   Rect(destination_point.x, destination_point.y, destination_rect.w, destination_rect.h),
		   *image,
		   source_rect,
		   blend);
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
void RenderTarget::drawanim
	(const Point& dst, uint32_t animation, uint32_t time, const Player* player)
{
	const Animation& anim = g_gr->animations().get_animation(animation);

	Point destination_point = dst - anim.hotspot();

	Rect srcrc(Point(0, 0), anim.width(), anim.height());

	if (to_surface_geometry(&destination_point, &srcrc))
		anim.blit(time, destination_point, srcrc, player ? &player->get_playercolor() : NULL, m_surface);

	//  Look if there is a sound effect registered for this frame and trigger
	//  the effect (see SoundHandler::stereo_position).
	anim.trigger_soundfx(time, 128);
}

/**
 * Draws a part of a frame of an animation at the given location
 */
void RenderTarget::drawanimrect
	(const Point& dst, uint32_t animation, uint32_t time, const Player* player, const Rect& gsrcrc)
{
	const Animation& anim = g_gr->animations().get_animation(animation);

	Point destination_point = dst - anim.hotspot();
	destination_point += gsrcrc.origin();

	Rect srcrc(gsrcrc);

	if (to_surface_geometry(&destination_point, &srcrc))
		anim.blit(time, destination_point, srcrc, player ? &player->get_playercolor() : NULL, m_surface);
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
bool RenderTarget::to_surface_geometry(Point* dst, Rect* srcrc) const
{
	assert(0 <= srcrc->x);
	assert(0 <= srcrc->y);
	*dst += m_offset;

	// Clipping
	if (dst->x < 0) {
		if (srcrc->w <= -dst->x)
			return false;
		srcrc->x -= dst->x;
		srcrc->w += dst->x;
		dst->x = 0;
	}

	if (dst->x + srcrc->w > m_rect.w) {
		if (m_rect.w <= dst->x)
			return false;
		srcrc->w = m_rect.w - dst->x;
	}

	if (dst->y < 0) {
		if (srcrc->h <= -dst->y)
			return false;
		srcrc->y -= dst->y;
		srcrc->h += dst->y;
		dst->y = 0;
	}

	if (dst->y + srcrc->h > m_rect.h) {
		if (m_rect.h <= dst->y)
			return false;
		srcrc->h = m_rect.h - dst->y;
	}

	*dst += m_rect.origin();
	return true;
}
