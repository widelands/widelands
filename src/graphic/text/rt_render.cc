/*
 * Copyright (C) 2006-2017 by the Widelands Development Team
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

#include <cmath>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#include <SDL.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/log.h"
#include "base/macros.h"
#include "base/rect.h"
#include "base/vector.h"
#include "base/wexception.h"
#include "graphic/align.h"
#include "graphic/graphic.h"
#include "graphic/image_cache.h"
#include "graphic/image_io.h"
#include "graphic/playercolor.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_io.h"
#include "graphic/text/font_set.h"
#include "graphic/text/rt_parse.h"
#include "graphic/text/sdl_ttf_font.h"
#include "graphic/text/textstream.h"
#include "graphic/text_layout.h"
#include "graphic/texture.h"
#include "io/filesystem/filesystem_exceptions.h"
#include "io/filesystem/layered_filesystem.h"

using namespace std;

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
	kAbsolute, // Width in pixels
	kPercent, // Width in percent
	kShrink, // Shrink width to content
	kFill // Expand width to fill all remaining space
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
	string font_face;
	uint16_t font_size;
	RGBColor font_color;
	int font_style;

	uint8_t spacing;
	UI::Align halign;
	UI::Align valign;
	string reference;
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
		string face;
		uint16_t size;

		bool operator<(const FontDescr& o) const {
			return size < o.size || (size == o.size && face < o.face);
		}
	};
	using FontMap = map<FontDescr, IFont*>;
	using FontMapPair = pair<const FontDescr, std::unique_ptr<IFont>>;

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
	if (i != fontmap_.end())
		return *i->second;

	std::unique_ptr<IFont> font;
	try {
		font.reset(load_font(ns->font_face, font_size));
	} catch (FileNotFoundError& e) {
		log("Font file not found. Falling back to sans: %s\n%s\n", ns->font_face.c_str(), e.what());
		font.reset(load_font(ns->fontset->sans(), font_size));
	}
	assert(font != nullptr);

	return *fontmap_.insert(std::make_pair(fd, font.release())).first->second;
}

struct Reference {
	Recti dim;
	string ref;
};

class RefMap : public IRefMap {
public:
	RefMap(const vector<Reference>& refs) : refs_(refs) {
	}
	string query(int16_t x, int16_t y) override {
		// Should this linear algorithm proof to be too slow (doubtful), the
		// RefMap could also be efficiently implemented using an R-Tree
		for (const Reference& c : refs_)
			if (c.dim.contains(Vector2i(x, y)))
				return c.ref;
		return "";
	}

private:
	vector<Reference> refs_;
};

class RenderNode {
public:
	enum Floating {
		NO_FLOAT = 0,
		FLOAT_RIGHT,
		FLOAT_LEFT,
	};
	RenderNode(NodeStyle& ns)
	   : floating_(NO_FLOAT), halign_(ns.halign), valign_(ns.valign), x_(0), y_(0) {
	}
	virtual ~RenderNode() {
	}

	virtual uint16_t width() = 0;
	virtual uint16_t height() = 0;
	virtual uint16_t hotspot_y() = 0;
	virtual Texture* render(TextureCache* texture_cache) = 0;

	virtual bool is_non_mandatory_space() {
		return false;
	}
	virtual bool is_expanding() {
		return false;
	}
	virtual void set_w(uint16_t) {
	}  // Only, when is_expanding

	virtual const vector<Reference> get_references() {
		return vector<Reference>();
	}

	Floating get_floating() {
		return floating_;
	}
	void set_floating(Floating f) {
		floating_ = f;
	}
	UI::Align halign() {
		return halign_;
	}
	void set_halign(UI::Align ghalign) {
		halign_ = ghalign;
	}
	UI::Align valign() {
		return valign_;
	}
	void set_valign(UI::Align gvalign) {
		valign_ = gvalign;
	}
	void set_x(int32_t nx) {
		x_ = nx;
	}
	void set_y(int32_t ny) {
		y_ = ny;
	}
	int32_t x() {
		return x_;
	}
	int32_t y() {
		return y_;
	}

private:
	Floating floating_;
	UI::Align halign_;
	UI::Align valign_;
	int32_t x_, y_;
};

class Layout {
public:
	Layout(vector<RenderNode*>& all) : h_(0), idx_(0), all_nodes_(all) {
	}
	virtual ~Layout() {
	}

	uint16_t height() {
		return h_;
	}
	uint16_t fit_nodes(vector<RenderNode*>& rv, uint16_t w, Borders p, bool shrink_to_fit);

private:
	// Represents a change in the rendering constraints. For example when an
	// Image is inserted, the width will become wider after it. This is a
	// constraint change.
	struct ConstraintChange {
		int at_y;
		int32_t delta_w;
		int32_t delta_offset_x;

		bool operator<(const ConstraintChange& o) const {
			return at_y > o.at_y || (at_y == o.at_y && delta_w > o.delta_w);
		}
	};

	uint16_t fit_line(uint16_t w, const Borders&, vector<RenderNode*>* rv, bool shrink_to_fit);

	uint16_t h_;
	size_t idx_;
	vector<RenderNode*>& all_nodes_;
	priority_queue<ConstraintChange> constraint_changes_;
};

uint16_t
Layout::fit_line(uint16_t w, const Borders& p, vector<RenderNode*>* rv, bool shrink_to_fit) {
	assert(rv->empty());

	// Remove leading spaces
	while (idx_ < all_nodes_.size() && all_nodes_[idx_]->is_non_mandatory_space() && shrink_to_fit) {
		delete all_nodes_[idx_++];
	}

	uint16_t x = p.left;
	std::size_t first_idx = idx_;

	// Calc fitting nodes
	while (idx_ < all_nodes_.size()) {
		RenderNode* n = all_nodes_[idx_];
		uint16_t nw = n->width();
		if (x + nw + p.right > w || n->get_floating() != RenderNode::NO_FLOAT) {
			if (idx_ == first_idx) {
				nw = w - p.right - x;
			} else {
				break;
			}
		}
		n->set_x(x);
		x += nw;
		rv->push_back(n);
		++idx_;
	}
	// Remove trailing spaces
	while (!rv->empty() && rv->back()->is_non_mandatory_space() && shrink_to_fit) {
		x -= rv->back()->width();
		delete rv->back();
		rv->pop_back();
	}

	// Remaining space in this line
	uint16_t remaining_space = 0;
	if (w < INFINITE_WIDTH) {
		remaining_space = w - p.right - x;
	}

	// Find expanding nodes
	vector<size_t> expanding_nodes;
	for (size_t idx = 0; idx < rv->size(); ++idx)
		if (rv->at(idx)->is_expanding())
			expanding_nodes.push_back(idx);

	if (!expanding_nodes.empty()) {  // If there are expanding nodes, we fill the space
		const uint16_t individual_w = remaining_space / expanding_nodes.size();
		for (const size_t idx : expanding_nodes) {
			rv->at(idx)->set_w(individual_w);
			for (size_t nidx = idx + 1; nidx < rv->size(); ++nidx)
				rv->at(nidx)->set_x(rv->at(nidx)->x() + individual_w);
		}
	} else {
		// Take last elements style in this line and check horizontal alignment
		if (!rv->empty() && (*rv->rbegin())->halign() != UI::Align::kLeft) {
			if ((*rv->rbegin())->halign() == UI::Align::kCenter) {
				remaining_space /= 2;  // Otherwise, we align right
			}
			for (RenderNode* node : *rv) {
				node->set_x(node->x() + remaining_space);
			}
		}
	}

	// Find the biggest hotspot of the truly remaining items.
	uint16_t cur_line_hotspot = 0;
	for (RenderNode* node : *rv) {
		cur_line_hotspot = max(cur_line_hotspot, node->hotspot_y());
	}
	return cur_line_hotspot;
}

/*
 * Take ownership of all nodes, delete those that we do not render anyways (for
 * example unneeded spaces), append the rest to the vector we got.
 */
