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
#define FONT_H 		7
#define FONT_W		7

#define MAX_FONTS	2

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
		  
					 Font_Handler();
					 ~Font_Handler();

					 
					 void set_font(const ushort, Pic*, const ushort, const ushort);
					 Pic* get_string(const char*, const ushort);
					 
					 /** inline ushort get_fh(ushort f) 
					  *
					  * This function returns the height of the given font
					  * Args: f	which font to check
					  * Returns: height
					  */
					 inline ushort get_fh(ushort f) { assert(f<MAX_FONTS); assert(h[f]); return h[f]; }
					 
					 /** inline ushort get_fw(ushort f) 
					  *
					  * This function returns the width of the given font
					  * Args: f	which font to check
					  * Returns: width
					  */
					 inline ushort get_fw(ushort f) { assert(f<MAX_FONTS); assert(w[f]); return w[f]; }
					 
		  private:
					 ushort w[MAX_FONTS];
					 ushort h[MAX_FONTS];
					 Pic*	pics[MAX_FONTS];
		  
};

#define g_fh	Font_Handler::get_singleton()

#endif /* __S__FONT_H */
