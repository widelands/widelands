/*
 * Copyright (C) 2006-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_GRAPHIC_TEXT_LAYOUT_H
#define WL_GRAPHIC_TEXT_LAYOUT_H

#include <memory>

#include <SDL.h>

#include "graphic/align.h"
#include "graphic/color.h"
#include "graphic/styles/font_style.h"
#include "graphic/styles/panel_styles.h"
#include "graphic/styles/paragraph_style.h"
#include "graphic/text/rendered_text.h"

#define HAS_PRIMARY_SELECTION_BUFFER SDL_VERSION_ATLEAST(2, 26, 0)

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
 * Checks if the given string is RichText or not. Does not do validity checking.
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
std::string as_richtext_paragraph(const std::string& text,
                                  UI::FontStyle style,
                                  UI::Align align = UI::Align::kLeft);
std::string as_richtext_paragraph(const std::string& text,
                                  const UI::FontStyleInfo& style,
                                  UI::Align align = UI::Align::kLeft);
std::string as_editor_richtext_paragraph(const std::string& text, const UI::FontStyleInfo& style);

std::string as_font_tag(UI::FontStyle style, const std::string& text);
std::string as_font_tag(const std::string& text, UI::FontStyle style);

std::string as_listitem(const std::string&, UI::FontStyle style);

std::string as_game_tip(const std::string&);
/// Format message to player. 'image' is either an image filename or a map object name.
std::string as_mapobject_message(const std::string& image,
                                 int width,
                                 const std::string& txt,
                                 const RGBColor* player_color = nullptr);
std::string as_message(const std::string& heading, const std::string& body);

std::string
as_url_hyperlink(const std::string& url, const std::string& text, const std::string& mouseover);
inline std::string as_url_hyperlink(const std::string& url) {
	return as_url_hyperlink(url, url, url);
}

void newlines_to_richtext(std::string&);

/**

  * Render 'text' with the given font style. If 'width' > 0 and the rendered image is too
  * wide, it will first use the condensed font face and then make the text
  * smaller until it fits 'width'. The resulting font size will not go below
  * 'StyleManager::minimum_font_size()'.
  */
std::shared_ptr<const UI::RenderedText>
autofit_text(const std::string& text, const UI::FontStyleInfo& font_info, int width);

/**
 * 'is_first' omits the vertical gap before the line.
 * 'noescape' is needed for error message formatting and does not call richtext_escape. */
std::string as_heading_with_content(const std::string& header,
                                    const std::string& content,
                                    UI::PanelStyle style,
                                    bool is_first = false,
                                    bool noescape = false);

/**
 * Heading in menu info texts
 * 'is_first' omits the vertical gap before the line.
 */
std::string as_heading(const std::string& txt, UI::PanelStyle style, bool is_first = false);
/// Paragraph in menu info texts
std::string as_content(const std::string& txt, UI::PanelStyle style);

std::string
as_tooltip_text_with_hotkey(const std::string& text, const std::string& hotkey, UI::PanelStyle);

/// Insert vertical space. Returns an empty string if gap is zero or negative.
[[nodiscard]] std::string as_vspace(int gap);

/// Format 'text' with paragraph style 'style'
std::string as_paragraph_style(UI::ParagraphStyle style, const std::string& text);

/// Format 'text' with paragraph style 'style' with optional 'attributes' to be included
/// in the paragraph tag. (Font attributes can be changed in a nested font tag.)
std::string as_paragraph_style(UI::ParagraphStyle style,
                               const std::string& attributes,
                               const std::string& text);

/// Return opening paragraph and font tags for formatting with paragraph style 'style'.
/// The optional 'attributes' will be included in the paragraph tag.
std::string open_paragraph_style(UI::ParagraphStyle style, const std::string& attributes = "");

/// Return closing font and paragraph tags for formatting as plain paragraph.
std::string close_paragraph_style(UI::ParagraphStyle style);

/// Return 'term' and 'description' formatted as a definition list entry.
/// (a paragraph with 'term' in bold and 'description' as normal text)
std::string as_definition_line(const std::string& term, const std::string& description);

#endif  // end of include guard: WL_GRAPHIC_TEXT_LAYOUT_H
