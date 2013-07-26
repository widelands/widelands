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

#ifndef TERRAIN_SDL_H
#define TERRAIN_SDL_H

#include <cassert>

#include "constants.h"
#include "graphic/graphic.h"
#include "graphic/texture.h"
#include "log.h"
#include "logic/roadtype.h"
#include "random.h"
#include "graphic/render/sdl_surface.h"
#include "upcast.h"
#include "vertex.h"
#include "wui/mapviewpixelconstants.h"

///Must be a power of two
#define DITHER_WIDTH 4

#define DITHER_RAND_MASK (DITHER_WIDTH * 2 - 1)
#define DITHER_RAND_SHIFT (16 / DITHER_WIDTH)

/**
 * \todo Dangerous: casting, assumptions for sizeof(X), bitshifting
 */
#define FTOFIX(f) (static_cast<int32_t>((f) * 0x10000))
#define ITOFIX(i) ((i)<<16)
#define FIXTOI(f) ((f)>>16)

void get_horiz_linearcomb
	(int32_t u1, int32_t u2, int32_t v1, int32_t v2,
	 float & lambda, float & mu);


struct LeftEdge {
	/**
	 * Height of the edge.
	 *
	 * This is the number of pixels spanned by the edge in a vertical direction
	 */
	uint32_t height;

	// the following are all fixed point
	int32_t x0;
	int32_t tx0;
	int32_t ty0;
	int32_t b0;
	int32_t dx;
	int32_t dtx;
	int32_t dty;
	int32_t db;
};

struct RightEdge {
	uint32_t height;

	int32_t x0;
	int32_t dx;
};


/**
 * Render a polygon based on the given edge lists.
 *
 * The edge lists will be overwritten with undefined values.
 */
template<typename T> static void render_edge_lists
	(SDLSurface & dst, const Texture & tex,
	 int32_t y, int32_t height,
	 LeftEdge * left, RightEdge * right,
	 int32_t dbdx, int32_t dtydx)
{
	if (-y >= height)
		return; // completely above screen

	uint8_t * texpixels;
	T * texcolormap;

	texpixels = tex.get_curpixels();
	texcolormap = static_cast<T *>(tex.get_colormap());

	// Skip lines that are above the screen
	while (y < 0) {
		int32_t skip = -y;
		if (skip > static_cast<int32_t>(left->height))
			skip = left->height;
		if (skip > static_cast<int32_t>(right->height))
			skip = right->height;

		if (skip < static_cast<int32_t>(left->height)) {
			left->x0     += skip * left->dx;
			left->tx0    += skip * left->dtx;
			left->ty0    += skip * left->dty;
			left->b0     += skip * left->db;
			left->height -= skip;
		} else {
			++left;
		}

		if (skip < static_cast<int32_t>(right->height)) {
			right->x0     += skip * right->dx;
			right->height -= skip;
		} else {
			++right;
		}

		height -= skip;
		y += skip;
	}

	// Cut off lines below screen
	if (y + height > static_cast<int32_t>(dst.height()))
		height = dst.height() - y;

	int32_t dstw = dst.width();
	while (height > 0) {
		int32_t leftx = FIXTOI(left->x0);
		int32_t rightx = FIXTOI(right->x0);

		if (leftx < 0)
			leftx = 0;
		if (rightx > dstw)
			rightx = dstw;

		if (leftx < rightx) {
			int32_t tx = left->tx0;
			int32_t ty = left->ty0;
			int32_t b = left->b0;

			int32_t adjust = ITOFIX(leftx) - left->x0;
			tx += adjust; // note: dtx/dx = 1
			ty += FIXTOI(static_cast<long long>(adjust) * dtydx);
			b  += FIXTOI(static_cast<long long>(adjust) * dbdx);

			// Technically, we should clamp b at every pixel, but that's too
			// expensive.  The following seems to be enough to get rid of
			// artifacts along the border between seen and unseen fields.
			if (b < ITOFIX(-128))
				b = ITOFIX(-128);

			tx = FIXTOI(tx);

			T * scanline =
				reinterpret_cast<T *>
					(static_cast<Uint8 *>(dst.get_pixels()) + y * dst.get_pitch())
				+
				leftx;

			uint32_t count = rightx - leftx;
			while (count--) {
				*scanline++ =
					texcolormap
						[texpixels
						 	[(tx & (TEXTURE_WIDTH - 1)) |
						 	 ((ty >> 10) & ((TEXTURE_HEIGHT - 1) << 6))]
						 |
						 ((b >> 8) & 0xFF00)];

				b += dbdx;
				++tx;
				ty += dtydx;
			}
		}

		// Advance the line
		++y;
		left->x0 += left->dx;
		left->tx0 += left->dtx;
		left->ty0 += left->dty;
		left->b0 += left->db;
		right->x0 += right->dx;

		if (--left->height == 0)
			++left;
		if (--right->height == 0)
			++right;
		--height;
	}
}

