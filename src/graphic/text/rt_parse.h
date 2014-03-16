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
#include <set>
#include <string>
#include <vector>

#include <stdint.h>

#include "rgbcolor.h"
#include "graphic/text/rt_errors.h"

namespace RT {

struct Child;
class IAttr {
public:
	virtual ~IAttr() {};

	virtual const std::string & name() const = 0;
	virtual long get_int() const = 0;
	virtual bool get_bool() const = 0;
	virtual std::string get_string() const = 0;
	virtual RGBColor get_color() const = 0;
};

class IAttrMap {
public:
	virtual ~IAttrMap() {};

	virtual const IAttr & operator[] (const std::string&) const = 0;
	virtual bool has(const std::string &) const = 0;
};

class ITag {
public:
	typedef std::vector<Child*> ChildList;

	virtual ~ITag() {};
	virtual const std::string & name() const = 0;
	virtual const IAttrMap & attrs() const = 0;
	virtual const ChildList & childs() const = 0;
};

struct Child {
	Child() : tag(nullptr), text() {}
	Child(ITag * t) : tag(t) {}
	Child(std::string t) : tag(nullptr), text(t) {}
	~Child() {
		if (tag) delete tag;
	}
	ITag * tag;
	std::string text;
};


typedef std::set<std::string> TagSet;

class IParser {
public:
	virtual ~IParser() {};

	virtual ITag * parse(std::string text, const TagSet &) = 0;
	virtual std::string remaining_text() = 0;
};

// This function is mainly for testing
IParser * setup_parser();
}

#endif /* end of include guard: RT_PARSER_H */
