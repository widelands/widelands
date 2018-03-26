/*
 * Copyright (C) 2018 by the Widelands Development Team
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

#include "graphic/font_styles.h"

namespace UI {

const std::string FontStyleInfo::face_to_string() const {
	switch (face) {
	case Face::kSans:
		return "sans";
	case Face::kSerif:
		return "serif";
	case Face::kCondensed:
		return "condensed";
	}
	return "sans";
}

void FontStyleInfo::set_face(const std::string& init_face) {
	if (init_face == "sans") {
		face = Face::kSans;
	} else if (init_face == "serif") {
		face = Face::kSerif;
	} else if (init_face == "condensed") {
		face = Face::kCondensed;
	} else {
		throw wexception("Unknown font face '%s', expected 'sans', 'serif' or 'condensed'", init_face.c_str());
	}
}

std::string FontStyleInfo::as_font_tag(const std::string& text) const {
	static boost::format f("<font face=%s size=%d color=%s%s>%s</font>");
	std::string optionals = "";
	if (bold) {
		optionals += " bold=1";
	}
	if (italic) {
		optionals += " italic=1";
	}
	if (shadow) {
		optionals += " shadow=1";
	}
	if (underline) {
		optionals += " underline=1";
	}
	f % face_to_string();
	f % size;
	f % color.hex_value();
	f % optionals;
	f % text;
	return f.str();
}

} // namespace UI
