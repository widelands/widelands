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
Texture implementation and terrain rendering for the 16-bit software renderer.
*/

#include "widelands.h"
#include "graphic.h"
#include "map.h"
#include "world.h"

#include "sw16_graphic.h"

#include <iostream>

using namespace std;

namespace Renderer_Software16
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
 * 16-to-8-bit conversion and shading.
 */
Colormap::Colormap (const SDL_Color *pal)
{
   int i,j,r,g,b;

	memcpy(palette, pal, sizeof(palette));
	
   colormap=new unsigned short[65536];
   
//    log ("Creating color map\n");
	for (i=0;i<256;i++)
		for (j=0;j<256;j++) {
			// Old shading method
/*			char shade=j;

			r=(palette[i].r+shade)>>3;
			g=(palette[i].g+shade)>>2;
			b=(palette[i].b+shade)>>3;

			if (r<0) r=0;
			if (r>31) r=31;

			if (g<0) g=0;
			if (g>63) g=63;

			if (b<0) b=0;
			if (b>31) b=31;*/

			int shade=(j<128)?j:(j-256);
			shade=256+2*shade;

			r=(palette[i].r*shade)>>11;
			g=(palette[i].g*shade)>>10;
			b=(palette[i].b*shade)>>11;

			if (r>31) r=31;
			if (g>63) g=63;
			if (b>31) b=31;

			colormap[(j<<8) | i]=(r<<11) | (g<<5) | b;
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
 * Currently it converts a 16 bit pic to a 8 bit texture. This should
 * be changed to load a 8 bit file directly, however.
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
unsigned short Texture::get_minimap_color(char shade)
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

#define ftofix(f) ((int) ((f)*0x10000))
#define itofix(i) ((i)<<16)
#define fixtoi(f) ((f)>>16)

/*  get lambda and mu so that lambda*u+mu*v=(1 0)^T
    with u=(u1 u2)^T and v=(v1 v2)^T */
static inline void get_horiz_linearcomb (int u1, int u2, int v1, int v2, float& lambda, float& mu)
{
    float det;
    
    det=u1*v2 - u2*v1;		// determinant of (u v)
    
    lambda=v2/det;		// by Cramer's rule
    mu=-u2/det;
}

static void render_top_triangle (Bitmap *dst,Texture *tex,Vertex *p1,Vertex *p2,Vertex *p3,int y2)
{
	int y,y1,w,h,ix1,ix2,count;
	int x1,x2,dx1,dx2;
	int b1,db1, tx1,dtx1, ty1,dty1;
	int b,db,tx,dtx,ty,dty;
	float lambda, mu;
	unsigned char *texpixels;
	unsigned short *texcolormap;
	
	get_horiz_linearcomb (p2->x-p1->x, p2->y-p1->y, p3->x-p1->x, p3->y-p1->y, lambda, mu);
	db=ftofix((p2->b-p1->b)*lambda + (p3->b-p1->b)*mu);
	dtx=ftofix((p2->tx-p1->tx)*lambda + (p3->tx-p1->tx)*mu);
	dty=ftofix((p2->ty-p1->ty)*lambda + (p3->ty-p1->ty)*mu);

	w=dst->w;
	h=dst->h;

	texpixels=tex->get_curpixels();
	texcolormap=tex->get_colormap();

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

	for (y=y1;y<=y2 && y<h;y++) {
		if (y>=0) {
			ix1=fixtoi(x1);
			ix2=fixtoi(x2);
			
			b=b1;
			tx=tx1;
			ty=ty1;

			if (ix2>=w) ix2=w-1;
			if (ix1<0) {
				b-=ix1*db;
				tx-=ix1*dtx;
				ty-=ix1*dty;
				ix1=0;
			}
			
			count=ix2-ix1;

			unsigned short *scanline=dst->pixels + y*dst->pitch + ix1;

			while (count-- >= 0) {
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

static void render_bottom_triangle (Bitmap *dst,Texture *tex,Vertex *p1,Vertex *p2,Vertex *p3,int y1)
{
	int y,y2,w,h,ix1,ix2,count;
	int x1,x2,dx1,dx2;
	int b1,db1, tx1,dtx1, ty1,dty1;
	int b,db,tx,dtx,ty,dty;
	float lambda, mu;
	unsigned char *texpixels;
	unsigned short *texcolormap;

	get_horiz_linearcomb (p2->x-p1->x, p2->y-p1->y, p3->x-p1->x, p3->y-p1->y, lambda, mu);
	db=ftofix((p2->b-p1->b)*lambda + (p3->b-p1->b)*mu);
	dtx=ftofix((p2->tx-p1->tx)*lambda + (p3->tx-p1->tx)*mu);
	dty=ftofix((p2->ty-p1->ty)*lambda + (p3->ty-p1->ty)*mu);

	w=dst->w;
	h=dst->h;

	texpixels=tex->get_curpixels();
	texcolormap=tex->get_colormap();

	y2=p3->y;

	x1=x2=itofix(p3->x);
	dx1=-(itofix(p1->x) - x1) / (p1->y - y2);
	dx2=-(itofix(p2->x) - x1) / (p2->y - y2);

	b1=itofix(p3->b);
	db1=-(itofix(p1->b) - b1) / (p1->y - y2);

	tx1=itofix(p3->tx);
	dtx1=-(itofix(p1->tx) - tx1) / (p1->y - y2);

	ty1=itofix(p3->ty);
	dty1=-(itofix(p1->ty) - ty1) / (p1->y - y2);

	for (y=y2;y>=y1 && y>=0;y--) {
		if (y<h) {
			ix1=fixtoi(x1);
			ix2=fixtoi(x2);

			b=b1;
			tx=tx1;
			ty=ty1;

			if (ix2>=w) ix2=w-1;
			if (ix1<0) {
				b-=ix1*db;
				tx-=ix1*dtx;
				ty-=ix1*dty;
				ix1=0;
			}
			
			count=ix2-ix1;
         
			unsigned short *scanline=dst->pixels + y*dst->pitch + ix1;

			while (count-- >= 0) {
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
static void render_triangle (Bitmap *dst,Vertex *p1,Vertex *p2,Vertex *p3, Texture *tex)
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
			render_top_triangle (dst,tex,p[top],p[mid],p[bot],ym);
		else
			render_top_triangle (dst,tex,p[top],p[bot],p[mid],ym);
	}

	if (ym < p[bot]->y) {
		if (p[mid]->x < p[top]->x)
			render_bottom_triangle (dst,tex,p[mid],p[top],p[bot],ym);
		else
			render_bottom_triangle (dst,tex,p[top],p[mid],p[bot],ym);
	}
}


/*
===============
render_road_horiz
render_road_vert

Render a road. This is really dumb right now, not using a texture
===============
*/
static void render_road_horiz(Bitmap *dst, Point start, Point end, ushort color)
{
	int dstw = dst->w;
	int dsth = dst->h;

	int ydiff = ((end.y - start.y) << 16) / (end.x - start.x);
	int centery = start.y << 16;

	for(int x = start.x; x < end.x; x++, centery += ydiff) {
		if (x < 0 || x >= dstw)
			continue;
		
		int y = (centery >> 16) - 2;
		
		for(int i = 0; i < 5; i++, y++) {
			if (y < 0 || y >= dsth)
				continue;
			
	      ushort *pix = dst->pixels + y*dst->pitch + x;
			*pix = color;
		}
	}
}

static void render_road_vert(Bitmap *dst, Point start, Point end, ushort color)
{
	int dstw = dst->w;
	int dsth = dst->h;

	int xdiff = ((end.x - start.x) << 16) / (end.y - start.y);
	int centerx = start.x << 16;

	for(int y = start.y; y < end.y; y++, centerx += xdiff) {
		if (y < 0 || y >= dsth)
			continue;
		
		int x = (centerx >> 16) - 2;
		
		for(int i = 0; i < 5; i++, x++) {
			if (x < 0 || x >= dstw)
				continue;
			
			ushort *pix = dst->pixels + y*dst->pitch + x;
			*pix = color;
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
	                const int posx, const int rposx, const int posy, 
	                const int blposx, const int rblposx, const int blposy, 
	                uchar roads, bool render_r, bool render_b)
{
	Vertex r, l, br, bl;

	r = Vertex(rposx, posy - MULTIPLY_WITH_HEIGHT_FACTOR(rf->get_height()), rf->get_brightness(), 0, 0);
	l = Vertex(posx, posy - MULTIPLY_WITH_HEIGHT_FACTOR(f->get_height()), f->get_brightness(), 64, 0);
	br = Vertex(rblposx, blposy - MULTIPLY_WITH_HEIGHT_FACTOR(rfl->get_height()), rfl->get_brightness(), 0, 64);
	bl = Vertex(blposx, blposy - MULTIPLY_WITH_HEIGHT_FACTOR(fl->get_height()), fl->get_brightness(), 64, 64);

/*
	r.b += 20; // debug override for shading (make field borders visible)
	bl.b -= 20;
*/

//	render_r=false; // debug overwrite: just render b triangle
//	render_b=false; // debug overwrite: just render r triangle

	// Render right triangle
	if(render_r) {
		Texture* tex = get_graphicimpl()->get_maptexture_data(f->get_terr()->get_texture());
		
		if (tex)
			render_triangle(this, &r, &l, &br, tex);
	}

	if(render_b) {
		Texture* tex = get_graphicimpl()->get_maptexture_data(f->get_terd()->get_texture());
		
		if (tex)
			render_triangle(this, &l, &br, &bl, tex);
	}

	// Render roads
	ushort color;
	uchar road;

	road = (roads >> Road_East) & Road_Mask;
	if (render_r && road) {
		if (road == Road_Normal)
			color = RGBColor(192, 192, 192).pack16();
		else if (road == Road_Busy)
			color = RGBColor(96, 96, 96).pack16();
		else
			color = RGBColor(0, 0, 128).pack16();
		render_road_horiz(this, l, r, color);
	}

	road = (roads >> Road_SouthEast) & Road_Mask;
	if (road) {
		if (road == Road_Normal)
			color = RGBColor(192, 192, 192).pack16();
		else if (road == Road_Busy)
			color = RGBColor(96, 96, 96).pack16();
		else
			color = RGBColor(0, 0, 128).pack16();
		render_road_vert(this, l, br, color);
	}

	road = (roads >> Road_SouthWest) & Road_Mask;
	if (road) {
		if (road == Road_Normal)
			color = pack_rgb(192, 192, 192);
		else if (road == Road_Busy)
			color = pack_rgb(96, 96, 96);
		else
			color = pack_rgb(0, 0, 128);
		render_road_vert(this, l, bl, color);
	}
}


}; // namespace Renderer_Software16