uint16_t Layout::fit_nodes(vector<RenderNode*>& rv, uint16_t w, Borders p, bool shrink_to_fit) {
	assert(rv.empty());
	h_ = p.top;

	uint16_t max_line_width = 0;
	while (idx_ < all_nodes_.size()) {
		vector<RenderNode*> nodes_in_line;
		size_t idx_before_iteration_ = idx_;
		uint16_t biggest_hotspot = fit_line(w, p, &nodes_in_line, shrink_to_fit);

		int line_height = 0;
		int line_start = INFINITE_WIDTH;
		// Compute real line height and width, taking into account alignement
		for (RenderNode* n : nodes_in_line) {
			line_height = max(line_height, biggest_hotspot - n->hotspot_y() + n->height());
			n->set_y(h_ + biggest_hotspot - n->hotspot_y());
			if (line_start >= INFINITE_WIDTH || n->x() < line_start) {
				line_start = n->x() - p.left;
			}
			max_line_width = std::max<int>(max_line_width, n->x() + n->width() + p.right - line_start);
		}

		// Go over again and adjust position for VALIGN
		for (RenderNode* n : nodes_in_line) {
			uint16_t space = line_height - n->height();
			if (!space || n->valign() == UI::Align::kBottom) {
				continue;
			}
			if (n->valign() == UI::Align::kCenter) {
				space /= 2;
			}
			// Space can become negative, for example when we have mixed fontsets on the same line
			// (e.g. "default" and "arabic"), due to differing font heights and hotspots.
			// So, we fix the sign.
			n->set_y(std::abs(n->y() - space));
		}
		rv.insert(rv.end(), nodes_in_line.begin(), nodes_in_line.end());

		h_ += line_height;
		while (!constraint_changes_.empty() && constraint_changes_.top().at_y <= h_) {
			const ConstraintChange& top = constraint_changes_.top();
			w += top.delta_w;
			p.left += top.delta_offset_x;
			constraint_changes_.pop();
		}

		if ((idx_ < all_nodes_.size()) && all_nodes_[idx_]->get_floating()) {
			RenderNode* n = all_nodes_[idx_];
			n->set_y(h_);
			ConstraintChange cc = {h_ + n->height(), 0, 0};
			if (n->get_floating() == RenderNode::FLOAT_LEFT) {
				n->set_x(p.left);
				p.left += n->width();
				cc.delta_offset_x = -n->width();
				max_line_width = max<int>(max_line_width, n->x() + n->width() + p.right);
			} else {
				n->set_x(w - n->width() - p.right);
				w -= n->width();
				cc.delta_w = n->width();
				max_line_width = max(max_line_width, w);
			}
			constraint_changes_.push(cc);
			rv.push_back(n);
			++idx_;
		}
		if (idx_ == idx_before_iteration_) {
			throw WidthTooSmall(
			   "Could not fit a single render node in line. Width of an Element is too small!");
		}
	}

	h_ += p.bottom;
	return max_line_width;
}

