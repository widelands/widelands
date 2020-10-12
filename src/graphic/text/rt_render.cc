/*
 * Copyright (C) 2006-2020 by the Widelands Development Team
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

#include "graphic/text/rt_render.h"

#include <cstdlib>
#include <memory>
#include <queue>

#include <boost/algorithm/string.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/rect.h"
#include "base/vector.h"
#include "base/wexception.h"
#include "graphic/align.h"
#include "graphic/animation/animation.h"
#include "graphic/animation/animation_manager.h"
#include "graphic/graphic.h"
#include "graphic/image_cache.h"
#include "graphic/image_io.h"
#include "graphic/playercolor.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_io.h"
#include "graphic/text/font_set.h"
#include "graphic/text/rendered_text.h"
#include "graphic/text/rt_errors.h"
#include "graphic/text/rt_parse.h"
#include "graphic/text/sdl_ttf_font.h"
#include "graphic/text/textstream.h"
#include "graphic/texture.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"

namespace {
/**
 * This function replaces some HTML entities in strings, e.g. &nbsp;.
 * It is used by the renderer after the tags have been parsed.
 */
void replace_entities(std::string* text) {
	boost::replace_all(*text, "&gt;", ">");
	boost::replace_all(*text, "&lt;", "<");
	boost::replace_all(*text, "&nbsp;", " ");
	boost::replace_all(*text, "&amp;", "&");  // Must be performed last
}
}  // namespace

namespace RT {

static const uint16_t INFINITE_WIDTH = 65535;  // 2^16-1

// Helper Stuff
struct Borders {
	Borders() {
		left = top = right = bottom = 0;
	}
	uint8_t left, top, right, bottom;
};

/// How the width of a div should be calculated
enum class WidthUnit {
	kAbsolute,  // Width in pixels
	kPercent,   // Width in percent
	kShrink,    // Shrink width to content
	kFill       // Expand width to fill all remaining space
};

struct DesiredWidth {
	DesiredWidth(int init_width, WidthUnit init_unit) : width(init_width), unit(init_unit) {
	}
	DesiredWidth() : DesiredWidth(0, WidthUnit::kShrink) {
	}

	int width;
	WidthUnit unit;
};

struct NodeStyle {
	UI::FontSet const* fontset;
	std::string font_face;
	uint16_t font_size;
	RGBColor font_color;
	int font_style;

	uint8_t spacing;
	UI::Align halign;
	UI::Align valign;
	const bool is_rtl;
	std::string reference;
};

/*
 * This class makes sure that we only load each font file once.
 */
class FontCache {
public:
	FontCache() = default;
	~FontCache();

	IFont& get_font(NodeStyle* style);

private:
	struct FontDescr {
		std::string face;
		uint16_t size;

		bool operator<(const FontDescr& o) const {
			return size < o.size || (size == o.size && face < o.face);
		}
	};
	using FontMap = std::map<FontDescr, IFont*>;
	using FontMapPair = std::pair<const FontDescr, std::unique_ptr<IFont>>;

	FontMap fontmap_;

	DISALLOW_COPY_AND_ASSIGN(FontCache);
};

FontCache::~FontCache() {
	for (FontMap::reference& entry : fontmap_) {
		delete entry.second;
	}
}

IFont& FontCache::get_font(NodeStyle* ns) {
	if (ns->font_face == "condensed") {
		ns->font_face = ns->fontset->condensed();
	} else if (ns->font_face == "serif") {
		ns->font_face = ns->fontset->serif();
	} else if (ns->font_face == "sans") {
		ns->font_face = ns->fontset->sans();
	}
	const bool is_bold = ns->font_style & IFont::BOLD;
	const bool is_italic = ns->font_style & IFont::ITALIC;
	if (is_bold && is_italic) {
		if (ns->font_face == ns->fontset->condensed() ||
		    ns->font_face == ns->fontset->condensed_bold() ||
		    ns->font_face == ns->fontset->condensed_italic()) {
			ns->font_face = ns->fontset->condensed_bold_italic();
		} else if (ns->font_face == ns->fontset->serif() ||
		           ns->font_face == ns->fontset->serif_bold() ||
		           ns->font_face == ns->fontset->serif_italic()) {
			ns->font_face = ns->fontset->serif_bold_italic();
		} else {
			ns->font_face = ns->fontset->sans_bold_italic();
		}
		ns->font_style &= ~IFont::ITALIC;
		ns->font_style &= ~IFont::BOLD;
	} else if (is_bold) {
		if (ns->font_face == ns->fontset->condensed()) {
			ns->font_face = ns->fontset->condensed_bold();
		} else if (ns->font_face == ns->fontset->serif()) {
			ns->font_face = ns->fontset->serif_bold();
		} else {
			ns->font_face = ns->fontset->sans_bold();
		}
		ns->font_style &= ~IFont::BOLD;
	} else if (is_italic) {
		if (ns->font_face == ns->fontset->condensed()) {
			ns->font_face = ns->fontset->condensed_italic();
		} else if (ns->font_face == ns->fontset->serif()) {
			ns->font_face = ns->fontset->serif_italic();
		} else {
			ns->font_face = ns->fontset->sans_italic();
		}
		ns->font_style &= ~IFont::ITALIC;
	}

	uint16_t font_size = ns->font_size + ns->fontset->size_offset();

	FontDescr fd = {ns->font_face, font_size};
	FontMap::iterator i = fontmap_.find(fd);
	if (i != fontmap_.end()) {
		return *i->second;
	}

	std::unique_ptr<IFont> font;
	try {
		font.reset(load_font(ns->font_face, font_size));
	} catch (FileNotFoundError& e) {
		log_warn(
		   "Font file not found. Falling back to sans: %s\n%s\n", ns->font_face.c_str(), e.what());
		font.reset(load_font(ns->fontset->sans(), font_size));
	}
	assert(font != nullptr);

	return *fontmap_.insert(std::make_pair(fd, font.release())).first->second;
}

struct Reference {
	Recti dim;
	std::string ref;
};

class RenderNode {
public:
	enum class Floating {
		kNone,
		kRight,
		kLeft,
	};
	explicit RenderNode(const NodeStyle& ns)
	   : floating_(Floating::kNone), halign_(ns.halign), valign_(ns.valign), x_(0), y_(0) {
	}
	virtual ~RenderNode() {
	}

	virtual uint16_t width() const = 0;
	virtual uint16_t height() const = 0;
	virtual uint16_t hotspot_y() const = 0;
	virtual std::shared_ptr<UI::RenderedText> render(TextureCache* texture_cache) = 0;

	// TODO(GunChleoc): Remove this function once conversion is finished and well tested.
	virtual std::string debug_info() const = 0;

	// If a node is a non-mandatory space, it can be removed as a leading/trailing space
	// by the positioning algorithm.
	virtual bool is_non_mandatory_space() const {
		return false;
	}
	virtual bool is_expanding() const {
		return false;
	}
	virtual void set_w(uint16_t) {
	}  // Only, when is_expanding

	virtual const std::vector<Reference> get_references() {
		return std::vector<Reference>();
	}

	Floating get_floating() const {
		return floating_;
	}
	void set_floating(Floating f) {
		floating_ = f;
	}
	UI::Align halign() const {
		return halign_;
	}
	UI::Align valign() const {
		return valign_;
	}
	void set_valign(UI::Align gvalign) {
		valign_ = gvalign;
	}

	// True for nodes that need to stay aligned to the text baseline.
	// False for the rest.
	virtual bool align_to_baseline() const {
		return false;
	}

