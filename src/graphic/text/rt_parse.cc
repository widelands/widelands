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

#include "graphic/text/rt_parse.h"

#include <cstdlib>
#include <memory>

#include "base/string.h"
#include "graphic/text/rt_errors_impl.h"
#include "graphic/text/textstream.h"

namespace RT {

Attr::Attr(const std::string& gname, const std::string& value) : name_(gname), value_(value) {
}

const std::string& Attr::name() const {
	return name_;
}

int64_t Attr::get_int(int64_t max_value) const {
	int64_t rv = strtol(value_.c_str(), nullptr, 10);
	if (rv > max_value) {
		throw NumberOutOfRange(value_);
	}
	return rv;
}

std::string Attr::get_string() const {
	return value_;
}

bool Attr::get_bool() const {
	return value_ == "true" || value_ == "1" || value_ == "yes";
}

RGBColor Attr::get_color() const {
	if (value_.size() != 6) {
		throw InvalidColor(format("Could not parse '%s' as a color.", value_));
	}

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
	return attrs_.count(s) != 0u;
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
	while (!children_.empty()) {
		delete children_.back();
		children_.pop_back();
	}
}

void Tag::parse_opening_tag(TextStream& ts, TagConstraints& tcs) {
	ts.expect("<");
	name_ = ts.till_any(" \t\n>");
	ts.skip_ws();

	while (ts.peek(1) != ">") {
		parse_attribute(ts, tcs[name_].allowed_attrs);
		ts.skip_ws();
	}

	ts.expect(">");
}

void Tag::parse_closing_tag(TextStream& ts) {
	ts.expect("</");
	ts.expect(name_, false);
	ts.expect(">", false);
}

void Tag::parse_attribute(TextStream& ts, std::unordered_set<std::string>& allowed_attrs) {
	std::string aname = ts.till_any("=");
	if (allowed_attrs.count(aname) == 0u) {
		const std::string error_info = format("an allowed attribute for '%s' tag", name_);
		throw SyntaxErrorImpl(ts.line(), ts.col(), error_info, aname, ts.peek(100));
	}

	ts.skip(1);

	attribute_map_.add_attribute(aname, new Attr(aname, ts.parse_string()));
}

void Tag::parse_content(TextStream& ts, TagConstraints& tcs, const TagSet& allowed_tags) {
	TagConstraint tc = tcs[name_];

	for (;;) {
		if (!tc.text_allowed) {
			ts.skip_ws();
		}

		size_t line = ts.line();
		size_t col = ts.col();
		std::string text = ts.till_any("<");
		if (!text.empty()) {
			if (!tc.text_allowed) {
				throw SyntaxErrorImpl(
				   line, col, "no text, as only tags are allowed here", text, ts.peek(100));
			}
			children_.push_back(new Child(text));
		}

		if (ts.peek(2 + name_.size()) == ("</" + name_)) {
			break;
		}

		Tag* child = new Tag();
		line = ts.line();
		col = ts.col();
		size_t cpos = ts.pos();
		child->parse(ts, tcs, allowed_tags);
		if (tc.allowed_children.count(child->name()) == 0u) {
			throw SyntaxErrorImpl(line, col, "an allowed tag", child->name(), ts.peek(100, cpos));
		}
		if (!allowed_tags.empty() && (allowed_tags.count(child->name()) == 0u)) {
			throw SyntaxErrorImpl(line, col, "an allowed tag", child->name(), ts.peek(100, cpos));
		}

		children_.push_back(new Child(child));
	}
}

void Tag::parse(TextStream& ts, TagConstraints& tcs, const TagSet& allowed_tags) {
	parse_opening_tag(ts, tcs);

	TagConstraint tc = tcs[name_];
	if (tc.has_closing_tag) {
		parse_content(ts, tcs, allowed_tags);
		parse_closing_tag(ts);
	}
}

/* RST
.. _rt_tags:

The Richtext Tags
=================

- :ref:`rt_tags_rt`
- :ref:`rt_tags_div`
- :ref:`rt_tags_br`
- :ref:`rt_tags_space`
- :ref:`rt_tags_vspace`
- :ref:`rt_tags_p`
- :ref:`rt_tags_font`
- :ref:`rt_tags_img`

For an introduction to our richtext system including a code example, see :ref:`wlrichtext`.

*/

/*
 * Class Parser
 */
Parser::Parser() {
	{
		/* RST
.. _rt_tags_rt:

Rich Text -- <rt>
-----------------

The main wrapper that will signal to the font renderer to go into richtext mode.
This tag surrounds your whole text, and is allowed only once.
You can also set some options here what will affect your whole text.

Attributes
^^^^^^^^^^

* **padding**: The rectangle of this tag is shrunk so leave a gap on its outside, on all four outer
  edges.
* **padding_r**: Padding on the right-hand side
* **padding_l**: Padding on the left-hand side
* **padding_b**: Padding on the bottom
* **padding_t**: Padding on the top

* **background**: Give this tag's rectangle a background color as a hex value.

* **keep_spaces**: Do now trim away trailing and double spaces. Use this where the user is editing
  text.
* **db_show_spaces**: Highlight all blank spaces for debugging purposes.

Sub-tags
^^^^^^^^

* :ref:`rt_tags_div`
* :ref:`rt_tags_font`
* :ref:`rt_tags_p`
* :ref:`rt_tags_vspace`

:ref:`Return to tag index<rt_tags>`
		*/
		TagConstraint tc;
		tc.allowed_attrs.insert("padding");
		tc.allowed_attrs.insert("padding_r");
		tc.allowed_attrs.insert("padding_l");
		tc.allowed_attrs.insert("padding_b");
		tc.allowed_attrs.insert("padding_t");
		tc.allowed_attrs.insert("db_show_spaces");
		tc.allowed_attrs.insert("keep_spaces");  // Keeps blank spaces intact for text editing
		tc.allowed_attrs.insert("background");

		tc.allowed_children.insert("p");
		tc.allowed_children.insert("vspace");
		tc.allowed_children.insert("font");
		tc.allowed_children.insert("div");
		tc.text_allowed = false;
		tc.has_closing_tag = true;
		tag_constraints_["rt"] = tc;
	}
	{
		/* RST
.. _rt_tags_div:

Division -- <div>
-----------------

This tag defines a rectangle and can be used as a layout control.

Attributes
^^^^^^^^^^

The same attributes as :ref:`rt_tags_rt`, plus the following:

* **margin**: Shrink all contents to leave a margin towards the outer edge of this tag's rectangle.
* **float**: Make text float around this div. Allowed values are ``left``, ``right``.
  The structure has to be something like: ``div("width=100%", div("float=left padding_r=6",
  p(img(imagepath))) .. p(text))``, with the first embedded div being the floating one.
* **valign**: Align the contents vertically. Allowed values are ``top`` (default), ``center`` or
  ``middle``, ``bottom``.
* **width**: The width of this element, as a pixel amount, or as a percentage.
  The last ``div`` in a row can be expanded automatically by using ``*``.

Sub-tags
^^^^^^^^

* :ref:`rt_tags_div`
* :ref:`rt_tags_font`
* :ref:`rt_tags_p`
* :ref:`rt_tags_vspace`

:ref:`Return to tag index<rt_tags>`
		*/
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
		tc.allowed_children.insert("div");

		tc.text_allowed = false;
		tc.has_closing_tag = true;
		tag_constraints_["div"] = tc;
	}
	{
		/* RST
.. _rt_tags_br:

Line Break -- <br>
------------------

A single line break. Use sparingly for things like poetry stanzas.
If you are starting a new paragraph, use :ref:`rt_tags_p` instead.

:ref:`Return to tag index<rt_tags>`
		*/
		TagConstraint tc;
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		tag_constraints_["br"] = tc;
	}
	{
		/* RST
.. _rt_tags_space:

Horizontal Space -- <space>
---------------------------

Inserts a horizontal gap between the previous and the following text.
This space can be filled with a character of your choice.

Attributes
^^^^^^^^^^

* **gap**: The size of the gap as a pixel amount
* **fill**: A character to fill the gap with

:ref:`Return to tag index<rt_tags>`
		*/
		TagConstraint tc;
		tc.allowed_attrs.insert("gap");
		tc.allowed_attrs.insert("fill");
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		tag_constraints_["space"] = tc;
	}
	{
		/* RST
.. _rt_tags_vspace:

Vertical Space -- <vspace>
--------------------------

Inserts a vertical gap between the previous and the following text.

Attributes
^^^^^^^^^^

* **gap**: The size of the gap as a pixel amount

:ref:`Return to tag index<rt_tags>`
		*/
		TagConstraint tc;
		tc.allowed_attrs.insert("gap");
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		tag_constraints_["vspace"] = tc;
	}
	{
		/* RST
.. _rt_tags_p:

Paragraph -- <p>
----------------

This tag encloses a text paragraph.

Attributes
^^^^^^^^^^

* **indent**: Adds an indent to the first line of the paragraph
* **align**: The horizontal alignment for the paragraph's text.
  Allowed values are ``left`` (default), ``center`` or ``middle``, ``right``.
* **valign**: See :ref:`rt_tags_div`
* **spacing**: Vertical line spacing as a pixel value

Sub-tags
^^^^^^^^

* :ref:`rt_tags_br`
* :ref:`rt_tags_div`
* :ref:`rt_tags_font`
* :ref:`rt_tags_img`
* :ref:`rt_tags_space`
* :ref:`rt_tags_vspace`

:ref:`Return to tag index<rt_tags>`
		*/
		TagConstraint tc;
		tc.allowed_attrs.insert("indent");
		tc.allowed_attrs.insert("align");
		tc.allowed_attrs.insert("valign");
		tc.allowed_attrs.insert("spacing");

		tc.allowed_children.insert("font");
		tc.allowed_children.insert("space");
		tc.allowed_children.insert("vspace");
		tc.allowed_children.insert("br");
		tc.allowed_children.insert("img");
		tc.allowed_children.insert("div");
		tc.text_allowed = true;
		tc.has_closing_tag = true;
		tag_constraints_["p"] = tc;
	}
	{
		/* RST
.. _rt_tags_font:

Font -- <font>
--------------

This tag defines the font style for the enclosed text.

Attributes
^^^^^^^^^^

* **size**: The font size as a pixel value
* **face**: The font face. Allowed values are ``sans`` (default), ``serif``  and ``condensed``.
* **color**: The font color as a hex value
* **bold**: Make the text bold
* **italic**: Make the text italic
* **underline**: Underline the text
* **shadow**: Add a background shadow
* **ref**: To be implemented

Sub-tags
^^^^^^^^

* :ref:`rt_tags_br`
* :ref:`rt_tags_div`
* :ref:`rt_tags_font`
* :ref:`rt_tags_img`
* :ref:`rt_tags_p`
* :ref:`rt_tags_space`
* :ref:`rt_tags_vspace`

:ref:`Return to tag index<rt_tags>`
		*/
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
		tc.allowed_children.insert("vspace");
		tc.allowed_children.insert("p");
		tc.allowed_children.insert("font");
		tc.allowed_children.insert("div");
		tc.allowed_children.insert("img");
		tc.text_allowed = true;
		tc.has_closing_tag = true;
		tag_constraints_["font"] = tc;
	}
	{
		/* RST
.. _rt_tags_img:

Image -- <img>
--------------

Displays an image with your text.

Attributes
^^^^^^^^^^

* **src**: The path to the image, relative to the ``data`` directory.
* **object**: Show the representative image of a map object instead of using ``src``.
* **ref**: To be implemented
* **color**: Player color for the image as a hex value
* **width**: Width of the image as a pixel amount.
  The corresponding height will be matched automatically.
  Not supported in conjunction with the ``object`` parameter.

:ref:`Return to tag index<rt_tags>`
		*/
		TagConstraint tc;
		tc.allowed_attrs.insert("src");
		tc.allowed_attrs.insert("object");
		tc.allowed_attrs.insert("ref");
		tc.allowed_attrs.insert("color");
		tc.allowed_attrs.insert("width");
		tc.text_allowed = false;
		tc.has_closing_tag = false;
		tag_constraints_["img"] = tc;
	}
}

Tag* Parser::parse(std::string text, const TagSet& allowed_tags) {
	replace_all(text, "\\", "\\\\");  // Prevent crashes with \.

	text_stream_.reset(new TextStream(text));

	text_stream_->skip_ws();
	text_stream_->rskip_ws();
	Tag* rv = new Tag();
	rv->parse(*text_stream_, tag_constraints_, allowed_tags);

	return rv;
}
std::string Parser::remaining_text() {
	if (text_stream_ == nullptr) {
		return "";
	}
	return text_stream_->remaining_text();
}
}  // namespace RT
