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
   return (text.substr(0,3) == "<rt");
}

class Text_Block {
   public:
   	Text_Block();
   	Text_Block(const Text_Block &src);

   	inline void set_font_size(int font_size) { m_font_size = font_size;};
		inline int get_font_size() { return m_font_size; };

		inline void set_font_color(RGBColor font_color){ m_font_color = font_color;};
		inline RGBColor get_font_color(){ return m_font_color; };

		inline void set_font_weight(std::string font_weight){ m_font_weight = font_weight;};
		inline std::string get_font_weight(){ return m_font_weight; };

		inline void set_font_style(std::string font_style){ m_font_style = font_style;};
		inline std::string get_font_style(){ return m_font_style; };

		inline void set_font_decoration(std::string font_decoration){ m_font_decoration = font_decoration;};
		inline std::string get_font_decoration(){ return m_font_decoration; };

		inline void set_font_face(std::string font_face) { m_font_face = font_face; };
   	inline std::string get_font_face() { return m_font_face; };

   	inline void set_line_spacing(int line_spacing) { m_line_spacing = line_spacing; };
   	inline int get_line_spacing() { return m_line_spacing; };

   	inline void set_words(std::vector<std::string> words) { m_words = words; };
   	inline std::vector<std::string> get_words() { return m_words; };

   	inline void set_line_breaks(std::vector<uint> line_breaks) { m_line_breaks = line_breaks; };
   	inline std::vector<uint> get_line_breaks() { return m_line_breaks; };
   private:
   	int m_font_size;
		RGBColor m_font_color;
		std::string m_font_weight;
		std::string m_font_style;
		std::string m_font_decoration;
		std::string m_font_face;
		int m_line_spacing;
		std::vector<std::string> m_words;
		std::vector<uint> m_line_breaks;
};

class Richtext_Block {
	public:
		Richtext_Block();
		Richtext_Block(const Richtext_Block &src);

		inline void set_images(std::vector<std::string> images) { m_images = images;};
		inline std::vector<std::string> get_images() { return m_images; };

		inline void set_image_align(Align image_align) { m_image_align = image_align; };
		inline Align get_image_align() { return m_image_align; };

		inline void set_text_align(Align text_align) { m_text_align = text_align; };
		inline Align get_text_align() { return m_text_align; };

		inline void set_text_blocks(std::vector<Text_Block> text_blocks) { m_text_blocks = text_blocks;};
		inline std::vector<Text_Block> get_text_blocks() { return m_text_blocks; };
  	private:
		std::vector<std::string> m_images;
		std::vector<Text_Block> m_text_blocks;
		Align m_image_align;
		Align m_text_align;
};

class Text_Parser {
   public:
      Text_Parser();
      ~Text_Parser();
		void parse(std::string *text, std::vector<Richtext_Block> *blocks, Varibale_Callback, void*);
   private:
		void parse_richtexttext_attributes(std::string format, Richtext_Block *element);
		bool parse_textblock(std::string *block, std::string *block_format, std::vector<std::string> *words, std::vector<uint> *line_breaks, Varibale_Callback, void*);
		void parse_text_attributes(std::string format, Text_Block *element);
		bool extract_format_block(std::string *block, std::string *block_text, std::string *block_format, std::string block_start, std::string format_end, std::string block_end);
      Align set_align(std::string align);
		void split_words(std::string in, std::vector<std::string>* plist);
};


#endif
