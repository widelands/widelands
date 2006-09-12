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
/*
Texture implementation and terrain rendering for the 16-bit software renderer.
*/

#include "constants.h"
#include "error.h"
#include "field.h"
#include "filesystem.h"
#include "graphic_impl.h"
#include "layeredfilesystem.h"
#include "mapviewpixelfunctions.h"
#include "random.h"
#include "world.h"
#include "wexception.h"

using namespace std;

/*
==========================================================================

Colormap

==========================================================================
*/

/** Colormap::Colormap
 *
 * Create a new Colormap, taking the palette as a parameter.
 * It automatically creates the colormap for shading.
 */
Colormap::Colormap (const SDL_Color *pal, SDL_PixelFormat* fmt)
{
	int i,j,r,g,b;

	memcpy(palette, pal, sizeof(palette));

   if( fmt->BytesPerPixel == 2)
      colormap=malloc(sizeof(ushort)*65536);
   else if( fmt->BytesPerPixel == 4)
      colormap=malloc(sizeof(ulong)*65536);

//    log ("Creating color map\n");
	for (i=0;i<256;i++)
		for (j=0;j<256;j++) {
			int shade=(j<128)?j:(j-256);
			shade=256+2*shade;

         r = (palette[i].r*shade)>>8;
         g = (palette[i].g*shade)>>8;
         b = (palette[i].b*shade)>>8;

         if (r>255) r=255;
         if (g>255) g=255;
         if (b>255) b=255;

         if( fmt->BytesPerPixel == 2)
            ((ushort*)colormap)[(j<<8) | i]= (ushort)SDL_MapRGB( fmt, r, g, b );
         else if( fmt->BytesPerPixel == 4)
            ((ulong*)colormap)[(j<<8) | i]= (ulong)SDL_MapRGB( fmt, r, g, b );
		}
}


/** Colormap::~Colormap
 *
 * Clean up.
 */
Colormap::~Colormap ()
{
	free(colormap);
}


/*
==========================================================================

Texture

==========================================================================
*/

/** Texture::Texture
 *
 * Create a texture, taking the pixel data from a Pic.
 * Currently it converts a 16 bit pic to a 8 bit texture. This should
 * be changed to load a 8 bit file directly, however.
 */