/*
 * A word in the text.
 */
class TextNode : public RenderNode {
public:
	TextNode(FontCache& font, NodeStyle&, const string& txt);
	virtual ~TextNode() {
	}

	uint16_t width() override {
		return w_;
	}
	uint16_t height() override {
		return h_ + nodestyle_.spacing;
	}
	uint16_t hotspot_y() override;
	const vector<Reference> get_references() override {
		vector<Reference> rv;
		if (!nodestyle_.reference.empty()) {
			Reference r = {Recti(0, 0, w_, h_), nodestyle_.reference};
			rv.push_back(r);
		}
		return rv;
	}

	Texture* render(TextureCache* texture_cache) override;

protected:
	uint16_t w_, h_;
	const string txt_;
	NodeStyle nodestyle_;
	FontCache& fontcache_;
	SdlTtfFont& font_;
};

TextNode::TextNode(FontCache& font, NodeStyle& ns, const string& txt)
   : RenderNode(ns),
     txt_(txt),
     nodestyle_(ns),
     fontcache_(font),
     font_(dynamic_cast<SdlTtfFont&>(fontcache_.get_font(&nodestyle_))) {
	font_.dimensions(txt_, ns.font_style, &w_, &h_);
}
uint16_t TextNode::hotspot_y() {
	return font_.ascent(nodestyle_.font_style);
}

Texture* TextNode::render(TextureCache* texture_cache) {
	const Texture& img =
	   font_.render(txt_, nodestyle_.font_color, nodestyle_.font_style, texture_cache);
	Texture* rv = new Texture(img.width(), img.height());
	rv->blit(Rectf(0, 0, img.width(), img.height()), img, Rectf(0, 0, img.width(), img.height()), 1.,
	         BlendMode::Copy);
	return rv;
}

/*
 * Text that might need to expand to fill the space between other elements. One
 * example are ... in a table like construction.
 */
class FillingTextNode : public TextNode {
public:
	FillingTextNode(
	   FontCache& font, NodeStyle& ns, uint16_t w, const string& txt, bool expanding = false)
	   : TextNode(font, ns, txt), is_expanding_(expanding) {
		w_ = w;
	}
	virtual ~FillingTextNode() {
	}
	Texture* render(TextureCache*) override;

	bool is_expanding() override {
		return is_expanding_;
	}
	void set_w(uint16_t w) override {
		w_ = w;
	}

private:
	bool is_expanding_;
};
Texture* FillingTextNode::render(TextureCache* texture_cache) {
	const Texture& t =
	   font_.render(txt_, nodestyle_.font_color, nodestyle_.font_style, texture_cache);
	Texture* rv = new Texture(w_, h_);
	for (uint16_t curx = 0; curx < w_; curx += t.width()) {
		Rectf srcrect(0.f, 0.f, min<int>(t.width(), w_ - curx), h_);
		rv->blit(Rectf(curx, 0, srcrect.w, srcrect.h), t, srcrect, 1., BlendMode::Copy);
	}
	return rv;
}

/*
 * The whitespace between two words. There is a debug option to make it red and therefore
 * visible.
 */
class WordSpacerNode : public TextNode {
public:
	WordSpacerNode(FontCache& font, NodeStyle& ns) : TextNode(font, ns, " ") {
	}
	static void show_spaces(bool t) {
		show_spaces_ = t;
	}