	void set_x(int32_t nx) {
		x_ = nx;
	}
	void set_y(int32_t ny) {
		y_ = ny;
	}
	int32_t x() const {
		return x_;
	}
	int32_t y() const {
		return y_;
	}

protected:
	/// Throws a TextureTooBig exception if the given dimensions would be bigger than the graphics
	/// can handle
	void check_size(int check_w, int check_h) {
		const int maximum_size = g_gr->max_texture_size_for_font_rendering();
		if (check_w > maximum_size || check_h > maximum_size) {
			const std::string error_message =
			   (boost::format("Texture (%d, %d) too big! Maximum size is %d.") % check_w % check_h %
			    maximum_size)
			      .str();
			log_err("%s\n", error_message.c_str());
			throw TextureTooBig(error_message);
		}
	}

	/// Check the size for the node's own dimensions
	void check_size() {
		check_size(width(), height());
	}

private:
	Floating floating_;
	UI::Align halign_;
	UI::Align valign_;
	int32_t x_, y_;
};

/*
 * Class to calculate positions of nodes within a div tag.
 */
class Layout {
public:
	explicit Layout(std::vector<std::shared_ptr<RenderNode>>& all)
	   : h_(0), idx_(0), all_nodes_(all) {
	}
	virtual ~Layout() {
	}

	uint16_t height() {
		return h_;
	}
	uint16_t
	fit_nodes(std::vector<std::shared_ptr<RenderNode>>* rv, uint16_t w, Borders p, bool trim_spaces);

private:
	bool calculate_line_width(uint16_t* x, uint16_t* w, uint16_t lineheight);
	uint16_t fit_line(uint16_t w,
	                  const Borders&,
	                  std::vector<std::shared_ptr<RenderNode>>* rv,
	                  bool trim_spaces);

	uint16_t h_;
	size_t idx_;
	std::vector<std::shared_ptr<RenderNode>>& all_nodes_;
	std::queue<std::shared_ptr<RenderNode>> floats_;
};

/**
 * Calculate the width of a line at h_ taking into account floating elements.
 * @param x The first useable x position in the line. Has to be set by the caller,
 *          might be modified inside this function.
 * @param w The useable width of the line. Has to be set by the caller,
 *          might be modified inside this function.
 * @param lineheight The height of the line the maximum width should be calculated of.
 * @return Whether less than the full width can be used (i.e., there is a float).
 */
bool Layout::calculate_line_width(uint16_t* x, uint16_t* w, uint16_t lineheight) {
	// Drop elements we already passed
	while (!floats_.empty() && h_ >= floats_.front()->y() + floats_.front()->height()) {
		floats_.pop();
	}
	if (floats_.empty()) {
		return false;
	}
	// Check whether there is an element at the current height
	std::shared_ptr<RenderNode> n = floats_.front();
	if (h_ + lineheight < n->y()) {
		// Nope, nothing in the current line
		// Since the elements are ordered, no further element can match
		return false;
	}

	if (n->get_floating() == RenderNode::Floating::kLeft) {
		*x += n->width();
	} else {
		assert(n->get_floating() == RenderNode::Floating::kRight);
		*w -= n->width();
	}
	return true;
}

/*
 * Calculate x positions of nodes in one line and return them in rv.
 * As many nodes of all_nodes_ are added to the line as there is space.
 * Remove leading/trailing spaces and assign x positions to all elements.
 * Use remaining space to distribute expanding elements a bit further.
 * Returns hotspot of the line.
 * Method is called from within Layout::fit_nodes().
 */
uint16_t Layout::fit_line(const uint16_t w_max,  // Maximum width of line
                          const Borders& p,      // Left/right border. Is left empty
                          std::vector<std::shared_ptr<RenderNode>>* rv,  // Output: Nodes to render
                          bool trim_spaces) {  // Whether leading/trailing space should be removed
	assert(rv->empty());

	// Remove leading spaces
	while (idx_ < all_nodes_.size() && all_nodes_[idx_]->is_non_mandatory_space() && trim_spaces) {
		all_nodes_[idx_++].reset();
	}

	uint16_t w;
	uint16_t x;
	std::size_t first_idx = idx_;
	uint16_t lineheight = 0;

	// Pass 1: Run through all nodes who *might* end up in this line and check for floats
	w = w_max - p.right;
	x = p.left;
	bool width_was_reduced = calculate_line_width(&x, &w, lineheight);
	lineheight = 0;
	uint16_t w_used = 0;
	for (; idx_ < all_nodes_.size(); ++idx_) {
		if (w_used + all_nodes_[idx_]->width() > w) {
			// Line is full
			break;
		}
		if (all_nodes_[idx_]->get_floating() == RenderNode::Floating::kNone) {
			// Normal, non-floating node
			w_used += all_nodes_[idx_]->width();
			assert(all_nodes_[idx_]->height() >= all_nodes_[idx_]->hotspot_y());
			lineheight = std::max(lineheight, all_nodes_[idx_]->height());
			continue;
		}
		// Found a float. Add it to list
		// New float start directly below lowest flow in list
		if (!floats_.empty()) {
			all_nodes_[idx_]->set_y(floats_.back()->y() + floats_.back()->height());
		} else {
			all_nodes_[idx_]->set_y(h_);
		}
		// Set x position of the float based on its desired orientation
		if (all_nodes_[idx_]->get_floating() == RenderNode::Floating::kLeft) {
			all_nodes_[idx_]->set_x(p.left);
		} else {
			assert(all_nodes_[idx_]->get_floating() == RenderNode::Floating::kRight);
			all_nodes_[idx_]->set_x(w_max - all_nodes_[idx_]->width() - p.right);
		}
		floats_.push(all_nodes_[idx_]);
		// When the line width hasn't been reduced by a float yet, do so now.
		// If it already has been reduced than the new float will be placed somewhere below
		// the current line so no need to adapt the line width
		if (!width_was_reduced) {
			// Don't need to reset x and w since they haven't been modified on last call
			width_was_reduced = calculate_line_width(&x, &w, lineheight);
			assert(width_was_reduced);
		}
	}

	idx_ = first_idx;
	// w and x now contain the width of the line and the x position of the first element in it

	// Calc fitting nodes
	while (idx_ < all_nodes_.size()) {
		std::shared_ptr<RenderNode> n = all_nodes_[idx_];
		if (n->get_floating() != RenderNode::Floating::kNone) {
			// Don't handle floaters here
			rv->push_back(n);
			if (idx_ == first_idx) {
				first_idx++;
			}
			++idx_;
			continue;
		}
		uint16_t nw = n->width();
		// Check whether the element is too big for the current line
		// (position + width-of-element + border) > width-of-line
		if (x + nw + p.right > w) {
			// Its too big
			if (idx_ == first_idx) {
				// If it is the first element in the line, add it anyway and pretend that it matches
				// exactly
				nw = w - p.right - x;
			} else {
				// Too wide and not the first element: We are done with the line
				break;
			}
		}
		n->set_x(x);
		x += nw;
		rv->push_back(n);
		++idx_;
	}
	// Remove trailing spaces
	while (!rv->empty() && rv->back()->is_non_mandatory_space() && trim_spaces) {
		x -= rv->back()->width();
		rv->pop_back();
	}

	// Remaining space in this line
	uint16_t remaining_space = 0;
	if (w < INFINITE_WIDTH) {
		remaining_space = w - p.right - x;
	}

	// Find expanding nodes
	std::vector<size_t> expanding_nodes;
	for (size_t idx = 0; idx < rv->size(); ++idx) {
		if (rv->at(idx)->is_expanding()) {
			expanding_nodes.push_back(idx);
		}
	}

	if (!expanding_nodes.empty()) {  // If there are expanding nodes, we fill the space
		const uint16_t individual_w = remaining_space / expanding_nodes.size();
		for (const size_t idx : expanding_nodes) {
			rv->at(idx)->set_w(individual_w);
			for (size_t nidx = idx + 1; nidx < rv->size(); ++nidx) {
				rv->at(nidx)->set_x(rv->at(nidx)->x() + individual_w);
			}
		}
	} else {
		// Take last elements style in this line and check horizontal alignment
		if (!rv->empty() && rv->back()->halign() != UI::Align::kLeft) {
			if (rv->back()->halign() == UI::Align::kCenter) {
				remaining_space /= 2;  // Otherwise, we align right
			}
			for (const auto& node : *rv) {
				node->set_x(node->x() + remaining_space);
			}
		}
	}

	// Find the biggest hotspot of the truly remaining non-floating items.
	uint16_t cur_line_hotspot = 0;
	for (const auto& node : *rv) {
		if (node->get_floating() != RenderNode::Floating::kNone) {
			continue;
		}
		if (node->align_to_baseline()) {
			cur_line_hotspot = std::max(cur_line_hotspot, node->hotspot_y());
		}
	}
	return cur_line_hotspot;
}

/*
 * Take ownership of all nodes, delete those that we do not render anyways (for
 * example unneeded spaces), append the rest to the vector we got.
 * Also, calculate positions for all nodes based on the given width w
 * and the widths of the nodes.
 * Method is called from within DivTagHandler::enter().
 */
uint16_t Layout::fit_nodes(std::vector<std::shared_ptr<RenderNode>>* rv,
                           uint16_t w,
                           Borders p,
                           bool trim_spaces) {
	assert(rv->empty());
	h_ = p.top;

	uint16_t max_line_width = 0;
	while (idx_ < all_nodes_.size()) {
		std::vector<std::shared_ptr<RenderNode>> nodes_in_line;
		size_t idx_before_iteration_ = idx_;
		uint16_t biggest_hotspot = fit_line(w, p, &nodes_in_line, trim_spaces);

		int line_height = 0;
		int biggest_descent = 0;
		int line_start = INFINITE_WIDTH;

		// Compute real line height and width, taking into account alignment
		for (const auto& n : nodes_in_line) {
			if (n->get_floating() == RenderNode::Floating::kNone) {
				if (n->align_to_baseline()) {
					biggest_descent = std::max(biggest_descent, n->height() - n->hotspot_y());
					line_height = std::max(line_height, biggest_hotspot - n->hotspot_y() + n->height());
				} else {
					line_height = std::max(line_height, static_cast<int>(n->height()));
				}
			}

			if (line_start >= INFINITE_WIDTH || n->x() < line_start) {
				line_start = n->x() - p.left;
			}
			max_line_width = std::max<int>(max_line_width, n->x() + n->width() + p.right - line_start);
		}

		// Go over again and set vertical positions of nodes depending on VALIGN
		for (const auto& n : nodes_in_line) {
			if (n->get_floating() != RenderNode::Floating::kNone) {
				continue;
			}

			// Empty space: how much the node can be moved within line boundaries
			int space;
			int baseline_correction;
			if (n->align_to_baseline()) {
				// Text nodes: Treat them as a group. Calculate the space using the same values
				// for all nodes and correct for hotspot differences to align them to one common
				// baseline.
				space = line_height - biggest_hotspot - biggest_descent;
				baseline_correction = biggest_hotspot - n->hotspot_y();
			} else {
				// Non-text nodes: Align each node independently to the top/bottom/center of
				// the line.
				space = line_height - n->height();
				baseline_correction = 0;
			}

			if (n->valign() == UI::Align::kTop) {
				space = 0;
			} else if (n->valign() == UI::Align::kCenter) {
				space /= 2;
			}
			n->set_y(h_ + space + baseline_correction);
		}
		rv->insert(rv->end(), nodes_in_line.begin(), nodes_in_line.end());

		h_ += line_height;

		if (idx_ == idx_before_iteration_) {
			throw WidthTooSmall(
			   "Could not fit a single render node in line. Width of an Element is too small!");
		}
	}

	if (!floats_.empty()) {
		// If there is a float left this means the floats go down further than the text.
		// If this is the case, reset the height of the div
		h_ = std::max<uint16_t>(h_, floats_.back()->y() + floats_.back()->height());
	}
	h_ += p.bottom;
	return max_line_width;
}

/*
 * A word in the text.
 */
class TextNode : public RenderNode {
public:
	TextNode(FontCache& font, NodeStyle&, const std::string& txt);
	~TextNode() override {
	}

