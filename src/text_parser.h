/*
 * Copyright (C) 2002-2005 by the Widelands Development Team
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

#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include <vector>
#include "font_handler.h"

class RGBColor;

inline bool is_richtext(std::string text) {
   return (text.substr(0,2) == "<p");
}

struct Text_Block {
   std::string text;
   Align text_align;
   std::string font_face;
   int font_size;
   RGBColor font_color;
   std::string font_weight;
   std::string font_style;
   std::string font_decoration;
   int line_spacing;
   std::string image;
   Align image_align;
};

class Text_Parser {
   public:
      Text_Parser();
      ~Text_Parser();
      void parse(std::string text, std::vector<Text_Block> *blocks);
   private:
      void parse_attributes(std::string format, Text_Block *element);
      Align set_align(std::string align);
};


#endif
