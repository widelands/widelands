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
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <SDL.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/format.hpp>

#include "graphic/text/rt_errors_impl.h"
#include "graphic/text/textstream.h"


namespace RT {

Attr::Attr(const std::string& gname, const std::string& value) : name_(gname), value_(value) {
}

const std::string& Attr::name() const {
	return name_;
}

long Attr::get_int() const {
	long rv = strtol(value_.c_str(), nullptr, 10);
	return rv;
}

std::string Attr::get_string() const {
	return value_;
}

bool Attr::get_bool() const {
	if (value_ == "true" || value_ == "1" || value_ == "yes")
		return true;
	return false;
}

RGBColor Attr::get_color() const {
	if (value_.size() != 6)
		throw InvalidColor((boost::format("Could not parse '%s' as a color.") % value_).str());

	uint32_t clrn = strtol(value_.c_str(), nullptr, 16);
	return RGBColor((clrn >> 16) & 0xff, (clrn >> 8) & 0xff, clrn & 0xff);
}

void AttrMap::add_attribute(const std::string& name, Attr* a) {
	attrs_[name] = std::unique_ptr<Attr>(a);
}

const Attr& AttrMap::operator[](const std::string& s) const {
	const auto i = attrs_.find(s);
	if (i == attrs_.end()) {
		throw AttributeNotFound(s);
	}
	return *(i->second);
}

bool AttrMap::has(const std::string& s) const {
	return attrs_.count(s);
}

const std::string& Tag::name() const {
	return name_;
}

const AttrMap& Tag::attrs() const {
	return attribute_map_;
}

const Tag::ChildList& Tag::children() const {
	return children_;
}

Tag::~Tag() {
	while (children_.size()) {
		delete children_.back();
		children_.pop_back();
	}
}

void Tag::parse_opening_tag(TextStream & ts, TagConstraints & tcs) {
	ts.expect("<");
	name_ = ts.till_any(" \t\n>");
	ts.skip_ws();

	while (ts.peek(1) != ">") {
		parse_attribute(ts, tcs[name_].allowed_attrs);
		ts.skip_ws();
	}

	ts.expect(">");
}

void Tag::parse_closing_tag(TextStream & ts) {
	ts.expect("</");
	ts.expect(name_, false);
	ts.expect(">", false);
}

void Tag::parse_attribute(TextStream & ts, std::unordered_set<std::string> & allowed_attrs) {
	std::string aname = ts.till_any("=");
	if (!allowed_attrs.count(aname))
		throw SyntaxErrorImpl(ts.line(), ts.col(), "an allowed attribute", aname, ts.peek(100));

	ts.skip(1);

	attribute_map_.add_attribute(aname, new Attr(aname, ts.parse_string()));
}

void Tag::parse_content(TextStream & ts, TagConstraints & tcs, const TagSet & allowed_tags)
{
	TagConstraint tc = tcs[name_];

	for (;;) {
		if (!tc.text_allowed)
			ts.skip_ws();

		size_t line = ts.line(), col = ts.col();
		std::string text = ts.till_any("<");
		if (text != "") {
			if (!tc.text_allowed) {
				throw SyntaxErrorImpl(line, col, "no text, as only tags are allowed here", text, ts.peek(100));
			}
			children_.push_back(new Child(text));
		}

		if (ts.peek(2 + name_.size()) == ("</" + name_))
			break;

		Tag * child = new Tag();
		line = ts.line(); col = ts.col(); size_t cpos = ts.pos();
		child->parse(ts, tcs, allowed_tags);
		if (!tc.allowed_children.count(child->name()))
			throw SyntaxErrorImpl(line, col, "an allowed tag", child->name(), ts.peek(100, cpos));
		if (!allowed_tags.empty() && !allowed_tags.count(child->name()))
			throw SyntaxErrorImpl(line, col, "an allowed tag", child->name(), ts.peek(100, cpos));

		children_.push_back(new Child(child));
	}
}

void Tag::parse(TextStream & ts, TagConstraints & tcs, const TagSet & allowed_tags) {
	parse_opening_tag(ts, tcs);

	TagConstraint tc = tcs[name_];
	if (tc.has_closing_tag) {
		parse_content(ts, tcs, allowed_tags);
		parse_closing_tag(ts);
	}
}


/*
 * Class Parser
 */
Parser::Parser() {
	{ // rt tag
		TagConstraint tc;
		tc.allowed_attrs.insert("padding");
		tc.allowed_attrs.insert("padding_r");
		tc.allowed_attrs.insert("padding_l");
		tc.allowed_attrs.insert("padding_b");
		tc.allowed_attrs.insert("padding_t");
		tc.allowed_attrs.insert("db_show_spaces");
		tc.allowed_attrs.insert("keep_spaces"); // Keeps blank spaces intact for text editing
		tc.allowed_attrs.insert("background");

		tc.allowed_children.insert("p");
		tc.allowed_children.insert("vspace");
		tc.allowed_children.insert("font");
		tc.allowed_children.insert("sub");
		tc.text_allowed = false;
		tc.has_closing_tag = true;
		tag_constraints_["rt"] = tc;
	}
	{ // br tag
		TagConstraint tc;
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		tag_constraints_["br"] = tc;
	}
	{ // img tag
		TagConstraint tc;
		tc.allowed_attrs.insert("src");
		tc.allowed_attrs.insert("ref");
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		tag_constraints_["img"] = tc;
	}
	{ // vspace tag
		TagConstraint tc;
		tc.allowed_attrs.insert("gap");
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		tag_constraints_["vspace"] = tc;
	}
	{ // space tag
		TagConstraint tc;
		tc.allowed_attrs.insert("gap");
		tc.allowed_attrs.insert("fill");
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		tag_constraints_["space"] = tc;
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

		tc.allowed_children.insert("p");
		tc.allowed_children.insert("vspace");
		tc.allowed_children.insert("font");
		tc.allowed_children.insert("sub");

		tc.text_allowed = false;
		tc.has_closing_tag = true;
		tag_constraints_["sub"] = tc;
	}
	{ // p tag
		TagConstraint tc;
		tc.allowed_attrs.insert("indent");
		tc.allowed_attrs.insert("align");
		tc.allowed_attrs.insert("valign");
		tc.allowed_attrs.insert("spacing");

		tc.allowed_children.insert("font");
		tc.allowed_children.insert("space");
		tc.allowed_children.insert("br");
		tc.allowed_children.insert("img");
		tc.allowed_children.insert("sub");
		tc.text_allowed = true;
		tc.has_closing_tag = true;
		tag_constraints_["p"] = tc;
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

		tc.allowed_children.insert("br");
		tc.allowed_children.insert("space");
		tc.allowed_children.insert("p");
		tc.allowed_children.insert("font");
		tc.allowed_children.insert("sub");
		tc.text_allowed = true;
		tc.has_closing_tag = true;
		tag_constraints_["font"] = tc;
	}
}

Parser::~Parser() {
}

Tag * Parser::parse(std::string text, const TagSet & allowed_tags) {
	boost::replace_all(text, "\\", "\\\\"); // Prevent crashes with \.

	text_stream_.reset(new TextStream(text));

	text_stream_->skip_ws(); text_stream_->rskip_ws();
	Tag * rv = new Tag();
	rv->parse(*text_stream_, tag_constraints_, allowed_tags);

	return rv;
}
std::string Parser::remaining_text() {
	if (text_stream_ == nullptr)
		return "";
	return text_stream_->remaining_text();
}

}