	std::string debug_info() const override {
		return "'" + txt_ + "'";
	}

	uint16_t width() const override {
		return w_;
	}
	uint16_t height() const override {
		return h_ + nodestyle_.spacing;
	}
	bool align_to_baseline() const override {
		return true;
	}
	uint16_t hotspot_y() const override;
	const std::vector<Reference> get_references() override {
		std::vector<Reference> rv;
		if (!nodestyle_.reference.empty()) {
			Reference r = {Recti(0, 0, w_, h_), nodestyle_.reference};
			rv.push_back(r);
		}
		return rv;
	}

	std::shared_ptr<UI::RenderedText> render(TextureCache* texture_cache) override;

protected:
	uint16_t w_, h_;
	const std::string txt_;
	NodeStyle nodestyle_;
	FontCache& fontcache_;
	SdlTtfFont& font_;
};

TextNode::TextNode(FontCache& font, NodeStyle& ns, const std::string& txt)
   : RenderNode(ns),
     txt_(txt),
     nodestyle_(ns),
     fontcache_(font),
     font_(dynamic_cast<SdlTtfFont&>(fontcache_.get_font(&nodestyle_))) {
	font_.dimensions(txt_, ns.font_style, &w_, &h_);
	check_size();
}
uint16_t TextNode::hotspot_y() const {
	// Getting the real ascent of a string from SDL_ttf is tricky.
	// It's equal to TTF_FontAscent() or the maximum 'maxy' value of any glyph in the string,
	// whichever is bigger.
	const icu::UnicodeString unicode_txt(txt_.c_str(), "UTF-8");
	int ascent = TTF_FontAscent(font_.get_ttf_font());
	int shadow_offset = font_.ascent(nodestyle_.font_style) - ascent;

	for (int i = 0; i < unicode_txt.length(); ++i) {
		// TODO(Niektory): Use the 32-bit functions when we can use SDL_ttf 2.0.16
		// (UChar32, char32At, TTF_GlyphIsProvided32, TTF_GlyphMetrics32)
		UChar codepoint = unicode_txt.charAt(i);
		int maxy;
		if (TTF_GlyphIsProvided(font_.get_ttf_font(), codepoint) &&
		    TTF_GlyphMetrics(
		       font_.get_ttf_font(), codepoint, nullptr, nullptr, nullptr, &maxy, nullptr) == 0) {
			ascent = std::max(ascent, maxy);
		}
	}

	return ascent + shadow_offset;
}

std::shared_ptr<UI::RenderedText> TextNode::render(TextureCache* texture_cache) {
	auto rendered_image =
	   font_.render(txt_, nodestyle_.font_color, nodestyle_.font_style, texture_cache);
	assert(rendered_image != nullptr);
	std::shared_ptr<UI::RenderedText> rendered_text(new UI::RenderedText());
	rendered_text->rects.push_back(
	   std::unique_ptr<UI::RenderedRect>(new UI::RenderedRect(rendered_image)));
	return rendered_text;
}

/*
 * Text that might need to expand to fill the space between other elements. One
 * example are ... in a table like construction.
 */
class FillingTextNode : public TextNode {
public:
	FillingTextNode(
	   FontCache& font, NodeStyle& ns, uint16_t w, const std::string& txt, bool expanding = false)
	   : TextNode(font, ns, txt), is_expanding_(expanding) {
		w_ = w;
		check_size();
	}
	~FillingTextNode() override {
	}

	std::string debug_info() const override {
		return "ft";
	}

	std::shared_ptr<UI::RenderedText> render(TextureCache*) override;

