/*
 * Copyright (C) 2002-2004 by the Wide Lands Development Team
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

#ifndef RGBCOLOR_H
#define RGBCOLOR_H

#include "types.h"

/*
==============================================================================

COLOR HANDLING

==============================================================================
*/

/** inline ushort pack_rgb(const uchar r, const uchar g, const uchar b);
 *
 * This functions packs a RGB tribble into a short
 *
 * Args: r 	red value
 * 		g 	green value
 * 		b	blue value (SUPRISE!)
 * Returns: packed value
 */
inline ushort pack_rgb(const uchar r, const uchar g, const uchar b)
{
   return ((b>>3) + ((g>>2)<<5)+ ((r>>3)<<11) );
}

/** inline void unpack_rgb(const ushort clr, uchar* r, uchar* g, uchar* b) ;
 *
 * this unpacks a clr and returns the RGB tribble
 *
 * Args: 	clr	clr to unpack
*   		r		ptr to var to hold red value
 *			g		ptr to var to hold green value
 *			b		ptr to var to hold blue value
 * Returns: Nothing
 */
inline void unpack_rgb(const ushort clr, uchar* r, uchar* g, uchar* b)
{
   *r= ((clr<<3)>>11);
   *g= ((clr<<2)>>5);
   *b= (clr<<3);
}

// the voodoo version...
// it's quite a bit faster than the original on my CPU
// note that this is not used by the new renderer anymore
inline ushort bright_up_clr(ushort clr, int factor)
{
   if(!factor) return clr;

   int r = ((clr >> 11) << 3);
   int g = ((clr >> 5)  << 2) & 0xFF;
   int b = ((clr)       << 3) & 0xFF;

   r += factor;
   if (r & 0xFF00) goto fix_r;
end_r:
   g += factor;
   if (g & 0xFF00) goto fix_g;
end_g:
   b += factor;
   if (b & 0xFF00) goto fix_b;
   goto end;

fix_r: r = (~r) >> 24; goto end_r;
fix_g: g = (~g) >> 24; goto end_g;
fix_b: b = (~b) >> 24;
end:
       return pack_rgb(r, g, b);
}

inline void bright_up_clr32(uchar* clr, int factor)
{
   int b = clr[0] + factor;
   int g = clr[1] + factor;
   int r = clr[2] + factor;

	if (b & 0xFF00) b = (~b) >> 24;
	if (g & 0xFF00) g = (~g) >> 24;
	if (r & 0xFF00) r = (~r) >> 24;

	clr[0] = b;
	clr[1] = g;
	clr[2] = r;
}


/*
class RGBColor
*/
class RGBColor {
	enum {
		Blue = 0,
		Green,
		Red
	};

	uchar		m_color[4];

public:
	inline RGBColor() { }
	inline RGBColor(uchar r, uchar g, uchar b) {
		m_color[Blue] = b; m_color[Green] = g; m_color[Red] = r;
	}

	inline void set(uchar r, uchar g, uchar b) {
		m_color[Blue] = b; m_color[Green] = g; m_color[Red] = r;
	}

	inline uchar r() const { return m_color[Red]; }
	inline uchar g() const { return m_color[Green]; }
	inline uchar b() const { return m_color[Blue]; }

	inline ushort pack16() const {
		return ((m_color[Blue]>>3) + ((m_color[Green]>>2)<<5)+ ((m_color[Red]>>3)<<11) );
	}
	inline void unpack16(ushort clr) {
		m_color[Red] = ((clr<<3)>>11);
		m_color[Green] = ((clr<<2)>>5);
		m_color[Blue] = (clr<<3);
	}
	inline uint pack32() const { return *(const uint*)m_color & 0x00FFFFFF; }
	inline void unpack32(uint clr) { *(uint*)m_color = clr; }

   inline bool operator==(const RGBColor& colin) const {
      return ( m_color[0] == colin.m_color[0] &&
            m_color[1] == colin.m_color[1] &&
            m_color[2] == colin.m_color[2] &&
            m_color[3] == colin.m_color[3]);  
   }
};


inline ushort blend_color16(ushort old_pixel, ushort aColor)
{
	return (((((old_pixel & 0xF800) + (aColor & 0xF800))/2) & 0xF800) +
	        ((((old_pixel & 0x07E1) + (aColor & 0x07E1))/2) & 0x07E1) +
	        ((((old_pixel & 0x001F) + (aColor & 0x001F))/2) & 0x001F));
}

inline void blend_color32(uchar* clr1, const RGBColor& clr2)
{
	clr1[0] = (clr1[0] + clr2.b()) >> 1;
	clr1[1] = (clr1[1] + clr2.g()) >> 1;
	clr1[2] = (clr1[2] + clr2.r()) >> 1;
}

#endif /* RGBCOLOR_H */
