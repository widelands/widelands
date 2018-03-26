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

#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/style_manager.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_set.h"

namespace {
bool is_paragraph(const std::string& text) {
	return boost::starts_with(text, "<p");
}

bool is_div(const std::string& text) {
	return boost::starts_with(text, "<div");
}
}  // namespace

void replace_entities(std::string* text) {
	boost::replace_all(*text, "&gt;", ">");
	boost::replace_all(*text, "&lt;", "<");
	boost::replace_all(*text, "&nbsp;", " ");
	boost::replace_all(*text, "&amp;", "&");  // Must be performed last
}

int text_width(const std::string& text, int ptsize) {
	return UI::g_fh1->render(as_editorfont(text, ptsize - UI::g_fh1->fontset()->size_offset()))
	   ->width();
}

int text_height(int ptsize, UI::FontStyleInfo::Face face) {
	return UI::g_fh1
	   ->render(as_aligned(UI::g_fh1->fontset()->representative_character(), UI::Align::kLeft,
	                       ptsize - UI::g_fh1->fontset()->size_offset(), RGBColor(0, 0, 0), face))
	   ->height();
}

std::string richtext_escape(const std::string& given_text) {
	std::string text = given_text;
	boost::replace_all(text, "&", "&amp;");  // Must be performed first
	boost::replace_all(text, ">", "&gt;");
	boost::replace_all(text, "<", "&lt;");
	return text;
}

std::string as_richtext(const std::string& txt) {
	static boost::format f("<rt>%s</rt>");
	f % txt;
	return f.str();
}

std::string as_richtext_paragraph(const std::string& text, UI::FontStyle style) {
	static boost::format f("<rt><p>%s</p></rt>");
	f % g_gr->styles().font_style(style).as_font_tag(text);
	return f.str();
}

std::string as_game_tip(const std::string& txt) {
	static boost::format f("<rt padding_l=48 padding_t=28 padding_r=48 padding_b=28>"
	                       "<p align=center>%s</p></rt>");
	f % g_gr->styles().font_style(UI::FontStyle::kFsMenuGameTip).as_font_tag(txt);
	return f.str();
}

std::string
as_uifont(const std::string& txt, int size, const RGBColor& clr, UI::FontStyleInfo::Face face) {
	return as_aligned(txt, UI::Align::kLeft, size, clr, face);
}

std::string
as_condensed(const std::string& text, UI::Align align, int ptsize, const RGBColor& clr) {
	return as_aligned(text, align, ptsize, clr, UI::FontStyleInfo::Face::kCondensed);
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
                       UI::Align align,
                       int ptsize,
                       const RGBColor& clr,
                       UI::FontStyleInfo::Face face) {
	std::string alignment = "left";
	switch (align) {
	case UI::Align::kCenter:
		alignment = "center";
		break;
	case UI::Align::kRight:
		alignment = "right";
		break;
	case UI::Align::kLeft:
		alignment = "left";
		break;
	}

	std::string font_face = "sans";

	switch (face) {
	case UI::FontStyleInfo::Face::kCondensed:
		font_face = "condensed";
		break;
	case UI::FontStyleInfo::Face::kSerif:
		font_face = "serif";
		break;
	case UI::FontStyleInfo::Face::kSans:
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

std::string as_message(const std::string& heading, const std::string& body) {
	return (
	   (boost::format(
	       "<rt><p>%s<br></p><vspace gap=6>%s</rt>") %
	    g_gr->styles().font_style(UI::FontStyle::kWuiMessageHeading).as_font_tag(heading) %
	    (is_paragraph(body) || is_div(body) ?
	        body :
	        (boost::format("<p>%s</p>") %
	         g_gr->styles().font_style(UI::FontStyle::kWuiMessageParagraph).as_font_tag(body))
	           .str()))
	      .str());
}

std::shared_ptr<const UI::RenderedText>
autofit_ui_text(const std::string& text, int width, RGBColor color, int fontsize) {
	std::shared_ptr<const UI::RenderedText> result =
	   UI::g_fh1->render(as_uifont(richtext_escape(text), fontsize, color));
	if (width > 0) {  // Autofit
		const int minimum_size = g_gr->styles().font_size(StyleManager::FontSize::kMinimum);
		for (; result->width() > width && fontsize >= minimum_size; --fontsize) {
			result = UI::g_fh1->render(
			   as_condensed(richtext_escape(text), UI::Align::kLeft, fontsize, color));
		}
	}
	return result;
}
