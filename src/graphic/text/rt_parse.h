/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_TEXT_RT_PARSE_H
#define WL_GRAPHIC_TEXT_RT_PARSE_H

#include <map>
#include <memory>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "graphic/color.h"

namespace RT {

class TextStream;
struct Child;

class Attr {
public:
	Attr(const std::string& gname, const std::string& value);

	const std::string& name() const;
	int64_t get_int(int64_t max_value) const;
	bool get_bool() const;
	std::string get_string() const;
	RGBColor get_color() const;

private:
	const std::string name_, value_;
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
	std::map<std::string, std::unique_ptr<Attr>> attrs_;
};

struct TagConstraint {
	std::unordered_set<std::string> allowed_attrs;
	std::unordered_set<std::string> allowed_children;
	bool text_allowed;
	bool has_closing_tag;
};
using TagConstraints = std::unordered_map<std::string, TagConstraint>;
using TagSet = std::set<std::string>;

class Tag {
public:
	using ChildList = std::vector<Child*>;

	~Tag();

	const std::string& name() const;
	const AttrMap& attrs() const;
	const ChildList& children() const;
	void parse(TextStream& ts, TagConstraints& tcs, const TagSet&);

private:
	void parse_opening_tag(TextStream& ts, TagConstraints& tcs);
	void parse_closing_tag(TextStream& ts);
	void parse_attribute(TextStream& ts, std::unordered_set<std::string>&);
	void parse_content(TextStream& ts, TagConstraints& tc, const TagSet&);

	std::string name_;
	AttrMap attribute_map_;
	ChildList children_;
};

struct Child {
	Child() : tag(nullptr), text() {
	}
	explicit Child(Tag* t) : tag(t) {
	}
	explicit Child(const std::string& t) : tag(nullptr), text(t) {
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
	~Parser() = default;
	Tag* parse(std::string text, const TagSet&);
	std::string remaining_text();

private:
	TagConstraints tag_constraints_;
	std::unique_ptr<TextStream> text_stream_;
};
}  // namespace RT

#endif  // end of include guard: WL_GRAPHIC_TEXT_RT_PARSE_H
