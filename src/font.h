/*
 * Copyright (C) 2001 by Holger Rapp 
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

#ifndef __S__FONT_H
#define __S__FONT_H

#include "graphic.h"
#include "singleton.h"

// Ugly: Size of chars is hard coded for widelands. 
// Somebody's feeling like defining a font file format?
#define FONT_H 	7
#define FONT_W		7

/** class Font_Handler
 *
 * This class generates font Pictures out of strings and returns them
 * This is sure that the user want's fixed font strings
 *
 * It's a singleton
 * 
 * DEPENDS: class	Graph::Pic
 * DEPENDS:	func	Graph::copy_pic
 */
class Font_Handler : public Singleton<Font_Handler> {
		  Font_Handler(const Font_Handler&);
		  Font_Handler& operator=(const Font_Handler&);

		  public:
					 const static unsigned short MAX_FONTS=2;
		  
					 Font_Handler();
					 ~Font_Handler();

					 
					 void set_font(const unsigned short, Pic*, const unsigned short, const unsigned short);
					 Pic* get_string(const char*, const unsigned short);
					 
					 /** inline unsigned short get_fh(unsigned short f) 
					  *
					  * This function returns the height of the given font
					  * Args: f	which font to check
					  * Returns: height
					  */
					 inline unsigned short get_fh(unsigned short f) { assert(f<MAX_FONTS); assert(h[f]); return h[f]; }
					 
					 /** inline unsigned short get_fw(unsigned short f) 
					  *
					  * This function returns the width of the given font
					  * Args: f	which font to check
					  * Returns: width
					  */
					 inline unsigned short get_fw(unsigned short f) { assert(f<MAX_FONTS); assert(w[f]); return w[f]; }
					 
		  private:
					 unsigned short w[MAX_FONTS];
					 unsigned short h[MAX_FONTS];
					 Pic*	pics[MAX_FONTS];
		  
};

#define g_fh	Font_Handler::get_singleton()

#endif /* __S__FONT_H */
