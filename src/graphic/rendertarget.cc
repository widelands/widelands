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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "rendertarget.h"

#include "graphic.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/overlay_manager.h"

#include "logic/player.h"
#include "logic/tribe.h"
#include "vertex.h"

#include "surface.h"
#include "graphic/render/surface_opengl.h"
#include "graphic/render/surface_sdl.h"

#include "log.h"
#include "upcast.h"

using Widelands::BaseImmovable;
using Widelands::Coords;
using Widelands::FCoords;
using Widelands::Map;
using Widelands::Map_Object_Descr;
using Widelands::Player;
using Widelands::TCoords;

/**
 * Build a render target for the given bitmap.
 * \note The bitmap will not be owned by the renderer, i.e. it won't be
 * deleted by the destructor.
 */
RenderTarget::RenderTarget(Surface * const bmp)
{
	m_surface = bmp;
	reset();
}


/**
 * Sets an arbitrary drawing window.
 */
void RenderTarget::set_window(Rect const & rc, Point const & ofs)
{
	m_rect = rc;
	m_offset = ofs;

	// safeguards clipping against the bitmap itself

	if (m_rect.x < 0) {
		m_offset.x += m_rect.x;
		m_rect.w = std::max(static_cast<int32_t>(m_rect.w) + m_rect.x, 0);
		m_rect.x = 0;
	}

	if (m_rect.x + m_rect.w > m_surface->get_w())
		m_rect.w =
			std::max(static_cast<int32_t>(m_surface->get_w()) - m_rect.x, 0);

	if (m_rect.y < 0) {
		m_offset.y += m_rect.y;
		m_rect.h = std::max(static_cast<int32_t>(m_rect.h) + m_rect.y, 0);
		m_rect.y = 0;
	}

	if (m_rect.y + m_rect.h > m_surface->get_h())
		m_rect.h =
			std::max(static_cast<int32_t>(m_surface->get_h()) - m_rect.y, 0);
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
	(Rect const & rc, Rect * const previous, Point * const prevofs)
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
int32_t RenderTarget::get_w() const
{
	return m_surface->get_w();
}

/**
 * Returns the true size of the render target (ignoring the window settings).
 */
int32_t RenderTarget::get_h() const
{
	return m_surface->get_h();
}

/**
 * This functions draws a (not horizontal or vertical)
 * line in the target, using Bresenham's algorithm
 *
 * This function is still quite slow, since it draws
 * every pixel as a rectangle. So use it with care
 */
void RenderTarget::draw_line
	(int32_t const x1, int32_t const y1, int32_t const x2, int32_t const y2,
	 RGBColor const color)
{
#if USE_OPENGL
	//use opengl drawing if available
	/* TODO: The Opengl code does not use the clipping rectangle
	 *       The opengl code should check if it should draw to screen
	 *       or if this is a offscreen surface.
	 */
	if (g_opengl) {
		//ToDo Clip against m_rect
		upcast(SurfaceOpenGL, oglsurf, m_surface);
		oglsurf->draw_line
			(x1 + m_offset.x + m_rect.x, y1 + m_offset.y + m_rect.y,
			 x2 + m_offset.x + m_rect.x, y2 + m_offset.y + m_rect.y, color);
		return;
	}
#endif

	int32_t dx = x2 - x1;      /* the horizontal distance of the line */
	int32_t dy = y2 - y1;      /* the vertical distance of the line */
	const uint32_t dxabs = abs(dx);
	const uint32_t dyabs = abs(dy);
	int32_t sdx = dx < 0 ? -1 : 1;
	int32_t sdy = dy < 0 ? -1 : 1;
	uint32_t x = dyabs / 2;
	uint32_t y = dxabs / 2;
	Point p(x1, y1);

	draw_rect(Rect(p, 1, 1), color);

	if (dxabs >= dyabs)
		for (uint32_t i = 0; i < dxabs; ++i) {
			//  the line is more horizontal than vertical
			y += dyabs;

			if (y >= dxabs) {
				y   -= dxabs;
				p.y += sdy;
			}

			p.x += sdx;
			draw_rect(Rect(p, 1, 1), color);
		}
	else
		for (uint32_t i = 0; i < dyabs; ++i) {
			//  the line is more vertical than horizontal
			x += dxabs;

			if (x >= dyabs) {
				x   -= dyabs;
				p.x += sdx;
			}

			p.y += sdy;
			draw_rect(Rect(p, 1, 1), color);
		}
}

/**
 * Clip against window and pass those primitives along to the bitmap.
 */
void RenderTarget::draw_rect(Rect r, const RGBColor clr)
{
	if (clip(r))
		m_surface->draw_rect(r, clr);
}

void RenderTarget::fill_rect(Rect r, const RGBAColor clr)
{
	if (clip(r))
		m_surface->fill_rect(r, clr);
}

void RenderTarget::brighten_rect(Rect r, const int32_t factor)
{
	if (clip(r))
		m_surface->brighten_rect(r, factor);
}

void RenderTarget::clear()
{
	if
		(not m_rect.x and not m_rect.y
		 and
		 m_rect.w == m_surface->get_w() and m_rect.h == m_surface->get_h())
		m_surface->clear();
	else m_surface->fill_rect(m_rect, RGBColor(0, 0, 0));
}

/**
 * Blits a Surface on the screen or (if possible) on another Surface
 * Check g_gr->caps().offscreen_rendering to see if it is possible to blit
 * to a non screen surface.
 *
 * This blit function copies the pixels to the destination surface. 
 * I the source surface contains a alpha channel this is used during
 * the blit.
 */
void RenderTarget::blit(const Point dst, const PictureID picture)
{
	if (Surface * const src = g_gr->get_picture_surface(picture))
		doblit
			(Rect(dst, 0, 0),
			 src, Rect(Point(0, 0), src->get_w(), src->get_h()));
}

/**
 * Blits a Surface on the screen or (if possible) on another Surface
 * Check g_gr->caps().offscreen_rendering to see if it is possible to blit
 * to a non screen surface.
 *
 * This blit function copies the pixels to the destination surface. 
 * I the source surface contains a alpha channel this is used during
 * the blit.
 */
void RenderTarget::blit(const Rect dst, const PictureID picture)
{
	if (Surface * const src = g_gr->get_picture_surface(picture))
		doblit(dst, src, Rect(Point(0, 0), src->get_w(), src->get_h()));
}

/**
 * Blits a Surface on the screen or (if possible) on another Surface
 * Check g_gr->caps().offscreen_rendering to see if it is possible to blit
 * to a non screen surface.
 *
 * This blit function copies the pixels values without alpha channel to the
 * destination surface.
 */
void RenderTarget::blit_solid(const Point dst, const PictureID picture)
{
	Surface * const src = g_gr->get_picture_surface(picture);
	if (not src)
		return;

	upcast(SurfaceSDL, sdlsurf, src);
	upcast(SurfaceOpenGL, oglsurf, src);

	if (sdlsurf) {
		bool alpha;
		uint8_t alphaval;
		alpha = sdlsurf->get_sdl_surface()->flags & SDL_SRCALPHA;
		alphaval = sdlsurf->get_sdl_surface()->format->alpha;
		SDL_SetAlpha(sdlsurf->get_sdl_surface(), 0, 0);
		doblit
			(Rect(dst, 0, 0),
			 src, Rect(Point(0, 0), src->get_w(), src->get_h()), false);
		SDL_SetAlpha(sdlsurf->get_sdl_surface(), alpha?SDL_SRCALPHA:0, alphaval);
	} else if (oglsurf) {
		doblit
			(Rect(dst, 0, 0),
			 src, Rect(Point(0, 0), src->get_w(), src->get_h()), false);
	}
	
}

/**
 * Blits a Surface on the screen or (if possible) on another Surface
 * Check g_gr->caps().offscreen_rendering to see if it is possible to blit
 * to a non screen surface.
 *
 * This blit function copies the pixels (including alpha channel) to the destination surface.
 * The destination pixels will be exactly like the source pixels.
 */
void RenderTarget::blit_copy(Point dst, PictureID picture)
{
	Surface * const src = g_gr->get_picture_surface(picture);
	upcast(SurfaceSDL, sdlsurf, src);
	bool alpha;
	uint8_t alphaval;
	if (sdlsurf) {
		alpha = sdlsurf->get_sdl_surface()->flags & SDL_SRCALPHA;
		alphaval = sdlsurf->get_sdl_surface()->format->alpha;
		SDL_SetAlpha(sdlsurf->get_sdl_surface(), 0, 0);
	}
	if (src)
		doblit
			(Rect(dst, 0, 0),
			 src, Rect(Point(0, 0), src->get_w(), src->get_h()), false);
	if (sdlsurf) {
		SDL_SetAlpha(sdlsurf->get_sdl_surface(), alpha?SDL_SRCALPHA:0, alphaval);
	}
}

void RenderTarget::blitrect
	(Point const dst, PictureID const picture, Rect const srcrc)
{
	assert(0 <= srcrc.x);
	assert(0 <= srcrc.y);

	if (Surface * const src = g_gr->get_picture_surface(picture))
		doblit(Rect(dst, 0, 0), src, srcrc);
}

/**
 * Fill the given rectangle with the given picture.
 *
 * The pixel from ofs inside picture is placed at the top-left corner of
 * the filled rectangle.
 */
void RenderTarget::tile(Rect r, PictureID const picture, Point ofs)
{
	Surface * const src = g_gr->get_picture_surface(picture);

	if (!src)
		return;

	if (clip(r)) {
		// Make sure the offset is within bounds
		ofs.x = ofs.x % src->get_w();

		if (ofs.x < 0)
			ofs.x += src->get_w();

		ofs.y = ofs.y % src->get_h();

		if (ofs.y < 0)
			ofs.y += src->get_h();

		// Blit the picture into the rectangle
		uint32_t ty = 0;

		while (ty < r.h) {
			uint32_t tx = 0;
			int32_t tofsx = ofs.x;
			Rect srcrc;

			srcrc.y = ofs.y;
			srcrc.h = src->get_h() - ofs.y;

			if (ty + srcrc.h > r.h)
				srcrc.h = r.h - ty;

			while (tx < r.w) {
				srcrc.x = tofsx;
				srcrc.w = src->get_w() - tofsx;

				if (tx + srcrc.w > r.w)
					srcrc.w = r.w - tx;

				m_surface->blit(r + Point(tx, ty), src, srcrc);

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
	//log("RenderTarget::drawanim()\n");
	AnimationData const * const data = g_anim.get_animation(animation);
	AnimationGfx        * const gfx  = g_gr-> get_animation(animation);
	if (!data || !g_gr) {
		log("WARNING: Animation %u does not exist\n", animation);
		return;
	}

	// Get the frame and its data
	uint32_t const framenumber = time / data->frametime % gfx->nr_frames();
	Surface * const frame =
		player ?
		gfx->get_frame
			(framenumber, player->player_number(), player->get_playercolor())
		:
		gfx->get_frame
			(framenumber);

	dst -= gfx->get_hotspot();

	Rect srcrc(Point(0, 0), frame->get_w(), frame->get_h());

	doblit(Rect(dst, 0, 0), frame, srcrc);

	//  Look if there is a sound effect registered for this frame and trigger
	//  the effect (see Sound_Handler::stereo_position).
	data->trigger_soundfx(framenumber, 128);
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
	//log("RenderTarget::drawanimrect()\n");
	AnimationData const * const data = g_anim.get_animation(animation);
	AnimationGfx        * const gfx  = g_gr-> get_animation(animation);
	if (!data || !g_gr) {
		log("WARNING: Animation %u does not exist\n", animation);
		return;
	}

	// Get the frame and its data
	uint32_t const framenumber = time / data->frametime % gfx->nr_frames();
	Surface * const frame =
		player ?
		gfx->get_frame
			(framenumber, player->player_number(), player->get_playercolor())
		:
		gfx->get_frame
			(framenumber);

	dst -= g_gr->get_animation(animation)->get_hotspot();

	dst += srcrc;

	doblit(Rect(dst, 0, 0), frame, srcrc);
}

/**
 * Called every time before the render target is handed out by the Graphic
 * implementation to start in a neutral state.
 */
void RenderTarget::reset()
{
	m_rect.x = m_rect.y = 0;
	m_rect.w = m_surface->get_w();
	m_rect.h = m_surface->get_h();

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
void RenderTarget::doblit(Rect dst, Surface * const src, Rect srcrc, bool enable_alpha)
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

	dst += m_rect;

	// Draw it
	upcast(SurfaceOpenGL, oglsurf, m_surface);
	if (oglsurf and dst.w and dst.h)
		oglsurf->blit(dst, src, srcrc, enable_alpha);
	else
		m_surface->blit(Point(dst.x, dst.y), src, srcrc, enable_alpha);
}