	Texture* render(TextureCache* texture_cache) override {
		if (show_spaces_) {
			Texture* rv = new Texture(w_, h_);
			rv->fill_rect(Rectf(0, 0, w_, h_), RGBAColor(0xcc, 0, 0, 0xcc));
			return rv;
		}
		return TextNode::render(texture_cache);
	}
	bool is_non_mandatory_space() override {
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
	NewlineNode(NodeStyle& ns) : RenderNode(ns) {
	}
	uint16_t height() override {
		return 0;
	}
	uint16_t width() override {
		return INFINITE_WIDTH;
	}
	uint16_t hotspot_y() override {
		return 0;
	}
	Texture* render(TextureCache* /* texture_cache */) override {
		NEVER_HERE();
	}
	bool is_non_mandatory_space() override {
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
	}

	uint16_t height() override {
		return h_;
	}
	uint16_t width() override {
		return w_;
	}
	uint16_t hotspot_y() override {
		return h_;
	}
	Texture* render(TextureCache* /* texture_cache */) override {
		Texture* rv = new Texture(w_, h_);

		// Draw background image (tiling)
		if (background_image_) {
			Rectf dst;
			Rectf srcrect(0, 0, 1, 1);
			for (uint16_t curx = 0; curx < w_; curx += background_image_->width()) {
				dst.x = curx;
				dst.y = 0;
				srcrect.w = dst.w = min<int>(background_image_->width(), w_ - curx);
				srcrect.h = dst.h = h_;
				rv->blit(dst, *background_image_, srcrect, 1., BlendMode::Copy);
			}
		} else {
			rv->fill_rect(Rectf(0, 0, w_, h_), RGBAColor(255, 255, 255, 0));
		}
		return rv;
	}
	bool is_expanding() override {
		return is_expanding_;
	}
	void set_w(uint16_t w) override {
		w_ = w;
	}

	void set_background(const Image* s) {
		background_image_ = s;
		h_ = s->height();
	}

private:
	uint16_t w_, h_;
	const Image* background_image_;  // not owned
	bool is_expanding_;
};

/*
 * This is a div tag node. It is also the same as a full rich text render node.
 */
class DivTagRenderNode : public RenderNode {
public:
	DivTagRenderNode(NodeStyle& ns)
	   : RenderNode(ns),
	     desired_width_(),
	     background_color_(0, 0, 0),
	     is_background_color_set_(false),
	     background_image_(nullptr) {
	}
	virtual ~DivTagRenderNode() {
		for (RenderNode* n : nodes_to_render_) {
			delete n;
		}
		nodes_to_render_.clear();
	}

	uint16_t width() override {
		return w_ + margin_.left + margin_.right;
	}
	uint16_t height() override {
		return h_ + margin_.top + margin_.bottom;
	}
	uint16_t hotspot_y() override {
		return height();
	}

	DesiredWidth desired_width() const {
		return desired_width_;
	}

	Texture* render(TextureCache* texture_cache) override {
		if (width() > g_gr->max_texture_size() || height() > g_gr->max_texture_size()) {
			const std::string error_message =
			   (boost::format("Texture (%d, %d) too big! Maximum size is %d.") % width() % height() %
			    g_gr->max_texture_size())
			      .str();
			log("%s\n", error_message.c_str());
			throw TextureTooBig(error_message);
		}
		Texture* rv = new Texture(width(), height());
		rv->fill_rect(Rectf(0, 0, rv->width(), rv->height()), RGBAColor(255, 255, 255, 0));

		// Draw Solid background Color
		bool set_alpha = true;
		if (is_background_color_set_) {
			rv->fill_rect(Rectf(margin_.left, margin_.top, w_, h_), background_color_);
			set_alpha = false;
		}

		// Draw background image (tiling)
		if (background_image_) {
			Rectf dst;
			Rectf src(0, 0, 0, 0);

			for (uint16_t cury = margin_.top; cury < h_ + margin_.top;
			     cury += background_image_->height()) {
				for (uint16_t curx = margin_.left; curx < w_ + margin_.left;
				     curx += background_image_->width()) {
					dst.x = curx;
					dst.y = cury;
					src.w = dst.w = min<int>(background_image_->width(), w_ + margin_.left - curx);
					src.h = dst.h = min<int>(background_image_->height(), h_ + margin_.top - cury);
					rv->blit(dst, *background_image_, src, 1., BlendMode::Copy);
				}
			}
			set_alpha = false;
		}

		for (RenderNode* n : nodes_to_render_) {
			Texture* node_texture = n->render(texture_cache);
			if (node_texture) {
				Rectf dst(n->x() + margin_.left, n->y() + margin_.top, node_texture->width(),
				          node_texture->height());
				Rectf src(0, 0, node_texture->width(), node_texture->height());
				rv->blit(
				   dst, *node_texture, src, 1., set_alpha ? BlendMode::Copy : BlendMode::UseAlpha);
				delete node_texture;
			}
			delete n;
		}

		nodes_to_render_.clear();

		return rv;
	}
	const vector<Reference> get_references() override {
		return refs_;
	}
	void set_dimensions(uint16_t inner_w, uint16_t inner_h, Borders margin) {
		w_ = inner_w;
		h_ = inner_h;
		margin_ = margin;
	}
	void set_desired_width(DesiredWidth desired_width) {
		desired_width_ = desired_width;
	}
	void set_background(RGBColor clr) {
		background_color_ = clr;
		is_background_color_set_ = true;
	}
	void set_background(const Image* img) {
		background_image_ = img;
	}
	void set_nodes_to_render(vector<RenderNode*>& n) {
		nodes_to_render_ = n;
	}
	void add_reference(int16_t gx, int16_t gy, uint16_t w, uint16_t h, const string& s) {
		Reference r = {Recti(gx, gy, w, h), s};
		refs_.push_back(r);
	}

private:
	DesiredWidth desired_width_;
	uint16_t w_, h_;
	vector<RenderNode*> nodes_to_render_;
	Borders margin_;
	RGBColor background_color_;
	bool is_background_color_set_;
	const Image* background_image_;  // Not owned.
	vector<Reference> refs_;
};

class ImgRenderNode : public RenderNode {
public:
	ImgRenderNode(NodeStyle& ns,
	              const std::string& image_filename,
	              double scale,
	              const RGBColor& color)
	   : RenderNode(ns), image_(g_gr->images().get(image_filename)), scale_(scale), color_(color) {
		if (color_.hex_value() != "000000") {
			image_ = playercolor_image(&color_, image_filename);
		}
	}

