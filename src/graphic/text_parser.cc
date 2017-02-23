/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "graphic/text_parser.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include "base/log.h"
#include "graphic/font_handler1.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_layout.h"
#include "helper.h"

namespace UI {

RichtextBlock::RichtextBlock() : image_align_(UI::Align::kLeft), text_align_(UI::Align::kLeft) {
}

RichtextBlock::RichtextBlock(const RichtextBlock& src) {
	images_.clear();
	text_blocks_.clear();
	for (uint32_t i = 0; i < src.images_.size(); ++i)
		images_.push_back(src.images_[i]);
	for (uint32_t i = 0; i < src.text_blocks_.size(); ++i)
		text_blocks_.push_back(src.text_blocks_[i]);
	image_align_ = src.image_align_;
	text_align_ = src.text_align_;
}

TextBlock::TextBlock()
   : font_size_(10),
     font_color_(RGBColor(255, 255, 0)),
     font_weight_("normal"),
     font_style_("normal"),
     font_decoration_("none"),
     font_face_((UI::g_fh1->fontset())->sans()),
     line_spacing_(0) {
}

void TextParser::parse(std::string& text, std::vector<RichtextBlock>& blocks) {
	bool more_richtext_blocks = true;
	// First while loop parses all richtext blocks (images)
	while (more_richtext_blocks) {
		RichtextBlock new_richtext_block;
		std::string unparsed_text;
		std::string richtext_format;

		more_richtext_blocks =
		   extract_format_block(text, unparsed_text, richtext_format, std::string("<rt"),
		                        std::string(">"), std::string("</rt>"));
		parse_richtexttext_attributes(richtext_format, &new_richtext_block);

		std::vector<TextBlock> text_blocks;

		// Second while loop parses all textblocks of current richtext block
		bool more_text_blocks = true;
		while (more_text_blocks) {
			std::string block_format;
			TextBlock new_block;

			std::vector<std::string> words;
			std::vector<std::vector<std::string>::size_type> line_breaks;

			more_text_blocks = parse_textblock(unparsed_text, block_format, words, line_breaks);
			parse_text_attributes(block_format, new_block);

			new_block.set_words(words);
			new_block.set_line_breaks(line_breaks);
			text_blocks.push_back(new_block);
		}
		new_richtext_block.set_text_blocks(text_blocks);
		blocks.push_back(new_richtext_block);
	}
}

bool TextParser::parse_textblock(std::string& block,
                                 std::string& block_format,
                                 std::vector<std::string>& words,
                                 std::vector<std::vector<std::string>::size_type>& line_breaks) {
	std::string block_text;

	const bool extract_more =
	   extract_format_block(block, block_text, block_format, "<p", ">", "</p>");

	// Split the the text because of " "
	std::vector<std::string> unwrapped_words;
	split_words(block_text, &unwrapped_words);

	// Handle user defined line breaks, and save them
	for (const std::string& temp_words : unwrapped_words) {
		for (std::string line = temp_words;;) {
			line = i18n::make_ligatures(line.c_str());
			std::string::size_type next_break = line.find("<br>");

			if (next_break == std::string::npos) {
				if (line.size()) {
					std::string word = line;
					replace_entities(&word);
					words.push_back(word);
				}
				break;
			} else if (next_break) {
				std::string word = line.substr(0, next_break);
				replace_entities(&word);
				words.push_back(word);
			}
			line_breaks.push_back(words.size());
			line.erase(0, next_break + 4);
		}
	}
	return extract_more;
}

void TextParser::split_words(const std::string& in, std::vector<std::string>* plist) {
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

bool TextParser::extract_format_block(std::string& block,
                                      std::string& block_text,
                                      std::string& block_format,
                                      const std::string& block_start,
                                      const std::string& format_end,
                                      const std::string& block_end) {
	if (block.compare(0, block_start.size(), block_start)) {
		const std::string::size_type format_begin_pos = block.find(block_start);
		if (format_begin_pos == std::string::npos) {
			return false;
		}
		block.erase(0, format_begin_pos);
	}

	block.erase(0, block_start.size());
	if (block.size() && *block.begin() == ' ')
		block.erase(0, 1);

	const std::string::size_type format_end_pos = block.find(format_end);
	if (format_end_pos == std::string::npos) {
		return false;
	}

	// Append block_format
	block_format.erase();
	block_format.append(block.substr(0, format_end_pos));

	// Delete whole format block
	block.erase(0, format_end_pos + format_end.size());

	// Find end of block
	const std::string::size_type block_end_pos = block.find(block_end);
	if (block_end_pos == std::string::npos) {
		return false;
	}
	// Extract text of block
	block_text.erase();
	block_text.append(block.substr(0, block_end_pos));

	// Erase text including closing tag
	block.erase(0, block_end_pos + block_end.size());
	// Is something left
	return block.find(block_start) != std::string::npos;
}

void TextParser::parse_richtexttext_attributes(std::string format, RichtextBlock* const element) {
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
			if (key == "image") {
				const std::vector<std::string> images(split_string(val, ";"));
				element->set_images(images);
			} else if (key == "image-align")
				element->set_image_align(set_align(val));
			else if (key == "text-align")
				element->set_text_align(set_align(val));
		}
	}
}

void TextParser::parse_text_attributes(std::string format, TextBlock& element) {
	if (format.empty())
		return;
	if (format[0] == ' ')
		format.erase(0, 1);

	while (format.size()) {
		std::string::size_type const key_end = format.find('=');
		if (key_end == std::string::npos) {
			return;
		} else {
			std::string key = format.substr(0, key_end);
			format.erase(0, key_end + 1);
			std::string::size_type val_end = format.find(' ');
			if (val_end == std::string::npos)
				val_end = format.size();
			std::string val = format.substr(0, val_end);
			format.erase(0, val_end + 1);
			if (key == "font-size") {
				element.set_font_size(atoi(val.c_str()));
			} else if (key == "font-face") {
				const UI::FontSet& fontset = *UI::g_fh1->fontset();
				if (val == fontset.condensed() || val == "condensed") {
					val = fontset.condensed();
				} else if (val == fontset.serif() || val == "serif") {
					val = fontset.serif();
				} else {
					val = fontset.sans();
				}
				element.set_font_style(val);
			} else if (key == "line-spacing") {
				element.set_line_spacing(atoi(val.c_str()));
			} else if (key == "font-color") {
				std::string::size_type const offset = val[0] == '#';
				std::string const r = "0x" + val.substr(offset, 2);
				std::string const g = "0x" + val.substr(offset + 2, 2);
				std::string const b = "0x" + val.substr(offset + 4, 2);

				char* ptr;
				long int const red = strtol(r.c_str(), &ptr, 0);
				long int const green = strtol(g.c_str(), &ptr, 0);
				long int const blue = strtol(b.c_str(), &ptr, 0);
				element.set_font_color(RGBColor(red, green, blue));
			} else if (key == "font-weight") {
				element.set_font_weight(val);
			} else if (key == "font-style") {
				element.set_font_style(val);
			} else if (key == "font-decoration") {
				element.set_font_decoration(val);
			}
		}
	}
}

Align TextParser::set_align(const std::string& align) {
	return align == "right"  ? UI::Align::kRight
         : align == "center" ? UI::Align::kCenter
	     :                     UI::Align::kLeft;
}
}
