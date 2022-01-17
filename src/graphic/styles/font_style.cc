/*
 * Copyright (C) 2018-2022 by the Widelands Development Team
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

#include "graphic/styles/font_style.h"

#include "base/string.h"
#include "base/wexception.h"

namespace UI {

FontStyleInfo::FontStyleInfo(const std::string& init_face,
                             const RGBColor& init_color,
                             int init_size,
                             bool init_bold,
                             bool init_italic,
                             bool init_underline,
                             bool init_shadow)
   : face_(string_to_face(init_face)),
     color_(init_color),
     size_(init_size),
     bold_(init_bold),
     italic_(init_italic),
     underline_(init_underline),
     shadow_(init_shadow) {
}

FontStyleInfo::FontStyleInfo(const FontStyleInfo& other)
   : face_(other.face()),
     color_(other.color()),
     size_(other.size()),
     bold_(other.bold()),
     italic_(other.italic()),
     underline_(other.underline()),
     shadow_(other.shadow()) {
}

const std::string FontStyleInfo::face_to_string() const {
	switch (face_) {
	case Face::kSans:
		return "sans";
	case Face::kSerif:
		return "serif";
	case Face::kCondensed:
		return "condensed";
	}
	return "sans";
}

FontStyleInfo::Face FontStyleInfo::string_to_face(const std::string& init_face) {
	FontStyleInfo::Face result;
	if (init_face == "sans") {
		result = Face::kSans;
	} else if (init_face == "serif") {
		result = Face::kSerif;
	} else if (init_face == "condensed") {
		result = Face::kCondensed;
	} else {
		throw wexception(
		   "Unknown font face '%s', expected 'sans', 'serif' or 'condensed'", init_face.c_str());
	}
	return result;
}

std::string FontStyleInfo::as_font_tag(const std::string& text) const {
	std::string optionals;
	if (bold_) {
		optionals += " bold=1";
	}
	if (italic_) {
		optionals += " italic=1";
	}
	if (shadow_) {
		optionals += " shadow=1";
	}
	if (underline_) {
		optionals += " underline=1";
	}
	return format("<font face=%s size=%d color=%s%s>%s</font>", face_to_string(), size_,
	              color_.hex_value(), optionals, text);
}

FontStyleInfo::Face FontStyleInfo::face() const {
	return face_;
}
void FontStyleInfo::make_condensed() {
	face_ = Face::kCondensed;
}
const RGBColor& FontStyleInfo::color() const {
	return color_;
}
void FontStyleInfo::set_color(const RGBColor& new_color) {
	color_ = new_color;
}
int FontStyleInfo::size() const {
	return size_;
}
void FontStyleInfo::set_size(int new_size) {
	size_ = new_size;
}
bool FontStyleInfo::bold() const {
	return bold_;
}
bool FontStyleInfo::italic() const {
	return italic_;
}
bool FontStyleInfo::underline() const {
	return underline_;
}
bool FontStyleInfo::shadow() const {
	return shadow_;
}

}  // namespace UI
