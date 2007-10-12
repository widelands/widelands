/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef TERRAIN_H
#define TERRAIN_H

#include "graphic.h"
#include "mapviewpixelconstants.h"
#include "random.h"
#include "surface.h"

#include "vertex.h"

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
(int32_t u1, int32_t u2, int32_t v1, int32_t v2, float& lambda, float& mu);

template<typename T> static void render_top_triangle
(Surface & dst,
 const Texture & tex,
 Vertex & p1, Vertex & p2, Vertex & p3,
 int32_t y2)
{
	int32_t y, y1, ix1, ix2, count;
	int32_t x1, x2, dx1, dx2;
	int32_t b1, db1, tx1, dtx1, ty1, dty1;
	int32_t b, tx, ty;
	float lambda, mu;
	uint8_t *texpixels;
	T *texcolormap;

	get_horiz_linearcomb
		(p2.x - p1.x, p2.y - p1.y, p3.x - p1.x, p3.y - p1.y, lambda, mu);
	const int32_t db =  FTOFIX((p2.b  - p1.b)  * lambda + (p3.b  - p1.b)  * mu);
	const int32_t dtx = FTOFIX((p2.tx - p1.tx) * lambda + (p3.tx - p1.tx) * mu);
	const int32_t dty = FTOFIX((p2.ty - p1.ty) * lambda + (p3.ty - p1.ty) * mu);

	const int32_t w = dst.get_w();
	const int32_t h = dst.get_h();

	texpixels = tex.get_curpixels();
	texcolormap = static_cast<T *>(tex.get_colormap());

	y1 = p1.y;

	x1 = x2 =  ITOFIX(p1.x);
	dx1     = (ITOFIX(p2.x) - x1)   / (p2.y - y1);
	dx2     = (ITOFIX(p3.x) - x1)   / (p3.y - y1);

	b1      =  ITOFIX(p1.b);
	db1     = (ITOFIX(p2.b) - b1)   / (p2.y - y1);

	tx1     =  ITOFIX(p1.tx);
	dtx1    = (ITOFIX(p2.tx) - tx1) / (p2.y - y1);

	ty1     =  ITOFIX(p1.ty);
	dty1    = (ITOFIX(p2.ty) - ty1) / (p2.y - y1);

	for (y = y1; y < y2 && y < h; ++y) {
		if (y>=0) {
			ix1=FIXTOI(x1);
			ix2=FIXTOI(x2);

			b=b1;
			tx=tx1;
			ty=ty1;

			if (ix2>w) ix2=w;
			if (ix1<0) {
				b-=ix1*db;
				tx-=ix1*dtx;
				ty-=ix1*dty;
				ix1=0;
			}

			count=ix2-ix1;

			T * scanline =
				reinterpret_cast<T *>
				(static_cast<Uint8 *>(dst.get_pixels())
				 +
				 y * dst.get_pitch())
				+
				ix1;

			while (count-->0) {
				int32_t texel=((tx>>16) & (TEXTURE_WIDTH-1)) | ((ty>>10) & ((TEXTURE_HEIGHT-1)<<6));

				*scanline++ = texcolormap[texpixels[texel] | ((b >> 8) & 0xFF00)];

				b+=db;
				tx+=dtx;
				ty+=dty;
			}
		}

		x1+=dx1;
		x2+=dx2;
		b1+=db1;
		tx1+=dtx1;
		ty1+=dty1;
	}
}

