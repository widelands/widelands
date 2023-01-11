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

#include "graphic/styles/paragraph_style.h"

#include "base/string.h"

namespace UI {

std::string ParagraphStyleInfo::as_paragraph(const std::string& text,
                                             const std::string& attrib) const {
	return format("%s%s%s", open_paragraph(attrib), text, close_paragraph());
}

// TODO(tothxa): this should be in graphic/text_layout.h, but may I include that here?
[[nodiscard]] std::string as_vspace(const int gap) {
	if (gap <= 0) {
		return "";
	}
	return format("<vspace gap=%d>", gap);
}

std::string ParagraphStyleInfo::open_paragraph(const std::string& attrib) const {
	return format("<p %s>%s%s", attrib, as_vspace(space_before_), font_->as_font_open());
}

std::string ParagraphStyleInfo::close_paragraph() const {
	const int gap2 = space_after_ / 2;
	const int gap1 = space_after_ - gap2;
	return format("%s</font>%s</p>", as_vspace(gap1), as_vspace(gap2));
}

}  // namespace UI
