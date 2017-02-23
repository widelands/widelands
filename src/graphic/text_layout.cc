/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

#include "graphic/text_layout.h"

#include <map>

#include <SDL_ttf.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/utf8.h"
#include "graphic/font_handler1.h"
#include "graphic/image.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"
#include "graphic/text_constants.h"

void replace_entities(std::string* text) {
	boost::replace_all(*text, "&gt;", ">");
	boost::replace_all(*text, "&lt;", "<");
	boost::replace_all(*text, "&nbsp;", " ");
	boost::replace_all(*text, "&amp;", "&");  // Must be performed last
}

uint32_t text_width(const std::string& text, int ptsize) {
	return UI::g_fh1->render(as_editorfont(text, ptsize - UI::g_fh1->fontset()->size_offset()))
	   ->width();
}

uint32_t text_height(const std::string& text, int ptsize) {
	return UI::g_fh1->render(as_editorfont(text.empty() ? "." : text,
	                                       ptsize - UI::g_fh1->fontset()->size_offset()))
	   ->height();
}

std::string richtext_escape(const std::string& given_text) {
	std::string text = given_text;
	boost::replace_all(text, "&", "&amp;");  // Must be performed first
	boost::replace_all(text, ">", "&gt;");
	boost::replace_all(text, "<", "&lt;");
	return text;
}

std::string as_game_tip(const std::string& txt) {
	static boost::format f(
	   "<rt padding_l=48 padding_t=28 padding_r=48 padding_b=28>"
	   "<p align=center><font color=21211b face=serif size=16>%s</font></p></rt>");

	f % txt;
	return f.str();
}

std::string
as_uifont(const std::string& txt, int size, const RGBColor& clr, UI::FontSet::Face face) {
	return as_aligned(txt, UI::HAlign::kLeft, size, clr, face);
}

std::string
as_condensed(const std::string& text, UI::HAlign align, int ptsize, const RGBColor& clr) {
	return as_aligned(text, align, ptsize, clr, UI::FontSet::Face::kCondensed);
}

std::string as_editorfont(const std::string& text, int ptsize, const RGBColor& clr) {
	// UI Text is always bold due to historic reasons
	static boost::format f(
	   "<rt keep_spaces=1><p><font face=sans size=%i bold=1 shadow=1 color=%s>%s</font></p></rt>");
	f % ptsize;
	f % clr.hex_value();
	f % richtext_escape(text);
	return f.str();
}

std::string as_aligned(const std::string& txt,
                       UI::HAlign align,
                       int ptsize,
                       const RGBColor& clr,
                       UI::FontSet::Face face) {
	std::string alignment = "left";
	switch (align) {
	case UI::HAlign::kHCenter:
		alignment = "center";
		break;
	case UI::HAlign::kRight:
		alignment = "right";
		break;
	case UI::HAlign::kLeft:
		alignment = "left";
		break;
	}

	std::string font_face = "sans";

	switch (face) {
	case UI::FontSet::Face::kCondensed:
		font_face = "condensed";
		break;
	case UI::FontSet::Face::kSerif:
		font_face = "serif";
		break;
	case UI::FontSet::Face::kSans:
		font_face = "sans";
		break;
	}

	// UI Text is always bold due to historic reasons
	static boost::format f(
	   "<rt><p align=%s><font face=%s size=%i bold=1 shadow=1 color=%s>%s</font></p></rt>");
	f % alignment;
	f % font_face;
	f % ptsize;
	f % clr.hex_value();
	f % txt;
	return f.str();
}

std::string as_tooltip(const std::string& txt) {
	static boost::format f("<rt><p><font face=sans size=%i bold=1 color=%s>%s</font></p></rt>");

	f % UI_FONT_SIZE_SMALL;
	f % UI_FONT_TOOLTIP_CLR.hex_value();
	f % txt;
	return f.str();
}

std::string as_waresinfo(const std::string& txt) {
	static boost::format f("<rt><p><font face=condensed size=10 bold=0 color=%s>%s</font></p></rt>");
	f % UI_FONT_TOOLTIP_CLR.hex_value();
	f % txt;
	return f.str();
}

const Image* autofit_ui_text(const std::string& text, int width, RGBColor color, int fontsize) {
	const Image* result = UI::g_fh1->render(as_uifont(richtext_escape(text), fontsize, color));
	if (width > 0) {  // Autofit
		for (; result->width() > width && fontsize >= kMinimumFontSize; --fontsize) {
			result = UI::g_fh1->render(
			   as_condensed(richtext_escape(text), UI::HAlign::kLeft, fontsize, color));
		}
	}
	return result;
}

namespace UI {

/**
 * Prepare the TTF style settings for rendering in this style.
 */
void TextStyle::setup() const {
	int32_t font_style = TTF_STYLE_NORMAL;
	if (bold)
		font_style |= TTF_STYLE_BOLD;
	if (italics)
		font_style |= TTF_STYLE_ITALIC;
	if (underline)
		font_style |= TTF_STYLE_UNDERLINE;
	TTF_SetFontStyle(font->get_ttf_font(), font_style);
}

/**
 * Get a width estimate for text wrapping.
 */
uint32_t TextStyle::calc_width_for_wrapping(const UChar& c) const {
	int result = 0;
	TTF_GlyphMetrics(font->get_ttf_font(), c, nullptr, nullptr, nullptr, nullptr, &result);
	return result;
}

/**
 * Get a width estimate for text wrapping.
 */
uint32_t TextStyle::calc_width_for_wrapping(const std::string& text) const {
	int result = 0;
	const icu::UnicodeString parseme(text.c_str(), "UTF-8");
	for (int i = 0; i < parseme.length(); ++i) {
		UChar c = parseme.charAt(i);
		if (!i18n::is_diacritic(c)) {
			result += calc_width_for_wrapping(c);
		}
	}
	return result;
}

/**
 * Compute the bare width (without caret padding) of the given string.
 */
uint32_t TextStyle::calc_bare_width(const std::string& text) const {
	int w, h;
	setup();

	TTF_SizeUTF8(font->get_ttf_font(), text.c_str(), &w, &h);
	return w;
}

/**
 * \note Please only use this function once you understand the definitions
 * of ascent/descent etc.
 *
 * Computes the actual line height we should use for rendering the given text.
 * This is heuristic, because it pre-initializes the miny and maxy values to
 * the ones that are typical for Latin scripts, so that lineskips should always
 * be the same for such scripts.
 */
void TextStyle::calc_bare_height_heuristic(const std::string& text,
                                           int32_t& miny,
                                           int32_t& maxy) const {
	miny = font->computed_typical_miny_;
	maxy = font->computed_typical_maxy_;

	setup();
	std::string::size_type pos = 0;
	while (pos < text.size()) {
		uint16_t ch = Utf8::utf8_to_unicode(text, pos);
		int32_t glyphminy, glyphmaxy;
		TTF_GlyphMetrics(font->get_ttf_font(), ch, nullptr, nullptr, &glyphminy, &glyphmaxy, nullptr);
		miny = std::min(miny, glyphminy);
		maxy = std::max(maxy, glyphmaxy);
	}
}

/*
=============================

Default styles

=============================
*/

TextStyle::TextStyle()
   : font(Font::get(UI::g_fh1->fontset()->sans(), UI_FONT_SIZE_SMALL)),
     fg(UI_FONT_CLR_FG),
     bold(true),
     italics(false),
     underline(false) {
}

}  // namespace UI