	uint16_t width() override {
		return scale_ * image_->width();
	}
	uint16_t height() override {
		return scale_ * image_->height();
	}
	uint16_t hotspot_y() override {
		return scale_ * image_->height();
	}
	Texture* render(TextureCache* texture_cache) override;

private:
	const Image* image_;
	const double scale_;
	const RGBColor color_;
};

Texture* ImgRenderNode::render(TextureCache* /* texture_cache */) {
	Texture* rv = new Texture(width(), height());
	rv->blit(Rectf(0, 0, width(), height()), *image_, Rectf(0, 0, image_->width(), image_->height()),
	         1., BlendMode::Copy);
	return rv;
}
// End: Helper Stuff

class TagHandler;
TagHandler* create_taghandler(Tag& tag,
                              FontCache& fc,
                              NodeStyle& ns,
                              ImageCache* image_cache,
                              RendererStyle& renderer_style,
                              const UI::FontSets& fontsets);

class TagHandler {
public:
	TagHandler(Tag& tag,
	           FontCache& fc,
	           NodeStyle ns,
	           ImageCache* image_cache,
	           RendererStyle& renderer_style,
	           const UI::FontSets& fontsets)
	   : tag_(tag),
	     font_cache_(fc),
	     nodestyle_(ns),
	     image_cache_(image_cache),
	     renderer_style_(renderer_style),
	     fontsets_(fontsets) {
	}
	virtual ~TagHandler() {
	}