	bool is_expanding() const override {
		return is_expanding_;
	}
	void set_w(uint16_t w) override {
		w_ = w;
	}

private:
	bool is_expanding_;
};
std::shared_ptr<UI::RenderedText> FillingTextNode::render(TextureCache* texture_cache) {
	std::shared_ptr<UI::RenderedText> rendered_text(new UI::RenderedText());
	const std::string hash =
	   (boost::format("rt:fill:%s:%s:%i:%i:%i:%s") % txt_ % nodestyle_.font_color.hex_value() %
	    nodestyle_.font_style % width() % height() % (is_expanding_ ? "e" : "f"))
	      .str();

	std::shared_ptr<const Image> rendered_image = texture_cache->get(hash);
	if (rendered_image == nullptr) {
		std::shared_ptr<const Image> ttf =
		   font_.render(txt_, nodestyle_.font_color, nodestyle_.font_style, texture_cache);
		auto texture = std::make_shared<Texture>(width(), height());
		for (uint16_t curx = 0; curx < w_; curx += ttf->width()) {
			Rectf srcrect(0.f, 0.f, std::min<int>(ttf->width(), w_ - curx), h_);
			texture->blit(
			   Rectf(curx, 0, srcrect.w, srcrect.h), *ttf.get(), srcrect, 1., BlendMode::Copy);
		}
		rendered_image = texture_cache->insert(hash, std::move(texture));
	}
	assert(rendered_image != nullptr);
	rendered_text->rects.push_back(
	   std::unique_ptr<UI::RenderedRect>(new UI::RenderedRect(rendered_image)));
	return rendered_text;
}

/*
 * The whitespace between two words. There is a debug option to make it red and therefore
 * visible.
 */
class WordSpacerNode : public TextNode {
public:
	WordSpacerNode(FontCache& font, NodeStyle& ns) : TextNode(font, ns, " ") {
		check_size();
	}
	static void show_spaces(bool t) {
		show_spaces_ = t;
	}

	std::string debug_info() const override {
		return "wsp";
	}

	std::shared_ptr<UI::RenderedText> render(TextureCache* texture_cache) override {
		if (show_spaces_) {
			std::shared_ptr<UI::RenderedText> rendered_text(new UI::RenderedText());
			const std::string hash = (boost::format("rt:wsp:%i:%i") % width() % height()).str();
			std::shared_ptr<const Image> rendered_image = texture_cache->get(hash);
			if (rendered_image == nullptr) {
				auto texture = std::make_shared<Texture>(width(), height());
				texture->fill_rect(Rectf(0.f, 0.f, w_, h_), RGBAColor(0xcc, 0, 0, 0xcc));
				rendered_image = texture_cache->insert(hash, std::move(texture));
			}
			assert(rendered_image != nullptr);
			rendered_text->rects.push_back(
			   std::unique_ptr<UI::RenderedRect>(new UI::RenderedRect(rendered_image)));
			return rendered_text;
		}
		return TextNode::render(texture_cache);
	}
	bool is_non_mandatory_space() const override {
		return true;
	}

private:
	static bool show_spaces_;
};
bool WordSpacerNode::show_spaces_;

/*
 * This is a forced newline that can either be inside the text from the user or
 * is inserted by Layout whenever appropriate.
 */
class NewlineNode : public RenderNode {
public:
	explicit NewlineNode(NodeStyle& ns) : RenderNode(ns) {
	}

	std::string debug_info() const override {
		return "nl";
	}

	uint16_t height() const override {
		return 0;
	}
	uint16_t width() const override {
		return INFINITE_WIDTH;
	}
	uint16_t hotspot_y() const override {
		return 0;
	}
	std::shared_ptr<UI::RenderedText> render(TextureCache* /* texture_cache */) override {
		return std::shared_ptr<UI::RenderedText>(new UI::RenderedText());
	}
	bool is_non_mandatory_space() const override {
		return true;
	}
};

/*
 * Arbitrary whitespace or a tiled image.
 */
class SpaceNode : public RenderNode {
public:
	SpaceNode(NodeStyle& ns, uint16_t w, uint16_t h = 0, bool expanding = false)
	   : RenderNode(ns), w_(w), h_(h), background_image_(nullptr), is_expanding_(expanding) {
		check_size();
	}

	std::string debug_info() const override {
		return "sp";
	}

	uint16_t height() const override {
		return h_;
	}
	uint16_t width() const override {
		return w_;
	}
	uint16_t hotspot_y() const override {
		return h_;
	}
	std::shared_ptr<UI::RenderedText> render(TextureCache* texture_cache) override {
		std::shared_ptr<UI::RenderedText> rendered_text(new UI::RenderedText());
		const std::string hash = (boost::format("rt:sp:%s:%i:%i:%s") % filename_ % width() %
		                          height() % (is_expanding_ ? "e" : "f"))
		                            .str();

		std::shared_ptr<const Image> rendered_image = texture_cache->get(hash);
		if (rendered_image == nullptr) {
			// Draw background image (tiling)
			auto texture = std::make_shared<Texture>(width(), height());
			if (background_image_ != nullptr) {
				Rectf dst;
				Rectf srcrect(0, 0, 1, 1);
				for (uint16_t curx = 0; curx < w_; curx += background_image_->width()) {
					dst.x = curx;
					dst.y = 0;
					srcrect.w = dst.w = std::min<int>(background_image_->width(), w_ - curx);
					srcrect.h = dst.h = h_;
					texture->blit(dst, *background_image_, srcrect, 1., BlendMode::Copy);
				}
			} else {
				texture->fill_rect(Rectf(0.f, 0.f, w_, h_), RGBAColor(255, 255, 255, 0));
			}
			rendered_image = texture_cache->insert(hash, std::move(texture));
		}
		assert(rendered_image != nullptr);
		rendered_text->rects.push_back(
		   std::unique_ptr<UI::RenderedRect>(new UI::RenderedRect(rendered_image)));
		return rendered_text;
	}

	bool is_expanding() const override {
		return is_expanding_;
	}
	void set_w(uint16_t w) override {
		w_ = w;
	}

	void set_background(const Image* s, const std::string& filename) {
		background_image_ = s;
		filename_ = filename;
		h_ = s->height();
	}

private:
	uint16_t w_, h_;
	const Image* background_image_;  // not owned
	std::string filename_;
	bool is_expanding_;
};

/*
 * This is a div tag node. It is also the same as a full rich text render node.
 */
class DivTagRenderNode : public RenderNode {
public:
	explicit DivTagRenderNode(const NodeStyle& ns)
	   : RenderNode(ns),
	     desired_width_(),
	     w_(0),
	     h_(0),
	     background_color_(0, 0, 0),
	     is_background_color_set_(false),
	     background_image_(nullptr) {
	}
	~DivTagRenderNode() override {
		nodes_to_render_.clear();
	}

	std::string debug_info() const override {
		return "div";
	}

	uint16_t width() const override {
		return w_ + margin_.left + margin_.right;
	}
	uint16_t height() const override {
		return h_ + margin_.top + margin_.bottom;
	}
	uint16_t hotspot_y() const override {
		return height();
	}

	DesiredWidth desired_width() const {
		return desired_width_;
	}