template<typename T> static void render_bottom_triangle
(Surface & dst,
 const Texture & tex,
 Vertex & p1, Vertex & p2, Vertex & p3,
 int32_t y1)
{
	int32_t y, y2, ix1, ix2, count;
	int32_t x1, x2, dx1, dx2;
	int32_t b1, db1, tx1, dtx1, ty1, dty1;
	int32_t b, tx, ty;
	float lambda, mu;
	uint8_t *texpixels;
	T *texcolormap;

	get_horiz_linearcomb
		(p2.x - p1.x, p2.y - p1.y, p3.x - p1.x, p3.y - p1.y, lambda, mu);
	const int32_t db  = FTOFIX((p2.b  - p1.b)  * lambda + (p3.b  - p1.b)  * mu);
	const int32_t dtx = FTOFIX((p2.tx - p1.tx) * lambda + (p3.tx - p1.tx) * mu);
	const int32_t dty = FTOFIX((p2.ty - p1.ty) * lambda + (p3.ty - p1.ty) * mu);

	const int32_t w = dst.get_w();
	const int32_t h = dst.get_h();

	texpixels = tex.get_curpixels();
	texcolormap = static_cast<T *>(tex.get_colormap());

	y2 = p3.y;

	x1 = x2 =   ITOFIX(p3.x);
	dx1     = -(ITOFIX(p1.x) - x1) / (p1.y - y2);
	dx2     = -(ITOFIX(p2.x) - x1) / (p2.y - y2);

	// this may seem redundant but reduces rounding artifacts
	x1      =   ITOFIX(p1.x) + dx1 * (p1.y - y2);
	x2      =   ITOFIX(p2.x) + dx2 * (p2.y - y2);

	b1      =   ITOFIX(p3.b);
	db1     = -(ITOFIX(p1.b) - b1)   / (p1.y - y2);

	tx1     =   ITOFIX(p3.tx);
	dtx1    = -(ITOFIX(p1.tx) - tx1) / (p1.y - y2);

	ty1     =   ITOFIX(p3.ty);
	dty1    = -(ITOFIX(p1.ty) - ty1) / (p1.y - y2);

	for (y = y2; y >= y1 && y > 0; --y) {
		if (y<h) {
			ix1=FIXTOI(x1);
			ix2=FIXTOI(x2);

			b=b1;
			tx=tx1;
			ty=ty1;

			if (ix2>w) ix2=w;
			if (ix1<0) {
				b-=ix1*db;
				tx-=ix1*dtx;
				ty-=ix1*dty;
				ix1=0;
			}

			count=ix2-ix1;

			T * scanline =
				reinterpret_cast<T *>
				(static_cast<Uint8 *>(dst.get_pixels())
				 +
				 y * dst.get_pitch())
				+
				ix1;

			while (count-->0) {
				int32_t texel=((tx>>16) & (TEXTURE_WIDTH-1)) | ((ty>>10) & ((TEXTURE_HEIGHT-1)<<6));

				*scanline++ = texcolormap[texpixels[texel] | ((b >> 8) & 0xFF00)];

				b+=db;
				tx+=dtx;
				ty+=dty;
			}
		}

		x1+=dx1;
		x2+=dx2;
		b1+=db1;
		tx1+=dtx1;
		ty1+=dty1;
	}
}

/**
 * Render a triangle. It is being split into to triangles which have one
 * horizontal edge, so that the resulting triangles have only two edges
 * which are not horizontal, one on the left, the other on the right.
 * The actual rendering is performed by render_top_triangle and
 * render_bottom_triangle, which require a horizontal edge at the bottom
 * or at the top, respectively.
 */