struct WLPolygon {
	Point p[3];
	uint8_t nrpoints;
};


/**
 * Render a triangle into the given destination surface.
 *
 * \note It is assumed that p1, p2, p3 are sorted in counter-clockwise order.
 *
 * \note The rendering code assumes that d(tx)/d(x) = 1. This can be achieved
 * by making sure that there is a 1:1 relation between x coordinates and
 * texture x coordinates.
 */
template<typename T> static void render_triangle
	(SDLSurface & dst,
	 const Vertex & p1, const Vertex & p2, const Vertex & p3,
	 const Texture & tex)
{
	if (p1.y == p2.y && p2.y == p3.y)
		return; // degenerate triangle

	// Clip the triangle
	WLPolygon polygon;

	polygon.p[0] = p1;
	polygon.p[1] = p2;
	polygon.p[2] = p3;
	polygon.nrpoints = 3;

	// Determine a top vertex
	int32_t top = 0, topy = 0;

	topy = 0x7fffffff;
	for (uint8_t i = 0; i < polygon.nrpoints; ++i) {
		if (polygon.p[i].y < topy) {
			top = i;
			topy = polygon.p[i].y;
		}
	}

	// Calculate d(b) / d(x) etc. as fixed point variables.
	// Remember that we assume d(tx) / d(x) == 1 and d(tx) / d(y) == 0.

	//  lA * (p2 - p1) + lB * (p3 - p1) = (1, 0)
	//  mA * (p2 - p1) + mB * (p3 - p1) = (0, 1)
	int32_t const det =
		(p2.x - p1.x) * (p3.y - p1.y) - (p2.y - p1.y) * (p3.x - p1.x);
	int32_t const lA =  ITOFIX(p3.y - p1.y) / det;
	int32_t const lB = -ITOFIX(p2.y - p1.y) / det;
	int32_t const mA = -ITOFIX(p3.x - p1.x) / det;
	int32_t const mB =  ITOFIX(p2.x - p1.x) / det;
	int32_t const dbdx = lA * (p2.b - p1.b) + lB * (p3.b - p1.b);
	int32_t const dbdy = mA * (p2.b - p1.b) + mB * (p3.b - p1.b);
	int32_t const dtydx = lA * (p2.ty - p1.ty) + lB * (p3.ty - p1.ty);
	int32_t const dtydy = mA * (p2.ty - p1.ty) + mB * (p3.ty - p1.ty);

	// Build left edges
	int32_t boty = topy;
	LeftEdge leftedges[3];

	{
		uint8_t nrleftedges = 0;
		uint8_t start = top;
		uint8_t end = (top + 1) % polygon.nrpoints;
		do {
			if (polygon.p[end].y > polygon.p[start].y) {
				boty = polygon.p[end].y;

				LeftEdge & edge = leftedges[nrleftedges++];
				assert(nrleftedges <= 3);

				edge.height = polygon.p[end].y - polygon.p[start].y;
				edge.x0 = ITOFIX(polygon.p[start].x);
				edge.dx =
					ITOFIX(polygon.p[end].x - polygon.p[start].x)
					/
					static_cast<int32_t>(edge.height);

				int32_t startdx = polygon.p[start].x - p1.x;
				int32_t startdy = polygon.p[start].y - p1.y;
				int32_t dx = polygon.p[end].x - polygon.p[start].x;
				int32_t dy = polygon.p[end].y - polygon.p[start].y;

				edge.tx0 = ITOFIX(p1.tx + startdx);
				edge.ty0 = ITOFIX(p1.ty) + startdx * dtydx + startdy * dtydy;
				edge.b0 = ITOFIX(p1.b) + startdx * dbdx + startdy * dbdy;
				edge.dtx = ITOFIX(dx) / static_cast<int32_t>(edge.height);
				edge.dty =
					(dx * dtydx + dy * dtydy) / static_cast<int32_t>(edge.height);
				edge.db =
					(dx * dbdx  + dy * dbdy)  / static_cast<int32_t>(edge.height);
			}

			start = end;
			end = (start + 1) % polygon.nrpoints;
		} while (polygon.p[end].y >= polygon.p[start].y);
	}

	// Build right edges
	RightEdge rightedges[3];

	{
		uint8_t nrrightedges = 0;
		uint8_t start = top;
		uint8_t end = (polygon.nrpoints + top - 1) % polygon.nrpoints;
		do {
			if (polygon.p[end].y > polygon.p[start].y) {
				RightEdge & edge = rightedges[nrrightedges++];
				assert(nrrightedges <= 3);

				edge.height = polygon.p[end].y - polygon.p[start].y;
				edge.x0 = ITOFIX(polygon.p[start].x);
				edge.dx =
					ITOFIX(polygon.p[end].x - polygon.p[start].x)
					/
					static_cast<int32_t>(edge.height);
			}

			start = end;
			end = (polygon.nrpoints + start - 1) % polygon.nrpoints;
		} while (polygon.p[end].y >= polygon.p[start].y);
	}

	render_edge_lists<T>
		(dst, tex, topy, boty - topy, leftedges, rightedges, dbdx, dtydx);
}

