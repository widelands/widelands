/*
 * Copyright (C) 2002-2007 by the Widelands Development Team
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

#include "font_handler.h"

inline bool is_richtext(std::string text) {
   return (text.substr(0, 3) == "<rt");
}

struct Text_Block {
	Text_Block();
	Text_Block(const Text_Block & src);

	void set_font_size(const int font_size) throw () {m_font_size = font_size;}
	int get_font_size() const throw () {return m_font_size;}

	void set_font_color(const RGBColor font_color) throw () {m_font_color = font_color;}
	RGBColor get_font_color() const throw () {return m_font_color;}

	void set_font_weight(const std::string & font_weight) throw ()
	{m_font_weight = font_weight;};
	const std::string & get_font_weight() const throw () {return m_font_weight;}

	void set_font_style(const std::string & font_style) throw ()
	{m_font_style = font_style;}
	const std::string & get_font_style() const throw () {return m_font_style;}

	void set_font_decoration(const std::string & font_decoration) throw ()
	{m_font_decoration = font_decoration;}
	const std::string & get_font_decoration() const throw ()
	{return m_font_decoration;}

	void set_font_face(const std::string & font_face) throw ()
	{m_font_face = font_face;}
	const std::string & get_font_face() const throw () {return m_font_face;}

	void set_line_spacing(const int line_spacing) throw ()
	{m_line_spacing = line_spacing;};
	int get_line_spacing() const throw () {return m_line_spacing;};

	void set_words(const std::vector<std::string> & words) {m_words = words;}
	const std::vector<std::string> & get_words() const throw () {return m_words;}

	void set_line_breaks
		(const std::vector<std::vector<std::string>::size_type> & line_breaks)
	{m_line_breaks = line_breaks;};
	const std::vector<std::vector<std::string>::size_type> & get_line_breaks
		() const throw ()
	{return m_line_breaks;}
private:
	int                      m_font_size;
		RGBColor m_font_color;
		std::string m_font_weight;
		std::string m_font_style;
		std::string m_font_decoration;
		std::string m_font_face;
		int m_line_spacing;
		std::vector<std::string> m_words;
	std::vector<std::vector<std::string>::size_type> m_line_breaks;
};

struct Richtext_Block {
		Richtext_Block();
		Richtext_Block(const Richtext_Block &src);

	void set_images(const std::vector<std::string> & images) throw ()
	{m_images = images;}
	const std::vector<std::string> & get_images() const throw ()
	{return m_images;}

	void set_image_align(const Align image_align) throw ()
	{m_image_align = image_align;}
	Align get_image_align() const throw () {return m_image_align;}

	void set_text_align(const Align text_align) throw ()
	{m_text_align = text_align;}
	Align get_text_align() const throw () {return m_text_align;}

	void set_text_blocks(const std::vector<Text_Block> & text_blocks) throw ()
	{m_text_blocks = text_blocks;}
	const std::vector<Text_Block> & get_text_blocks() const throw ()
	{return m_text_blocks;}
private:
		std::vector<std::string> m_images;
		std::vector<Text_Block> m_text_blocks;
		Align m_image_align;
		Align m_text_align;
};

struct Text_Parser {
      Text_Parser();
      ~Text_Parser();
	void parse
		(std::string & text,
		 std::vector<Richtext_Block> & blocks,
		 Varibale_Callback, void * const);
private:
		void parse_richtexttext_attributes(std::string format, Richtext_Block *element);
	bool parse_textblock
		(std::string                                       & block,
		 std::string                                       & block_format,
		 std::vector<std::string>                          & words,
		 std::vector<std::vector<std::string>::size_type>  & line_breaks,
		 Varibale_Callback vcb, void * const vcdata);
		void parse_text_attributes(std::string format, Text_Block *element);
	bool extract_format_block
		(std      ::string & block,
		 std      ::string & block_text,
		 std      ::string & block_format,
		 const std::string & block_start,
		 const std::string & format_end,
		 const std::string & block_end);
      Align set_align(std::string align);
		void split_words(std::string in, std::vector<std::string>* plist);
};


#endif
