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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "richtext.h"

#include "font.h"
#include "graphic.h"
#include "picture.h"
#include "rect.h"
#include "text_parser.h"
#include "rendertarget.h"
#include "font_handler.h"

namespace UI {

/**
 * Layouted rich text is essentially a bunch of drawable elements, each with a
 * rectangular bounding box.
 */
struct Element {
	explicit Element(const Rect & _bbox) : bbox(_bbox) {}
	virtual ~Element() {}

	/**
	 * Draw the element, assuming that @p dst is already set up to be
	 * relative to the element's internal frame of reference.
	 */
	virtual void draw(RenderTarget & dst) = 0;

	Rect bbox;
};

struct ImageElement : Element {
	ImageElement(const Rect & _bbox, const PictureID & _image)
		: Element(_bbox), image(_image) {}

	virtual void draw(RenderTarget & dst)
	{
		dst.blit(Point(0, 0), image);
	}

	PictureID image;
};

struct TextlineElement : Element {
	TextlineElement
		(const Rect & _bbox, const TextStyle & _style,
		 std::vector<std::string>::const_iterator words_begin,
		 std::vector<std::string>::const_iterator words_end)
		: Element(_bbox), style(_style), words(words_begin, words_end) {}

	virtual void draw(RenderTarget & dst)
	{
		assert(words.size());
		uint32_t x = g_fh->draw_text_raw(dst, style, Point(0, 0), words[0]);

		std::vector<std::string>::const_iterator it = words.begin() + 1;
		if (it != words.end()) {
			uint32_t spacewidth = style.calc_bare_width(" ");

			do {
				if (style.underline)
					x += g_fh->draw_text_raw(dst, style, Point(x, 0), " ");
				else
					x += spacewidth;
				x += g_fh->draw_text_raw(dst, style, Point(x, 0), *it++);
			} while (it != words.end());
		}
	}

	TextStyle style;
	std::vector<std::string> words;
};

struct RichText::Impl {
	/// Layouted elements
	std::vector<Element *> elements;

	/// Width of the rich-text area, controlled by the user of the
	/// rich-text field
	uint32_t width;

	/// Height of parsed rich-text area, determined by layouting
	uint32_t height;

	Impl();
	~Impl();

