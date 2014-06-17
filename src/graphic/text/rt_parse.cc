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

#include "graphic/text/rt_parse.h"

#include <memory>
#include <vector>

#include <SDL.h>
#include <boost/format.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "graphic/text/rt_errors_impl.h"
#include "graphic/text/textstream.h"

using namespace std;
using namespace boost;

namespace RT {

struct TagConstraint {
	unordered_set<string> allowed_attrs;
	unordered_set<string> allowed_childs;
	bool text_allowed;
	bool has_closing_tag;
};
typedef unordered_map<string, TagConstraint> TagConstraints;

// Interface Stuff {{{
class Attr : public IAttr {
public:
	Attr(string gname, string value) : m_name(gname), m_value(value) {};
	virtual ~Attr() {};

	virtual const string & name() const override {return m_name;}
	virtual long get_int() const override;
	virtual bool get_bool() const override;
	virtual string get_string() const override;
	virtual RGBColor get_color() const override;

private:
	string m_name, m_value;
};

long Attr::get_int() const {
	long rv = strtol(m_value.c_str(), nullptr, 10);
	return rv;
}
string Attr::get_string() const {
	return m_value;
}
bool Attr::get_bool() const {
	if (m_value == "true" or m_value == "1" or m_value == "yes")
		return true;
	return false;
}
RGBColor Attr::get_color() const {
	if (m_value.size() != 6)
		throw InvalidColor((format("Could not parse '%s' as a color.") % m_value).str());

	uint32_t clrn = strtol(m_value.c_str(), nullptr, 16);
	return RGBColor((clrn >> 16) & 0xff, (clrn >> 8) & 0xff, clrn & 0xff);
}

// This is basically a map<string, Attr>, but because there is no
// .at() in the STL, we need to define our own read only map
class AttrMap : public IAttrMap {
public:
	void add_attribute(const string& name, Attr* a) {
		m_attrs[name] = std::unique_ptr<Attr>(a);
	}

	const IAttr& operator[](const std::string& s) const override {
		AttributesMap::const_iterator i = m_attrs.find(s);
		if (i == m_attrs.end()) {
			throw AttributeNotFound(s);
		}
		return *(i->second);
	}

	bool has(const std::string & s) const override {
		return m_attrs.count(s);
	}


private:
	typedef map<string, std::unique_ptr<Attr>> AttributesMap;
	AttributesMap m_attrs;
};

class Tag : public ITag {
public:
	Tag();
	virtual ~Tag();

	virtual const string & name() const override {return m_name;}
	virtual const AttrMap & attrs() const override {return m_am;}
	virtual const ChildList & childs() const override {return m_childs;}
	void parse(TextStream & ts, TagConstraints & tcs, const TagSet &);

private:
	void m_parse_opening_tag(TextStream & ts, TagConstraints & tcs);
	void m_parse_closing_tag(TextStream & ts);
	void m_parse_attribute(TextStream & ts, unordered_set<string> &);
	void m_parse_content(TextStream & ts, TagConstraints & tc, const TagSet &);

