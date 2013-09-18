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

#include "graphic/animation.h"
#include "graphic/animation_gfx.h"
#include "graphic/graphic.h"
#include "graphic/image_transformations.h"
#include "graphic/surface.h"
#include "log.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "upcast.h"
#include "vertex.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/overlay_manager.h"

using Widelands::BaseImmovable;
using Widelands::Coords;
using Widelands::FCoords;
using Widelands::Map;
using Widelands::Map_Object_Descr;
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
void RenderTarget::set_window(const Rect & rc, const Point & ofs)
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
	(const Rect & rc, Rect* previous, Point* prevofs)
{
	Rect newrect = rc;

	if (clip(newrect)) {
		if (previous)
			*previous = m_rect;
		if (prevofs)
			*prevofs = m_offset;

		// Apply the changes
		m_offset = rc - (newrect - m_rect - m_offset);
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
void RenderTarget::draw_line
	(int32_t const x1, int32_t const y1, int32_t const x2, int32_t const y2,
	 const RGBColor& color, uint8_t gwidth)
{
	m_surface->draw_line
		(x1 + m_offset.x + m_rect.x, y1 + m_offset.y + m_rect.y,
		 x2 + m_offset.x + m_rect.x, y2 + m_offset.y + m_rect.y, color,
		 gwidth);
}

/**
 * Clip against window and pass those primitives along to the bitmap.
 */
void RenderTarget::draw_rect(Rect r, const RGBColor& clr)
{
	if (clip(r))
		m_surface->draw_rect(r, clr);
}

void RenderTarget::fill_rect(Rect r, const RGBAColor& clr)
{
	if (clip(r))
		m_surface->fill_rect(r, clr);
}

void RenderTarget::brighten_rect(Rect r, const int32_t factor)
{
	if (clip(r))
		m_surface->brighten_rect(r, factor);
}

/**
 * Blits a Image on another Surface
 *
 * This blit function copies the pixels to the destination surface.
 * If the source surface contains a alpha channel this is used during
 * the blit.
 */
void RenderTarget::blit(const Point& dst, const Image* image, Composite cm, UI::Align align)
{
	Point dstpoint(dst);

	UI::correct_for_align(align, image->width(), image->height(), &dstpoint);
	doblit(dstpoint, image, Rect(Point(0, 0), image->width(), image->height()), cm);
}

/**
 * Like \ref blit, but use only a sub-rectangle of the source image.
 */
void RenderTarget::blitrect
	(Point const dst, const Image* image, Rect const srcrc, Composite cm)
{
	assert(0 <= srcrc.x);
	assert(0 <= srcrc.y);

	doblit(dst, image, srcrc, cm);
}

/**
 * Fill the given rectangle with the given image.
 *
 * The pixel from ofs inside image is placed at the top-left corner of
 * the filled rectangle.
 */
void RenderTarget::tile(Rect r, const Image* image, Point ofs, Composite cm)
{
	int32_t srcw = image->width();
	int32_t srch = image->height();

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
		uint32_t ty = 0;

		while (ty < r.h) {
			uint32_t tx = 0;
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

				m_surface->blit(r + Point(tx, ty), image->surface(), srcrc, cm);

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
 * Plays sound effect that is registered with this frame (the Sound_Handler
 * decides if the fx really does get played)
 *
 * \param dstx, dsty the on-screen location of the animation hot spot
 * \param animation the animation ID
 * \param time the time, in milliseconds, in the animation
 * \param player the player this object belongs to, for player colour
 * purposes. May be 0 (for example, for world objects).
 *
 * \todo Correctly calculate the stereo position for sound effects
 * \todo The chosen semantics of animation sound effects is problematic:
 * What if the game runs very slowly or very quickly?
 */
void RenderTarget::drawanim
	(Point                dst,
	 uint32_t       const animation,
	 uint32_t       const time,
	 Player const * const player)
{
	const AnimationData& data = g_anim.get_animation(animation);
	AnimationGfx        * const gfx  = g_gr-> get_animation(animation);
	if (!gfx) {
		log("WARNING: Animation %u does not exist\n", animation);
		return;
	}

	// Get the frame and its data
	uint32_t const framenumber = time / data.frametime % gfx->nr_frames();
	const Image& frame =
		player ? gfx->get_frame(framenumber, player->get_playercolor()) : gfx->get_frame(framenumber);

	dst -= gfx->hotspot();

	Rect srcrc(Point(0, 0), frame.width(), frame.height());

	doblit(dst, &frame, srcrc);

	//  Look if there is a sound effect registered for this frame and trigger
	//  the effect (see Sound_Handler::stereo_position).
	data.trigger_soundfx(framenumber, 128);
}

void RenderTarget::drawstatic
	(Point                dst,
	 uint32_t       const animation,
	 Player const * const player)
{
	AnimationGfx        * const gfx  = g_gr-> get_animation(animation);
	if (!gfx) {
		log("WARNING: Animation %u does not exist\n", animation);
		return;
	}

	// Get the frame and its data
	const Image& frame = player ? gfx->get_frame(0, player->get_playercolor()) : gfx->get_frame(0);
	const Image* dark_frame = ImageTransformations::change_luminosity(&frame, 1.22, true);

	dst -= Point(frame.width() / 2, frame.height() / 2);
	Rect srcrc(Point(0, 0), frame.width(), frame.height());
	doblit(Rect(dst, 0, 0), dark_frame, srcrc);
}

/**
 * Draws a part of a frame of an animation at the given location
 */
void RenderTarget::drawanimrect
	(Point                dst,
	 uint32_t       const animation,
	 uint32_t       const time,
	 Player const * const player,
	 Rect                 srcrc)
{
	const AnimationData& data = g_anim.get_animation(animation);
	AnimationGfx        * const gfx  = g_gr-> get_animation(animation);
	if (!gfx) {
		log("WARNING: Animation %u does not exist\n", animation);
		return;
	}

	// Get the frame and its data
	uint32_t const framenumber = time / data.frametime % gfx->nr_frames();
	const Image& frame =
		player ?
		gfx->get_frame
			(framenumber, player->get_playercolor())
		:
		gfx->get_frame
			(framenumber);

	dst -= g_gr->get_animation(animation)->hotspot();

	dst += srcrc;

	doblit(dst, &frame, srcrc);
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
bool RenderTarget::clip(Rect & r) const throw ()
{
	r += m_offset;

	if (r.x < 0) {
		if (r.w <= static_cast<uint32_t>(-r.x))
			return false;

		r.w += r.x;

		r.x = 0;
	}

	if (r.x + r.w > m_rect.w) {
		if (static_cast<int32_t>(m_rect.w) <= r.x)
			return false;
		r.w = m_rect.w - r.x;
	}

	if (r.y < 0) {
		if (r.h <= static_cast<uint32_t>(-r.y))
			return false;
		r.h += r.y;
		r.y = 0;
	}

	if (r.y + r.h > m_rect.h) {
		if (static_cast<int32_t>(m_rect.h) <= r.y)
			return false;
		r.h = m_rect.h - r.y;
	}

	r += m_rect;

	return r.w and r.h;
}

/**
 * Clip against window and source bitmap, then call the Bitmap blit routine.
 */
void RenderTarget::doblit
	(Point dst, const Image* src, Rect srcrc, Composite cm)
{
	assert(0 <= srcrc.x);
	assert(0 <= srcrc.y);
	dst += m_offset;

	// Clipping
	if (dst.x < 0) {
		if (srcrc.w <= static_cast<uint32_t>(-dst.x))
			return;
		srcrc.x -= dst.x;
		srcrc.w += dst.x;
		dst.x = 0;
	}

	if (dst.x + srcrc.w > m_rect.w) {
		if (static_cast<int32_t>(m_rect.w) <= dst.x)
			return;
		srcrc.w = m_rect.w - dst.x;
	}

	if (dst.y < 0) {
		if (srcrc.h <= static_cast<uint32_t>(-dst.y))
			return;
		srcrc.y -= dst.y;
		srcrc.h += dst.y;
		dst.y = 0;
	}

	if (dst.y + srcrc.h > m_rect.h) {
		if (static_cast<int32_t>(m_rect.h) <= dst.y)
			return;
		srcrc.h = m_rect.h - dst.y;
	}

	// Also ensure srcrc is not bigger than src
	// so opengl blits correctly
	if (src->width() < srcrc.x + srcrc.w) {
		srcrc.w = src->width() - srcrc.x;
	}
	if (src->height() < srcrc.y + srcrc.h) {
		srcrc.h = src->height() - srcrc.y;
	}

	dst += m_rect;

	m_surface->blit(dst, src->surface(), srcrc, cm);
}
