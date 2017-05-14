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

#include "graphic/richtext.h"

#include "base/rect.h"
#include "graphic/font.h"
#include "graphic/font_handler.h"
#include "graphic/font_handler1.h"  // Needed for fontset's direction
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "graphic/text/bidi.h"
#include "graphic/text_layout.h"
#include "graphic/text_parser.h"

namespace UI {

namespace {
int32_t const h_space = 3;
}  // namespace

/**
 * Layouted rich text is essentially a bunch of drawable elements, each with a
 * rectangular bounding box.
 */
struct Element {
	explicit Element(const Recti& bounding_box) : bbox(bounding_box) {
	}
	virtual ~Element() {
	}

	/**
	 * Draw the element, assuming that @p dst is already set up to be
	 * relative to the element's internal frame of reference.
	 */
	virtual void draw(RenderTarget& dst) = 0;

	Recti bbox;
};

struct ImageElement : Element {
	ImageElement(const Recti& bounding_box, const Image* init_image)
	   : Element(bounding_box), image(init_image) {
	}

	void draw(RenderTarget& dst) override {
		dst.blit(Vector2i::zero(), image);
	}

	const Image* image;
};

struct TextlineElement : Element {
	TextlineElement(const Recti& bounding_box,
	                const TextStyle& init_style,
	                std::vector<std::string>::const_iterator words_begin,
	                std::vector<std::string>::const_iterator words_end)
	   : Element(bounding_box), style(init_style), words(words_begin, words_end) {
	}

	void draw(RenderTarget& dst) override {
		assert(words.size());
		uint32_t spacewidth = style.calc_bare_width(" ");

		std::vector<std::string> result_words;
		std::vector<std::string>::iterator it = result_words.begin();

		// Reorder words for BiDi
		if (UI::g_fh1->fontset()->is_rtl() && i18n::has_rtl_character(words)) {
			std::string previous_word;
			for (std::vector<std::string>::iterator source_it = words.begin();
			     source_it != words.end(); ++source_it) {
				std::string& word = *source_it;
				if (source_it != words.end()) {
					if (i18n::has_rtl_character(word.c_str()) ||
					    i18n::has_rtl_character(previous_word.c_str())) {
						it = result_words.insert(result_words.begin(), word);
					} else {  // Sequences of Latin words go to the right from current position
						if (it < result_words.end()) {
							++it;
						}
						it = result_words.insert(it, word);
					}
					previous_word = word;
				}
			}
		} else {
			for (std::string& word : words) {
				result_words.push_back(word);
			}
		}
		// Now render
		uint32_t x = g_fh->draw_text_raw(dst, style, Vector2i::zero(), result_words[0]);

		it = result_words.begin() + 1;
		if (it != result_words.end()) {
			do {
				if (style.underline)
					x += g_fh->draw_text_raw(dst, style, Vector2i(x, 0), " ");
				else
					x += spacewidth;
				x += g_fh->draw_text_raw(dst, style, Vector2i(x, 0), *it++);
			} while (it != result_words.end());
		}
	}

	TextStyle style;
	std::vector<std::string> words;
};

struct RichTextImpl {
	/// Solid background color for all elements
	RGBColor background_color;

	/// Layouted elements
	std::vector<Element*> elements;

	/// Width of the rich-text area, controlled by the user of the
	/// rich-text field
	uint32_t width;

	/// Height of parsed rich-text area, determined by layouting
	uint32_t height;

	RichTextImpl();
	~RichTextImpl();

	void clear();
};

RichText::RichText() : m(new RichTextImpl) {
}

RichText::~RichText() {
}

RichTextImpl::RichTextImpl() : background_color(0, 0, 0) {
	width = 0;
	height = 0;
}

RichTextImpl::~RichTextImpl() {
	clear();
}

/**
 * Reset all layouting data.
 */
void RichTextImpl::clear() {
	while (!elements.empty()) {
		delete elements.back();
		elements.pop_back();
	}

	height = 0;
}

/**
 * Set the width for the rich text field.
 * Default width is undefined. This must be called before @ref parse.
 */
void RichText::set_width(uint32_t gwidth) {
	m->width = gwidth;
}

/**
 * Set a solid background color that is used by @ref draw when a
 * solid background is requested.
 */
void RichText::set_background_color(RGBColor color) {
	m->background_color = color;
}

/**
 * @return the width of the rich text field, set by @ref set_width
 */
uint32_t RichText::width() {
	return m->width;
}

/**
 * @return the actual total height of layouted rich text, computed by @ref parse
 */
uint32_t RichText::height() {
	return m->height;
}

struct TextBuilder {
	RichTextImpl& rti;

	/// Current richtext block
	std::vector<RichtextBlock>::iterator richtext;

