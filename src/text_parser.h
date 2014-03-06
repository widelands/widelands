/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef TEXT_PARSER_H
#define TEXT_PARSER_H

#include <string>
#include <vector>

#include "align.h"
#include "rgbcolor.h"

namespace UI {

/**
 * Corresponds to a richtext block that is enclosed in <p></p> tags.
 * Has uniform font style, contains text pre-split into words, and keeps track of
 * manual line breaks (<br>) in a separate structure.
 */
struct Text_Block {
	Text_Block();
	Text_Block(const Text_Block & src);

	void set_font_size(int32_t const font_size) {m_font_size = font_size;}
	int32_t get_font_size() const {return m_font_size;}

	void set_font_color(const RGBColor & font_color) {m_font_color = font_color;}
	RGBColor get_font_color() const {return m_font_color;}

	void set_font_weight(const std::string & font_weight) {
		m_font_weight = font_weight;
	}
	const std::string & get_font_weight() const {return m_font_weight;}

	void set_font_style(const std::string & font_style) {
		m_font_style = font_style;
	}
	const std::string & get_font_style() const {return m_font_style;}

	void set_font_decoration(const std::string & font_decoration) {
		m_font_decoration = font_decoration;
	}
	const std::string & get_font_decoration() const {return m_font_decoration;}

	void set_font_face(const std::string & font_face) {m_font_face = font_face;}
	const std::string & get_font_face() const {return m_font_face;}

	void set_line_spacing(int32_t const line_spacing) {
		m_line_spacing = line_spacing;
	}
	int32_t get_line_spacing() const {return m_line_spacing;}

	void set_words(const std::vector<std::string> & words) {m_words = words;}
	const std::vector<std::string> & get_words() const {return m_words;}

	void set_line_breaks
		(const std::vector<std::vector<std::string>::size_type> & line_breaks)
	{
		m_line_breaks = line_breaks;
	}
	const std::vector<std::vector<std::string>::size_type> & get_line_breaks
		() const
	{
		return m_line_breaks;
	}
private:
	int32_t                                          m_font_size;
	RGBColor                                         m_font_color;
	std::string                                      m_font_weight;
	std::string                                      m_font_style;
	std::string                                      m_font_decoration;
	std::string                                      m_font_face;
	int32_t                                          m_line_spacing;
	std::vector<std::string>                         m_words;

	/**
	 * Position of manual line breaks (<br>) with respect to @ref m_words.
	 * Sorted in ascending order.
	 * An entry j in this vector means that a manual line break occurs
	 * before the j-th word in @ref m_words. In particular, an entry 0
	 * means that a manual line break occurs before the first word.
	 * Entries can appear with multiplicity, indicating that multiple
	 * manual line breaks exist without any words in-between.
	 */
	std::vector<std::vector<std::string>::size_type> m_line_breaks;
};

struct Richtext_Block {
	Richtext_Block();
	Richtext_Block(const Richtext_Block & src);

	void set_images(const std::vector<std::string> & images) {
		m_images = images;
	}
	const std::vector<std::string> & get_images() const {return m_images;}

	void set_image_align(Align const image_align) {m_image_align = image_align;}
	Align get_image_align() const {return m_image_align;}

	void set_text_align(Align const text_align) {m_text_align = text_align;}
	Align get_text_align() const {return m_text_align;}

	void set_text_blocks(const std::vector<Text_Block> & text_blocks) {
		m_text_blocks = text_blocks;
	}
	const std::vector<Text_Block> & get_text_blocks() const {
		return m_text_blocks;
	}
private:
	std::vector<std::string> m_images;
	std::vector<Text_Block>  m_text_blocks;
	Align                    m_image_align;
	Align                    m_text_align;
};

struct Text_Parser {
	void parse
		(std::string & text,
		 std::vector<Richtext_Block> & blocks);
private:
	void parse_richtexttext_attributes(std::string format, Richtext_Block *);
	bool parse_textblock
		(std::string                                       & block,
		 std::string                                       & block_format,
		 std::vector<std::string>                          & words,
		 std::vector<std::vector<std::string>::size_type>  & line_breaks);
	void parse_text_attributes(std::string format, Text_Block &);
	bool extract_format_block
		(std      ::string & block,
		 std      ::string & block_text,
		 std      ::string & block_format,
		 const std::string & block_start,
		 const std::string & format_end,
		 const std::string & block_end);
	Align set_align(const std::string &);
	void split_words(const std::string & in, std::vector<std::string> * plist);
};

}

#endif
