/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#include <string>
#include "types.h"
#include "rgbcolor.h"

/*
 * Font
 *
 * this represents a loaded font used by the FontHandler
 */
class Font {
   public:
      Font(std::string name, int pointsize, RGBColor fg, RGBColor bg);
      ~Font();

      // TODO: hackish: c==index to m_pictures
      void  get_char_size(int c, int* w, int *h) { *w=m_pictures[c].width, *h=get_pixel_height(); }
      int get_char_pic(int c) { return m_pictures[c].pic; }
      int get_font_height(void) { return m_height; }

      std::string get_name(void) const { return m_name; }
      int get_pixel_height(void) const { return m_height; }
      int get_point_height(void) const { return m_pointheight; }
      RGBColor get_fg_clr(void) const { return m_fg; }
      RGBColor get_bg_clr(void) const { return m_bg; }

      void do_load();

   private:
     struct Char {
		int	width;
		uint	pic;
	};

     std::string m_name;
      int m_height; // height in pixeln. a 12pt TT-Font has m_height>12
      int m_pointheight;
      Char m_pictures[96];
      RGBColor m_fg, m_bg;

};

#endif // __S__FONT_H
