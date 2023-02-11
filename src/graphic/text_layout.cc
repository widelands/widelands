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

#include "graphic/text_layout.h"

#include <memory>

#include "base/string.h"
#include "graphic/font_handler.h"
#include "graphic/image.h"
#include "graphic/style_manager.h"
#include "graphic/text/font_set.h"

namespace {
inline bool is_paragraph(const std::string& text) {
	return starts_with(text, "<p");
}

inline bool is_div(const std::string& text) {
	return starts_with(text, "<div");
}

std::string as_richtext_paragraph(const std::string& text, UI::Align align) {
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

	return format("<rt><p align=%s>%s</p></rt>", alignment, text);
}
}  // namespace

int text_width(const std::string& text, const UI::FontStyleInfo& style, float scale) {
	UI::FontStyleInfo info(style);
	info.set_size(info.size() * scale - UI::g_fh->fontset()->size_offset());
	return UI::g_fh->render(as_editor_richtext_paragraph(richtext_escape(text), info))->width();
}

int text_height(const UI::FontStyleInfo& style, float scale) {
	UI::FontStyleInfo info(style);
	info.set_size(info.size() * scale - UI::g_fh->fontset()->size_offset());
	return UI::g_fh
	   ->render(as_richtext_paragraph(UI::g_fh->fontset()->representative_character(), info))
	   ->height();
}

int text_height(UI::FontStyle style, float scale) {
	return text_height(g_style_manager->font_style(style), scale);
}

std::string richtext_escape(const std::string& given_text) {
	std::string text = given_text;
	replace_all(text, "&", "&amp;");  // Must be performed first
	replace_all(text, ">", "&gt;");
	replace_all(text, "<", "&lt;");
	return text;
}

/** Converts all newlines in the given string to <br> and &nbsp; tags. */
void newlines_to_richtext(std::string& text) {
	// Double paragraphs should generate an empty line.
	// We do this here rather than in the font renderer, because a single \n
	// should only create a new line without any added space.
	// \n\n or \n\n\n will give us 1 blank line,
	// \n\n\n or \n\n\n\” will give us 2 blank lines etc.
	// TODO(GunChleoc): Revisit this once the old font renderer is completely gone.
	replace_all(text, "\n\n", "<br>&nbsp;<br>");
	replace_all(text, "\n", "<br>");
}

std::string as_font_tag(UI::FontStyle style, const std::string& text) {
	return g_style_manager->font_style(style).as_font_tag(text);
}

std::string as_font_tag(const std::string& text, UI::FontStyle style) {
	return as_font_tag(style, text);
}

/// Bullet list item
std::string as_listitem(const std::string& txt, UI::FontStyle style) {
	const UI::FontStyleInfo& font_style = g_style_manager->font_style(style);
	return format("<div width=100%%><div><p><font size=%d "
	              "color=%s>•</font></p></div><div><p><space gap=6></p></div><div "
	              "width=*><p><font size=%d color=%s>%s<vspace "
	              "gap=6></font></p></div></div>",
	              font_style.size(), font_style.color().hex_value(), font_style.size(),
	              font_style.color().hex_value(), txt);
}

std::string as_richtext(const std::string& txt) {
	return format("<rt>%s</rt>", txt);
}

std::string as_richtext_paragraph(const std::string& text, UI::FontStyle style, UI::Align align) {
	return as_richtext_paragraph(text, g_style_manager->font_style(style), align);
}

std::string
as_richtext_paragraph(const std::string& text, const UI::FontStyleInfo& style, UI::Align align) {
	return as_richtext_paragraph(style.as_font_tag(text), align);
}

std::string as_editor_richtext_paragraph(const std::string& text, const UI::FontStyleInfo& style) {
	return format("<rt keep_spaces=1><p>%s</p></rt>", style.as_font_tag(text));
}

std::string as_game_tip(const std::string& txt) {
	return format(
	   "<rt><p align=center>%s</p></rt>", as_font_tag(UI::FontStyle::kFsMenuGameTip, txt));
}

std::string as_mapobject_message(const std::string& image,
                                 int width,
                                 const std::string& txt,
                                 const RGBColor* player_color) {
	assert(!image.empty());
	assert(!txt.empty());
	const std::string image_type = g_image_cache->has(image) ? "src" : "object";
	if (player_color != nullptr) {
		return format("<div padding_r=10><p><img width=%d %s=%s color=%s></p></div>"
		              "<div width=*><p>%s</p></div>",
		              width, image_type, image, player_color->hex_value(),
		              as_font_tag(UI::FontStyle::kWuiMessageParagraph, txt));
	}
	return format("<div padding_r=10><p><img width=%d %s=%s></p></div>"
	              "<div width=*><p>%s</p></div>",
	              width, image_type, image, as_font_tag(UI::FontStyle::kWuiMessageParagraph, txt));
}

std::string as_message(const std::string& heading, const std::string& body) {
	return (
	   format("<rt><p>%s<br></p><vspace gap=6>%s</rt>",
	          as_font_tag(UI::FontStyle::kWuiMessageHeading, heading),
	          (is_paragraph(body) || is_div(body) ?
                 body :
                 format("<p>%s</p>", as_font_tag(UI::FontStyle::kWuiMessageParagraph, body)))));
}

