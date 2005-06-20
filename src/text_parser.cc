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

#include <vector>
#include <string>
#include "text_parser.h"
#include "graphic.h"
#include "error.h"


Text_Parser::Text_Parser(){
}

Text_Parser::~Text_Parser(){
}

void Text_Parser::parse(std::string text, std::vector<Text_Block> *blocks) {  
   while (text.size()) {
      if (text.substr(0,2) != "<p") {
         uint block_start = text.find("<p");
         text.erase(0,block_start);
         continue;
      }
      text.erase(0,2);
      
      uint format_end = text.find(">");
      if (format_end == std::string::npos) {
         log("WARNING: Formatdefinition of block '%s' not closed\n",(text.substr(0,30)+"...").c_str());
         return;
      }
      
      std::string block_format = text.substr(0,format_end);
      text.erase(0,format_end+1);
            
      uint block_end = text.find("</p>");
      if (block_end == std::string::npos) {
      
         log("WARNING: Block '%s' not closed!\n",(text.substr(0,30)+"...").c_str());
         return;
      }
      
      std::string block_text = text.substr(0,block_end);
      text.erase(0,block_end+4);
      
      if (!block_text.size())
         block_text = " ";
      
      Text_Block new_block = {
         block_text,
         Align_Left,
         "Vera.ttf",
         10,
         RGBColor(255,255,0),
         "normal",
         "normal",
         "none",
         0,
         "",
         Align_Left,
      };
      
      parse_attributes(block_format,&new_block);
      blocks->push_back(new_block);
   }
}

void Text_Parser::parse_attributes(std::string format, Text_Block *element) {
   if (!format.size())
      return;
   if (format[0] == ' ')
      format.erase(0,1);

   while (format.size()) {
      uint key_end = format.find("=");
      if (key_end == std::string::npos)
         return;
      else {
         std::string key = format.substr(0,key_end);
         format.erase(0,key_end+1);
         uint val_end = format.find(" ");
         if (val_end == std::string::npos)
            val_end = format.size();
         std::string val = format.substr(0,val_end);
         format.erase(0,val_end+1);
         if (key == "font-face")
            element->font_face = val+".ttf";
         else if (key == "font-color") {
            std::string r = "0x"+val.substr(1,2);
            std::string g = "0x"+val.substr(3,2);
            std::string b = "0x"+val.substr(5,2);
            char *ptr;
            int red = strtol(r.c_str(),&ptr,0);
            int green = strtol(g.c_str(),&ptr,0);
            int blue = strtol(b.c_str(),&ptr,0);
            element->font_color = RGBColor(red,green,blue);
         }
         else if (key == "font-size")
            element->font_size = atoi(val.c_str());
         else if (key == "font-weight")
            element->font_weight = val;
         else if (key == "font-style")
            element->font_style = val;
         else if (key == "font-decoration")
            element->font_decoration = val;
         else if (key == "text-align")
            element->text_align = set_align(val);
         else if (key == "line-spacing")
            element->line_spacing = atoi(val.c_str());
         else if (key == "image")
            element->image = val;
         else if (key == "image-align")
            element->image_align = set_align(val);
      }
   }
}

Align Text_Parser::set_align(std::string align) {
   if (align == "right")
      return Align_Right;
   else if (align == "center")
      return Align_HCenter;
   else
      return Align_Left;
}

