/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_STYLES_PARAGRAPH_STYLE_H
#define WL_GRAPHIC_STYLES_PARAGRAPH_STYLE_H

#include <memory>
#include <string>

#include "graphic/styles/font_style.h"

namespace UI {
enum class ParagraphStyle {

	/************************************************************************************
	 *
	 * Don't forget to update doc/sphinx/source/themes.rst when you add or remove styles!
	 *
	 ************************************************************************************/

	kReadmeTitle,
	kAboutTitle,
	kAboutSubtitle,
	kAuthorsHeading1,
	kFsHeading1,
	kFsHeading2,
	kFsHeading3,
	kFsHeading4,
	kFsText,
	kWuiObjectivesHeading,
	kWuiHeading1,
	kWuiHeading2,
	kWuiHeading3,
	kWuiHeading4,
	kWuiText,
	kWuiImageLine,
	kWuiLoreAuthor,

	// Returned when lookup by name fails
	kUnknown
};

struct ParagraphStyleInfo {
	explicit ParagraphStyleInfo(UI::FontStyleInfo* init_font,
	                            const std::string& init_halign,
	                            const std::string& init_valign,
	                            const int init_indent,
	                            const int init_spacing,
	                            const int init_before,
	                            const int init_after)
	   : font_(init_font),
	     halign_(init_halign),
	     valign_(init_valign),
	     indent_(init_indent),
	     spacing_(init_spacing),
	     space_before_(init_before),
	     space_after_(init_after) {
	}
	ParagraphStyleInfo(const ParagraphStyleInfo& other)
	   : font_(new UI::FontStyleInfo(other.font())),
	     halign_(other.halign()),
	     valign_(other.valign()),
	     indent_(other.indent()),
	     spacing_(other.spacing()),
	     space_before_(other.space_before()),
	     space_after_(other.space_after()) {
	}

	[[nodiscard]] const UI::FontStyleInfo& font() const {
		return *font_;
	}
	[[nodiscard]] const std::string halign() const {
		return halign_;
	}
	[[nodiscard]] const std::string valign() const {
		return valign_;
	}
	[[nodiscard]] int indent() const {
		return indent_;
	}
	[[nodiscard]] int spacing() const {
		return spacing_;
	}
	[[nodiscard]] int space_before() const {
		return space_before_;
	}
	[[nodiscard]] int space_after() const {
		return space_after_;
	}

	/**
	 * Add enclosing paragraph and font richtext tags to the given text to format it according to
	 * this style.
	 */
	[[nodiscard]] std::string as_paragraph(const std::string& text,
	                                       const std::string& attrib = "") const;

	/** Return opening paragraph and font richtext tags for this style */
	[[nodiscard]] std::string open_paragraph(const std::string& attrib = "") const;

	/** Return closing font and paragraph richtext tags for this style */
	[[nodiscard]] std::string close_paragraph() const;

private:
	std::unique_ptr<const UI::FontStyleInfo> font_;
	std::string halign_;
	std::string valign_;
	int indent_;
	int spacing_;
	int space_before_;
	int space_after_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_PARAGRAPH_STYLE_H
