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

#include "widelands.h"
#include "font.h"


/** class Font_Handler
 *
 * This class generates font Pictures out of strings and returns them
 *
 * It's a singleton
 *
 * It's a little ugly, since every char is hold in it's own pic which is quite a waste of 
 * good resources
 * 
 * DEPENDS: class	Graph::Pic
 * DEPENDS:	func	Graph::copy_pic
 * DEPENDS: class	myfile
 */

/** Font_Handler::Font_Handler(void)
 *
 * Simple inits
 *
 * Agrs: None
 * Returns: Nothing
 */
Font_Handler::Font_Handler(void) {
		  for (uint i=0; i<MAX_FONTS; i++) {
					 fonts[i].h=0;
		  }
}

/** Font_Handler::~Font_Handler(void) 
 *
 * Simple cleanups
 *
 * Args: None
 * Returns: Nothing
 */
Font_Handler::~Font_Handler(void) {
}

/** int Font_Handler::load_font(const char* str, ushort fn )
 *
 * This registers a certain font with the given
 * objects
 *
 * Args:	str	name of the font we want to load
 * 		fn 	number of font to register
 *	Returns: ERR_FAILED, FERR_INVAL_VERSION, FERR_INVAL_FILE, RET_OK
 */
int Font_Handler::load_font(const char* str, ushort fn)
{
	assert(fn<MAX_FONTS);
	assert(str);

	char buf[200];
	FileRead f;
	
	snprintf(buf, sizeof(buf), "fonts/%s.wff", str);
	f.Open(g_fs, buf);
	
	// TODO: actually use FileRead's nice features for endian safety
	FHeader *fh = (FHeader*)f.Data(sizeof(FHeader));

	if(WLFF_VERSIONMAJOR(fh->version) > WLFF_VERSIONMAJOR(WLFF_VERSION)) {
		return FERR_INVAL_VERSION;
	}
	if(WLFF_VERSIONMAJOR(fh->version) == WLFF_VERSIONMAJOR(WLFF_VERSION)) {
		if(WLFF_VERSIONMINOR(fh->version) > WLFF_VERSIONMINOR(WLFF_VERSION)) {
			return FERR_INVAL_VERSION;
		}
	}

	fonts[fn].h = fh->height;

	uchar c;
	ushort w;
	for(unsigned int i=0; i<96; i++) {
		c = f.Unsigned8();
		if (c != (i+32))
			return FERR_INVAL_FILE;
		
		w = f.Unsigned16();

		ushort *data = (ushort*)f.Data(sizeof(ushort)*w*fonts[fn].h);
		fonts[fn].p[i].create(w, fonts[fn].h, data);
		fonts[fn].p[i].set_clrkey(fh->clrkey);
	}
	
	return RET_OK;
}

/*
===============
Font_Handler::get_string

This function constructs a Picture containing the given text and
returns it

TODO: Does it make sense to keep this function? It's not used, but I'll leave
it around for now.
===============
*/
Pic* Font_Handler::get_string(const char* str, const ushort f)
{
	int w, h;
	Pic* pic;
	
	get_size(str, &w, &h, -1, f);
	
	pic = new Pic;
	pic->set_clrkey(fonts[f].p[0].get_clrkey());
	pic->set_size(w, h);
	
	draw_string(pic, 0, 0, str, Align_Left, -1, f);
	
	return pic;
}


/*
===============
Font_Handler::calc_linewidth

Calculates the width of the given line (up to \n or NUL).
If wrap is positive, the function will wrap the line after that many pixels.
*nextline will point to the first character on the next line. That is, it 
points to:
- the character after the \n if the line is ended by a line-break
- the NUL if the string ends after the line
- the first visible character on the next line in case the line is wrapped

The function returns the width of the line, in pixels.
===============
*/
int Font_Handler::calc_linewidth(const char* line, int wrap, const char** nextline, ushort font)
{
	const char* string = line; // current pointer into the string
	int width = 0; // width of line
	
	// Loop forward
	for(;;)
		{
		if (*string == ' ' || *string == '\t') // whitespace
			{
			int cw = fonts[font].p[0].get_w();
			if (*string == '\t')
				cw *= 8;
			
			string++;
			
			if (wrap > 0 && width+cw > wrap)
				break;
			else
				width += cw;
			}
		else if (!*string || *string == '\n') // explicit end of line
			{
			if (*string)
				string++;
			break;
			}
		else // normal word
			{
			const char* p;
			int wordwidth = 0;
			
			for(p = string;; p++)
				{
				if (!*p || *p == ' ' || *p == '\t' || *p == '\n') // whitespace break
					break;
				
				uchar c = (uchar)*p;
				if (c < 32 || c > 127)
					c = 127;
				
				c -= 32;
				wordwidth += fonts[font].p[c].get_w();
				
				if (*p == '-') // other character break
					{
					p++;
					break;
					}
				}
			
			if (wrap > 0 && width && width+wordwidth > wrap)
				break;
			else
				{
				string = p;
				width += wordwidth;
				}
			}
		}
	
	// That's it
	if (nextline)
		*nextline = string;
	
	return width;
}


/*
===============
Font_Handler::draw_string

Draw a string directly into the destination bitmap with the desired alignment.
The function honours line-breaks.
If wrap is positive, the function will wrap a line after that many pixels.
===============
*/
void Font_Handler::draw_string(Bitmap* dst, int dstx, int dsty, const char* string,
                               Align align, int wrap, ushort font)
{
	// Adjust for vertical alignment
	if (align & (Align_VCenter|Align_Bottom))
		{
		int h;
		
		get_size(string, 0, &h, wrap, font);
		
		if (align & Align_VCenter)
			dsty -= (h+1)/2; // +1 for slight bias to top
		else
			dsty -= h;
		}
	
	// Draw the string
	while(*string)
		{
		const char* nextline;
		int x = dstx;
		
		if (wrap <= 0 && (align & Align_Horizontal) == Align_Left)
			{
			// straightforward path with no alignment and no wrapping
			nextline = string + strcspn(string, "\n");
			}
		else
			{
			int width = calc_linewidth(string, wrap, &nextline, font);
			
			if (align & Align_HCenter)
				x -= width/2;
			else if (align & Align_Right)
				x -= width;
			}
		
		while(string < nextline)
			{
			uchar c = (uchar)*string;
			
			if (c == ' ' || c == '\t') // whitespace
				{
				int cw = fonts[font].p[0].get_w();
				if (c == '\t')
					cw *= 8;

				x += cw;
				}
			else if (c && c != '\n')
				{
				if (c < 32 || c > 127)
					c = 127;

				c -= 32;
				copy_pic(dst, &fonts[font].p[c], x, dsty, 0, 0, fonts[font].p[c].get_w(), fonts[font].h);
				x += fonts[font].p[c].get_w();
				}
			
			string++;
			}
		
		dsty += fonts[font].h;
		}
}
		

/*
===============
Font_Handler::get_size

Calculate the size of the given string.
pw and ph may be NULL.
If wrap is positive, the function will wrap a line after that many pixels
===============
*/
void Font_Handler::get_size(const char* string, int* pw, int* ph, int wrap, ushort font)
{
	int maxw = 0; // width of widest line
	int maxh = 0; // total height
	
	while(*string)
		{
		const char* nextline;
		int width = calc_linewidth(string, wrap, &nextline, font);
		
		if (width > maxw)
			maxw = width;
		maxh += fonts[font].h;

		string = nextline;
		}
	
	if (pw)
		*pw = maxw;
	if (ph)
		*ph = maxh;
}