	virtual void enter() {
	}
	virtual void emit_nodes(vector<RenderNode*>&);

private:
	void make_text_nodes(const string& txt, vector<RenderNode*>& nodes, NodeStyle& ns);

protected:
	Tag& tag_;
	FontCache& font_cache_;
	NodeStyle nodestyle_;
	ImageCache* image_cache_;        // Not owned
	RendererStyle& renderer_style_;  // Reference to global renderer style in the renderer
	const UI::FontSets& fontsets_;
};

void TagHandler::make_text_nodes(const string& txt, vector<RenderNode*>& nodes, NodeStyle& ns) {
	TextStream ts(txt);
	std::string word;
	std::vector<RenderNode*> text_nodes;

	// Bidirectional text (Arabic etc.)
	if (i18n::has_rtl_character(txt.c_str())) {
		std::string previous_word;
		std::vector<RenderNode*>::iterator it = text_nodes.begin();
		std::vector<WordSpacerNode*> spacer_nodes;

		// Collect the word nodes
		while (ts.pos() < txt.size()) {
			std::size_t cpos = ts.pos();
			ts.skip_ws();
			spacer_nodes.clear();

			// We only know if the spacer goes to the left or right after having a look at the current
			// word.
			for (uint16_t ws_indx = 0; ws_indx < ts.pos() - cpos; ws_indx++) {
				spacer_nodes.push_back(new WordSpacerNode(font_cache_, ns));
			}

			word = ts.till_any_or_end(" \t\n\r");
			ns.fontset = i18n::find_fontset(word.c_str(), fontsets_);
			if (!word.empty()) {
				replace_entities(&word);
				bool word_is_bidi = i18n::has_rtl_character(word.c_str());
				word = i18n::make_ligatures(word.c_str());
				if (word_is_bidi || i18n::has_rtl_character(previous_word.c_str())) {
					for (WordSpacerNode* spacer : spacer_nodes) {
						it = text_nodes.insert(text_nodes.begin(), spacer);
					}
					if (word_is_bidi) {
						word = i18n::line2bidi(word.c_str());
					}
					it = text_nodes.insert(
					   text_nodes.begin(), new TextNode(font_cache_, ns, word.c_str()));
				} else {  // Sequences of Latin words go to the right from current position
					if (it < text_nodes.end()) {
						++it;
					}
					for (WordSpacerNode* spacer : spacer_nodes) {
						it = text_nodes.insert(it, spacer);
						if (it < text_nodes.end()) {
							++it;
						}
					}
					it = text_nodes.insert(it, new TextNode(font_cache_, ns, word));
				}
			}
			previous_word = word;
		}
		// Add the nodes to the end of the previously existing nodes.
		for (RenderNode* node : text_nodes) {
			nodes.push_back(node);
		}

	} else {  // LTR
		while (ts.pos() < txt.size()) {
			std::size_t cpos = ts.pos();
			ts.skip_ws();
			for (uint16_t ws_indx = 0; ws_indx < ts.pos() - cpos; ws_indx++) {
				nodes.push_back(new WordSpacerNode(font_cache_, ns));
			}
			word = ts.till_any_or_end(" \t\n\r");
			ns.fontset = i18n::find_fontset(word.c_str(), fontsets_);
			if (!word.empty()) {
				replace_entities(&word);
				word = i18n::make_ligatures(word.c_str());
				if (i18n::has_script_character(word.c_str(), UI::FontSets::Selector::kCJK)) {
					std::vector<std::string> units = i18n::split_cjk_word(word.c_str());
					for (const std::string& unit : units) {
						nodes.push_back(new TextNode(font_cache_, ns, unit));
					}
				} else {
					nodes.push_back(new TextNode(font_cache_, ns, word));
				}
			}
		}
	}
}

void TagHandler::emit_nodes(vector<RenderNode*>& nodes) {
	for (Child* c : tag_.children()) {
		if (c->tag) {
			std::unique_ptr<TagHandler> th(create_taghandler(
			   *c->tag, font_cache_, nodestyle_, image_cache_, renderer_style_, fontsets_));
			th->enter();
			th->emit_nodes(nodes);
		} else
			make_text_nodes(c->text, nodes, nodestyle_);
	}
}

class FontTagHandler : public TagHandler {
public:
	FontTagHandler(Tag& tag,
	               FontCache& fc,
	               NodeStyle ns,
	               ImageCache* image_cache,
	               RendererStyle& init_renderer_style,
	               const UI::FontSets& fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();
		if (a.has("color"))
			nodestyle_.font_color = a["color"].get_color();
		if (a.has("size"))
			nodestyle_.font_size = a["size"].get_int();
		if (a.has("face"))
			nodestyle_.font_face = a["face"].get_string();
		if (a.has("bold"))
			nodestyle_.font_style |= a["bold"].get_bool() ? IFont::BOLD : 0;
		if (a.has("italic"))
			nodestyle_.font_style |= a["italic"].get_bool() ? IFont::ITALIC : 0;
		if (a.has("underline"))
			nodestyle_.font_style |= a["underline"].get_bool() ? IFont::UNDERLINE : 0;
		if (a.has("shadow"))
			nodestyle_.font_style |= a["shadow"].get_bool() ? IFont::SHADOW : 0;
		if (a.has("ref"))
			nodestyle_.reference = a["ref"].get_string();
	}
};

class PTagHandler : public TagHandler {
public:
	PTagHandler(Tag& tag,
	            FontCache& fc,
	            NodeStyle ns,
	            ImageCache* image_cache,
	            RendererStyle& init_renderer_style,
	            const UI::FontSets& fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets), indent_(0) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();
		if (a.has("indent"))
			indent_ = a["indent"].get_int();
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
		nodestyle_.halign = mirror_alignment(nodestyle_.halign);
		if (a.has("valign")) {
			const string align = a["valign"].get_string();
			if (align == "bottom") {
				nodestyle_.valign = UI::Align::kBottom;
			} else if (align == "center" || align == "middle") {
				nodestyle_.valign = UI::Align::kCenter;
			} else {
				nodestyle_.valign = UI::Align::kTop;
			}
		}
		if (a.has("spacing"))
			nodestyle_.spacing = a["spacing"].get_int();
	}
	void emit_nodes(vector<RenderNode*>& nodes) override {
		// Put a newline if this is not the first paragraph
		if (!nodes.empty()) {
			nodes.push_back(new NewlineNode(nodestyle_));
		}
		if (indent_) {
			nodes.push_back(new SpaceNode(nodestyle_, indent_));
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
	              NodeStyle ns,
	              ImageCache* image_cache,
	              RendererStyle& init_renderer_style,
	              const UI::FontSets& fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets), render_node_(nullptr) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();
		RGBColor color;
		const std::string image_filename = a["src"].get_string();
		double scale = 1.0;

		if (a.has("color")) {
			color = a["color"].get_color();
		}
		if (a.has("width")) {
			int width = a["width"].get_int();
			if (width > renderer_style_.overall_width) {
				log("WARNING: Font renderer: Specified image width of %d exceeds the overall available "
				    "width of %d. Setting width to %d.\n",
				    width, renderer_style_.overall_width, renderer_style_.overall_width);
				width = renderer_style_.overall_width;
			}
			const int image_width = image_cache_->get(image_filename)->width();
			if (width < image_width) {
				scale = static_cast<double>(width) / image_width;
			}
		}
		render_node_ = new ImgRenderNode(nodestyle_, image_filename, scale, color);
	}
	void emit_nodes(vector<RenderNode*>& nodes) override {
		nodes.push_back(render_node_);
	}

private:
	ImgRenderNode* render_node_;
};

class VspaceTagHandler : public TagHandler {
public:
	VspaceTagHandler(Tag& tag,
	                 FontCache& fc,
	                 NodeStyle ns,
	                 ImageCache* image_cache,
	                 RendererStyle& init_renderer_style,
	                 const UI::FontSets& fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets), space_(0) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();

		space_ = a["gap"].get_int();
	}
	void emit_nodes(vector<RenderNode*>& nodes) override {
		nodes.push_back(new SpaceNode(nodestyle_, 0, space_));
		nodes.push_back(new NewlineNode(nodestyle_));
	}

private:
	uint16_t space_;
};