/**
 * Blur the polygon edge between vertices start and end.
 *
 * It is dithered by randomly placing points taken from the texture of the
 * adjacent polygon. The blend area is a few pixels wide, and the chance for
 * replacing a pixel depends on the distance from the center line. Texture
 * coordinates and brightness are interpolated across the center line (outer
 * loop). To the sides these are approximated (inner loop): Brightness is kept
 * constant, and the texture is mapped orthogonally to the center line. It is
 * important that only those pixels are drawn whose texture actually changes in
 * order to minimize artifacts.
 *
 * \note All this is preliminary and subject to change. For example, a special
 * edge texture could be used instead of stochastically dithering. Road
 * rendering could be handled as a special case then.
*/
template<typename T> static void dither_edge_horiz
	(SDLSurface & dst,
	 const Vertex & start, const Vertex & end,
	 const Texture & ttex, const Texture & btex)
{
	uint8_t * tpixels, * bpixels;
	T * tcolormap, * bcolormap;

	tpixels = ttex.get_curpixels();
	tcolormap = static_cast<T *>(ttex.get_colormap());
	bpixels = btex.get_curpixels();
	bcolormap = static_cast<T *>(btex.get_colormap());

	int32_t tx, ty, b, dtx, dty, db, tx0, ty0;

	tx  = ITOFIX(start.tx);
	ty  = ITOFIX(start.ty);
	b   = ITOFIX(start.b);
	dtx = (ITOFIX(end.tx) - tx) / (end.x - start.x + 1);
	dty = (ITOFIX(end.ty) - ty) / (end.x - start.x + 1);
	db  = (ITOFIX(end.b)  - b)  / (end.x - start.x + 1);

	// TODO: seed this depending on field coordinates
	uint32_t rnd = 0;

	const int32_t dstw = dst.width();
	const int32_t dsth = dst.height();

	int32_t ydiff = ITOFIX(end.y - start.y) / (end.x - start.x);
	int32_t centery = ITOFIX(start.y);

	for (int32_t x = start.x; x < end.x; x++, centery += ydiff) {
		rnd = SIMPLE_RAND(rnd);

		if (x >= 0 && x < dstw) {
			int32_t y = FIXTOI(centery) - DITHER_WIDTH;

			tx0 = tx - DITHER_WIDTH * dty;
			ty0 = ty + DITHER_WIDTH * dtx;

			uint32_t rnd0 = rnd;

			// dither above the edge
			for (uint32_t i = 0; i < DITHER_WIDTH; i++, y++) {
				if ((rnd0 & DITHER_RAND_MASK) <= i && y >= 0 && y < dsth) {
					T * const pix =
						reinterpret_cast<T *>
						(static_cast<uint8_t *>(dst.get_pixels())
						 +
						 y * dst.get_pitch())
						+
						x;
					*pix =
						tcolormap
							[tpixels
							 	[((tx0 >> 16) &  (TEXTURE_WIDTH  - 1)) |
							 	 ((ty0 >> 10) & ((TEXTURE_HEIGHT - 1) << 6))]
							 |
							 ((b >> 8) & 0xFF00)];
				}

				tx0 += dty;
				ty0 -= dtx;
				rnd0 >>= DITHER_RAND_SHIFT;
			}

			// dither below the edge
			for
				(uint32_t i = 0;
				 i < DITHER_WIDTH;
				 ++i, ++y, tx0 += dty, ty0 -= dtx, rnd0 >>= DITHER_RAND_SHIFT)
				if
					((rnd0 & DITHER_RAND_MASK) >= i + DITHER_WIDTH &&
					 y >= 0 && y < dsth)
					reinterpret_cast<T *>
						(static_cast<uint8_t *>(dst.get_pixels())
						 +
						 y * dst.get_pitch())
						[x]
						=
						bcolormap
							[bpixels
							 	[((tx0 >> 16) &  (TEXTURE_WIDTH  - 1)) |
							 	 ((ty0 >> 10) & ((TEXTURE_HEIGHT - 1) << 6))]
							 |
							 ((b >> 8) & 0xFF00)];
		}

		tx += dtx;
		ty += dty;
		b  += db;
	}
}