	/// Extent of images in the current richtext block
	/*@{*/
	uint32_t images_width;
	uint32_t images_height;
	/*@}*/

	/// Maximum width, in pixels, for current line of text
	uint32_t maxwidth;

	/// y-coordinate of top of current line of text
	uint32_t text_y;

	/// Width of the current line, in pixels
	uint32_t linewidth;

	/// Current text block
	std::vector<TextBlock>::const_iterator textblock;
	TextStyle style;
	uint32_t spacewidth;
	uint32_t linespacing;

	struct Elt {
		TextlineElement* element;
		int32_t miny, maxy;
	};

	/// Elements in the current line (also already added to the full richtext list
	/// of elements, but we keep this around to store miny/maxy data to adjust all
	/// parts of a line onto the same text baseline).
	std::vector<Elt> elements;

	TextBuilder(RichTextImpl& impl)
	   : rti(impl),
	     images_width(0),
	     images_height(0),
	     maxwidth(0),
	     text_y(0),
	     linewidth(0),
	     spacewidth(0),
	     linespacing(0) {
	}

	/**
	 * Update data that is specific to the current @ref textblock.
	 */
	void reset_block() {
		style.font = Font::get(textblock->get_font_face(), textblock->get_font_size());
		style.fg = textblock->get_font_color();

		style.bold = textblock->get_font_weight() == "bold";
		style.italics = textblock->get_font_style() == "italic";
		style.underline = textblock->get_font_decoration() == "underline";

		spacewidth = style.calc_bare_width(" ");
		linespacing = textblock->get_line_spacing();
	}