class HspaceTagHandler : public TagHandler {
public:
	HspaceTagHandler(Tag& tag,
	                 FontCache& fc,
	                 NodeStyle ns,
	                 ImageCache* image_cache,
	                 RendererStyle& init_renderer_style,
	                 const UI::FontSets& fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets),
	     background_image_(nullptr),
	     space_(0) {
	}

	void enter() override {
		const AttrMap& a = tag_.attrs();

		if (a.has("gap"))
			space_ = a["gap"].get_int();
		else
			space_ = INFINITE_WIDTH;

		if (a.has("fill")) {
			fill_text_ = a["fill"].get_string();
			try {
				background_image_ = image_cache_->get(fill_text_);
				fill_text_ = "";
			} catch (ImageNotFound&) {
			}
		}
	}

	void emit_nodes(vector<RenderNode*>& nodes) override {
		RenderNode* rn = nullptr;
		if (!fill_text_.empty()) {
			if (space_ < INFINITE_WIDTH)
				rn = new FillingTextNode(font_cache_, nodestyle_, space_, fill_text_);
			else
				rn = new FillingTextNode(font_cache_, nodestyle_, 0, fill_text_, true);
		} else {
			SpaceNode* sn;
			if (space_ < INFINITE_WIDTH)
				sn = new SpaceNode(nodestyle_, space_, 0);
			else
				sn = new SpaceNode(nodestyle_, 0, 0, true);

			if (background_image_)
				sn->set_background(background_image_);
			rn = sn;
		}
		nodes.push_back(rn);
	}

private:
	string fill_text_;
	const Image* background_image_;
	uint16_t space_;
};

class BrTagHandler : public TagHandler {
public:
	BrTagHandler(Tag& tag,
	             FontCache& fc,
	             NodeStyle ns,
	             ImageCache* image_cache,
	             RendererStyle& init_renderer_style,
	             const UI::FontSets& fontsets)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets) {
	}

	void emit_nodes(vector<RenderNode*>& nodes) override {
		nodes.push_back(new NewlineNode(nodestyle_));
	}
};

class DivTagHandler : public TagHandler {
public:
	DivTagHandler(Tag& tag,
	              FontCache& fc,
	              NodeStyle ns,
	              ImageCache* image_cache,
	              RendererStyle& init_renderer_style,
	              const UI::FontSets& fontsets,
	              uint16_t max_w = 0,
	              bool shrink_to_fit = true)
	   : TagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets),
	     shrink_to_fit_(shrink_to_fit),
	     w_(max_w),
	     render_node_(new DivTagRenderNode(ns)) {
	}

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
			uint8_t p = a["padding"].get_int();
			padding.left = padding.top = padding.right = padding.bottom = p;
		}
		if (a.has("padding_r"))
			padding.right = a["padding_r"].get_int();
		if (a.has("padding_b"))
			padding.bottom = a["padding_b"].get_int();
		if (a.has("padding_l"))
			padding.left = a["padding_l"].get_int();
		if (a.has("padding_t"))
			padding.top = a["padding_t"].get_int();
		if (a.has("margin")) {
			uint8_t p = a["margin"].get_int();
			margin.left = margin.top = margin.right = margin.bottom = p;
		}

		vector<RenderNode*> subnodes;
		TagHandler::emit_nodes(subnodes);

		if (!w_) {  // Determine the width by the width of the widest subnode
			for (RenderNode* n : subnodes) {
				if (n->width() >= INFINITE_WIDTH)
					continue;
				w_ = max<int>(w_, n->width() + padding.left + padding.right);
			}
		}

		switch (render_node_->desired_width().unit) {
		case WidthUnit::kPercent:
			w_ = render_node_->desired_width().width * renderer_style_.overall_width / 100;
			renderer_style_.remaining_width -= w_;
			break;
		case WidthUnit::kFill:
			w_ = renderer_style_.remaining_width;
			renderer_style_.remaining_width = 0;
			break;
		default:;  // Do nothing
		}

		// Layout takes ownership of subnodes
		Layout layout(subnodes);
		vector<RenderNode*> nodes_to_render;
		uint16_t max_line_width = layout.fit_nodes(nodes_to_render, w_, padding, shrink_to_fit_);
		uint16_t extra_width = 0;
		if (w_ < INFINITE_WIDTH && w_ > max_line_width) {
			extra_width = w_ - max_line_width;
		} else if (render_node_->desired_width().unit == WidthUnit::kShrink) {
			w_ = max_line_width;
			renderer_style_.remaining_width -= w_;
		}

		// Collect all tags from children
		for (RenderNode* rn : nodes_to_render) {
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

		if (renderer_style_.remaining_width < w_) {
			renderer_style_.remaining_width = renderer_style_.overall_width;
		}

		render_node_->set_dimensions(w_, layout.height(), margin);
		render_node_->set_nodes_to_render(nodes_to_render);
	}
	void emit_nodes(vector<RenderNode*>& nodes) override {
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
					log("WARNING: Font renderer: Do not use width > 100%%\n");
					width_percent = 100;
				}
				render_node_->set_desired_width(DesiredWidth(width_percent, WidthUnit::kPercent));
			} else {
				w_ = a["width"].get_int();
				if (w_ > renderer_style_.overall_width) {
					log("WARNING: Font renderer: Specified width of %d exceeds the overall available "
					    "width of %d. Setting width to %d.\n",
					    w_, renderer_style_.overall_width, renderer_style_.overall_width);
					w_ = renderer_style_.overall_width;
				}
				render_node_->set_desired_width(DesiredWidth(w_, WidthUnit::kAbsolute));
			}
		}
		if (a.has("float")) {
			const string s = a["float"].get_string();
			if (s == "right")
				render_node_->set_floating(RenderNode::FLOAT_RIGHT);
			else if (s == "left")
				render_node_->set_floating(RenderNode::FLOAT_LEFT);
		}
		if (a.has("valign")) {
			const string align = a["valign"].get_string();
			if (align == "top")
				render_node_->set_valign(UI::Align::kTop);
			else if (align == "bottom")
				render_node_->set_valign(UI::Align::kBottom);
			else if (align == "center" || align == "middle")
				render_node_->set_valign(UI::Align::kCenter);
		}
	}

