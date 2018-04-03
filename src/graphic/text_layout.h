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

#ifndef WL_GRAPHIC_TEXT_LAYOUT_H
#define WL_GRAPHIC_TEXT_LAYOUT_H

#include <string>

#include "graphic/align.h"
#include "graphic/styles/font_styles.h"
#include "graphic/text/rendered_text.h"

/**
  * Returns the exact width of the text rendered as editorfont for the given font size.
  * This function is inefficient; only call when we need the exact width.
  */
int text_width(const std::string& text, const UI::FontStyleInfo& style, float scale = 1.0f);

/**
  * Returns the exact height of the text rendered for the given font size and face.
  * This function is inefficient; only call when we need the exact height.
  */
int text_height(const UI::FontStyleInfo& style, float scale = 1.0f);
int text_height(UI::FontStyle style, float scale = 1.0f);

/**
 * Checks it the given string is RichText or not. Does not do validity checking.
 */
inline bool is_richtext(const std::string& text) {
	return text.compare(0, 3, "<rt") == 0;
}

/**
 * Escapes reserved characters for Richtext.
 */
std::string richtext_escape(const std::string& given_text);

std::string as_richtext(const std::string&);

/**
 * Convenience functions to convert simple text into a valid block
 * of rich text which can be rendered.
 */
std::string as_richtext_paragraph(const std::string& text, UI::FontStyle style, UI::Align align = UI::Align::kLeft);
std::string as_richtext_paragraph(const std::string& text, const UI::FontStyleInfo& style, UI::Align align = UI::Align::kLeft);
std::string as_editor_richtext_paragraph(const std::string& text, const UI::FontStyleInfo& style);

std::string as_game_tip(const std::string&);
std::string as_message(const std::string& heading, const std::string& body);

/**
  * Render 'text' with the given font style. If 'width' > 0 and the rendered image is too
  * wide, it will first use the condensed font face and then make the text
  * smaller until it fits 'width'. The resulting font size will not go below
  * 'StyleManager::minimum_font_size()'.
  */
std::shared_ptr<const UI::RenderedText> autofit_text(const std::string& text,
																	  const UI::FontStyleInfo& font_info,
																	  int width);


#endif  // end of include guard: WL_GRAPHIC_TEXT_LAYOUT_H