	string m_name;
	AttrMap m_am;
	ChildList m_childs;
};
Tag::Tag() {}

Tag::~Tag() {
	while (m_childs.size()) {
		delete m_childs.back();
		m_childs.pop_back();
	}
}

void Tag::m_parse_opening_tag(TextStream & ts, TagConstraints & tcs) {
	ts.expect("<");
	m_name = ts.till_any(" \t\n>");
	ts.skip_ws();

	while (ts.peek(1) != ">") {
		m_parse_attribute(ts, tcs[m_name].allowed_attrs);
		ts.skip_ws();
	}

	ts.expect(">");
}

void Tag::m_parse_closing_tag(TextStream & ts) {
	ts.expect("</");
	ts.expect(m_name, false);
	ts.expect(">", false);
}

void Tag::m_parse_attribute(TextStream & ts, unordered_set<string> & allowed_attrs) {
	string aname = ts.till_any("=");
	if (!allowed_attrs.count(aname))
		throw SyntaxError_Impl(ts.line(), ts.col(), "an allowed attribute", aname, ts.peek(100));

	ts.skip(1);

	m_am.add_attribute(aname, new Attr(aname, ts.parse_string()));
}

void Tag::m_parse_content(TextStream & ts, TagConstraints & tcs, const TagSet & allowed_tags)
{
	TagConstraint tc = tcs[m_name];

	for (;;) {
		if (not tc.text_allowed)
			ts.skip_ws();

		size_t line = ts.line(), col = ts.col();
		string text = ts.till_any("<");
		if (text != "") {
			if (not tc.text_allowed)
				throw SyntaxError_Impl(line, col, "no text, as only tags are allowed here", text, ts.peek(100));
			m_childs.push_back(new Child(text));
		}

		if (ts.peek(2 + m_name.size()) == ("</" + m_name))
			break;

		Tag * child = new Tag();
		line = ts.line(); col = ts.col(); size_t cpos = ts.pos();
		child->parse(ts, tcs, allowed_tags);
		if (!tc.allowed_childs.count(child->name()))
			throw SyntaxError_Impl(line, col, "an allowed tag", child->name(), ts.peek(100, cpos));
		if (!allowed_tags.empty() and !allowed_tags.count(child->name()))
			throw SyntaxError_Impl(line, col, "an allowed tag", child->name(), ts.peek(100, cpos));

		m_childs.push_back(new Child(child));
	}
}

void Tag::parse(TextStream & ts, TagConstraints & tcs, const TagSet & allowed_tags) {
	m_parse_opening_tag(ts, tcs);

	TagConstraint tc = tcs[m_name];
	if (tc.has_closing_tag) {
		m_parse_content(ts, tcs, allowed_tags);
		m_parse_closing_tag(ts);
	}
}


/*
 * Class Parser
 */
class Parser : public IParser {
public:
	Parser();
	virtual ~Parser();
	virtual ITag * parse(string text, const TagSet &) override;
	virtual string remaining_text() override;

private:
	TagConstraints m_tcs;
	TextStream * m_ts;
};

Parser::Parser() :
	m_ts(nullptr)
{
	{ // rt tag
		TagConstraint tc;
		tc.allowed_attrs.insert("padding");
		tc.allowed_attrs.insert("padding_r");
		tc.allowed_attrs.insert("padding_l");
		tc.allowed_attrs.insert("padding_b");
		tc.allowed_attrs.insert("padding_t");
		tc.allowed_attrs.insert("db_show_spaces");
		tc.allowed_attrs.insert("background");

		tc.allowed_childs.insert("p");
		tc.allowed_childs.insert("vspace");
		tc.allowed_childs.insert("font");
		tc.allowed_childs.insert("sub");
		tc.text_allowed = false;
		tc.has_closing_tag = true;
		m_tcs["rt"] = tc;
	}
	{ // br tag
		TagConstraint tc;
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		m_tcs["br"] = tc;
	}
	{ // img tag
		TagConstraint tc;
		tc.allowed_attrs.insert("src");
		tc.allowed_attrs.insert("ref");
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		m_tcs["img"] = tc;
	}
	{ // vspace tag
		TagConstraint tc;
		tc.allowed_attrs.insert("gap");
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		m_tcs["vspace"] = tc;
	}
	{ // space tag
		TagConstraint tc;
		tc.allowed_attrs.insert("gap");
		tc.allowed_attrs.insert("fill");
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		m_tcs["space"] = tc;
	}
	{ // sub tag
		TagConstraint tc;
		tc.allowed_attrs.insert("padding");
		tc.allowed_attrs.insert("padding_r");
		tc.allowed_attrs.insert("padding_l");
		tc.allowed_attrs.insert("padding_b");
		tc.allowed_attrs.insert("padding_t");
		tc.allowed_attrs.insert("margin");
		tc.allowed_attrs.insert("float");
		tc.allowed_attrs.insert("valign");
		tc.allowed_attrs.insert("background");
		tc.allowed_attrs.insert("width");

		tc.allowed_childs.insert("p");
		tc.allowed_childs.insert("vspace");
		tc.allowed_childs.insert("font");
		tc.allowed_childs.insert("sub");

		tc.text_allowed = false;
		tc.has_closing_tag = true;
		m_tcs["sub"] = tc;
	}
	{ // p tag
		TagConstraint tc;
		tc.allowed_attrs.insert("indent");
		tc.allowed_attrs.insert("align");
		tc.allowed_attrs.insert("valign");
		tc.allowed_attrs.insert("spacing");

		tc.allowed_childs.insert("font");
		tc.allowed_childs.insert("space");
		tc.allowed_childs.insert("br");
		tc.allowed_childs.insert("img");
		tc.allowed_childs.insert("sub");
		tc.text_allowed = true;
		tc.has_closing_tag = true;
		m_tcs["p"] = tc;
	}
	{ // font tag
		TagConstraint tc;
		tc.allowed_attrs.insert("size");
		tc.allowed_attrs.insert("face");
		tc.allowed_attrs.insert("color");
		tc.allowed_attrs.insert("bold");
		tc.allowed_attrs.insert("italic");
		tc.allowed_attrs.insert("underline");
		tc.allowed_attrs.insert("shadow");
		tc.allowed_attrs.insert("ref");

		tc.allowed_childs.insert("br");
		tc.allowed_childs.insert("space");
		tc.allowed_childs.insert("p");
		tc.allowed_childs.insert("font");
		tc.allowed_childs.insert("sub");
		tc.text_allowed = true;
		tc.has_closing_tag = true;
		m_tcs["font"] = tc;
	}
}

Parser::~Parser() {
	if (m_ts)
		delete m_ts;
}

ITag * Parser::parse(string text, const TagSet & allowed_tags) {
	if (m_ts) {
		delete m_ts;
		m_ts = nullptr;
	}
	m_ts = new TextStream(text);

	m_ts->skip_ws(); m_ts->rskip_ws();
	Tag * rv = new Tag();
	rv->parse(*m_ts, m_tcs, allowed_tags);

	return rv;
}
string Parser::remaining_text() {
	if (!m_ts)
		return "";
	return m_ts->remaining_text();
}
// End: Interface Stuff }}}

IParser * setup_parser() {
	return new Parser();
};

}