protected:
	bool shrink_to_fit_;

private:
	uint16_t w_;
	DivTagRenderNode* render_node_;
};

class RTTagHandler : public DivTagHandler {
public:
	RTTagHandler(Tag& tag,
	             FontCache& fc,
	             NodeStyle ns,
	             ImageCache* image_cache,
	             RendererStyle& init_renderer_style,
	             const UI::FontSets& fontsets,
	             uint16_t w)
	   : DivTagHandler(tag, fc, ns, image_cache, init_renderer_style, fontsets, w, true) {
	}

	// Handle attributes that are in rt, but not in div.
	void handle_unique_attributes() override {
		const AttrMap& a = tag_.attrs();
		WordSpacerNode::show_spaces(a.has("db_show_spaces") ? a["db_show_spaces"].get_bool() : 0);
		shrink_to_fit_ =
		   shrink_to_fit_ && (a.has("keep_spaces") ? !a["keep_spaces"].get_bool() : true);
	}
};

template <typename T>
TagHandler* create_taghandler(Tag& tag,
                              FontCache& fc,
                              NodeStyle& ns,
                              ImageCache* image_cache,
                              RendererStyle& renderer_style,
                              const UI::FontSets& fontsets) {
	return new T(tag, fc, ns, image_cache, renderer_style, fontsets);
}
using TagHandlerMap = map<const string,
                          TagHandler* (*)(Tag& tag,
                                          FontCache& fc,
                                          NodeStyle& ns,
                                          ImageCache* image_cache,
                                          RendererStyle& renderer_style,
                                          const UI::FontSets& fontsets)>;

TagHandler* create_taghandler(Tag& tag,
                              FontCache& fc,
                              NodeStyle& ns,
                              ImageCache* image_cache,
                              RendererStyle& renderer_style,
                              const UI::FontSets& fontsets) {
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
	if (i == map.end())
		throw RenderError(
		   (boost::format("No Tag handler for %s. This is a bug, please submit a report.") %
		    tag.name())
		      .str());
	return i->second(tag, fc, ns, image_cache, renderer_style, fontsets);
}

Renderer::Renderer(ImageCache* image_cache,
                   TextureCache* texture_cache,
                   const UI::FontSets& fontsets)
   : font_cache_(new FontCache()),
     parser_(new Parser()),
     image_cache_(image_cache),
     texture_cache_(texture_cache),
     fontsets_(fontsets),
     renderer_style_("sans", 16, INFINITE_WIDTH, INFINITE_WIDTH) {
	TextureCache* render(const std::string&, uint16_t, const TagSet&);
}

Renderer::~Renderer() {
}

RenderNode* Renderer::layout_(const string& text, uint16_t width, const TagSet& allowed_tags) {
	std::unique_ptr<Tag> rt(parser_->parse(text, allowed_tags));

	if (!width) {
		width = INFINITE_WIDTH;
	}

	renderer_style_.remaining_width = width;
	renderer_style_.overall_width = width;

	UI::FontSet const* fontset = fontsets_.get_fontset(i18n::get_locale());

	NodeStyle default_style = {fontset,
	                           renderer_style_.font_face,
	                           renderer_style_.font_size,
	                           RGBColor(255, 255, 0),
	                           IFont::DEFAULT,
	                           0,
	                           UI::Align::kLeft,
	                           UI::Align::kTop,
	                           ""};

	RTTagHandler rtrn(
	   *rt, *font_cache_, default_style, image_cache_, renderer_style_, fontsets_, width);
	vector<RenderNode*> nodes;
	rtrn.enter();
	rtrn.emit_nodes(nodes);

	assert(nodes.size() == 1);
	assert(nodes[0]);
	return nodes[0];
}

Texture* Renderer::render(const string& text, uint16_t width, const TagSet& allowed_tags) {
	std::unique_ptr<RenderNode> node(layout_(text, width, allowed_tags));

	return node->render(texture_cache_);
}

IRefMap*
Renderer::make_reference_map(const string& text, uint16_t width, const TagSet& allowed_tags) {
	std::unique_ptr<RenderNode> node(layout_(text, width, allowed_tags));
	return new RefMap(node->get_references());
}
}
