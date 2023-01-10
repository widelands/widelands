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
	kIngameHeading1,
	kIngameHeading2,
	kIngameHeading3,
	kIngameHeading4,
	kIngameText
};

struct ParagraphStyleInfo {
	explicit ParagraphStyleInfo(
	   UI::FontStyleInfo* init_font, const int init_before, const int init_after) :
	   font_(init_font), space_before_(init_before), space_after_(init_after) {
	}
	ParagraphStyleInfo(const ParagraphStyleInfo& other)
	   : font_(new UI::FontStyleInfo(other.font())),
        space_before_(other.space_before()),
	     space_after_(other.space_after()) {
	}

	[[nodiscard]] const UI::FontStyleInfo& font() const {
		return *font_;
	}
	[[nodiscard]] int space_before() const {
		return space_before_;
	}
	[[nodiscard]] int space_after() const {
		return space_after_;
	}
	void set_space_before(int new_space) {
		space_before_ = new_space;
	}
	void set_space_after(int new_space) {
		space_after_ = new_space;
	}

	/**
    * Add enclosing paragraph and font richtext tags to the given text to format it according to
    * this style.
    */
	[[nodiscard]] std::string as_paragraph(const std::string& text, const std::string& attrib = "") const;

	/** Return opening paragraph and font richtext tags for this style */
	[[nodiscard]] std::string as_paragraph_open(const std::string& attrib = "") const;

	/** Return closing font and paragraph richtext tags for this style */
	[[nodiscard]] std::string as_paragraph_close() const;

private:
	std::unique_ptr<const UI::FontStyleInfo> font_;
	int space_before_;
	int space_after_;
};

}  // namespace UI

#endif  // end of include guard: WL_GRAPHIC_STYLES_PARAGRAPH_STYLE_H
