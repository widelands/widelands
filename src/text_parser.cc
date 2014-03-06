/*
 * Copyright (C) 2002-2005, 2007-2011 by the Widelands Development Team
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

#include "text_parser.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "constants.h"
#include "container_iterate.h"
#include "helper.h"
#include "log.h"

namespace UI {

Richtext_Block::Richtext_Block() :
	m_image_align(Align_Left),
	m_text_align (Align_Left)
{}

Richtext_Block::Richtext_Block(const Richtext_Block & src) {
	m_images.clear();
	m_text_blocks.clear();
	for (uint32_t i = 0; i < src.m_images.size(); ++i)
		m_images.push_back(src.m_images[i]);
	for (uint32_t i = 0; i < src.m_text_blocks.size(); ++i)
		m_text_blocks.push_back(src.m_text_blocks[i]);
	m_image_align = src.m_image_align;
	m_text_align = src.m_text_align;
}

Text_Block::Text_Block() {
	m_font_size = 10;
	m_font_color = RGBColor(255, 255, 0);
	m_font_weight = "normal";
	m_font_style = "normal";
	m_font_decoration = "none";
	m_font_face = "DejaVuSans.ttf";
	m_line_spacing = 0;
}

Text_Block::Text_Block(const Text_Block & src) {
	m_words.clear();
	m_line_breaks.clear();
	for (uint32_t i = 0; i < src.m_words.size(); ++i)
		m_words.push_back(src.m_words[i]);
	for (uint32_t i = 0; i < src.m_line_breaks.size(); ++i)
		m_line_breaks.push_back(src.m_line_breaks[i]);
	m_font_size       = src.m_font_size;
	m_font_color      = src.m_font_color;
	m_font_weight     = src.m_font_weight;
	m_font_style      = src.m_font_style;
	m_font_decoration = src.m_font_decoration;
	m_font_face       = src.m_font_face;
	m_line_spacing    = src.m_line_spacing;
}


void Text_Parser::parse
	(std::string                 & text,
	 std::vector<Richtext_Block> & blocks)
{
	bool more_richtext_blocks = true;
	//First while loop parses all richtext blocks (images)
	while (more_richtext_blocks) {
		Richtext_Block new_richtext_block;
		std::string unparsed_text;
		std::string richtext_format;

		more_richtext_blocks =
			extract_format_block
				(text,
				 unparsed_text,
				 richtext_format,
				 std::string("<rt"),
				 std::string(">"),
				 std::string("</rt>"));
		parse_richtexttext_attributes(richtext_format, &new_richtext_block);

		std::vector<Text_Block> text_blocks;

		//Second while loop parses all textblocks of current richtext block
		bool more_text_blocks = true;
		while (more_text_blocks) {
			std::string block_format;
			Text_Block new_block;

			std::vector<std::string> words;
			std::vector<std::vector<std::string>::size_type> line_breaks;

			more_text_blocks =
				parse_textblock
					(unparsed_text, block_format, words, line_breaks);
			parse_text_attributes(block_format, new_block);

			new_block.set_words(words);
			new_block.set_line_breaks(line_breaks);
			text_blocks.push_back(new_block);
		}
		new_richtext_block.set_text_blocks(text_blocks);
		blocks.push_back(new_richtext_block);
	}
}

bool Text_Parser::parse_textblock
	(std::string                                      & block,
	 std::string                                      & block_format,
	 std::vector<std::string>                         & words,
	 std::vector<std::vector<std::string>::size_type> & line_breaks)
{
	std::string block_text;

	const bool extract_more = extract_format_block(block, block_text, block_format, "<p", ">", "</p>");

	//Split the the text because of " "
	std::vector<std::string> unwrapped_words;
	split_words(block_text, &unwrapped_words);

	//Handle user defined line breaks, and save them
	container_iterate_const(std::vector<std::string>, unwrapped_words, i)
		for (std::string line = *i.current;;) {
			std::string::size_type next_break = line.find("<br>");

			// Replace &lt; with <
			std::string::size_type smaller = line.find("&lt;");
			while (smaller != std::string::npos) {
				line.replace(smaller, 4, "<");
				if (next_break > smaller)
					// Fix position of <br> tag
					next_break -= 3;
				smaller = line.find("&lt;");
			}

			if (next_break == std::string::npos) {
				if (line.size())
					words.push_back(line);
				break;
			} else if (next_break)
				words.push_back(line.substr(0, next_break));
			line_breaks.push_back(words.size());
			line.erase(0, next_break + 4);
		}
	return extract_more;
}

void Text_Parser::split_words(const std::string & in, std::vector<std::string>* plist)
{
	std::string::size_type pos = 0;

	while (pos < in.size()) {
		while (pos < in.size() && isspace(in[pos]))
			++pos;
		if (pos >= in.size())
			break;

		std::string::size_type nextspace = pos;
		while (nextspace < in.size() && !isspace(in[nextspace]))
			++nextspace;

		if (nextspace > pos)
			plist->push_back(in.substr(pos, nextspace - pos));

		pos = nextspace;
	}
}

bool Text_Parser::extract_format_block
	(std::string       & block,
	 std::string       & block_text,
	 std::string       & block_format,
	 const std::string & block_start,
	 const std::string & format_end,
	 const std::string & block_end)
{
	if (block.compare(0, block_start.size(), block_start)) {
		const std::string::size_type format_begin_pos = block.find(block_start);
		if (format_begin_pos == std::string::npos) {
			return false;
		}
		block.erase(0, format_begin_pos);
	}

	block.erase(0, block_start.size());
	if (block.size() and *block.begin() == ' ')
		block.erase(0, 1);

	const std::string::size_type format_end_pos = block.find(format_end);
	if (format_end_pos == std::string::npos) {
		return false;
	}

	//Append block_format
	block_format.erase();
	block_format.append(block.substr(0, format_end_pos));

	//Delete whole format block
	block.erase(0, format_end_pos + format_end.size());

	//Find end of block
	const std::string::size_type block_end_pos = block.find(block_end);
	if (block_end_pos == std::string::npos) {
		return false;
	}
	//Extract text of block
	block_text.erase();
	block_text.append(block.substr(0, block_end_pos));

	//Erase text including closing tag
	block.erase(0, block_end_pos + block_end.size());
	//Is something left
	return block.find(block_start) != std::string::npos;
}

void Text_Parser::parse_richtexttext_attributes
	(std::string format, Richtext_Block * const element)
{
	if (format.empty())
		return;
	if (format[0] == ' ')
		format.erase(0, 1);

	while (format.size()) {
		std::string::size_type const key_end = format.find('=');
		if (key_end == std::string::npos)
			return;
		else {
			std::string const key = format.substr(0, key_end);
			format.erase(0, key_end + 1);
			std::string::size_type val_end = format.find(' ');
			if (val_end == std::string::npos)
				val_end = format.size();
			std::string val = format.substr(0, val_end);
			format.erase(0, val_end + 1);
			if        (key == "image") {
				const std::vector<std::string> images(split_string(val, ";"));
				element->set_images(images);
			} else if (key == "image-align")
				element->set_image_align(set_align(val));
			else if   (key == "text-align")
				element->set_text_align(set_align(val));
		}
	}
}

void Text_Parser::parse_text_attributes
	(std::string format, Text_Block & element)
{
	if (format.empty())
		return;
	if (format[0] == ' ')
		format.erase(0, 1);

	while (format.size()) {
		std::string::size_type const key_end = format.find('=');
		if (key_end == std::string::npos)
			return;
		else {
			std::string key = format.substr(0, key_end);
			format.erase(0, key_end + 1);
			std::string::size_type val_end = format.find(' ');
			if (val_end == std::string::npos)
				val_end = format.size();
			std::string val = format.substr(0, val_end);
			format.erase(0, val_end + 1);
			if (key == "font-size") {
				element.set_font_size(atoi(val.c_str()));
			} else if (key == "font-face")
				element.set_font_face(val + ".ttf");
			else if (key == "line-spacing")
				element.set_line_spacing(atoi(val.c_str()));
			else if (key == "font-color") {
				std::string::size_type const offset = val[0] == '#';
				std::string const r = "0x" + val.substr(offset,     2);
				std::string const g = "0x" + val.substr(offset + 2, 2);
				std::string const b = "0x" + val.substr(offset + 4, 2);

				char * ptr;
				long int const red   = strtol(r.c_str(), &ptr, 0);
				long int const green = strtol(g.c_str(), &ptr, 0);
				long int const blue  = strtol(b.c_str(), &ptr, 0);
				element.set_font_color(RGBColor(red, green, blue));
			} else if (key == "font-weight")
				element.set_font_weight(val);
			else if (key == "font-style")
				element.set_font_style(val);
			else if (key == "font-decoration")
				element.set_font_decoration(val);
		}
	}
}

Align Text_Parser::set_align(const std::string & align) {
	return
		align == "right"  ? Align_Right   :
		align == "center" ? Align_HCenter :
		Align_Left;
}

}
