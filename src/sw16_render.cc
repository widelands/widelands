/*
 * Copyright (C) 2002 by the Widelands Development Team
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
Rendering functions of the 16-bit software renderer.
*/

#include "widelands.h"
#include "graphic.h"
#include "animation.h"
#include "map.h"

#include "sw16_graphic.h"
#include "editor_game_base.h"

namespace Renderer_Software16
{

/*
==============================================================================

Bitmap

==============================================================================
*/

#define PIXEL(x, y)		pixels[(y)*pitch+(x)]

/*
===============
Bitmap::draw_rect

Draws the outline of a rectangle
===============
*/
void Bitmap::draw_rect(Rect rc, RGBColor clr)
{
	ushort color = clr.pack16();

	rc.w += rc.x-1;
	rc.h += rc.y-1;

	for (int x=rc.x+1; x<rc.w; x++)
	{
		PIXEL(x, rc.y) = color;
		PIXEL(x, rc.h) = color;
	}
	for (int y=rc.y; y<=rc.h; y++)
	{
		PIXEL(rc.x, y) = color;
		PIXEL(rc.w, y) = color;
	}
}


/*
===============
Bitmap::fill_rect

Draws a filled rectangle
===============
*/
void Bitmap::fill_rect(Rect rc, RGBColor clr)
{
	ushort color = clr.pack16();

	rc.w += rc.x;
	rc.h += rc.y;

	for (int y = rc.y; y < rc.h; y++)
	{
		ushort* p = &pixels[y * pitch + rc.x];
		for (int x = rc.x; x < rc.w; x++)
			*p++ = color;
	}
}

/*
===============
Bitmap::brighten_rect

Change the brightness of the given rectangle
===============
*/
void Bitmap::brighten_rect(Rect rc, int factor)
{
	rc.w += rc.x;
	rc.h += rc.y;

	for (int y = rc.y; y < rc.h; y++)
	{
		ushort* p = &pixels[y * pitch + rc.x];
		for (int x = rc.x; x < rc.w; x++, p++)
			*p = bright_up_clr(*p, factor);
	}
}


/*
===============
Bitmap::clear

Clear the entire bitmap to black
===============
*/
void Bitmap::clear()
{
	// this function clears the bitmap completly (draws it black)
	// it assumes, that: (2*w*h)%4 == 0.
	// This function is speedy
	if (w != pitch || (w & 1 && h & 1))
		fill_rect(Rect(0, 0, w, h), RGBColor(0, 0, 0));
	else {
		int i = w*h;
		assert(!(i&3));
		i>>=1;
		while(--i) *(((long*)(pixels))+i)=0; // faster than a memset?
		*pixels=0;
	}
}

/*
===============
Bitmap::blit

Blit this given source bitmap to this bitmap.
===============
*/
void Bitmap::blit(Point dst, Bitmap* src, Rect srcrc)
{
	ushort* dstpixels = pixels + dst.y*pitch + dst.x;
	ushort* srcpixels = src->pixels + srcrc.y*src->pitch + srcrc.x;
	
	if (src->hasclrkey)
	{
		uint dstskip = pitch - srcrc.w;
		uint srcskip = src->pitch - srcrc.w;
		
		for (int i = srcrc.h; i; i--, dstpixels += dstskip, srcpixels += srcskip) {
			for (int j = srcrc.w; j; j--, dstpixels++, srcpixels++) {
				ushort clr = *srcpixels;
				if (clr != src->clrkey)
					*dstpixels = clr;
			}
		}
	}
	else
	{
		if (srcrc.w == pitch && srcrc.w == src->pitch)
		{
			// copying entire rows, so it can be all done in a single memcpy
			memcpy(dstpixels, srcpixels, (srcrc.w*srcrc.h) << 1);
		}
		else
		{
			// fast blitting, using one memcpy per row
			int bw = srcrc.w << 1;

			for(int i = srcrc.h; i; i--, dstpixels += pitch, srcpixels += src->pitch)
				memcpy(dstpixels, srcpixels, bw);
		}
	}
}


/*
===============
 Bitmap::draw_minimap

 Draw the minimap for the given rectangular part of the map at the given point.
 It centers the curent view point of the player.
 ===============
 */
void Bitmap::draw_minimap(Point dst, const MapRenderInfo* mri, Rect rc, uint fx, uint fy, int vp_x, int vp_y, char flags)
{
    // calculate the offset
    vp_x = (vp_x + 1 + (int) mri->egbase->get_map()->get_width()/2)%rc.w;
    vp_y = (vp_y + 1 + (int) mri->egbase->get_map()->get_height()/2)%rc.h;

    if(fx==(uint)rc.w && fy==(uint)rc.h) {
        // forced size == natural size. 
        // use fast rendering
        int mapwidth = mri->egbase->get_map()->get_width();

        for(int y = 0; y < rc.h; y++) {
            ushort* pix = pixels + (dst.y+y)*pitch + dst.x;
            Field* f = mri->egbase->get_map()->get_field(rc.x, (rc.y+y+vp_y)%rc.h);

            f = &f[vp_x];  // move by vp_x to begin
            for(int x = 0; x < rc.w; x++, f++, pix++)
            {
                if (x + vp_x%rc.w == rc.w) 	// if we reach the end
                    f = f - rc.w;		// move from end to begining

                if (mri->visibility && !(*mri->visibility)[((y+vp_y)%rc.h)*mapwidth + (x+vp_x)%rc.w])
                    *pix = 0;
                else {
                    Texture* tex = get_graphicimpl()->get_maptexture_data(f->get_terd()->get_texture());
                    *pix = tex->get_minimap_color(f->get_brightness(), mri, (x+vp_x)%rc.w, (y+vp_y)%rc.h, f->get_owned_by(), flags);
                }
            }
        }

    } else { 
        // forced size is somehow different. slow rendering needed
        // we center the minimap in the area we got.
        int mapwidth = mri->egbase->get_map()->get_width();

        float xslope=(float)mri->egbase->get_map()->get_width()/(float)fx;
        float yslope=(float)mri->egbase->get_map()->get_height()/(float)fy;
        float xfield=0, yfield=0;
        for(uint y = 0; y < fy; y++, yfield+=yslope) {
            ushort* pix = pixels + (dst.y+y)*pitch + dst.x;

            for(uint x = 0; x < fx; x++, pix++, xfield+=xslope)
            {
                Field* f = mri->egbase->get_map()->get_field((int)(xfield+vp_x)%fx, (int)(yfield+vp_y)%fy);
                if (mri->visibility && !(*mri->visibility)[(int)(yfield+vp_y)%fy*mapwidth + (int)(xfield+vp_x)%fx])
                    *pix = 0;
                else {
                    Texture* tex = get_graphicimpl()->get_maptexture_data(f->get_terd()->get_texture());
                    *pix = tex->get_minimap_color(f->get_brightness(), mri, (x+vp_x)%rc.w, (y+vp_y)%rc.h, f->get_owned_by(), flags);
                }
            }
            xfield=0;
        }
    }
}


/*
===============
Bitmap::draw_animframe

Draws the given animation frame into the bitmap.
dst is the position of the upper-left corner of the destination (i.e. hotspot
has already been applied).
rc is the sub-rectangle of the frame that is to be drawn.

Note: This implementation is general purpose. It should be possible to
      make speedups by creating a "special" path without clipping (this should
      actually be the path that is taken most often).
===============
*/
void Bitmap::draw_animframe(Point dstpt, const AnimFrame* frame, Rect rc, const RGBColor* plrclrs)
{
	ushort plrclrs_packed[4];

	// build packed player colors
	if (!plrclrs) {
		static RGBColor ownerless_playercolor[4] = {
			RGBColor(10,   79,  54),
			RGBColor(15,  110,  75),
			RGBColor(28,  208, 142),
			RGBColor(35,  255, 174)
		};
		plrclrs = ownerless_playercolor;
	}

	for(int i = 0; i < 4; i++)
		plrclrs_packed[i] = plrclrs[i].pack16();

	const ushort* in = frame->data;
	
	rc.w += rc.x;
	rc.h += rc.y;
	
	//log("%ix%i; %i,%i,%i,%i\n", frame->width, frame->height, rc.x, rc.y, rc.w, rc.h);
	
	for(int y = 0; y < rc.h; y++) {
		ushort* out = &pixels[(dstpt.y+y-rc.y)*pitch + dstpt.x-rc.x];
		int x = 0;
	
		while(x < frame->width) {
			ushort cmd;
			int count;
			int clipcount, clipofs;

			cmd = *in++;
			count = cmd & 0x3fff;
			cmd = cmd >> 14;
			
			//log(" %i - %i\n", cmd, count);

			switch(cmd) {
			case 0: // normal pixels
				if (y >= rc.y && x < rc.w) {
					clipcount = count;
					clipofs = 0;

					if (x < rc.x) {
						clipcount -= rc.x - x;
						clipofs = rc.x - x;
					}
					if (x+clipcount > rc.w)
						clipcount -= x+clipcount - rc.w;

					if (clipcount > 0)
						memcpy(out+clipofs, in+clipofs, clipcount*sizeof(ushort));
				}

				x += count;
				out += count;
				in += count;
				break;

			case 1: // transparent pixels
				x += count;
				out += count;
				break;

			case 2: // player color
				if (y >= rc.y && x < rc.w) {
					for(int i = 0; i < count; i++, x++, in++, out++)
						if (x >= rc.x && x < rc.w) {
							ushort clr = *in;

							assert(clr < 4);

							*out = plrclrs_packed[clr];
						}
				} else {
					x += count;
					in += count;
					out += count;
				}
				break;

			case 3: // shadow
				if (y >= rc.y && x < rc.w) {
					for(int i = 0; i < count; i++, x++, out++)
						if (x >= rc.x && x < rc.w)
							*out = (*out >> 1) & 0x7BEF;
				} else {
					x += count;
					out += count;
				}
				break;
			}
		}

		assert(x == frame->width);
	}
}


/*
==============================================================================

AnimationGfx -- contains graphics data for an animtion

==============================================================================
*/

/*
===============
AnimationGfx::AnimationGfx

Load the animation
===============
*/
AnimationGfx::AnimationGfx(const AnimationData* data)
{
	m_nrframes = 0;
	m_frames = 0;

   const int nextensions=4;
   const char extensions[nextensions][5] = {
      ".bmp",
      ".png",
      ".gif",
      ".jpg"
   };
   
   for(;;) {
      char fname[256];
      int nr = m_nrframes;
      char *p;

      bool done=false;
      bool alldone=false;
      bool cycling=false;
      for(int i=0; i<nextensions; i++) {
         if(done) continue;

         // create the file name by reverse-scanning for '?' and replacing
         nr=m_nrframes;
         snprintf(fname, sizeof(fname), "%s%s", data->picnametempl.c_str(),extensions[i]);
         p = fname + strlen(fname);
         while(p > fname) {
            if (*--p != '?')
               continue;

            cycling=true;
            
            *p = '0' + (nr % 10);
            nr = nr / 10;
         }

         if (nr) // cycled up to maximum possible frame number
            break;


         // is the frame actually there?
         if (!g_fs->FileExists(fname)) {
            if(i==(nextensions-1)) { alldone=true; break; }
            continue;
         }

         // Load the image
         SDL_Surface* bmp = 0;

         try
         {
            bmp = LoadImage(fname);
         }
         catch(std::exception& e)
         {
            log("WARNING: Couldn't load animation frame %s: %s\n", fname, e.what());
            continue;
         }

         // Get a new AnimFrame
         AnimFrame* frame;

         m_frames = (AnimFrame*)realloc(m_frames, sizeof(AnimFrame)*(m_nrframes+1));
         frame = &m_frames[m_nrframes];

         frame->width = bmp->w;
         frame->height = bmp->h;

         frame->hotspot = data->hotspot;

         try {
            encode(frame, bmp, &data->encdata);
         } catch(std::exception& e) {
            SDL_FreeSurface(bmp);
            throw wexception("Error encoding %s: %s", fname, e.what());
         }

         m_nrframes++;
         done=true;
         if(!cycling) alldone=true;

         SDL_FreeSurface(bmp);
      }
      if(alldone==true) break;
   }

	if (!m_nrframes)
		throw wexception("Animation %s has no frames", data->picnametempl.c_str());
}


/*
===============
AnimationGfx::~AnimationGfx

Free all resources
===============
*/
AnimationGfx::~AnimationGfx()
{
	if (m_frames) {
		for(int i = 0; i < m_nrframes; i++)
			free(m_frames[i].data);
		
		free(m_frames);
	}
}


/*
===============
AnimationGfx::encode

Encodes the given surface into a frame
===============
*/
void AnimationGfx::encode(AnimFrame* frame, SDL_Surface* bmp, const EncodeData* encdata)
{
	// Pack the EncodeData colorkey&co. to 16 bit
	ushort clrkey = 0;
	ushort shadowclr = 0;
	int hasplrclrs = 0;
	ushort plrclrs[4];

	if (encdata->hasclrkey)
		clrkey = encdata->clrkey.pack16();
	if (encdata->hasshadow)
		shadowclr = encdata->shadow.pack16();
	if (encdata->hasplrclrs) {
		hasplrclrs = 4;
		for(int i = 0; i < 4; i++)
			plrclrs[i] = encdata->plrclr[i].pack16();
	}

	// Ready to encode
   ushort* data = 0;
	SDL_Surface* surf = 0;
	uint out;

	try
	{
		surf = SDL_DisplayFormat(bmp);
		data = (ushort*)malloc(surf->w*surf->h*sizeof(ushort)*2);

		assert(surf->w == frame->width && surf->h == frame->height);

		out = 0;

		for(int y = 0; y < frame->height; y++)
		{
			ushort* pixels = (ushort*)((Uint8*)surf->pixels + y*surf->pitch);
			int cmd = -1;
			int runstart = 0;
			int count = 0; // runlength

			//log("width: %i\n", frame->width);

			for(int x = 0; x < frame->width; x++, pixels++) {
				int thiscmd = 0;
				int clridx = -1;

				// determine the type of pixel
				if (encdata->hasclrkey && *pixels == clrkey)
					thiscmd = 1;
				else if (encdata->hasshadow && *pixels == shadowclr)
					thiscmd = 3;
				else if (hasplrclrs) {
					for(clridx = 0; clridx < hasplrclrs; clridx++)
						if (*pixels == plrclrs[clridx]) {
							thiscmd = 2;
							break;
						}
				}

				// emit code
				if (thiscmd != cmd) {
					if (cmd >= 0) {
						data[runstart] = (cmd<<14)|count;

						//log(" %i - %i\n", cmd, count);
					}

					cmd = thiscmd;
					runstart = out++;
					count = 0;
				}

				count++;

				if (cmd == 0) // normal pixel
					data[out++] = *pixels;
				else if (cmd == 2) // player color
					data[out++] = clridx;
			}

			// finish the last run
			data[runstart] = (cmd<<14)|count;

			//log(" %i - %i\n", cmd, count);
		}

		frame->data = (ushort*)malloc(out*sizeof(ushort));
		memcpy(frame->data, data, out*sizeof(ushort));

		free(data),
		data = 0;

		SDL_FreeSurface(surf);
		surf = 0;
	} catch(...) {
		if (data)
			free(data);
		if (surf)
			SDL_FreeSurface(surf);
		throw;
	}
}

}; // namespace Renderer_Software16