	std::shared_ptr<UI::RenderedText> render(TextureCache* texture_cache) override {
		std::shared_ptr<UI::RenderedText> rendered_text(new UI::RenderedText());
		// Preserve padding
		rendered_text->rects.push_back(std::unique_ptr<UI::RenderedRect>(
		   new UI::RenderedRect(Recti(0, 0, width(), height()), nullptr)));

		// Draw Solid background Color
		if (is_background_color_set_) {
			// TODO(Notabilis): I think margin_.right and .bottom are missing in next line
			UI::RenderedRect* bg_rect =
			   new UI::RenderedRect(Recti(margin_.left, margin_.top, w_, h_), background_color_);
			// Size is automatically adjusted in RenderedText while blitting, so no need to call
			// check_size() here.
			rendered_text->rects.push_back(std::unique_ptr<UI::RenderedRect>(bg_rect));
		}

		// Draw background image (tiling)
		if (background_image_ != nullptr) {
			UI::RenderedRect* bg_rect =
			   new UI::RenderedRect(Recti(margin_.left, margin_.top, w_, h_), background_image_);
			check_size(bg_rect->width(), bg_rect->height());
			rendered_text->rects.push_back(std::unique_ptr<UI::RenderedRect>(bg_rect));
		}

		for (const auto& n : nodes_to_render_) {
			const auto& renderme = n->render(texture_cache);
			for (auto& rendered_rect : renderme->rects) {
				if (rendered_rect->was_visited()) {
					rendered_rect->set_origin(
					   Vector2i(x() + rendered_rect->x(), y() + rendered_rect->y() + margin_.top));

				} else {
					rendered_rect->set_origin(
					   Vector2i(x() + n->x() + margin_.left, y() + n->y() + margin_.top));
					rendered_rect->set_visited();
				}
				rendered_text->rects.push_back(std::move(rendered_rect));
			}
		}
		nodes_to_render_.clear();

		return rendered_text;
	}
	const std::vector<Reference> get_references() override {
		return refs_;
	}
	void set_dimensions(uint16_t inner_w, uint16_t inner_h, Borders margin) {
		w_ = inner_w;
		h_ = inner_h;
		margin_ = margin;
	}
	void set_desired_width(DesiredWidth input_width) {
		desired_width_ = input_width;
	}
	void set_background(RGBColor clr) {
		background_color_ = clr;
		is_background_color_set_ = true;
	}
	void set_background(const Image* img) {
		background_image_ = img;
	}
	void set_nodes_to_render(const std::vector<std::shared_ptr<RenderNode>>& n) {
		nodes_to_render_ = n;
	}
	void add_reference(int16_t gx, int16_t gy, uint16_t w, uint16_t h, const std::string& s) {
		Reference r = {Recti(gx, gy, w, h), s};
		refs_.push_back(r);
	}

private:
	DesiredWidth desired_width_;
	uint16_t w_, h_;
	std::vector<std::shared_ptr<RenderNode>> nodes_to_render_;
	Borders margin_;
	RGBColor background_color_;
	bool is_background_color_set_;
	const Image* background_image_;  // Not owned.
	std::vector<Reference> refs_;
};

class ImgRenderNode : public RenderNode {
public:
	ImgRenderNode(NodeStyle& ns,
	              const std::string& image_filename,
	              double scale,
	              const RGBColor& color,
	              bool use_playercolor)
	   : RenderNode(ns),
	     image_(use_playercolor ? playercolor_image(color, image_filename) :
	                              g_image_cache->get(image_filename)),
	     filename_(image_filename),
	     scale_(scale),
	     color_(color),
	     use_playercolor_(use_playercolor) {
		check_size();
	}

	ImgRenderNode(NodeStyle& ns, const Image* image)
	   : RenderNode(ns),
	     image_(image),
	     filename_(""),
	     scale_(1.0),
	     color_(RGBColor(0, 0, 0)),
	     use_playercolor_(false) {
		check_size();
	}

	std::string debug_info() const override {
		return "img";
	}

	uint16_t width() const override {
		return scale_ * image_->width();
	}
	uint16_t height() const override {
		return scale_ * image_->height();
	}
	uint16_t hotspot_y() const override {
		return scale_ * image_->height();
	}
	std::shared_ptr<UI::RenderedText> render(TextureCache* texture_cache) override;

private:
	const Image* image_;
	const std::string filename_;
	const double scale_;
	const RGBColor color_;
	bool use_playercolor_;
};

std::shared_ptr<UI::RenderedText> ImgRenderNode::render(TextureCache* texture_cache) {
	std::shared_ptr<UI::RenderedText> rendered_text(new UI::RenderedText());

	if (scale_ == 1.0 || filename_.empty()) {
		// Image can be used as is, and has already been cached in g_image_cache
		assert(image_ != nullptr);
		rendered_text->rects.push_back(
		   std::unique_ptr<UI::RenderedRect>(new UI::RenderedRect(image_)));
	} else {
		const std::string hash = (boost::format("rt:img:%s:%s:%i:%i") % filename_ %
		                          (use_playercolor_ ? color_.hex_value() : "") % width() % height())
		                            .str();
		std::shared_ptr<const Image> rendered_image = texture_cache->get(hash);
		if (rendered_image == nullptr) {
			auto texture = std::make_shared<Texture>(width(), height());
			texture->blit(Rectf(0.f, 0.f, width(), height()), *image_,
			              Rectf(0.f, 0.f, image_->width(), image_->height()), 1., BlendMode::Copy);
			rendered_image = texture_cache->insert(hash, std::move(texture));
		}

		assert(rendered_image != nullptr);
		rendered_text->rects.push_back(
		   std::unique_ptr<UI::RenderedRect>(new UI::RenderedRect(rendered_image)));
	}

	return rendered_text;
}
// End: Helper Stuff

class TagHandler;
TagHandler* create_taghandler(Tag& tag,
                              FontCache& fc,
                              NodeStyle& ns,
                              ImageCache* image_cache,
                              RendererStyle& renderer_style,
                              const UI::FontSets* fontsets);

class TagHandler {
public:
	TagHandler(Tag& tag,
	           FontCache& fc,
	           NodeStyle ns,
	           ImageCache* image_cache,
	           RendererStyle& renderer_style,
	           const UI::FontSets* fontsets)
	   : tag_(tag),
	     font_cache_(fc),
	     nodestyle_(std::move(ns)),
	     image_cache_(image_cache),
	     renderer_style_(renderer_style),
	     fontsets_(fontsets) {
	}
	virtual ~TagHandler() {
	}

