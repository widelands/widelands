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
#include "graphic/text_layout.h"

namespace UI {

std::string ParagraphStyleInfo::as_paragraph(const std::string& text,
                                             const std::string& attrib) const {
	return format("%s%s%s", open_paragraph(attrib), text, close_paragraph());
}

std::string ParagraphStyleInfo::open_paragraph(const std::string& attrib) const {
	std::string attribs_out;
	if (!halign_.empty()) {
		attribs_out += " align=" + halign_;
	}
	if (!valign_.empty()) {
		attribs_out += " valign=" + valign_;
	}
	if (indent_ > 0) {
		attribs_out += format(" indent=%d", indent_);
	}
	if (spacing_ > 0) {
		attribs_out += format(" spacing=%d", spacing_);
	}
	if (!attrib.empty()) {
		attribs_out += " " + attrib;
	}
	return format("<p%s>%s%s", attribs_out, as_vspace(space_before_), font_->as_font_open());
}

std::string ParagraphStyleInfo::close_paragraph() const {
	return format("</font>%s</p>", as_vspace(space_after_));
}

}  // namespace UI