/**
 * \see dither_edge_horiz
 */
template<typename T> static void dither_edge_vert
	(SDLSurface & dst,
	 const Vertex & start, const Vertex & end,
	 const Texture & ltex, const Texture & rtex)
{
	uint8_t * lpixels, * rpixels;
	T * lcolormap, * rcolormap;

	lpixels = ltex.get_curpixels();
	lcolormap = static_cast<T *>(ltex.get_colormap());
	rpixels = rtex.get_curpixels();
	rcolormap = static_cast<T *>(rtex.get_colormap());

	int32_t tx, ty, b, dtx, dty, db, tx0, ty0;

	tx  =  ITOFIX(start.tx);
	ty  =  ITOFIX(start.ty);
	b   =  ITOFIX(start.b);
	dtx = (ITOFIX(end.tx) - tx) / (end.y - start.y + 1);
	dty = (ITOFIX(end.ty) - ty) / (end.y - start.y + 1);
	db  = (ITOFIX(end.b)  - b)  / (end.y - start.y + 1);

	// TODO: seed this depending on field coordinates
	uint32_t rnd = 0;

	const int32_t dstw = dst.width();
	const int32_t dsth = dst.height();

	int32_t xdiff = ITOFIX(end.x - start.x) / (end.y - start.y);
	int32_t centerx = ITOFIX(start.x);

	for (int32_t y = start.y; y < end.y; y++, centerx += xdiff) {
		rnd = SIMPLE_RAND(rnd);

		if (y >= 0 && y < dsth) {
			int32_t x = FIXTOI(centerx) - DITHER_WIDTH;

			tx0 = tx - DITHER_WIDTH * dty;
			ty0 = ty + DITHER_WIDTH * dtx;

			uint32_t rnd0 = rnd;

			// dither on left side
			for
				(uint32_t i = 0;
				 i < DITHER_WIDTH;
				 ++i, ++x, tx0 += dty, ty0 -= dtx, rnd0 >>= DITHER_RAND_SHIFT)
				if ((rnd0 & DITHER_RAND_MASK) <= i && x >= 0 && x < dstw)
					reinterpret_cast<T *>
						(static_cast<Uint8 *>(dst.get_pixels())
						 +
						 y * dst.get_pitch())
						[x]
						=
						lcolormap
							[lpixels
							 	[((tx0 >> 16) &  (TEXTURE_WIDTH  - 1)) |
							 	 ((ty0 >> 10) & ((TEXTURE_HEIGHT - 1) << 6))]
							 |
							 ((b >> 8) & 0xFF00)];

			// dither on right side
			for
				(uint32_t i = 0;
				 i < DITHER_WIDTH;
				 ++i, ++x, tx0 += dty, ty0 -= dtx, rnd0 >>= DITHER_RAND_SHIFT)
				if
					((rnd0 & DITHER_RAND_MASK) >= i + DITHER_WIDTH
					 &&
					 x >= 0 && x < dstw)
					reinterpret_cast<T *>
						(static_cast<Uint8 *>(dst.get_pixels())
						 +
						 y * dst.get_pitch())
						[x]
						=
						rcolormap
							[rpixels
							 	[((tx0 >> 16) & (TEXTURE_WIDTH - 1)) |
							 	 ((ty0 >> 10) & ((TEXTURE_HEIGHT - 1) << 6))]
							 	|
							 	((b >> 8) & 0xFF00)];
		}

		tx += dtx;
		ty += dty;
		b  += db;
	}
}

template<typename T> static void render_road_horiz
	(SDLSurface & dst, Point const start, Point const end, const SDLSurface & src)
{
	int32_t const dstw = dst.width();
	int32_t const dsth = dst.height();

	int32_t const ydiff = ((end.y - start.y) << 16) / (end.x - start.x);
	int32_t centery = start.y << 16;

	for (int32_t x = start.x, sx = 0; x < end.x; ++x, centery += ydiff, ++sx) {
		if (x < 0 || x >= dstw)
			continue;

		for (int32_t i = 0, y = (centery >> 16) - 2; i < 5; ++i, ++y)
			if (0 < y and y < dsth)
				reinterpret_cast<T *>
					(static_cast<uint8_t *>(dst.get_pixels()) + y * dst.get_pitch())
					[x]
					=
					reinterpret_cast<T const *>
						(static_cast<uint8_t const *>(src.get_pixels())
						 +
						 i * src.get_pitch())
						[sx];
	}
}