	virtual void enter() {
	}
	virtual void emit_nodes(std::vector<std::shared_ptr<RenderNode>>&);

private:
	void make_text_nodes(const std::string& txt,
	                     std::vector<std::shared_ptr<RenderNode>>& nodes,
	                     NodeStyle& ns);

protected:
	Tag& tag_;
	FontCache& font_cache_;
	NodeStyle nodestyle_;
	ImageCache* image_cache_;        // Not owned
	RendererStyle& renderer_style_;  // Reference to global renderer style in the renderer
	const UI::FontSets* fontsets_;
};

void TagHandler::make_text_nodes(const std::string& txt,
                                 std::vector<std::shared_ptr<RenderNode>>& nodes,
                                 NodeStyle& ns) {
	TextStream ts(txt);
	std::string word;
	std::vector<std::shared_ptr<RenderNode>> text_nodes;

	// Bidirectional text (Arabic etc.)
	if (i18n::has_rtl_character(txt.c_str())) {
		std::string previous_word;
		std::vector<std::shared_ptr<RenderNode>>::iterator it = text_nodes.begin();
		std::vector<std::shared_ptr<RenderNode>> spacer_nodes;

		// Collect the word nodes
		while (ts.pos() < txt.size()) {
			std::size_t cpos = ts.pos();
			ts.skip_ws();
			spacer_nodes.clear();

			// We only know if the spacer goes to the left or right after having a look at the current
			// word.
			for (uint16_t ws_indx = 0; ws_indx < ts.pos() - cpos; ws_indx++) {
				spacer_nodes.push_back(
				   std::shared_ptr<RenderNode>(new WordSpacerNode(font_cache_, ns)));
			}

			word = ts.till_any_or_end(" \t\n\r");
			ns.fontset = i18n::find_fontset(word.c_str(), *fontsets_);
			if (!word.empty()) {
				replace_entities(&word);
				bool word_is_bidi = i18n::has_rtl_character(word.c_str());
				word = i18n::make_ligatures(word.c_str());
				if (word_is_bidi || i18n::has_rtl_character(previous_word.c_str())) {
					for (const auto& spacer : spacer_nodes) {
						text_nodes.insert(text_nodes.begin(), spacer);
					}
					if (word_is_bidi) {
						word = i18n::line2bidi(word.c_str());
					}
					it = text_nodes.insert(text_nodes.begin(), std::shared_ptr<RenderNode>(new TextNode(
					                                              font_cache_, ns, word.c_str())));
				} else {  // Sequences of Latin words go to the right from current position
					if (it < text_nodes.end()) {
						++it;
					}
					for (const auto& spacer : spacer_nodes) {
						it = text_nodes.insert(it, spacer);
						if (it < text_nodes.end()) {
							++it;
						}
					}
					it = text_nodes.insert(
					   it, std::shared_ptr<RenderNode>(new TextNode(font_cache_, ns, word)));
				}
			}
			previous_word = word;
		}
		// Add the nodes to the end of the previously existing nodes.
		for (const auto& node : text_nodes) {
			nodes.push_back(node);
		}

	} else {  // LTR
		while (ts.pos() < txt.size()) {
			std::size_t cpos = ts.pos();
			ts.skip_ws();
			for (uint16_t ws_indx = 0; ws_indx < ts.pos() - cpos; ws_indx++) {
				nodes.push_back(std::shared_ptr<RenderNode>(new WordSpacerNode(font_cache_, ns)));
			}
			word = ts.till_any_or_end(" \t\n\r");
			ns.fontset = i18n::find_fontset(word.c_str(), *fontsets_);
			if (!word.empty()) {
				replace_entities(&word);
				word = i18n::make_ligatures(word.c_str());
				if (i18n::has_script_character(word.c_str(), UI::FontSets::Selector::kCJK)) {
					std::vector<std::string> units = i18n::split_cjk_word(word.c_str());
					for (const std::string& unit : units) {
						nodes.push_back(std::shared_ptr<RenderNode>(new TextNode(font_cache_, ns, unit)));
					}
				} else {
					nodes.push_back(std::shared_ptr<RenderNode>(new TextNode(font_cache_, ns, word)));
				}
			}
		}
	}
}

void TagHandler::emit_nodes(std::vector<std::shared_ptr<RenderNode>>& nodes) {
	for (Child* c : tag_.children()) {
		if (c->tag) {
			std::unique_ptr<TagHandler> th(create_taghandler(
			   *c->tag, font_cache_, nodestyle_, image_cache_, renderer_style_, fontsets_));
			th->enter();
			th->emit_nodes(nodes);
		} else {
			make_text_nodes(c->text, nodes, nodestyle_);
		}
	}
}

class FontTagHandler : public TagHandler {
public:
	FontTagHandler(Tag& tag,
	               FontCache& fc,
	               const NodeStyle& ns,
	               ImageCache* image_cache,
	               RendererStyle& init_renderer_style,
	               const UI::FontSets* fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();
		if (a.has("color")) {
			nodestyle_.font_color = a["color"].get_color();
		}
		if (a.has("size")) {
			nodestyle_.font_size = a["size"].get_int(std::numeric_limits<uint16_t>::max());
		}
		if (a.has("face")) {
			nodestyle_.font_face = a["face"].get_string();
		}
		if (a.has("bold")) {
			nodestyle_.font_style |= a["bold"].get_bool() ? IFont::BOLD : 0;
		}
		if (a.has("italic")) {
			nodestyle_.font_style |= a["italic"].get_bool() ? IFont::ITALIC : 0;
		}
		if (a.has("underline")) {
			nodestyle_.font_style |= a["underline"].get_bool() ? IFont::UNDERLINE : 0;
		}
		if (a.has("shadow")) {
			nodestyle_.font_style |= a["shadow"].get_bool() ? IFont::SHADOW : 0;
		}
		if (a.has("ref")) {
			nodestyle_.reference = a["ref"].get_string();
		}
	}
};

class PTagHandler : public TagHandler {
public:
	PTagHandler(Tag& tag,
	            FontCache& fc,
	            const NodeStyle& ns,
	            ImageCache* image_cache,
	            RendererStyle& init_renderer_style,
	            const UI::FontSets* fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets), indent_(0) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();
		if (a.has("indent")) {
			indent_ = a["indent"].get_int(std::numeric_limits<uint16_t>::max());
		}
		if (a.has("align")) {
			const std::string align = a["align"].get_string();
			if (align == "right") {
				nodestyle_.halign = UI::Align::kRight;
			} else if (align == "center" || align == "middle") {
				nodestyle_.halign = UI::Align::kCenter;
			} else {
				nodestyle_.halign = UI::Align::kLeft;
			}
		}
		nodestyle_.halign = mirror_alignment(nodestyle_.halign, nodestyle_.is_rtl);
		if (a.has("valign")) {
			const std::string align = a["valign"].get_string();
			if (align == "bottom") {
				nodestyle_.valign = UI::Align::kBottom;
			} else if (align == "center" || align == "middle") {
				nodestyle_.valign = UI::Align::kCenter;
			} else {
				nodestyle_.valign = UI::Align::kTop;
			}
		}
		if (a.has("spacing")) {
			nodestyle_.spacing = a["spacing"].get_int(std::numeric_limits<uint8_t>::max());
		}
	}
	void emit_nodes(std::vector<std::shared_ptr<RenderNode>>& nodes) override {
		// Put a newline if this is not the first paragraph
		if (!nodes.empty()) {
			nodes.push_back(std::shared_ptr<RenderNode>(new NewlineNode(nodestyle_)));
		}
		if (indent_) {
			nodes.push_back(std::shared_ptr<RenderNode>(new SpaceNode(nodestyle_, indent_)));
		}
		TagHandler::emit_nodes(nodes);
	}

private:
	uint16_t indent_;
};

class ImgTagHandler : public TagHandler {
public:
	ImgTagHandler(Tag& tag,
	              FontCache& fc,
	              const NodeStyle& ns,
	              ImageCache* image_cache,
	              RendererStyle& init_renderer_style,
	              const UI::FontSets* fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets), render_node_(nullptr) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();
		RGBColor color;
		bool use_playercolor = false;

		if (a.has("color")) {
			color = a["color"].get_color();
			use_playercolor = true;
		}
		if (a.has("object")) {
			const Image* representative_image = g_animation_manager->get_representative_image(
			   a["object"].get_string(), use_playercolor ? &color : nullptr);
			render_node_.reset(new ImgRenderNode(nodestyle_, representative_image));
		} else {
			const std::string image_filename = a["src"].get_string();
			double scale = 1.0;

			if (a.has("width")) {
				int width = a["width"].get_int(std::numeric_limits<uint16_t>::max());
				if (width > renderer_style_.overall_width) {
					log_warn("Font renderer: Specified image width of %d exceeds the overall "
					         "available "
					         "width of %d. Setting width to %d.\n",
					         width, renderer_style_.overall_width, renderer_style_.overall_width);
					width = renderer_style_.overall_width;
				}
				const int image_width = image_cache_->get(image_filename)->width();
				if (width < image_width) {
					scale = static_cast<double>(width) / image_width;
				}
			}
			render_node_.reset(
			   new ImgRenderNode(nodestyle_, image_filename, scale, color, use_playercolor));
		}
	}
	void emit_nodes(std::vector<std::shared_ptr<RenderNode>>& nodes) override {
		nodes.push_back(render_node_);
	}

private:
	std::shared_ptr<ImgRenderNode> render_node_;
};

class VspaceTagHandler : public TagHandler {
public:
	VspaceTagHandler(Tag& tag,
	                 FontCache& fc,
	                 const NodeStyle& ns,
	                 ImageCache* image_cache,
	                 RendererStyle& init_renderer_style,
	                 const UI::FontSets* fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets), space_(0) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();

		space_ = a["gap"].get_int(std::numeric_limits<uint16_t>::max());
	}
	void emit_nodes(std::vector<std::shared_ptr<RenderNode>>& nodes) override {
		nodes.push_back(std::shared_ptr<RenderNode>(new SpaceNode(nodestyle_, 0, space_)));
		nodes.push_back(std::shared_ptr<RenderNode>(new NewlineNode(nodestyle_)));
	}

