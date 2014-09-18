/*
 * Copyright (C) 2006-2012 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXT_RT_PARSE_H
#define WL_GRAPHIC_TEXT_RT_PARSE_H

#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <stdint.h>

#include "graphic/color.h"
#include "graphic/text/rt_errors.h"

namespace RT {

class TextStream;
struct Child;

class Attr {
public:
	Attr(const std::string& gname, const std::string& value);

	const std::string& name() const;
	long get_int() const;
	bool get_bool() const;
	std::string get_string() const;
	RGBColor get_color() const;

private:
	const std::string m_name, m_value;
};

// This is basically a map<string, Attr>.
class AttrMap {
public:
	// Adds a 'a' with 'name' to the list of attributes.
	void add_attribute(const std::string& name, Attr* a);

	// Returns the attribute with 'name' or throws an error if it is not found.
	const Attr& operator[](const std::string& name) const;

	// Returns true if 'name' is a known attribute.
	bool has(const std::string& name) const;

private:
	std::map<std::string, std::unique_ptr<Attr>> m_attrs;
};

struct TagConstraint {
	std::unordered_set<std::string> allowed_attrs;
	std::unordered_set<std::string> allowed_childs;
	bool text_allowed;
	bool has_closing_tag;
};
using TagConstraints = std::unordered_map<std::string, TagConstraint>;
using TagSet = std::set<std::string>;

class Tag {
public:
	using ChildList = std::vector<Child*>;

	~Tag();

	const std::string & name() const;
	const AttrMap & attrs() const;
	const ChildList & childs() const;
	void parse(TextStream& ts, TagConstraints& tcs, const TagSet&);

private:
	void m_parse_opening_tag(TextStream & ts, TagConstraints & tcs);
	void m_parse_closing_tag(TextStream & ts);
	void m_parse_attribute(TextStream & ts, std::unordered_set<std::string> &);
	void m_parse_content(TextStream & ts, TagConstraints & tc, const TagSet &);

	std::string m_name;
	AttrMap m_am;
	ChildList m_childs;
};

struct Child {
	Child() : tag(nullptr), text() {
	}
	Child(Tag* t) : tag(t) {
	}
	Child(std::string t) : tag(nullptr), text(t) {
	}
	~Child() {
		if (tag)
			delete tag;
	}
	Tag* tag;
	std::string text;
};

class Parser {
public:
	Parser();
	~Parser();
	Tag* parse(std::string text, const TagSet&);
	std::string remaining_text();

private:
	TagConstraints m_tcs;
	std::unique_ptr<TextStream> m_ts;
};
}

#endif  // end of include guard: WL_GRAPHIC_TEXT_RT_PARSE_H