template<typename T> static void render_road_vert
	(SDLSurface & dst, Point const start, Point const end, const SDLSurface & src)
{
	int32_t const dstw = dst.width();
	int32_t const dsth = dst.height();

	int32_t const xdiff = ((end.x - start.x) << 16) / (end.y - start.y);
	int32_t centerx = start.x << 16;

	for (int32_t y = start.y, sy = 0; y < end.y; ++y, centerx += xdiff, ++sy) {
		if (y < 0 || y >= dsth)
			continue;

		for (int32_t i = 0, x = (centerx >> 16) - 2; i < 5; ++i, ++x)
			if (0 < x and x < dstw)
				reinterpret_cast<T *>
					(static_cast<uint8_t *>(dst.get_pixels()) + y * dst.get_pitch())
					[x]
					=
					reinterpret_cast<T const *>
						(static_cast<uint8_t const *>(src.get_pixels())
						 +
						 sy * src.get_pitch())
						[i];
	}
}

template<typename T> static void draw_field_int
	(SDLSurface       & dst,
	 const Vertex  &  f_vert,
	 const Vertex  &  r_vert,
	 const Vertex  & bl_vert,
	 const Vertex  & br_vert,
	 uint8_t         roads,
	 const Texture & tr_d_texture,
	 const Texture &  l_r_texture,
	 const Texture &  f_d_texture,
	 const Texture &  f_r_texture)
{
	SDLSurface& rt_busy = static_cast<SDLSurface&>(g_gr->get_road_texture(Widelands::Road_Busy));
	SDLSurface& rt_normal = static_cast<SDLSurface&>(g_gr->get_road_texture(Widelands::Road_Normal));

	dst.lock(Surface::Lock_Normal);

	render_triangle<T> (dst, f_vert, br_vert, r_vert, f_r_texture);
	render_triangle<T> (dst, f_vert, bl_vert, br_vert, f_d_texture);

	// Render roads and dither polygon edges
	uint8_t road;

	road = (roads >> Widelands::Road_East) & Widelands::Road_Mask;
	if (-128 < f_vert.b or -128 < r_vert.b) {
		if (road) {
			switch (road) {
			case Widelands::Road_Normal:
				render_road_horiz<T> (dst, f_vert, r_vert, rt_normal);
				break;
			case Widelands::Road_Busy:
				render_road_horiz<T> (dst, f_vert, r_vert, rt_busy);
				break;
			default:
				assert(false);
				break;
			}
		} else if (&f_r_texture != &tr_d_texture) {
			dither_edge_horiz<T>(dst, f_vert, r_vert, f_r_texture, tr_d_texture);
		}
	}

	road = (roads >> Widelands::Road_SouthEast) & Widelands::Road_Mask;
	if (-128 < f_vert.b or -128 < br_vert.b) {
		if (road) {
			switch (road) {
			case Widelands::Road_Normal:
				render_road_vert<T> (dst, f_vert, br_vert, rt_normal);
				break;
			case Widelands::Road_Busy:
				render_road_vert<T> (dst, f_vert, br_vert, rt_busy);
				break;
			default:
				assert(false);
				break;
			}
		} else if (&f_r_texture != &f_d_texture) {
			dither_edge_vert<T>(dst, f_vert, br_vert, f_r_texture, f_d_texture);
		}
	}

	road = (roads >> Widelands::Road_SouthWest) & Widelands::Road_Mask;
	if (-128 < f_vert.b or -128 < bl_vert.b) {
		if (road) {
			switch (road) {
			case Widelands::Road_Normal:
				render_road_vert<T> (dst, f_vert, bl_vert, rt_normal);
				break;
			case Widelands::Road_Busy:
				render_road_vert<T> (dst, f_vert, bl_vert, rt_busy);
				break;
			default:
				assert(false);
				break;
			}
		} else if (&l_r_texture != &f_d_texture) {
			dither_edge_vert<T>(dst, f_vert, bl_vert, f_d_texture, l_r_texture);
		}
	}

	dst.unlock(Surface::Unlock_Update);

	// FIXME: similar textures may not need dithering
}

#endif