	/**
	 * Properly align elements in the current line, and advance @ref text_y and
	 * other data so that we can begin the next line.
	 */
	void advance_line() {
		int32_t miny = 0;
		int32_t maxy = 0;

		if (elements.empty()) {
			style.calc_bare_height_heuristic(" ", miny, maxy);
		} else {
			int32_t alignref_left = 0;
			int32_t alignref_right = rti.width;

			if (text_y < rti.height + images_height) {
				if (mirror_alignment(richtext->get_image_align()) == UI::Align::kRight) {
					alignref_right -= images_width + h_space;
				} else {
					// Note: center image alignment with text is not properly supported
					// It is unclear what the semantics should be.
					alignref_left += images_width + h_space;
				}
			}

			int32_t textleft = 0;

			switch (mirror_alignment(richtext->get_text_align())) {
			case UI::Align::kRight:
				textleft = alignref_right - int32_t(linewidth);
				break;
			case UI::Align::kCenter:
				textleft = alignref_left + (alignref_right - alignref_left - int32_t(linewidth)) / 2;
				break;
			case UI::Align::kLeft:
				textleft = alignref_left;
				break;
			}

			for (std::vector<Elt>::const_iterator it = elements.begin(); it != elements.end(); ++it) {
				it->element->bbox.x += textleft;
				miny = std::min(miny, it->miny);
				maxy = std::max(maxy, it->maxy);
			}

			int32_t baseline = text_y + maxy;
			for (std::vector<Elt>::const_iterator it = elements.begin(); it != elements.end(); ++it)
				it->element->bbox.y = baseline - it->element->style.font->ascent();
		}

		text_y += maxy - miny + linespacing;
		if (text_y >= rti.height + images_height)
			maxwidth = rti.width;

		elements.clear();
		linewidth = 0;
	}
};

/**
 * Parse and layout the given rich text.
 */
void RichText::parse(const std::string& rtext) {
	m->clear();

	std::vector<RichtextBlock> blocks;
	TextParser p;
	std::string copy(rtext);
	p.parse(copy, blocks);

	// Guard against weirdness in text and image alignment
	m->width = std::min(m->width, uint32_t(std::numeric_limits<int32_t>::max()));

	TextBuilder text(*m);

	for (text.richtext = blocks.begin(); text.richtext != blocks.end(); ++text.richtext) {
		const std::vector<TextBlock>& cur_text_blocks = text.richtext->get_text_blocks();
		const std::vector<std::string>& cur_block_images = text.richtext->get_images();

		// First obtain the data of all images of this richtext block and prepare
		// the corresponding elements, then do the alignment once the total width
		// is known
		const uint32_t firstimageelement = m->elements.size();
		text.images_height = 0;
		text.images_width = 0;

		for (std::vector<std::string>::const_iterator image_it = cur_block_images.begin();
		     image_it != cur_block_images.end(); ++image_it) {
			const Image* image = g_gr->images().get(*image_it);
			if (!image)
				continue;

			Recti bbox;
			bbox.x = text.images_width;
			bbox.y = m->height;
			bbox.w = image->width();
			bbox.h = image->height();

			text.images_height = std::max<int>(text.images_height, bbox.h);
			text.images_width += bbox.w;

			m->elements.push_back(new ImageElement(bbox, image));
		}

		// Fix up the alignment
		int32_t imagealigndelta = 0;

		switch (mirror_alignment(text.richtext->get_image_align())) {
		case UI::Align::kCenter:
			imagealigndelta = (int32_t(m->width) - int32_t(text.images_width)) / 2;
			break;
		case UI::Align::kRight:
			imagealigndelta = int32_t(m->width) - int32_t(text.images_width);
			break;
		case UI::Align::kLeft:
			break;
		}

		for (uint32_t idx = firstimageelement; idx < m->elements.size(); ++idx)
			m->elements[idx]->bbox.x += imagealigndelta;

		// Now layout the text elements; they are already broken down to words,
		// which makes our job a bit easier.
		text.text_y = m->height;
		text.maxwidth = m->width;
		text.linewidth = 0;

		if (text.images_height > 0) {
			if (h_space + text.images_width < text.maxwidth)
				text.maxwidth -= h_space + text.images_width;
			else
				text.text_y = m->height + text.images_height;
		}

		text.textblock = cur_text_blocks.begin();

		while (text.textblock != cur_text_blocks.end()) {
			text.reset_block();

			const std::vector<std::string>& words = text.textblock->get_words();
			const std::vector<std::vector<std::string>::size_type>& line_breaks =
			   text.textblock->get_line_breaks();

			uint32_t word_cnt = 0;
			std::vector<std::vector<std::string>::size_type>::const_iterator br_it =
			   line_breaks.begin();

			while (word_cnt < words.size() || br_it != line_breaks.end()) {
				if (br_it != line_breaks.end() && *br_it <= word_cnt) {
					text.advance_line();
					++br_it;
					continue;
				}

				// Now eat up words up to the next line break
				assert(word_cnt < words.size());

				bool wrap = false;
				uint32_t nrwords = 0;
				TextBuilder::Elt elt;
				elt.miny = elt.maxy = 0;

				Recti bbox;
				bbox.x = text.linewidth ? text.linewidth + text.spacewidth : 0;
				bbox.y = 0;  // filled in later
				bbox.w = 0;
				bbox.h = text.style.font->height();

				// TODO(GunChleoc): Arabic: width calculation for alignment is broken (Arabic)
				do {
					uint32_t wordwidth = text.style.calc_bare_width(words[word_cnt + nrwords]);

					if (nrwords)
						wordwidth += text.spacewidth;

					// Break only if this is not the first word of the line
					if ((text.linewidth || nrwords) && bbox.x + bbox.w + wordwidth > text.maxwidth) {
						wrap = true;
						break;
					}

					int32_t wordminy, wordmaxy;
					text.style.calc_bare_height_heuristic(words[word_cnt + nrwords], wordminy, wordmaxy);
					elt.maxy = std::max(elt.maxy, wordmaxy);
					elt.miny = std::min(elt.miny, wordminy);

					bbox.w += wordwidth;
					++nrwords;
				} while (word_cnt + nrwords < words.size() &&
				         (br_it == line_breaks.end() || *br_it > word_cnt + nrwords));

				if (nrwords) {
					m->elements.push_back(new TextlineElement(
					   bbox, text.style, words.begin() + word_cnt, words.begin() + word_cnt + nrwords));
					word_cnt += nrwords;

					elt.element = static_cast<TextlineElement*>(m->elements.back());
					text.elements.push_back(elt);
					text.linewidth = bbox.x + bbox.w;
				}

				if (wrap)
					text.advance_line();
			}

			++text.textblock;
		}

		if (!text.elements.empty())
			text.advance_line();

		// Update total height
		m->height = std::max(m->height + text.images_height, text.text_y);
	}
}

/**
 * Draw pre-parsed and layouted rich text content at the given offset.
 *
 * @p background if true, all richtext elements are given a solid background,
 * per the color set by @ref set_background_color
 *
 * @note this function may draw content outside the box given offset
 * and @ref width and @ref height, if there were wrapping problems.
 */
void RichText::draw(RenderTarget& dst, const Vector2i& offset, bool background) {
	for (std::vector<Element*>::const_iterator elt = m->elements.begin(); elt != m->elements.end();
	     ++elt) {
		Recti oldbox;
		Vector2i oldofs = Vector2i::zero();
		Recti bbox((*elt)->bbox.origin() + offset, (*elt)->bbox.w, (*elt)->bbox.h);

		if (dst.enter_window(bbox, &oldbox, &oldofs)) {
			if (background)
				dst.fill_rect(Recti(0, 0, bbox.w, bbox.h), m->background_color);
			(*elt)->draw(dst);
			dst.set_window(oldbox, oldofs);
		}
	}
}

}  // namespace UI