	void clear();
};

RichText::RichText()
	: m(new Impl)
{
}

RichText::~RichText()
{
}

RichText::Impl::Impl()
{
	width = 0;
	height = 0;
}

RichText::Impl::~Impl()
{
	clear();
}

/**
 * Reset all layouting data.
 */
void RichText::Impl::clear()
{
	while (elements.size()) {
		delete elements.back();
		elements.pop_back();
	}

	height = 0;
}

/**
 * Set the width for the rich text field.
 * Default width is undefined. This must be called before @ref parse.
 */
void RichText::set_width(uint32_t width)
{
	m->width = width;
}

/**
 * @return the width of the rich text field, set by @ref set_width
 */
uint32_t RichText::width()
{
	return m->width;
}

/**
 * @return the actual total height of layouted rich text, computed by @ref parse
 */
uint32_t RichText::height()
{
	return m->height;
}

/**
 * Parse and layout the given rich text.
 */
void RichText::parse(const std::string & text)
{
	m->clear();

	std::vector<Richtext_Block> blocks;
	Text_Parser p;
	std::string copy(text);
	p.parse(copy, blocks);

	// Guard against weirdness in text and image alignment
	m->width = std::min(m->width, uint32_t(std::numeric_limits<int32_t>::max()));

	for
		(std::vector<Richtext_Block>::iterator richtext_it = blocks.begin();
		 richtext_it != blocks.end();
		 ++richtext_it)
	{
		const std::vector<Text_Block> & cur_text_blocks = richtext_it->get_text_blocks();
		const std::vector<std::string> & cur_block_images = richtext_it->get_images();

		// First obtain the data of all images of this richtext block and prepare
		// the corresponding elements, then do the alignment once the total width
		// is known
		const uint32_t firstimageelement = m->elements.size();
		uint32_t images_height = 0;
		uint32_t images_width = 0;

		for
			(std::vector<std::string>::const_iterator img_it = cur_block_images.begin();
			 img_it != cur_block_images.end();
			 ++img_it)
		{
			const PictureID image = g_gr->get_picture(PicMod_Game, *img_it);
			if (!image)
				continue;

			Rect bbox;
			bbox.x = images_width;
			bbox.y = m->height;
			bbox.w = image->get_w();
			bbox.h = image->get_h();

			images_height = std::max(images_height, bbox.h);
			images_width += bbox.w;

			m->elements.push_back(new ImageElement(bbox, image));
		}

		// Fix up the alignment
		int32_t imagealigndelta = 0;

		if ((richtext_it->get_image_align() & Align_Horizontal) == Align_HCenter)
			imagealigndelta = (int32_t(m->width) - int32_t(images_width)) / 2;
		else if ((richtext_it->get_image_align() & Align_Horizontal) == Align_Right)
			imagealigndelta = int32_t(m->width) - int32_t(images_width);

		for (uint32_t idx = firstimageelement; idx < m->elements.size(); ++idx)
			m->elements[idx]->bbox.x += imagealigndelta;

		// Now layout the text elements; they are already broken down to words,
		// which makes our job a bit easier.
		int32_t const h_space = 3;
		uint32_t maxtextwidth = m->width;

		uint32_t text_y = m->height;

		if (images_height > 0) {
			if (h_space + images_width < maxtextwidth)
				maxtextwidth -= h_space + images_width;
			else
				text_y = m->height + images_height;
		}

		for
			(std::vector<Text_Block>::const_iterator text_it = cur_text_blocks.begin();
			 text_it != cur_text_blocks.end();
			 ++text_it)
		{
			// Set up font for this block
			TextStyle style;
			style.font = Font::get(text_it->get_font_face(), text_it->get_font_size());
			style.fg = text_it->get_font_color();

			style.bold = text_it->get_font_weight() == "bold";
			style.italics = text_it->get_font_style() == "italic";
			style.underline = text_it->get_font_decoration() == "underline";

			uint32_t lineheight = style.font->height();
			uint32_t spacewidth = style.calc_bare_width(" ");

			// Do the actual layouting
			const std::vector<std::string> & words = text_it->get_words();
			const std::vector<std::vector<std::string>::size_type> & line_breaks =
				text_it->get_line_breaks();

			uint32_t word_cnt = 0;
			std::vector<std::vector<std::string>::size_type>::const_iterator br_it = line_breaks.begin();

			while (word_cnt < words.size() || br_it != line_breaks.end()) {
				if (br_it != line_breaks.end() && *br_it <= word_cnt) {
					text_y += style.font->lineskip() + text_it->get_line_spacing();
					br_it++;
					continue;
				}

				if (text_y >= m->height + images_height)
					maxtextwidth = m->width;

				// Now eat up words up to the next line break
				assert(word_cnt < words.size());
				uint32_t nrwords = 1;
				uint32_t linewidth = style.calc_bare_width(words[word_cnt]);
				int32_t lineminy;
				int32_t linemaxy;
				style.calc_bare_height_heuristic(words[word_cnt], lineminy, linemaxy);

				while (word_cnt + nrwords < words.size()) {
					if (br_it != line_breaks.end() && *br_it <= word_cnt + nrwords) {
						br_it++;
						break;
					}

					uint32_t wordwidth = style.calc_bare_width(words[word_cnt + nrwords]);

					if (linewidth + spacewidth + wordwidth > maxtextwidth)
						break;

					int32_t wordminy, wordmaxy;
					style.calc_bare_height_heuristic(words[word_cnt + nrwords], wordminy, wordmaxy);
					linemaxy = std::max(linemaxy, wordmaxy);
					lineminy = std::min(lineminy, wordminy);

					linewidth += spacewidth + wordwidth;
					++nrwords;
				}

				// Compute bounding box for line based on alignment settings
				Rect bbox;
				bbox.x = 0;
				bbox.y = text_y - style.font->ascent() + linemaxy;
				bbox.w = linewidth;
				bbox.h = lineheight;

				int32_t alignref_left = 0;
				int32_t alignref_right = m->width;

				if (text_y < m->height + images_height) {
					if ((richtext_it->get_image_align() & Align_Horizontal) == Align_Right) {
						alignref_right -= images_width + h_space;
					} else {
						// Note: center image alignment with text is not properly supported
						// It is unclear what the semantics should be.
						alignref_left += images_width + h_space;
					}
				}

				switch (richtext_it->get_text_align() & Align_Horizontal) {
				case Align_Right:
					bbox.x = alignref_right - int32_t(linewidth);
					break;
				case Align_HCenter:
					bbox.x = alignref_left + (alignref_right - alignref_left - int32_t(linewidth)) / 2;
					break;
				default:
					bbox.x = alignref_left;
					break;
				}

				m->elements.push_back
					(new TextlineElement
						(bbox, style,
						 words.begin() + word_cnt, words.begin() + word_cnt + nrwords));

				word_cnt += nrwords;
				text_y += linemaxy - lineminy + text_it->get_line_spacing();
			}
		}

		// Update total height
		m->height = std::max(m->height + images_height, text_y);
	}
}

/**
 * Draw pre-parsed and layouted rich text content at the given offset.
 *
 * @note this function may draw content outside the box given offset
 * and @ref width and @ref height, if there were wrapping problems.
 */
void RichText::draw(RenderTarget & dst, Point offset)
{
	for
		(std::vector<Element *>::const_iterator elt = m->elements.begin();
		 elt != m->elements.end();
		 ++elt)
	{
		Rect oldbox;
		Point oldofs;
		Rect bbox = (*elt)->bbox;
		bbox += offset;

		if (dst.enter_window(bbox, &oldbox, &oldofs)) {
			(*elt)->draw(dst);
			dst.set_window(oldbox, oldofs);
		}
	}
}


} // namespace UI