template<typename T> static void render_triangle
(Surface & dst, Vertex & p1, Vertex & p2, Vertex & p3, const Texture & tex)
{
	Vertex * p[3]= {&p1, &p2, &p3};
	int32_t top, bot, mid, y, ym;

	top=bot=0; // to avoid compiler warning

	y=0x7fffffff;
	for (uint8_t i = 0; i < 3; ++i)
		if (p[i]->y<y) {top=i; y=p[i]->y;}

	y=-0x7fffffff;
	for (uint8_t i = 0; i < 3; ++i)
		if (p[i]->y>y) {bot=i; y=p[i]->y;}

	for (mid = 0; mid == top || mid == bot; ++mid);
	ym=p[mid]->y;

	if (p[top]->y < ym) {
		if (p[mid]->x < p[bot]->x)
			render_top_triangle<T>    (dst, tex, *p[top], *p[mid], *p[bot], ym);
		else
			render_top_triangle<T>    (dst, tex, *p[top], *p[bot], *p[mid], ym);
	}

	if (ym < p[bot]->y) {
		if (p[mid]->x < p[top]->x)
			render_bottom_triangle<T> (dst, tex, *p[mid], *p[top], *p[bot], ym);
		else
			render_bottom_triangle<T> (dst, tex, *p[top], *p[mid], *p[bot], ym);
	}
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
(Surface & dst,
 const Vertex & start, const Vertex & end,
 const Texture & ttex, const Texture & btex)
{
	uint8_t *tpixels, *bpixels;
	T *tcolormap, *bcolormap;

	tpixels = ttex.get_curpixels();
	tcolormap = static_cast<T *>(ttex.get_colormap());
	bpixels = btex.get_curpixels();
	bcolormap = static_cast<T *>(btex.get_colormap());

	int32_t tx, ty, b, dtx, dty, db, tx0, ty0;

	tx=ITOFIX(start.tx);
	ty=ITOFIX(start.ty);
	b=ITOFIX(start.b);
	dtx=(ITOFIX(end.tx)-tx) / (end.x-start.x+1);
	dty=(ITOFIX(end.ty)-ty) / (end.x-start.x+1);
	db=(ITOFIX(end.b)-b) / (end.x-start.x+1);

	// TODO: seed this depending on field coordinates
	uint32_t rnd=0;

	const int32_t dstw = dst.get_w();
	const int32_t dsth = dst.get_h();

	int32_t ydiff = ITOFIX(end.y - start.y) / (end.x - start.x);
	int32_t centery = ITOFIX(start.y);

	for (int32_t x = start.x; x < end.x; x++, centery += ydiff) {
		rnd=SIMPLE_RAND(rnd);

		if (x>=0 && x<dstw) {
			int32_t y = FIXTOI(centery) - DITHER_WIDTH;

			tx0=tx - DITHER_WIDTH*dty;
			ty0=ty + DITHER_WIDTH*dtx;

			uint32_t rnd0=rnd;

			// dither above the edge
			for (uint32_t i = 0; i < DITHER_WIDTH; i++, y++) {
				if ((rnd0&DITHER_RAND_MASK)<=i && y>=0 && y<dsth) {
					T * const pix = (T*) ((uint8_t*)dst.get_pixels() + y*dst.get_pitch()) + x;
					int32_t texel=((tx0>>16) & (TEXTURE_WIDTH-1)) | ((ty0>>10) & ((TEXTURE_HEIGHT-1)<<6));
					*pix = tcolormap[tpixels[texel] | ((b >> 8) & 0xFF00)];
				}

				tx0+=dty;
				ty0-=dtx;
				rnd0>>=DITHER_RAND_SHIFT;
			}

			// dither below the edge
			for (uint32_t i = 0; i < DITHER_WIDTH; i++, y++) {
				if ((rnd0&DITHER_RAND_MASK)>=i+DITHER_WIDTH && y>=0 && y<dsth) {
					T * const pix = (T*) ((uint8_t*)dst.get_pixels() + y*dst.get_pitch()) + x;
					int32_t texel=((tx0>>16) & (TEXTURE_WIDTH-1)) | ((ty0>>10) & ((TEXTURE_HEIGHT-1)<<6));
					*pix = bcolormap[bpixels[texel] | ((b >> 8) & 0xFF00)];
				}

				tx0+=dty;
				ty0-=dtx;
				rnd0>>=DITHER_RAND_SHIFT;
			}
		}

		tx+=dtx;
		ty+=dty;
		b+=db;
	}
}

/**
 * \see dither_edge_horiz
 */
template<typename T> static void dither_edge_vert
(Surface & dst,
 const Vertex & start, const Vertex & end,
 const Texture & ltex, const Texture & rtex)
{
	uint8_t *lpixels, *rpixels;
	T* lcolormap, *rcolormap;

	lpixels = ltex.get_curpixels();
	lcolormap = static_cast<T *>(ltex.get_colormap());
	rpixels = rtex.get_curpixels();
	rcolormap = static_cast<T *>(rtex.get_colormap());

	int32_t tx, ty, b, dtx, dty, db, tx0, ty0;

	tx=ITOFIX(start.tx);
	ty=ITOFIX(start.ty);
	b=ITOFIX(start.b);
	dtx=(ITOFIX(end.tx)-tx) / (end.y-start.y+1);
	dty=(ITOFIX(end.ty)-ty) / (end.y-start.y+1);
	db=(ITOFIX(end.b)-b) / (end.y-start.y+1);

	// TODO: seed this depending on field coordinates
	uint32_t rnd=0;

	const int32_t dstw = dst.get_w();
	const int32_t dsth = dst.get_h();

	int32_t xdiff = ITOFIX(end.x - start.x) / (end.y - start.y);
	int32_t centerx = ITOFIX(start.x);

	for (int32_t y = start.y; y < end.y; y++, centerx += xdiff) {
		rnd=SIMPLE_RAND(rnd);

		if (y>=0 && y<dsth) {
			int32_t x = FIXTOI(centerx) - DITHER_WIDTH;

			tx0=tx - DITHER_WIDTH*dty;
			ty0=ty + DITHER_WIDTH*dtx;

			uint32_t rnd0=rnd;

			// dither on left side
			for (uint32_t i = 0; i < DITHER_WIDTH; i++, x++) {
				if ((rnd0&DITHER_RAND_MASK)<=i && x>=0 && x<dstw) {
					T * const pix = reinterpret_cast<T *>
						(static_cast<Uint8 *>(dst.get_pixels())
						 +
						 y * dst.get_pitch())
						+
						x;
					int32_t texel=((tx0>>16) & (TEXTURE_WIDTH-1)) | ((ty0>>10) & ((TEXTURE_HEIGHT-1)<<6));
					*pix = lcolormap[lpixels[texel] | ((b >> 8) & 0xFF00)];
				}

				tx0+=dty;
				ty0-=dtx;
				rnd0>>=DITHER_RAND_SHIFT;
			}

			// dither on right side
			for (uint32_t i = 0; i < DITHER_WIDTH; i++, x++) {
				if ((rnd0 & DITHER_RAND_MASK)>=i+DITHER_WIDTH && x>=0 && x<dstw) {
					T * const pix = reinterpret_cast<T *>
						(static_cast<Uint8 *>(dst.get_pixels())
						 +
						 y * dst.get_pitch())
						+
						x;
					int32_t texel=((tx0>>16) & (TEXTURE_WIDTH-1)) | ((ty0>>10) & ((TEXTURE_HEIGHT-1)<<6));
					*pix = rcolormap[rpixels[texel] | ((b >> 8) & 0xFF00)];
				}

				tx0+=dty;
				ty0-=dtx;
				rnd0>>=DITHER_RAND_SHIFT;
			}
		}

		tx+=dtx;
		ty+=dty;
		b+=db;
	}
}

template<typename T> static void render_road_horiz
(Surface & dst, const Point start, const Point end, const Surface & src)
{
	int32_t dstw = dst.get_w();
	int32_t dsth = dst.get_h();

	int32_t ydiff = ((end.y - start.y) << 16) / (end.x - start.x);
	int32_t centery = start.y << 16;

	for (int32_t x = start.x, sx = 0; x < end.x; x++, centery += ydiff, sx ++) {
		if (x < 0 || x >= dstw)
			continue;

		int32_t y = (centery >> 16) - 2;

		for (int32_t i = 0; i < 5; i++, y++) if (0 < y and y < dsth)
			*(reinterpret_cast<T *>
			  (static_cast<uint8_t *>(dst.get_pixels()) + y * dst.get_pitch())
			  +
			  x)
			=
			*(reinterpret_cast<const T *>
			  (static_cast<const uint8_t *>(src.get_pixels())
			   +
			   i * src.get_pitch())
			  +
			  sx);
	}
}

template<typename T> static void render_road_vert
(Surface & dst, const Point start, const Point end, const Surface & src)
{
	int32_t dstw = dst.get_w();
	int32_t dsth = dst.get_h();

	int32_t xdiff = ((end.x - start.x) << 16) / (end.y - start.y);
	int32_t centerx = start.x << 16;

	for (int32_t y = start.y, sy = 0; y < end.y; y++, centerx += xdiff, sy ++) {
		if (y < 0 || y >= dsth)
			continue;

		int32_t x = (centerx >> 16) - 2;

		for (int32_t i = 0; i < 5; i++, x++) if (0 < x and x < dstw)
			*(reinterpret_cast<T *>
			  (static_cast<uint8_t *>(dst.get_pixels()) +  y * dst.get_pitch())
			  +
			  x)
			=
			*(reinterpret_cast<const T *>
			  (static_cast<const uint8_t *>(src.get_pixels())
			   +
			   sy * src.get_pitch())
			  +
			  i);
	}
}

template<typename T> static void draw_field_int
(Surface & dst,
 Field * const f,
 Field * const r,
 Field * const bl,
 Field * const br,
 const int32_t     posx,
 const int32_t     rposx,
 const int32_t     posy,
 const int32_t     blposx,
 const int32_t     rblposx,
 const int32_t     blposy,
 uint8_t         roads,
 Sint8            f_brightness,
 Sint8            r_brightness,
 Sint8           bl_brightness,
 Sint8           br_brightness,
 const Texture & tr_d_texture,
 const Texture &  l_r_texture,
 const Texture &  f_d_texture,
 const Texture &  f_r_texture,
 bool          draw_all)
{
	Vertex  r_vert
		(rposx, posy - r->get_height() * HEIGHT_FACTOR, r_brightness, 0, 0);
	Vertex  f_vert
		(posx, posy - f->get_height() * HEIGHT_FACTOR, f_brightness, 64, 0);
	Vertex br_vert
		(rblposx, blposy - br->get_height() * HEIGHT_FACTOR, br_brightness, 0, 64);
	Vertex bl_vert
		(blposx, blposy - bl->get_height() * HEIGHT_FACTOR, bl_brightness, 64, 64);

	const Surface & rt_normal = *g_gr->get_road_texture(Road_Normal);
	const Surface & rt_busy   = *g_gr->get_road_texture(Road_Busy);

	if (draw_all) {
		render_triangle<T> (dst, r_vert,  f_vert, br_vert, f_r_texture);
		render_triangle<T> (dst, f_vert, br_vert, bl_vert, f_d_texture);
	} else {
		if (f_r_texture.was_animated())
			render_triangle<T> (dst, r_vert,  f_vert, br_vert, f_r_texture);
		if (f_d_texture.was_animated())
			render_triangle<T> (dst, f_vert, br_vert, bl_vert, f_d_texture);
	}

	// Render roads and dither polygon edges
	uint8_t road;

	road = (roads >> Road_East) & Road_Mask;
	if (-128 < f_brightness or -128 < r_brightness) {
		if (road) {
			switch (road) {
			case Road_Normal:
				render_road_horiz<T> (dst, f_vert, r_vert, rt_normal);
				break;
			case Road_Busy:
				render_road_horiz<T> (dst, f_vert, r_vert, rt_busy);
				break;
			default: assert(false);
			}
		}
		else if
			((draw_all
			  or
			  f_r_texture.was_animated() or tr_d_texture.was_animated())
			 and
			 &f_r_texture != &tr_d_texture)
			dither_edge_horiz<T>(dst, f_vert, r_vert, f_r_texture, tr_d_texture);
	}

	road = (roads >> Road_SouthEast) & Road_Mask;
	if (-128 < f_brightness or -128 < br_brightness) {
		if (road) {
			switch (road) {
			case Road_Normal:
				render_road_vert<T> (dst, f_vert, br_vert, rt_normal);
				break;
			case Road_Busy:
				render_road_vert<T> (dst, f_vert, br_vert, rt_busy);
				break;
			default: assert(false);
			}
		}
		else if
			((draw_all or f_r_texture.was_animated() or f_d_texture.was_animated())
			 and
			 &f_r_texture != &f_d_texture)
			dither_edge_vert<T>(dst, f_vert, br_vert, f_r_texture, f_d_texture);
	}

	road = (roads >> Road_SouthWest) & Road_Mask;
	if (-128 < f_brightness or -128 < bl_brightness) {
		if (road) {
			switch (road) {
			case Road_Normal:
				render_road_vert<T> (dst, f_vert, bl_vert, rt_normal);
				break;
			case Road_Busy:
				render_road_vert<T> (dst, f_vert, bl_vert, rt_busy);
				break;
			default: assert(false);
			}
		}
		else if
			((draw_all or f_d_texture.was_animated() or l_r_texture.was_animated())
			 and
			 &l_r_texture != &f_d_texture)
			dither_edge_vert<T>(dst, f_vert, bl_vert, f_d_texture, l_r_texture);
	}

	// FIXME: similar textures may not need dithering
}

#endif
