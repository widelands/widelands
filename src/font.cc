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

// Font file definitions - deprecated
// TODO: Replace font files with more standard formats (ttf, bmp?)

#define WLFF_VERSION 	0x0001

#define WLFF_SUFFIX		".wff"
#define WLFF_MAGIC      "WLff"
#define WLFF_VERSIONMAJOR(a)  (a >> 8)
#define WLFF_VERSIONMINOR(a)  (a & 0xFF)

#define FERR_INVAL_FILE -127
#define FERR_INVAL_VERSION -128
// END font file definitions

Font* g_font = 0; // the default font

/*
===============================================================================

Font IMPLEMENTATION

===============================================================================
*/

std::map<const char*, Font*> Font::m_fonts;

/*
===============
Font::Font [private]

Load and initialize a font
===============
*/
Font::Font(const char* name)
{
	m_refs = 0;
	
	// Read the font
	char buf[200];
	FileRead f;
	
	snprintf(buf, sizeof(buf), "fonts/%s.wff", name);
	f.Open(g_fs, buf);
	
	f.Data(6); // skip magic
	
	ushort version = f.Unsigned16();

	if(WLFF_VERSIONMAJOR(version) > WLFF_VERSIONMAJOR(WLFF_VERSION))
		throw wexception("%s: bad font version", buf);
	if(WLFF_VERSIONMAJOR(version) == WLFF_VERSIONMAJOR(WLFF_VERSION))
		if(WLFF_VERSIONMINOR(version) > WLFF_VERSIONMINOR(WLFF_VERSION))
			throw wexception("%s: bad minor font version", buf);

	f.Data(20); // skip name
	
	ushort clrkey = f.Unsigned16();
	m_height = f.Unsigned16();

	// Read in the characters
	for(int i = 0; i < 96; i++)
		{
		uchar c = f.Unsigned8();
		if (c != (i+32))
			throw wexception("%s: bad character order", buf);
		
		int w = f.Unsigned16();

		ushort *data = (ushort*)f.Data(sizeof(ushort)*w*m_height);
		m_pictures[i].create(w, m_height, data);
		m_pictures[i].set_clrkey(clrkey);
		}

	// Add to list
	m_name = strdup(name);
	m_fonts[m_name] = this;
}


/*
===============
Font::~Font [private]

Free resources.
===============
*/
Font::~Font()
{
	if (m_refs)
		log("Font::~Font: Oops, m_refs == %i, name = %s\n", m_refs, m_name);
	
	if (m_name)
		{
		std::map<const char*, Font*>::iterator it;
		 
		it = m_fonts.find(m_name);
		if (it != m_fonts.end())
			m_fonts.erase(it);

		free(m_name);
		}
}


/*
===============
Font::load [static]

Load the given font. If the font is already loaded, the old object is reused.
Throws an exception if the font cannot be loaded.
===============
*/
Font* Font::load(const char* name)
{
	std::map<const char*, Font*>::iterator it;
	Font* font;
	
	it = m_fonts.find(name);
	if (it != m_fonts.end())
		font = it->second;
	else
		try
			{
			font = new Font(name);
			}
		catch(std::exception& e)
			{
			throw wexception("Couldn't load font %s: %s\n", name, e.what());
			}
	
	font->addref();
	return font;
}


/*
===============
Font::addref

Increase the reference counter of the font.
===============
*/
void Font::addref()
{
	m_refs++;
	
	assert(m_refs > 0);
}


/*
===============
Font::release

Decrease the reference counter of the font.
Free the font when the refcount when it reaches zero.
===============
*/
void Font::release()
{
	assert(m_refs > 0);
	
	m_refs--;
	if (!m_refs)
		delete this;
}


/*
===============
Font::get_string

This function constructs a Picture containing the given text and
returns it

TODO: Does it make sense to keep this function? It's not used, but I'll leave
it around for now.
===============
*/
Pic* Font::get_string(const char* str)
{
	int w, h;
	Pic* pic;
	
	get_size(str, &w, &h, -1);
	
	pic = new Pic;
	pic->set_clrkey(m_pictures[0].get_clrkey());
	pic->set_size(w, h);
	
	draw_string(pic, 0, 0, str, Align_Left, -1);
	
	return pic;
}


/*
===============
Font::calc_linewidth

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
int Font::calc_linewidth(const char* line, int wrap, const char** nextline)
{
	const char* string = line; // current pointer into the string
	int width = 0; // width of line
	
	// Loop forward
	for(;;)
		{
		if (*string == ' ' || *string == '\t') // whitespace
			{
			int cw = m_pictures[0].get_w();
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
				wordwidth += m_pictures[c].get_w();
				
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
Font::draw_string

Draw a string directly into the destination bitmap with the desired alignment.
The function honours line-breaks.
If wrap is positive, the function will wrap a line after that many pixels.
===============
*/
void Font::draw_string(Bitmap* dst, int dstx, int dsty, const char* string,
                       Align align, int wrap)
{
	// Adjust for vertical alignment
	if (align & (Align_VCenter|Align_Bottom))
		{
		int h;
		
		get_size(string, 0, &h, wrap);
		
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
			int width = calc_linewidth(string, wrap, &nextline);
			
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
				int cw = m_pictures[0].get_w();
				if (c == '\t')
					cw *= 8;

				x += cw;
				}
			else if (c && c != '\n')
				{
				if (c < 32 || c > 127)
					c = 127;

				c -= 32;
				copy_pic(dst, &m_pictures[c], x, dsty, 0, 0, m_pictures[c].get_w(), m_height);
				x += m_pictures[c].get_w();
				}
			
			string++;
			}
		
		dsty += m_height;
		}
}
		

/*
===============
Font::get_size

Calculate the size of the given string.
pw and ph may be NULL.
If wrap is positive, the function will wrap a line after that many pixels
===============
*/
void Font::get_size(const char* string, int* pw, int* ph, int wrap)
{
	int maxw = 0; // width of widest line
	int maxh = 0; // total height
	
	while(*string)
		{
		const char* nextline;
		int width = calc_linewidth(string, wrap, &nextline);
		
		if (width > maxw)
			maxw = width;
		maxh += m_height;

		string = nextline;
		}
	
	if (pw)
		*pw = maxw;
	if (ph)
		*ph = maxh;
}


/*
===============
Font::get_fontheight

Returns the height of the font, in pixels.
===============
*/
int Font::get_fontheight()
{
	return m_height;
}
