/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

/*
Texture implementation and terrain rendering for the 32-bit software renderer.
*/

#include "constants.h"
#include "error.h"
#include "field.h"
#include "filesystem.h"
#include "sw32_graphic.h"
#include "wexception.h"
#include "world.h"
#include "random.h"

using namespace std;

namespace Renderer_Software32
{


/*
==========================================================================

Colormap

==========================================================================
*/

/** Colormap::Colormap
 *
 * Create a new Colormap, taking the palette as a parameter.
 * It automatically creates the rgbmap and the colormap for
 * 32-to-8-bit conversion and shading.
 */
Colormap::Colormap (const SDL_Color *pal)
{
   int i,j,r,g,b;

	memcpy(palette, pal, sizeof(palette));

   colormap = new unsigned int[65536];

//    log ("Creating color map\n");
	for (i=0;i<256;i++)
		for (j=0;j<256;j++) {
			// Old shading method
			int shade=(j<128)?j:(j-256);
			shade=256+2*shade;

			r = (palette[i].r*shade)>>8;
			g = (palette[i].g*shade)>>8;
			b = (palette[i].b*shade)>>8;

			if (r>255) r=255;
			if (g>255) g=255;
			if (b>255) b=255;

			colormap[(j<<8) | i] = (r<<16) | (g<<8) | b;
		}
}


/** Colormap::~Colormap
 *
 * Clean up.
 */
Colormap::~Colormap ()
{
	delete[] colormap;
}


/*
==========================================================================

Texture

==========================================================================
*/

/** Texture::Texture
 *
 * Create a texture, taking the pixel data from a Pic.
 */
Texture::Texture (const char* fnametmpl, uint frametime)
{
	m_colormap = 0;
	m_nrframes = 0;
	m_pixels = 0;
	m_frametime = frametime;

	// Load the pictures one by one
	char fname[256];

	for(;;) {
		int nr = m_nrframes;
		char *p;

		// create the file name by reverse-scanning for '?' and replacing
		snprintf(fname, sizeof(fname), "%s", fnametmpl);
		p = fname + strlen(fname);
		while(p > fname) {
			if (*--p != '?')
				continue;

			*p = '0' + (nr % 10);
			nr = nr / 10;
		}

		if (nr) // cycled up to maximum possible frame number
			break;

		// is the frame actually there?
		if (!g_fs->FileExists(fname))
			break;

		// Load it
		SDL_Surface* surf;

      m_texture_picture =fname;

		try
		{
			surf = LoadImage(fname);
		}
		catch(std::exception& e)
		{
			log("WARNING: Failed to load texture frame %s: %s\n", fname, e.what());
			break;
		}

		if (surf->w != TEXTURE_W || surf->h != TEXTURE_H) {
			SDL_FreeSurface(surf);
			log("WARNING: %s: texture must be %ix%i pixels big\n", fname, TEXTURE_W, TEXTURE_H);
			break;
		}

		// Determine color map if it's the first frame
		if (!m_nrframes) {
			if (surf->format->BitsPerPixel == 8)
				m_colormap = new Colormap(surf->format->palette->colors);
			else {
				SDL_Color pal[256];

				log("WARNING: %s: using 332 default palette\n", fname);

				for (int r=0;r<8;r++)
					for (int g=0;g<8;g++)
						for (int b=0;b<4;b++) {
							pal[(r<<5) | (g<<2) | b].r=r<<5;
							pal[(r<<5) | (g<<2) | b].g=g<<5;
							pal[(r<<5) | (g<<2) | b].b=b<<6;
						}

				m_colormap = new Colormap(pal);
			}
		}

		// Convert to our palette
		SDL_Palette palette;
		SDL_PixelFormat fmt;

		palette.ncolors = 256;
		palette.colors = m_colormap->get_palette();

		memset(&fmt, 0, sizeof(fmt));
		fmt.BitsPerPixel = 8;
		fmt.BytesPerPixel = 1;
		fmt.palette = &palette;

		SDL_Surface* cv = SDL_ConvertSurface(surf, &fmt, 0);

		// Add the frame
		m_pixels = (uchar*)realloc(m_pixels, TEXTURE_W*TEXTURE_H*(m_nrframes+1));
		m_curframe = &m_pixels[TEXTURE_W*TEXTURE_H*m_nrframes];
		m_nrframes++;

		SDL_LockSurface(cv);

		for(int y = 0; y < TEXTURE_H; y++)
			memcpy(m_curframe + y*TEXTURE_W, (Uint8*)cv->pixels + y*cv->pitch, TEXTURE_W);

		SDL_UnlockSurface(cv);

		SDL_FreeSurface(cv);
		SDL_FreeSurface(surf);
	}

	if (!m_nrframes)
		throw wexception("%s: texture has no frames", fnametmpl);
}


/** Texture::~Texture
 *
 * Clean up.
 */
Texture::~Texture ()
{
	if (m_colormap)
		delete m_colormap;
	if (m_pixels)
		free(m_pixels);
}


/*
===============
Texture::get_minimap_color

Return the basic terrain colour to be used in the minimap.
===============
*/
uint Texture::get_minimap_color(char shade)
{
	uchar clr = m_pixels[0]; // just use the top-left pixel
	uint table = (uchar)shade;

	return (m_colormap->get_colormap())[clr | (table << 8)];
}


/** Texture::animate
 *
 * Set the current frame according to the game time.
 */
void Texture::animate(uint time)
{
	int frame = (time / m_frametime) % m_nrframes;

	m_curframe = &m_pixels[TEXTURE_W*TEXTURE_H*frame];
}

/*
==============================================================================

TERRAIN RENDERING

==============================================================================
*/


struct VEdge {
	Vertex start;
	Vertex end;
	int deltax;		// in 16.16 fixed point
	int deltab;
	int deltatx;
	int deltaty;
};

struct VEdgeBuffer {
	VEdge	edges[3];
	int	numedges;
};


/*
===============
vedge_from_vertices
===============
*/
static bool vedge_from_vertices(VEdge* edge, Vertex p1, Vertex p2)
{
	if (p1.y < p2.y)
	{
		edge->start = p1;
		edge->end = p2;
	}
	else if (p2.y < p1.y)
	{
		edge->start = p2;
		edge->end = p1;
	}
	else
		return false;

	edge->deltax = ((edge->end.x - edge->start.x) << 16) / (edge->end.y - edge->start.y);
	edge->deltab = ((edge->end.b - edge->start.b) << 16) / (edge->end.y - edge->start.y);
	edge->deltatx = ((edge->end.tx - edge->start.tx) << 16) / (edge->end.y - edge->start.y);
	edge->deltaty = ((edge->end.ty - edge->start.ty) << 16) / (edge->end.y - edge->start.y);
	return true;
}


/*
===============
calculate_vedges

Calculate and sort the edges for the triangle given by the three points.
===============
*/
static void calculate_vedges(VEdgeBuffer* buf, const Vertex& p1, const Vertex& p2, const Vertex& p3)
{
	// Build the edges
	buf->numedges = 0;

	if (vedge_from_vertices(&buf->edges[buf->numedges], p1, p2))
		buf->numedges++;
	if (vedge_from_vertices(&buf->edges[buf->numedges], p1, p3))
		buf->numedges++;
	if (vedge_from_vertices(&buf->edges[buf->numedges], p2, p3))
		buf->numedges++;

	// Sort the edges
	for(int base = 0; base < buf->numedges-1; base++) {
		int best = base;
		int besty = buf->edges[base].start.y;

		for(int other = base+1; other < buf->numedges; other++) {
			if (buf->edges[other].start.y < besty) {
				besty = buf->edges[other].start.y;
				best = other;
			}
		}

		if (best != base) {
			VEdge temp;

			memcpy(&temp, &buf->edges[base], sizeof(VEdge));
			memcpy(&buf->edges[base], &buf->edges[best], sizeof(VEdge));
			memcpy(&buf->edges[best], &temp, sizeof(VEdge));
		}
	}

	assert(buf->numedges != 1);
}


/*
===============
render_triangle

Render a triangle using a texture, taking brightness into account
===============
*/
static void render_triangle(Bitmap* dst, const Vertex& p1, const Vertex& p2,
									const Vertex& p3, Texture* tex)
{
	uchar* texpixels = tex->get_curpixels();
	uint* texcolormap = tex->get_colormap();
	VEdgeBuffer edgebuf;

	calculate_vedges(&edgebuf, p1, p2, p3);

	if (!edgebuf.numedges)
		return;

	if (edgebuf.edges[edgebuf.numedges-1].end.y < 0)
		return;

	// render loop
	VEdge* leftedge;
	VEdge* rightedge;
	int nextedgeidx;
	int y;
	int leftx, rightx; // in 16.16 fixed point
	int leftb, rightb;
	int lefttx, righttx;
	int leftty, rightty;

	// Figure out the starting edges and starting position
	leftedge = &edgebuf.edges[0];
	rightedge = &edgebuf.edges[1];
	if (leftedge->start.x > rightedge->start.x)
		std::swap(leftedge, rightedge);
	else if (leftedge->start.x == rightedge->start.x) {
		if (leftedge->deltax > rightedge->deltax)
			std::swap(leftedge, rightedge);
	}

	y = leftedge->start.y;
	leftx = leftedge->start.x << 16; // note: in some cases, leftx != rightx here
	rightx = rightedge->start.x << 16;
	leftb = leftedge->start.b << 16;
	rightb = rightedge->start.b << 16;
	lefttx = leftedge->start.tx << 16;
	righttx = rightedge->start.tx << 16;
	leftty = leftedge->start.ty << 16;
	rightty = rightedge->start.ty << 16;

	nextedgeidx = 2;

	// Loop line by line
	for(;;) {
		if (y >= dst->h)
			break;

		// Draw the scanline
		if (y >= 0 && y < dst->h) {
			uint* pix;
			int lx = leftx >> 16;
			int rx = rightx >> 16;
			int w = rx - lx;

			if (w > 0 && lx < dst->w && rx > 0)
			{
				int b = leftb;
				int tx = lefttx;
				int ty = leftty;
				int deltab = (rightb - leftb) / w;
				int deltatx = (righttx - lefttx) / w;
				int deltaty = (rightty - leftty) / w;

				if (lx < 0) {
					b -= lx * deltab;
					tx -= lx * deltatx;
					ty -= lx * deltaty;
					lx = 0;
				}
				if (rx > dst->w) rx = dst->w;

				pix = dst->pixels + y * dst->pitch + lx;
				w = rx - lx;

				for(lx = 0; lx < w; lx++) {
					int itx = (tx>>16) & (TEXTURE_W-1);
					int ity = (ty>>16) & (TEXTURE_H-1);

					pix[lx] = texcolormap[texpixels[itx+(ity<<6)] | ((b>>8) & 0xFF00)];

					b += deltab;
					tx += deltatx;
					ty += deltaty;
				}
			}
		}

		// Advance
		leftx += leftedge->deltax;
		rightx += rightedge->deltax;
		leftb += leftedge->deltab;
		rightb += rightedge->deltab;
		lefttx += leftedge->deltatx;
		righttx += rightedge->deltatx;
		leftty += leftedge->deltaty;
		rightty += rightedge->deltaty;
		y++;

		if (y == leftedge->end.y) {
			if (nextedgeidx >= edgebuf.numedges)
				break;

			leftedge = &edgebuf.edges[nextedgeidx++];
			leftx = leftedge->start.x << 16;
			leftb = leftedge->start.b << 16;
			lefttx = leftedge->start.tx << 16;
			leftty = leftedge->start.ty << 16;

			assert(y == leftedge->start.y);
		}
		if (y == rightedge->end.y) {
			if (nextedgeidx >= edgebuf.numedges)
				break;

			rightedge = &edgebuf.edges[nextedgeidx++];
			rightx = rightedge->start.x << 16;
			rightb = rightedge->start.b << 16;
			righttx = rightedge->start.tx << 16;
			rightty = rightedge->start.ty << 16;

			assert(y == rightedge->start.y);
		}
	}
}


struct Edge {
	Point start;
	Point end;
	int deltax;		// in 16.16 fixed point
};

struct EdgeBuffer {
	Edge	edges[3];
	int	numedges;
};


/*
===============
edge_from_points
===============
*/
static bool edge_from_points(Edge* edge, Point p1, Point p2)
{
	if (p1.y < p2.y)
	{
		edge->start = p1;
		edge->end = p2;
	}
	else if (p2.y < p1.y)
	{
		edge->start = p2;
		edge->end = p1;
	}
	else
		return false;

	edge->deltax = ((edge->end.x - edge->start.x) << 16) / (edge->end.y - edge->start.y);
	return true;
}


/*
===============
calculate_edges

Calculate and sort the edges for the triangle given by the three points.
===============
*/
static void calculate_edges(EdgeBuffer* buf, Point p1, Point p2, Point p3)
{
	// Build the edges
	buf->numedges = 0;

	if (edge_from_points(&buf->edges[buf->numedges], p1, p2))
		buf->numedges++;
	if (edge_from_points(&buf->edges[buf->numedges], p1, p3))
		buf->numedges++;
	if (edge_from_points(&buf->edges[buf->numedges], p2, p3))
		buf->numedges++;

	// Sort the edges
	for(int base = 0; base < buf->numedges-1; base++) {
		int best = base;
		int besty = buf->edges[base].start.y;

		for(int other = base+1; other < buf->numedges; other++) {
			if (buf->edges[other].start.y < besty) {
				besty = buf->edges[other].start.y;
				best = other;
			}
		}

		if (best != base) {
			Edge temp;

			memcpy(&temp, &buf->edges[base], sizeof(Edge));
			memcpy(&buf->edges[base], &buf->edges[best], sizeof(Edge));
			memcpy(&buf->edges[best], &temp, sizeof(Edge));
		}
	}

	assert(buf->numedges != 1);
}


/*
===============
fill_triangle

Fill a triangle with a single color
===============
*/
static void fill_triangle(Bitmap* dst, Point p1, Point p2, Point p3, uint color)
{
	EdgeBuffer edgebuf;

	calculate_edges(&edgebuf, p1, p2, p3);
/*
	log("numedges = %i (%i, %i) (%i, %i) (%i, %i)\n", edgebuf.numedges,
		p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);

	for(int i = 0; i < edgebuf.numedges; i++) {
		Edge* edge = &edgebuf.edges[i];
		log("  (%i, %i) (%i, %i) %i\n", edge->start.x, edge->start.y, edge->end.x, edge->end.y,
					edge->deltax);
	}
*/
	if (!edgebuf.numedges)
		return;

	if (edgebuf.edges[edgebuf.numedges-1].end.y < 0)
		return;

	// render loop
	Edge* leftedge;
	Edge* rightedge;
	int nextedgeidx;
	int y;
	int leftx, rightx; // in 16.16 fixed point

	// Figure out the starting edges and starting position
	leftedge = &edgebuf.edges[0];
	rightedge = &edgebuf.edges[1];
	if (leftedge->start.x > rightedge->start.x)
		std::swap(leftedge, rightedge);
	else if (leftedge->start.x == rightedge->start.x) {
		if (leftedge->deltax > rightedge->deltax)
			std::swap(leftedge, rightedge);
	}

	y = leftedge->start.y;
	leftx = leftedge->start.x << 16; // note: in some cases, leftx != rightx here
	rightx = rightedge->start.x << 16;

	nextedgeidx = 2;

	// Loop line by line
	for(;;) {
		if (y >= dst->h)
			break;

		// Draw the scanline
		if (y >= 0 && y < dst->h) {
			uint* pix;
			int lx = leftx >> 16;
			int rx = rightx >> 16;
			int w;

			if (lx < 0) lx = 0;
			if (lx > dst->w) lx = dst->w;
			if (rx < 0) rx = 0;
			if (rx > dst->w) rx = dst->w;

			pix = dst->pixels + y * dst->pitch + lx;
			w = rx - lx;

			for(lx = 0; lx < w; lx++)
				pix[lx] = color;
		}

		// Advance
		leftx += leftedge->deltax;
		rightx += rightedge->deltax;
		y++;

		if (y == leftedge->end.y) {
			if (nextedgeidx >= edgebuf.numedges)
				break;

			leftedge = &edgebuf.edges[nextedgeidx++];
			leftx = leftedge->start.x << 16;

			assert(y == leftedge->start.y);
		}
		if (y == rightedge->end.y) {
			if (nextedgeidx >= edgebuf.numedges)
				break;

			rightedge = &edgebuf.edges[nextedgeidx++];
			rightx = rightedge->start.x << 16;

			assert(y == rightedge->start.y);
		}
	}
}


#define ftofix(f) ((int) ((f)*0x10000))
#define itofix(i) ((i)<<16)
#define fixtoi(f) ((f)>>16)

/*  dither_edge_horiz and dither_edge_vert:

    Blur the polygon edge between vertices start and end.
    It is dithered by randomly placing points taken from the
    texture of the adjacent polygon.
    The blend area is a few pixels wide, and the chance for replacing
    a pixel depends on the distance from the center line.
    Texture coordinates and brightness are interpolated across the center
    line (outer loop). To the sides these are approximated (inner loop):
    Brightness is kept constant, and the texture is mapped orthogonally
    to the center line. It is important that only those pixels are drawn
    whose texture actually changes in order to minimize artifacts.

    Note that all this is preliminary and subject to change.
    For example, a special edge texture could be used instead of
    stochastically dithering. Road rendering could be handled as a
    special case then.
*/

#define DITHER_WIDTH		4
// DITHER_WIDTH must be a power of two

#define DITHER_RAND_MASK	(DITHER_WIDTH*2-1)
#define DITHER_RAND_SHIFT	(16/DITHER_WIDTH)

static void dither_edge_horiz (Bitmap* dst, const Vertex& start, const Vertex& end, Texture* ttex, Texture* btex)
{
	unsigned char *tpixels, *bpixels;
	uint *tcolormap, *bcolormap;

	tpixels=ttex->get_curpixels();
	tcolormap=ttex->get_colormap();
	bpixels=btex->get_curpixels();
	bcolormap=btex->get_colormap();

	int tx,ty,b,dtx,dty,db,tx0,ty0;

	tx=itofix(start.tx);
	ty=itofix(start.ty);
	b=itofix(start.b);
	dtx=(itofix(end.tx)-tx) / (end.x-start.x+1);
	dty=(itofix(end.ty)-ty) / (end.x-start.x+1);
	db=(itofix(end.b)-b) / (end.x-start.x+1);

	// TODO: seed this depending on field coordinates
	uint rnd=0;

	int dstw = dst->w;
	int dsth = dst->h;

	int ydiff = itofix(end.y - start.y) / (end.x - start.x);
	int centery = itofix(start.y);

	for(int x = start.x; x < end.x; x++, centery += ydiff) {
		rnd=SIMPLE_RAND(rnd);
		
		if (x>=0 && x<dstw) {
			int y = fixtoi(centery) - DITHER_WIDTH;

			tx0=tx - DITHER_WIDTH*dty;
			ty0=ty + DITHER_WIDTH*dtx;

			unsigned long rnd0=rnd;

			// dither above the edge
			for (unsigned int i = 0; i < DITHER_WIDTH; i++, y++) {
				if ((rnd0&DITHER_RAND_MASK)<=i && y>=0 && y<dsth) {
					uint *pix = dst->pixels + y*dst->pitch + x;
					int texel=((tx0>>16) & (TEXTURE_W-1)) | ((ty0>>10) & ((TEXTURE_H-1)<<6));
					*pix=tcolormap[tpixels[texel] | ((b>>8)&0xFF00)];
				}

				tx0+=dty;
				ty0-=dtx;
				rnd0>>=DITHER_RAND_SHIFT;
			}

			// dither below the edge
			for (unsigned int i = 0; i < DITHER_WIDTH; i++, y++) {
				if ((rnd0&DITHER_RAND_MASK)>=i+DITHER_WIDTH && y>=0 && y<dsth) {
				    uint *pix = dst->pixels + y*dst->pitch + x;
				    int texel=((tx0>>16) & (TEXTURE_W-1)) | ((ty0>>10) & ((TEXTURE_H-1)<<6));
				    *pix=bcolormap[bpixels[texel] | ((b>>8)&0xFF00)];
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


static void dither_edge_vert (Bitmap* dst, const Vertex& start, const Vertex& end, Texture* ltex, Texture* rtex)
{
	unsigned char *lpixels, *rpixels;
	uint *lcolormap, *rcolormap;

	lpixels=ltex->get_curpixels();
	lcolormap=ltex->get_colormap();
	rpixels=rtex->get_curpixels();
	rcolormap=rtex->get_colormap();

	int tx,ty,b,dtx,dty,db,tx0,ty0;

	tx=itofix(start.tx);
	ty=itofix(start.ty);
	b=itofix(start.b);
	dtx=(itofix(end.tx)-tx) / (end.y-start.y+1);
	dty=(itofix(end.ty)-ty) / (end.y-start.y+1);
	db=(itofix(end.b)-b) / (end.y-start.y+1);

	// TODO: seed this depending on field coordinates
	uint rnd=0;

	int dstw = dst->w;
	int dsth = dst->h;

	int xdiff = itofix(end.x - start.x) / (end.y - start.y);
	int centerx = itofix(start.x);

	for(int y = start.y; y < end.y; y++, centerx += xdiff) {
		rnd=SIMPLE_RAND(rnd);
		
		if (y>=0 && y<dsth) {
			int x = fixtoi(centerx) - DITHER_WIDTH;

			tx0=tx - DITHER_WIDTH*dty;
			ty0=ty + DITHER_WIDTH*dtx;

			unsigned long rnd0=rnd;

			// dither on left side
			for (unsigned int i = 0; i < DITHER_WIDTH; i++, x++) {
				if ((rnd0&DITHER_RAND_MASK)<=i && x>=0 && x<dstw) {
					uint *pix = dst->pixels + y*dst->pitch + x;
					int texel=((tx0>>16) & (TEXTURE_W-1)) | ((ty0>>10) & ((TEXTURE_H-1)<<6));
					*pix=lcolormap[lpixels[texel] | ((b>>8)&0xFF00)];
				}

				tx0+=dty;
				ty0-=dtx;
				rnd0>>=DITHER_RAND_SHIFT;
			}

			// dither on right side
			for (unsigned int i = 0; i < DITHER_WIDTH; i++, x++) {
				if ((rnd0 & DITHER_RAND_MASK)>=i+DITHER_WIDTH && x>=0 && x<dstw) {
					uint *pix = dst->pixels + y*dst->pitch + x;
					int texel=((tx0>>16) & (TEXTURE_W-1)) | ((ty0>>10) & ((TEXTURE_H-1)<<6));
					*pix=rcolormap[rpixels[texel] | ((b>>8)&0xFF00)];
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


/*
===============
render_road_horiz
render_road_vert

Render a road. This is really dumb right now, not using a texture
===============
*/
static void render_road_horiz(Bitmap *dst, Point start, Point end, Bitmap* src)
{
	int dstw = dst->w;
	int dsth = dst->h;

	int ydiff = ((end.y - start.y) << 16) / (end.x - start.x);
	int centery = start.y << 16;

	for(int x = start.x, sx = 0; x < end.x; x++, centery += ydiff, sx ++) {
		if (x < 0 || x >= dstw)
			continue;

		int y = (centery >> 16) - 2;

		for(int i = 0; i < 5; i++, y++) {
			if (y < 0 || y >= dsth)
				continue;

	      uint *pix = dst->pixels + y*dst->pitch + x;
			*pix = * (src->pixels + i*src->pitch + sx ) ;
		}
	}
}

static void render_road_vert(Bitmap *dst, Point start, Point end, Bitmap* src)
{
	int dstw = dst->w;
	int dsth = dst->h;

	int xdiff = ((end.x - start.x) << 16) / (end.y - start.y);
	int centerx = start.x << 16;

	for(int y = start.y, sy = 0; y < end.y; y++, centerx += xdiff, sy ++ ) {
		if (y < 0 || y >= dsth)
			continue;

		int x = (centerx >> 16) - 2;

		for(int i = 0; i < 5; i++, x++) {
			if (x < 0 || x >= dstw)
				continue;

			uint *pix = dst->pixels + y*dst->pitch + x;
			*pix = * (src->pixels + sy*src->pitch + i ) ;
		}
	}
}


/*
===============
Bitmap::draw_field

Draw ground textures and roads for the given parallelogram (two triangles)
into the bitmap.
===============
*/
void Bitmap::draw_field(Field * const f, Field * const rf, Field * const fl, Field * const rfl,
			Field * const lf, Field * const ft,
	                const int posx, const int rposx, const int posy,
	                const int blposx, const int rblposx, const int blposy,
	                uchar roads, bool render_r, bool render_b)
{
	Vertex r, l, br, bl;

	r = Vertex(rposx, posy - MULTIPLY_WITH_HEIGHT_FACTOR(rf->get_height()), rf->get_brightness(), 0, 0);
	l = Vertex(posx, posy - MULTIPLY_WITH_HEIGHT_FACTOR(f->get_height()), f->get_brightness(), 63, 0);
	br = Vertex(rblposx, blposy - MULTIPLY_WITH_HEIGHT_FACTOR(rfl->get_height()), rfl->get_brightness(), 0, 63);
	bl = Vertex(blposx, blposy - MULTIPLY_WITH_HEIGHT_FACTOR(fl->get_height()), fl->get_brightness(), 63, 63);

/*
	r.b += 20; // debug override for shading (make field borders visible)
	bl.b -= 20;
*/

//	render_r=false; // debug overwrite: just render b triangle
//	render_b=false; // debug overwrite: just render r triangle

	Texture* rtex = get_graphicimpl()->get_maptexture_data(f->get_terr()->get_texture());
	Texture* btex = get_graphicimpl()->get_maptexture_data(f->get_terd()->get_texture());

   Bitmap *rt_normal= get_graphicimpl()->get_road_texture( Road_Normal );
   Bitmap *rt_busy= get_graphicimpl()->get_road_texture( Road_Busy );

	// Render right triangle
	if(render_r)
	{
		if (rtex)
			render_triangle(this, r, l, br, rtex);
	}
	else
		fill_triangle(this, r, l, br, 0);

	if(render_b)
	{
		if (btex)
			render_triangle(this, l, br, bl, btex);
	}
	else
		fill_triangle(this, l, br, bl, 0);

	Texture* ltex = get_graphicimpl()->get_maptexture_data(lf->get_terr()->get_texture());
	Texture* ttex = get_graphicimpl()->get_maptexture_data(ft->get_terd()->get_texture());

	// Render roads and dither polygon edges
	uchar road;

	road = (roads >> Road_East) & Road_Mask;
	if (render_r) {
      if (road) {
         switch(road) {
            case Road_Normal: render_road_horiz(this, l, r, rt_normal); break; 
            case Road_Busy: render_road_horiz(this, l, r, rt_busy); break;
            default: assert(0); break; // never here
         }
      }
		else if (rtex!=0 && ttex!=0 && rtex!=ttex)
			dither_edge_horiz(this, l, r, rtex, ttex);
	}

	// FIXME: this will try to work on some undiscovered terrain
	road = (roads >> Road_SouthEast) & Road_Mask;
	if (render_r || render_b) {
      if (road) {
         switch(road) {
            case Road_Normal: render_road_vert(this, l, br, rt_normal); break; 
            case Road_Busy: render_road_vert(this, l, br, rt_busy); break;
            default: assert(0); break; // never here
         }
      }
		else if (rtex!=0 && btex!=0 && rtex!=btex)
			dither_edge_vert(this, l, br, rtex, btex);
	}

	road = (roads >> Road_SouthWest) & Road_Mask;
	if (render_b) {
		if (road) {
         switch(road) {
            case Road_Normal: render_road_vert(this, l, bl, rt_normal); break; 
            case Road_Busy: render_road_vert(this, l, bl, rt_busy); break;
            default: assert(0); break; // never here
         }
      }
			else if (ltex!=0 && btex!=0 && ltex!=btex)
			dither_edge_vert(this, l, bl, btex, ltex);
	}

	// FIXME: similar textures may not need dithering
}


} // namespace Renderer_Software32