std::string
as_url_hyperlink(const std::string& url, const std::string& text, const std::string& mouseover) {
	return format(
	   "<link type=url target=%1$s mouseover=\"%2$s\"><font underline=1>%3$s</font></link>", url,
	   mouseover, text);
}

std::shared_ptr<const UI::RenderedText>
autofit_text(const std::string& text, const UI::FontStyleInfo& font_info, int width) {
	std::shared_ptr<const UI::RenderedText> rendered_text =
	   UI::g_fh->render(as_richtext_paragraph(text, font_info));

	// Autoshrink if it doesn't fit
	if (width > 0 && rendered_text->width() > width) {
		const int minimum_size = g_style_manager->minimum_font_size();
		// We take a copy, because we are changing values during the autofit.
		UI::FontStyleInfo temp_font_info(font_info);
		temp_font_info.make_condensed();
		while (rendered_text->width() > width && temp_font_info.size() >= minimum_size) {
			rendered_text = UI::g_fh->render(as_richtext_paragraph(text, temp_font_info));
			temp_font_info.set_size(temp_font_info.size() - 1);
		}
	}
	return rendered_text;
}

std::string as_heading_with_content(const std::string& header,
                                    const std::string& content,
                                    UI::PanelStyle style,
                                    bool is_first,
                                    bool noescape) {
	switch (style) {
	case UI::PanelStyle::kFsMenu:
		return format("<p>%s%s %s</p>", (is_first ? "" : "<vspace gap=9>"),
		              as_font_tag(UI::FontStyle::kFsMenuInfoPanelHeading,
		                          noescape ? header : richtext_escape(header)),
		              as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph,
		                          noescape ? content : richtext_escape(content)));
	case UI::PanelStyle::kWui:
		return format("<p>%s%s %s</p>", (is_first ? "" : "<vspace gap=6>"),
		              as_font_tag(UI::FontStyle::kWuiInfoPanelHeading,
		                          noescape ? header : richtext_escape(header)),
		              as_font_tag(UI::FontStyle::kWuiInfoPanelParagraph,
		                          noescape ? content : richtext_escape(content)));
	}
	NEVER_HERE();
}

std::string as_heading(const std::string& txt, UI::PanelStyle style, bool is_first) {
	switch (style) {
	case UI::PanelStyle::kFsMenu:
		return format("<p>%s%s</p>", (is_first ? "" : "<vspace gap=9>"),
		              as_font_tag(UI::FontStyle::kFsMenuInfoPanelHeading, richtext_escape(txt)));
	case UI::PanelStyle::kWui:
		return format("<p>%s%s</p>", (is_first ? "" : "<vspace gap=6>"),
		              as_font_tag(UI::FontStyle::kWuiInfoPanelHeading, richtext_escape(txt)));
	}
	NEVER_HERE();
}

std::string as_content(const std::string& txt, UI::PanelStyle style) {
	switch (style) {
	case UI::PanelStyle::kFsMenu:
		return format("<p><vspace gap=2>%s</p>",
		              as_font_tag(UI::FontStyle::kFsMenuInfoPanelParagraph, richtext_escape(txt)));
	case UI::PanelStyle::kWui:
		return format("<p><vspace gap=2>%s</p>",
		              as_font_tag(UI::FontStyle::kWuiInfoPanelParagraph, richtext_escape(txt)));
	}
	NEVER_HERE();
}

std::string as_tooltip_text_with_hotkey(const std::string& text,
                                        const std::string& hotkey,
                                        const UI::PanelStyle style) {
	return format("<rt><p>%s %s</p></rt>",
	              as_font_tag(style == UI::PanelStyle::kWui ? UI::FontStyle::kWuiTooltip :
                                                             UI::FontStyle::kFsTooltip,
	                          text),
	              as_font_tag(style == UI::PanelStyle::kWui ? UI::FontStyle::kWuiTooltipHotkey :
                                                             UI::FontStyle::kFsTooltipHotkey,
	                          "(" + hotkey + ")"));
}

std::string as_vspace(const int gap) {
	if (gap <= 0) {
		return "";
	}
	return format("<vspace gap=%d>", gap);
}

std::string as_paragraph_style(UI::ParagraphStyle style, const std::string& text) {
	return g_style_manager->paragraph_style(style).as_paragraph(text);
}

std::string
as_paragraph_style(UI::ParagraphStyle style, const std::string& attrib, const std::string& text) {
	return g_style_manager->paragraph_style(style).as_paragraph(text, attrib);
}

std::string open_paragraph_style(UI::ParagraphStyle style, const std::string& attrib) {
	return g_style_manager->paragraph_style(style).open_paragraph(attrib);
}

std::string close_paragraph_style(UI::ParagraphStyle style) {
	return g_style_manager->paragraph_style(style).close_paragraph();
}

std::string as_definition_line(const std::string& term, const std::string& description) {
	return as_paragraph_style(UI::ParagraphStyle::kWuiText,
	                          format("<font bold=1>%s</font> %s",
	                                 format(pgettext("definitionterm", "%s:"), term), description));
}