private:
	uint16_t space_;
};

class HspaceTagHandler : public TagHandler {
public:
	HspaceTagHandler(Tag& tag,
	                 FontCache& fc,
	                 const NodeStyle& ns,
	                 ImageCache* image_cache,
	                 RendererStyle& init_renderer_style,
	                 const UI::FontSets* fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets),
	     background_image_(nullptr),
	     space_(0) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();

		if (a.has("gap")) {
			space_ = a["gap"].get_int(std::numeric_limits<uint16_t>::max());
		} else {
			space_ = INFINITE_WIDTH;
		}

		if (a.has("fill")) {
			fill_text_ = a["fill"].get_string();
			try {
				background_image_ = image_cache_->get(fill_text_);
				image_filename_ = fill_text_;
				fill_text_ = "";
			} catch (ImageNotFound&) {
			}
		}
	}

	void emit_nodes(std::vector<std::shared_ptr<RenderNode>>& nodes) override {
		if (!fill_text_.empty()) {
			std::shared_ptr<FillingTextNode> node;
			if (space_ < INFINITE_WIDTH) {
				node.reset(new FillingTextNode(font_cache_, nodestyle_, space_, fill_text_));
			} else {
				node.reset(new FillingTextNode(font_cache_, nodestyle_, 0, fill_text_, true));
			}
			nodes.push_back(node);
		} else {
			std::shared_ptr<SpaceNode> node;
			if (space_ < INFINITE_WIDTH) {
				node.reset(new SpaceNode(nodestyle_, space_, 0));
			} else {
				node.reset(new SpaceNode(nodestyle_, 0, 0, true));
			}
			if (background_image_) {
				node->set_background(background_image_, image_filename_);
			}
			nodes.push_back(node);
		}
	}

private:
	std::string fill_text_;
	const Image* background_image_;
	std::string image_filename_;
	uint16_t space_;
};

class BrTagHandler : public TagHandler {
public:
	BrTagHandler(Tag& tag,
	             FontCache& fc,
	             const NodeStyle& ns,
	             ImageCache* image_cache,
	             RendererStyle& init_renderer_style,
	             const UI::FontSets* fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets) {
	}

	void emit_nodes(std::vector<std::shared_ptr<RenderNode>>& nodes) override {
		nodes.push_back(std::shared_ptr<RenderNode>(new NewlineNode(nodestyle_)));
	}
};

class DivTagHandler : public TagHandler {
public:
	DivTagHandler(Tag& tag,
	              FontCache& fc,
	              const NodeStyle& ns,
	              ImageCache* image_cache,
	              RendererStyle& init_renderer_style,
	              const UI::FontSets* fontsets,
	              uint16_t max_w = 0,
	              bool shrink_to_fit = true)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets),
	     shrink_to_fit_(shrink_to_fit),
	     trim_spaces_(true),
	     w_(max_w),
	     render_node_(new DivTagRenderNode(ns)) {
	}

	/*
	 * Calculate width of all children of this div.
	 * Width is either fixed, a percentage of the parent or fill. If the width should fill
	 * the line, it is set to the remaining width of the current line. New lines aren't really
	 * started here but percent/filling elements are made so big that they won't fit into the
	 * previous line when their final position is calculated in Layout::fit_nodes().
	 */
	void enter() override {
		Borders padding, margin;

		handle_unique_attributes();
		const AttrMap& a = tag_.attrs();
		if (a.has("background")) {
			RGBColor clr;
			try {
				clr = a["background"].get_color();
				render_node_->set_background(clr);
			} catch (InvalidColor&) {
				render_node_->set_background(image_cache_->get(a["background"].get_string()));
			}
		}
		if (a.has("padding")) {
			uint8_t p = a["padding"].get_int(std::numeric_limits<uint8_t>::max());
			padding.left = padding.top = padding.right = padding.bottom = p;
		}
		// TODO(GunChleoc): padding_l and padding_r don't seem to produce balanced results.
		// We ran into that with the game tips,
		// using "<rt padding_l=48 padding_t=28 padding_r=48 padding_b=28>" there.
		if (a.has("padding_r")) {
			padding.right = a["padding_r"].get_int(std::numeric_limits<uint8_t>::max());
		}
		if (a.has("padding_b")) {
			padding.bottom = a["padding_b"].get_int(std::numeric_limits<uint8_t>::max());
		}
		if (a.has("padding_l")) {
			padding.left = a["padding_l"].get_int(std::numeric_limits<uint8_t>::max());
		}
		if (a.has("padding_t")) {
			padding.top = a["padding_t"].get_int(std::numeric_limits<uint8_t>::max());
		}
		if (a.has("margin")) {
			uint8_t p = a["margin"].get_int(std::numeric_limits<uint8_t>::max());
			margin.left = margin.top = margin.right = margin.bottom = p;
		}

		std::vector<std::shared_ptr<RenderNode>> subnodes;
		// If a percentage width is used, temporarily set it as the overall width. This way,
		// divs with width "fill" only use the width of their parent node. Also, percentages
		// given in child nodes are relative to the width of their parent node.
		const uint16_t old_line_width = renderer_style_.overall_width;
		if (render_node_->desired_width().unit == WidthUnit::kPercent) {
			renderer_style_.overall_width =
			   render_node_->desired_width().width * renderer_style_.overall_width / 100;
		}
		TagHandler::emit_nodes(subnodes);
		renderer_style_.overall_width = old_line_width;

		// Determine the required width by the width of the widest subnode
		uint16_t width_first_subnode = INFINITE_WIDTH;
		uint16_t widest_subnode = 0;
		for (const auto& n : subnodes) {
			if (n->width() >= INFINITE_WIDTH) {
				continue;
			}
			if (width_first_subnode >= INFINITE_WIDTH && n->width()) {
				width_first_subnode = n->width() + padding.left + padding.right;
			}
			widest_subnode = std::max<int>(widest_subnode, n->width() + padding.left + padding.right);
		}
		if (renderer_style_.remaining_width < width_first_subnode) {
			// Not enough space for first subnode. Move to next line
			renderer_style_.remaining_width = renderer_style_.overall_width;
		}

		switch (render_node_->desired_width().unit) {
		case WidthUnit::kPercent:
			w_ = render_node_->desired_width().width * renderer_style_.overall_width / 100;

			if (render_node_->get_floating() != RenderNode::Floating::kNone) {
				break;
			}
			// Reduce remaining width
			if (renderer_style_.remaining_width <= w_) {
				// Not enough space. Div will be placed in the next line, calculate the remaining space
				// there
				renderer_style_.remaining_width = renderer_style_.overall_width - w_;
			} else {
				renderer_style_.remaining_width -= w_;
			}
			break;
		case WidthUnit::kFill:
			w_ = renderer_style_.remaining_width;
			if (render_node_->get_floating() == RenderNode::Floating::kNone) {
				renderer_style_.remaining_width = 0;
			}
			break;
		default:
			if (!w_) {
				w_ = widest_subnode;
			}
			// Else do nothing
		}

		// Layout takes ownership of subnodes
		Layout layout(subnodes);
		std::vector<std::shared_ptr<RenderNode>> nodes_to_render;
		uint16_t max_line_width = layout.fit_nodes(&nodes_to_render, w_, padding, trim_spaces_);
		uint16_t extra_width = 0;
		if (w_ < INFINITE_WIDTH && w_ > max_line_width) {
			extra_width = w_ - max_line_width;
		} else if (render_node_->desired_width().unit == WidthUnit::kShrink) {
			w_ = max_line_width;
			if (render_node_->get_floating() == RenderNode::Floating::kNone) {
				renderer_style_.remaining_width -= w_;
			}
		}

		// Collect all tags from children
		for (const auto& rn : nodes_to_render) {
			for (const Reference& r : rn->get_references()) {
				render_node_->add_reference(
				   rn->x() + r.dim.x, rn->y() + r.dim.y, r.dim.w, r.dim.h, r.ref);
			}
			if (shrink_to_fit_) {
				switch (rn->halign()) {
				case UI::Align::kCenter:
					rn->set_x(rn->x() - extra_width / 2);
					break;
				case UI::Align::kRight:
					rn->set_x(rn->x() - extra_width);
					break;
				case UI::Align::kLeft:
					break;
				}
			}
		}
		if (shrink_to_fit_ || w_ >= INFINITE_WIDTH) {
			w_ = max_line_width;
		}

		render_node_->set_dimensions(w_, layout.height(), margin);
		render_node_->set_nodes_to_render(nodes_to_render);
	}
	void emit_nodes(std::vector<std::shared_ptr<RenderNode>>& nodes) override {
		nodes.push_back(render_node_);
	}

	// Handle attributes that are in div, but not in rt.
	virtual void handle_unique_attributes() {
		const AttrMap& a = tag_.attrs();
		if (a.has("width")) {
			shrink_to_fit_ = false;
			w_ = INFINITE_WIDTH;
			std::string width_string = a["width"].get_string();
			if (width_string == "*") {
				render_node_->set_desired_width(DesiredWidth(INFINITE_WIDTH, WidthUnit::kFill));
			} else if (boost::algorithm::ends_with(width_string, "%")) {
				width_string = width_string.substr(0, width_string.length() - 1);
				uint8_t width_percent = strtol(width_string.c_str(), nullptr, 10);
				if (width_percent > 100) {
					log_warn("Font renderer: Do not use width > 100%%\n");
					width_percent = 100;
				}
				render_node_->set_desired_width(DesiredWidth(width_percent, WidthUnit::kPercent));
			} else {
				w_ = a["width"].get_int(std::numeric_limits<uint16_t>::max());
				if (w_ > renderer_style_.overall_width) {
					log_warn("Font renderer: Specified width of %d exceeds the overall available "
					         "width of %d. Setting width to %d.\n",
					         w_, renderer_style_.overall_width, renderer_style_.overall_width);
					w_ = renderer_style_.overall_width;
				}
				render_node_->set_desired_width(DesiredWidth(w_, WidthUnit::kAbsolute));
			}
		}
		if (a.has("float")) {
			const std::string s = a["float"].get_string();
			if (s == "right") {
				render_node_->set_floating(RenderNode::Floating::kRight);
			} else if (s == "left") {
				render_node_->set_floating(RenderNode::Floating::kLeft);
			}
		}
		if (a.has("valign")) {
			const std::string align = a["valign"].get_string();
			if (align == "top") {
				render_node_->set_valign(UI::Align::kTop);
			} else if (align == "bottom") {
				render_node_->set_valign(UI::Align::kBottom);
			} else if (align == "center" || align == "middle") {
				render_node_->set_valign(UI::Align::kCenter);
			}
		}
	}

