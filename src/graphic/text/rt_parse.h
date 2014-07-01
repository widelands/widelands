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

#ifndef RT_PARSER_H
#define RT_PARSER_H

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

class ITag {
public:
	typedef std::vector<Child*> ChildList;

	virtual ~ITag() {
	}
	virtual const std::string& name() const = 0;
	virtual const AttrMap& attrs() const = 0;
	virtual const ChildList& childs() const = 0;
};

struct Child {
	Child() : tag(nullptr), text() {
	}
	Child(ITag* t) : tag(t) {
	}
	Child(std::string t) : tag(nullptr), text(t) {
	}
	~Child() {
		if (tag)
			delete tag;
	}
	ITag* tag;
	std::string text;
};

struct TagConstraint {
	std::unordered_set<std::string> allowed_attrs;
	std::unordered_set<std::string> allowed_childs;
	bool text_allowed;
	bool has_closing_tag;
};
typedef std::unordered_map<std::string, TagConstraint> TagConstraints;
typedef std::set<std::string> TagSet;

class Parser {
public:
	Parser();
	~Parser();
	ITag* parse(std::string text, const TagSet&);
	std::string remaining_text();

private:
	TagConstraints m_tcs;
	std::unique_ptr<TextStream> m_ts;
};
}

#endif /* end of include guard: RT_PARSER_H */