Texture::Texture (const char* fnametmpl, uint frametime, SDL_PixelFormat* screenfmt)
{
	m_colormap = 0;
	m_nrframes = 0;
	m_pixels = 0;
	m_frametime = frametime;
   is_32bit = screenfmt->BytesPerPixel == 4;

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
				m_colormap = new Colormap(surf->format->palette->colors, screenfmt);
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

				m_colormap = new Colormap(pal, screenfmt);
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
ulong Texture::get_minimap_color(char shade)
{
	uchar clr = m_pixels[0]; // just use the top-left pixel
	uint table = (uchar)shade;

   if( is_32bit )
      return ((ulong*)m_colormap->get_colormap())[clr | (table << 8)];
   else
      return ((ushort*)m_colormap->get_colormap())[clr | (table << 8)];
}


/** Texture::animate
 *
 * Set the current frame according to the game time.
 */
void Texture::animate(uint time)
{
	int frame = (time / m_frametime) % m_nrframes;

   uchar* lastframe = m_curframe;

	m_curframe = &m_pixels[TEXTURE_W*TEXTURE_H*frame];
   if( lastframe != m_curframe )
      m_was_animated = true;
}

/*
==============================================================================

TERRAIN RENDERING

==============================================================================
*/

#define ftofix(f) ((int) ((f)*0x10000))
#define itofix(i) ((i)<<16)
#define fixtoi(f) ((f)>>16)

/*  get lambda and mu so that lambda*u+mu*v=(1 0)^T
    with u=(u1 u2)^T and v=(v1 v2)^T */
static inline void get_horiz_linearcomb (int u1, int u2, int v1, int v2, float& lambda, float& mu)
{
	float det;

	det=u1*v2 - u2*v1;	// determinant of (u v)

	lambda=v2/det;		// by Cramer's rule
	mu=-u2/det;
}

template<typename T>
void render_top_triangle (Surface *dst,Texture *tex,Vertex *p1,Vertex *p2,Vertex *p3,int y2)
{
	int y,y1,w,h,ix1,ix2,count;
	int x1,x2,dx1,dx2;
	int b1,db1, tx1,dtx1, ty1,dty1;
	int b,db,tx,dtx,ty,dty;
	float lambda, mu;
	unsigned char *texpixels;
	T *texcolormap;

	get_horiz_linearcomb (p2->x-p1->x, p2->y-p1->y, p3->x-p1->x, p3->y-p1->y, lambda, mu);
	db=ftofix((p2->b-p1->b)*lambda + (p3->b-p1->b)*mu);
	dtx=ftofix((p2->tx-p1->tx)*lambda + (p3->tx-p1->tx)*mu);
	dty=ftofix((p2->ty-p1->ty)*lambda + (p3->ty-p1->ty)*mu);

	w=dst->get_w();
	h=dst->get_h();

	texpixels=tex->get_curpixels();
	texcolormap=(T*) (tex->get_colormap());

	y1=p1->y;

	x1=x2=itofix(p1->x);
	dx1=(itofix(p2->x) - x1) / (p2->y - y1);
	dx2=(itofix(p3->x) - x1) / (p3->y - y1);

	b1=itofix(p1->b);
	db1=(itofix(p2->b) - b1) / (p2->y - y1);

	tx1=itofix(p1->tx);
	dtx1=(itofix(p2->tx) - tx1) / (p2->y - y1);

	ty1=itofix(p1->ty);
	dty1=(itofix(p2->ty) - ty1) / (p2->y - y1);

	for (y=y1;y<y2 && y<h;y++) {
		if (y>=0) {
			ix1=fixtoi(x1);
			ix2=fixtoi(x2);

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

			T *scanline=(T*) ((uchar*)dst->get_pixels() + y*dst->get_pitch()) + ix1;

			while (count-->0) {
				int texel=((tx>>16) & (TEXTURE_W-1)) | ((ty>>10) & ((TEXTURE_H-1)<<6));

            			*scanline++=texcolormap[texpixels[texel] | ((b>>8) & 0xFF00)];

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

template<typename T>
void render_bottom_triangle (Surface* dst,Texture *tex,Vertex *p1,Vertex *p2,Vertex *p3,int y1)
{
	int y,y2,w,h,ix1,ix2,count;
	int x1,x2,dx1,dx2;
	int b1,db1, tx1,dtx1, ty1,dty1;
	int b,db,tx,dtx,ty,dty;
	float lambda, mu;
	unsigned char *texpixels;
	T *texcolormap;

	get_horiz_linearcomb (p2->x-p1->x, p2->y-p1->y, p3->x-p1->x, p3->y-p1->y, lambda, mu);
	db=ftofix((p2->b-p1->b)*lambda + (p3->b-p1->b)*mu);
	dtx=ftofix((p2->tx-p1->tx)*lambda + (p3->tx-p1->tx)*mu);
	dty=ftofix((p2->ty-p1->ty)*lambda + (p3->ty-p1->ty)*mu);

	w=dst->get_w();
	h=dst->get_h();

	texpixels=tex->get_curpixels();
	texcolormap=(T*) (tex->get_colormap());

	y2=p3->y;

	x1=x2=itofix(p3->x);
	dx1=-(itofix(p1->x) - x1) / (p1->y - y2);
	dx2=-(itofix(p2->x) - x1) / (p2->y - y2);

	// this may seem redundant but reduces rounding artifacts
	x1=itofix(p1->x) + dx1*(p1->y-y2);
	x2=itofix(p2->x) + dx2*(p2->y-y2);

	b1=itofix(p3->b);
	db1=-(itofix(p1->b) - b1) / (p1->y - y2);

	tx1=itofix(p3->tx);
	dtx1=-(itofix(p1->tx) - tx1) / (p1->y - y2);

	ty1=itofix(p3->ty);
	dty1=-(itofix(p1->ty) - ty1) / (p1->y - y2);

	for (y=y2;y>=y1 && y>0;y--) {
		if (y<h) {
			ix1=fixtoi(x1);
			ix2=fixtoi(x2);

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

			T* scanline=(T*) ((uchar*)dst->get_pixels() + y*dst->get_pitch()) + ix1;

			while (count-->0) {
				int texel=((tx>>16) & (TEXTURE_W-1)) | ((ty>>10) & ((TEXTURE_H-1)<<6));

            			*scanline++=texcolormap[texpixels[texel] | ((b>>8) & 0xFF00)];

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

/** render_triangle
 *
 * Render a triangle. It is being split into to triangles which have one
 * horizontal edge, so that the resulting triangles have only two edges
 * which are not horizontal, one on the left, the other on the right.
 * The actual rendering is performed by render_top_triangle and
 * render_bottom_triangle, which require a horizontal edge at the bottom
 * or at the top, respectively.
 */
template<typename T>
void render_triangle (Surface* dst,Vertex *p1,Vertex *p2,Vertex *p3, Texture *tex)
{
	Vertex *p[3]={ p1,p2,p3 };
	int top,bot,mid,y,ym,i;

	top=bot=0; // to avoid compiler warning

	y=0x7fffffff;
	for (i=0;i<3;i++)
		if (p[i]->y<y) { top=i; y=p[i]->y; }

	y=-0x7fffffff;
	for (i=0;i<3;i++)
		if (p[i]->y>y) { bot=i; y=p[i]->y; }

	for (mid=0;mid==top || mid==bot;mid++);
	ym=p[mid]->y;

	if (p[top]->y < ym) {
		if (p[mid]->x < p[bot]->x)
			render_top_triangle<T> (dst,tex,p[top],p[mid],p[bot],ym);
		else
			render_top_triangle<T> (dst,tex,p[top],p[bot],p[mid],ym);
	}

	if (ym < p[bot]->y) {
		if (p[mid]->x < p[top]->x)
			render_bottom_triangle<T> (dst,tex,p[mid],p[top],p[bot],ym);
		else
			render_bottom_triangle<T> (dst,tex,p[top],p[mid],p[bot],ym);
	}
}


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

template<typename T>
void dither_edge_horiz (Surface* dst, const Vertex& start, const Vertex& end, Texture* ttex, Texture* btex)
{
	unsigned char *tpixels, *bpixels;
	T *tcolormap, *bcolormap;

	tpixels=ttex->get_curpixels();
	tcolormap=(T*) (ttex->get_colormap());
	bpixels=btex->get_curpixels();
	bcolormap=(T*) (btex->get_colormap());

	int tx,ty,b,dtx,dty,db,tx0,ty0;

	tx=itofix(start.tx);
	ty=itofix(start.ty);
	b=itofix(start.b);
	dtx=(itofix(end.tx)-tx) / (end.x-start.x+1);
	dty=(itofix(end.ty)-ty) / (end.x-start.x+1);
	db=(itofix(end.b)-b) / (end.x-start.x+1);

	// TODO: seed this depending on field coordinates
	uint rnd=0;

	int dstw = dst->get_w();
	int dsth = dst->get_h();

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
					T *pix = (T*) ((uchar*)dst->get_pixels() + y*dst->get_pitch()) + x;
					int texel=((tx0>>16) & (TEXTURE_W-1)) | ((ty0>>10) & ((TEXTURE_H-1)<<6));
                			*pix=tcolormap[tpixels[texel] | ((b>>8) & 0xFF00)];
				}

				tx0+=dty;
				ty0-=dtx;
				rnd0>>=DITHER_RAND_SHIFT;
			}

			// dither below the edge
			for (unsigned int i = 0; i < DITHER_WIDTH; i++, y++) {
				if ((rnd0&DITHER_RAND_MASK)>=i+DITHER_WIDTH && y>=0 && y<dsth) {
					T *pix = (T*) ((uchar*)dst->get_pixels() + y*dst->get_pitch()) + x;
					int texel=((tx0>>16) & (TEXTURE_W-1)) | ((ty0>>10) & ((TEXTURE_H-1)<<6));
                			*pix=bcolormap[bpixels[texel] | ((b>>8) & 0xFF00)];
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


template<typename T>
static void dither_edge_vert (Surface* dst, const Vertex& start, const Vertex& end, Texture* ltex, Texture* rtex)
{
	unsigned char *lpixels, *rpixels;
	T* lcolormap, *rcolormap;

	lpixels=ltex->get_curpixels();
	lcolormap=(T*) (ltex->get_colormap());
	rpixels=rtex->get_curpixels();
	rcolormap=(T*) (rtex->get_colormap());

	int tx,ty,b,dtx,dty,db,tx0,ty0;

	tx=itofix(start.tx);
	ty=itofix(start.ty);
	b=itofix(start.b);
	dtx=(itofix(end.tx)-tx) / (end.y-start.y+1);
	dty=(itofix(end.ty)-ty) / (end.y-start.y+1);
	db=(itofix(end.b)-b) / (end.y-start.y+1);

	// TODO: seed this depending on field coordinates
	uint rnd=0;

	int dstw = dst->get_w();
	int dsth = dst->get_h();

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
					T *pix = (T*) ((uchar*)dst->get_pixels() + y*dst->get_pitch()) + x;
					int texel=((tx0>>16) & (TEXTURE_W-1)) | ((ty0>>10) & ((TEXTURE_H-1)<<6));
                			*pix=lcolormap[lpixels[texel] | ((b>>8) & 0xFF00)];
        			}

				tx0+=dty;
				ty0-=dtx;
				rnd0>>=DITHER_RAND_SHIFT;
			}

			// dither on right side
			for (unsigned int i = 0; i < DITHER_WIDTH; i++, x++) {
				if ((rnd0 & DITHER_RAND_MASK)>=i+DITHER_WIDTH && x>=0 && x<dstw) {
					T *pix = (T*) ((uchar*)dst->get_pixels() + y*dst->get_pitch()) + x;
					int texel=((tx0>>16) & (TEXTURE_W-1)) | ((ty0>>10) & ((TEXTURE_H-1)<<6));
            				*pix=rcolormap[rpixels[texel] | ((b>>8) & 0xFF00)];
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

Render a road.
===============
*/
template<typename T>
void render_road_horiz(Surface* dst, const Point& start, const Point& end, Surface* src )
{
	int dstw = dst->get_w();
	int dsth = dst->get_h();

	int ydiff = ((end.y - start.y) << 16) / (end.x - start.x);
	int centery = start.y << 16;

	for(int x = start.x, sx = 0; x < end.x; x++, centery += ydiff, sx ++) {
		if (x < 0 || x >= dstw)
			continue;

		int y = (centery >> 16) - 2;

		for(int i = 0; i < 5; i++, y++) {
			if (y < 0 || y >= dsth)
				continue;

        		T* dpix = (T*)((uchar*)dst->get_pixels() + y*dst->get_pitch()) + x;
        		T* spix = (T*)((uchar*)src->get_pixels() + i*src->get_pitch()) + sx;
        		*dpix = *spix;
		}
	}
}

template<typename T>
void render_road_vert(Surface* dst, const Point& start, const Point& end, Surface* src)
{
	int dstw = dst->get_w();
	int dsth = dst->get_h();

	int xdiff = ((end.x - start.x) << 16) / (end.y - start.y);
	int centerx = start.x << 16;

	for(int y = start.y, sy = 0; y < end.y; y++, centerx += xdiff, sy ++ ) {
		if (y < 0 || y >= dsth)
			continue;

		int x = (centerx >> 16) - 2;

		for(int i = 0; i < 5; i++, x++) {
			if (x < 0 || x >= dstw)
				continue;


        		T* dpix = (T*)((uchar*)dst->get_pixels() + y*dst->get_pitch()) + x;
        		T* spix = (T*)((uchar*)src->get_pixels() + sy*src->get_pitch()) + i;
        		*dpix = *spix;
		}
	}
}


template<typename T>
void draw_field_int(Surface* dst, Field * const f, Field * const rf, Field * const fl, Field * const rfl,
		    Field * const lf, Field * const ft,
	    	    const int posx, const int rposx, const int posy,
	    	    const int blposx, const int rblposx, const int blposy,
	    	    uchar roads, uchar darken, bool draw_all)
{
	Vertex r, l, br, bl;

	r = Vertex(rposx, posy - rf->get_height() * HEIGHT_FACTOR, rf->get_brightness(), 0, 0);
	l = Vertex(posx, posy - f->get_height() * HEIGHT_FACTOR, f->get_brightness(), 64, 0);
	br = Vertex(rblposx, blposy - rfl->get_height() * HEIGHT_FACTOR, rfl->get_brightness(), 0, 64);
	bl = Vertex(blposx, blposy - fl->get_height() * HEIGHT_FACTOR, fl->get_brightness(), 64, 64);

	if (darken&1) l.b=-128;
	if (darken&2) r.b=-128;
	if (darken&4) bl.b=-128;
	if (darken&8) br.b=-128;

	Surface* rt_normal = get_graphicimpl()->get_road_texture(Road_Normal);
	Surface* rt_busy = get_graphicimpl()->get_road_texture(Road_Busy);

	Texture* rtex = get_graphicimpl()->get_maptexture_data(f->get_terr()->get_texture());
	Texture* btex = get_graphicimpl()->get_maptexture_data(f->get_terd()->get_texture());
	Texture* ltex = get_graphicimpl()->get_maptexture_data(lf->get_terr()->get_texture());
	Texture* ttex = get_graphicimpl()->get_maptexture_data(ft->get_terd()->get_texture());

	if( draw_all ) {
    		render_triangle<T> (dst, &r, &l, &br, rtex);
		render_triangle<T> (dst, &l, &br, &bl, btex);
	} else {
		if( rtex->was_animated())
			render_triangle<T> (dst, &r, &l, &br, rtex);
		if( btex->was_animated())
			render_triangle<T>(dst, &l, &br, &bl, btex);
	}

	// Render roads and dither polygon edges
	uchar road;

	road = (roads >> Road_East) & Road_Mask;
	if ((darken&3)!=3) {
		if (road) {
			switch(road) {
				case Road_Normal:
					render_road_horiz<T> (dst, l, r, rt_normal);
					break;
				case Road_Busy:
					render_road_horiz<T> (dst, l, r, rt_busy);
					break;
        			default:
					assert(0); break; // never here
			}
		}
		else {
			if( draw_all || rtex->was_animated() || ttex->was_animated())
				if (rtex!=0 && ttex!=0 && rtex!=ttex)
					dither_edge_horiz<T> (dst, l, r, rtex, ttex);
		}
	}

	road = (roads >> Road_SouthEast) & Road_Mask;
	if ((darken&9)!=9) {
		if (road) {
			switch(road) {
				case Road_Normal:
					render_road_vert<T> (dst, l, br, rt_normal);
					break;
				case Road_Busy:
					render_road_vert<T> (dst, l, br, rt_busy);
					break;
				default:
					assert(0); break; // never here
			}
		}
		else {
			if( draw_all || rtex->was_animated() || btex->was_animated())
				if (rtex!=0 && btex!=0 && rtex!=btex)
					dither_edge_vert<T> (dst, l, br, rtex, btex);
		}
	}

	road = (roads >> Road_SouthWest) & Road_Mask;
	if ((darken&5)!=5) {
		if (road) {
			switch(road) {
				case Road_Normal:
					render_road_vert<T> (dst, l, bl, rt_normal);
					break;
				case Road_Busy:
					render_road_vert<T> (dst, l, bl, rt_busy);
					break;
				default:
					assert(0); break; // never here
			}
		}
		else {
			if( draw_all || btex->was_animated() || ltex->was_animated())
				if (ltex!=0 && btex!=0 && ltex!=btex)
					dither_edge_vert<T>(dst, l, bl, btex, ltex);
		}
	}

	// FIXME: similar textures may not need dithering
}

/*
===============
Surface::draw_field

Draw ground textures and roads for the given parallelogram (two triangles)
into the bitmap.
===============
*/
void Surface::draw_field(Rect& subwin, Field * const f, Field * const rf, Field * const fl, Field * const rfl,
			Field * const lf, Field * const ft,
	                const int posx, const int rposx, const int posy,
	                const int blposx, const int rblposx, const int blposy,
	                uchar roads, uchar darken, bool draw_all)
{
	set_subwin( subwin );

	switch (get_format()->BytesPerPixel) {
	    case 2:
		draw_field_int<ushort>
		    (this, f, rf, fl, rfl, lf, ft, posx, rposx, posy, blposx, rblposx, blposy, roads, darken, draw_all);
		break;
	    case 4:
		draw_field_int<ulong>
		    (this, f, rf, fl, rfl, lf, ft, posx, rposx, posy, blposx, rblposx, blposy, roads, darken, draw_all);
		break;
	    default:
		assert (0);
	}

	unset_subwin();
}