protected:
	bool shrink_to_fit_;
	// Always true for DivTagHandler but might be overwritten in RTTagHandler
	bool trim_spaces_;

private:
	uint16_t w_;
	std::shared_ptr<DivTagRenderNode> render_node_;
};

class RTTagHandler : public DivTagHandler {
public:
	RTTagHandler(Tag& tag,
	             FontCache& fc,
	             const NodeStyle& ns,
	             ImageCache* image_cache,
	             RendererStyle& init_renderer_style,
	             const UI::FontSets* fontsets,
	             uint16_t w)
	   : DivTagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets, w, true) {
	}

	// Handle attributes that are in rt, but not in div.
	void handle_unique_attributes() override {
		const AttrMap& a = tag_.attrs();
		WordSpacerNode::show_spaces(a.has("db_show_spaces") ? a["db_show_spaces"].get_bool() : 0);
		trim_spaces_ = (a.has("keep_spaces") ? !a["keep_spaces"].get_bool() : true);
		shrink_to_fit_ = shrink_to_fit_ && trim_spaces_;
	}
};

template <typename T>
TagHandler* create_taghandler(Tag& tag,
                              FontCache& fc,
                              NodeStyle& ns,
                              ImageCache* image_cache,
                              RendererStyle& renderer_style,
                              const UI::FontSets* fontsets) {
	return new T(tag, fc, ns, image_cache, renderer_style, fontsets);
}
using TagHandlerMap = std::map<const std::string,
                               TagHandler* (*)(Tag& tag,
                                               FontCache& fc,
                                               NodeStyle& ns,
                                               ImageCache* image_cache,
                                               RendererStyle& renderer_style,
                                               const UI::FontSets* fontsets)>;

TagHandler* create_taghandler(Tag& tag,
                              FontCache& fc,
                              NodeStyle& ns,
                              ImageCache* image_cache,
                              RendererStyle& renderer_style,
                              const UI::FontSets* fontsets) {
	static TagHandlerMap map;
	if (map.empty()) {
		map["br"] = &create_taghandler<BrTagHandler>;
		map["font"] = &create_taghandler<FontTagHandler>;
		map["div"] = &create_taghandler<DivTagHandler>;
		map["p"] = &create_taghandler<PTagHandler>;
		map["img"] = &create_taghandler<ImgTagHandler>;
		map["vspace"] = &create_taghandler<VspaceTagHandler>;
		map["space"] = &create_taghandler<HspaceTagHandler>;
	}
	TagHandlerMap::iterator i = map.find(tag.name());
	if (i == map.end()) {
		throw RenderError(
		   (boost::format("No Tag handler for %s. This is a bug, please submit a report.") %
		    tag.name())
		      .str());
	}
	return i->second(tag, fc, ns, image_cache, renderer_style, fontsets);
}

Renderer::Renderer(ImageCache* image_cache,
                   TextureCache* texture_cache,
                   const UI::FontSets* fontsets)
   : font_cache_(new FontCache()),
     parser_(new Parser()),
     image_cache_(image_cache),
     texture_cache_(texture_cache),
     fontsets_(fontsets),
     renderer_style_("sans", 16, INFINITE_WIDTH, INFINITE_WIDTH) {
	assert(image_cache);
	TextureCache* render(const std::string&, uint16_t, const TagSet&);
}

Renderer::~Renderer() {
}

std::shared_ptr<RenderNode>
Renderer::layout(const std::string& text, uint16_t width, bool is_rtl, const TagSet& allowed_tags) {
	std::unique_ptr<Tag> rt(parser_->parse(text, allowed_tags));

	if (!width) {
		width = INFINITE_WIDTH;
	}

	renderer_style_.remaining_width = width;
	renderer_style_.overall_width = width;

	UI::FontSet const* fontset = fontsets_->get_fontset(i18n::get_locale());

	NodeStyle default_style = {fontset,
	                           renderer_style_.font_face,
	                           renderer_style_.font_size,
	                           RGBColor(255, 255, 0),
	                           IFont::DEFAULT,
	                           0,
	                           UI::Align::kLeft,
	                           UI::Align::kTop,
	                           is_rtl,
	                           ""};

	RTTagHandler rtrn(
	   *rt, *font_cache_, default_style, image_cache_, renderer_style_, fontsets_, width);
	std::vector<std::shared_ptr<RenderNode>> nodes;
	rtrn.enter();
	rtrn.emit_nodes(nodes);

	assert(nodes.size() == 1);
	assert(nodes[0]);
	return nodes[0];
}

std::shared_ptr<const UI::RenderedText>
Renderer::render(const std::string& text, uint16_t width, bool is_rtl, const TagSet& allowed_tags) {
	std::shared_ptr<RenderNode> node(layout(text, width, is_rtl, allowed_tags));
	return std::shared_ptr<const UI::RenderedText>(node->render(texture_cache_));
}
}  // namespace RT
