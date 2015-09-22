/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#include <memory>
#include <queue>
#include <string>
#include <vector>

#include <SDL.h>
#include <boost/format.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "base/point.h"
#include "base/rect.h"
#include "graphic/image_cache.h"
#include "graphic/image_io.h"
#include "graphic/text/bidi.h"
#include "graphic/text/font_io.h"
#include "graphic/text/font_set.h"
#include "graphic/text/rt_parse.h"
#include "graphic/text/textstream.h"
#include "graphic/texture.h"
#include "io/filesystem/filesystem_exceptions.h"

using namespace std;

namespace RT {

static const uint16_t INFINITE_WIDTH = 65535; // 2^16-1

// Helper Stuff
enum HAlign {
	HALIGN_LEFT,
	HALIGN_RIGHT,
	HALIGN_CENTER,
};
enum VAlign {
	VALIGN_BOTTOM,
	VALIGN_TOP,
	VALIGN_CENTER,
};
struct Borders {
	Borders() {left = top = right = bottom = 0;}
	uint8_t left, top, right, bottom;
};

struct NodeStyle {
	const UI::FontSet* const fontset;  // Not owned.
	string font_face;
	uint16_t font_size;
	RGBColor font_color;
	int font_style;
	UI::FontSet::Direction direction;

	uint8_t spacing;
	HAlign halign;
	VAlign valign;
	string reference;
};

struct Reference {
	Rect dim;
	string ref;
};

class RefMap : public IRefMap {
public:
	RefMap(const vector<Reference>& refs) : m_refs(refs) {}
	string query(int16_t x, int16_t y) override {
		// Should this linear algorithm proof to be too slow (doubtful), the
		// RefMap could also be efficiently implemented using an R-Tree
		for (const Reference& c : m_refs)
			if (c.dim.contains(Point(x, y)))
				return c.ref;
		return "";
	}

private:
	vector<Reference> m_refs;
};

class RenderNode {
public:
	enum Floating {
		NO_FLOAT = 0,
		FLOAT_RIGHT,
		FLOAT_LEFT,
	};
	RenderNode(NodeStyle& ns)
		: m_floating(NO_FLOAT), m_halign(ns.halign), m_valign(ns.valign), m_x(0), m_y(0) {}
	virtual ~RenderNode() {}

	virtual uint16_t width() = 0;
	virtual uint16_t height() = 0;
	virtual uint16_t hotspot_y() = 0;
	virtual Texture* render(TextureCache* texture_cache) = 0;

	virtual bool is_non_mandatory_space() {return false;}
	virtual bool is_expanding() {return false;}
	virtual void set_w(uint16_t) {} // Only, when expanding

	virtual const vector<Reference> get_references() {return vector<Reference>();}

	Floating get_floating() {return m_floating;}
	void set_floating(Floating f) {m_floating = f;}
	HAlign halign() {return m_halign;}
	void set_halign(HAlign ghalign) {m_halign = ghalign;}
	VAlign valign() {return m_valign;}
	void set_valign(VAlign gvalign) {m_valign = gvalign;}
	void set_x(uint16_t nx) {m_x = nx;}
	void set_y(uint16_t ny) {m_y = ny;}
	uint16_t x() {return m_x;}
	uint16_t y() {return m_y;}

private:
	Floating m_floating;
	HAlign m_halign;
	VAlign m_valign;
	uint16_t m_x, m_y;
};

class Layout {
public:
	Layout(vector<RenderNode*>& all) : m_h(0), m_idx(0), m_all_nodes(all) {}
	virtual ~Layout() {}

	uint16_t height() {return m_h;}
	uint16_t fit_nodes(vector<RenderNode*>& rv, uint16_t w, Borders p);

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

	uint16_t m_fit_line(uint16_t w, const Borders&, vector<RenderNode*>* rv);

	uint16_t m_h;
	size_t m_idx;
	vector<RenderNode*>& m_all_nodes;
	priority_queue<ConstraintChange> m_constraint_changes;
};

uint16_t Layout::m_fit_line(uint16_t w, const Borders& p, vector<RenderNode*>* rv) {
	assert(rv->empty());

	while (m_idx < m_all_nodes.size() && m_all_nodes[m_idx]->is_non_mandatory_space()) {
		delete m_all_nodes[m_idx++];
	}

	uint16_t x = p.left;
	while (m_idx < m_all_nodes.size()) {
		RenderNode* n = m_all_nodes[m_idx];
		const uint16_t nw = n->width();
		if (x + nw + p.right > w || n->get_floating()) {
			break;
		}
		n->set_x(x); x += nw;
		rv->push_back(n);
		++m_idx;
	}

	if (!rv->empty() && rv->back()->is_non_mandatory_space()) {
		x -= rv->back()->width();
		delete rv->back();
		rv->pop_back();
	}

	// Remaining space in this line
	uint16_t rem_space = w - p.right - x;

	// Find expanding nodes
	vector<size_t> expanding_nodes;
	for (size_t idx = 0; idx < rv->size(); ++idx)
		if (rv->at(idx)->is_expanding())
			expanding_nodes.push_back(idx);

	if (!expanding_nodes.empty()) { // If there are expanding nodes, we fill the space
		const uint16_t individual_w = rem_space / expanding_nodes.size();
		for (const size_t idx : expanding_nodes) {
			rv->at(idx)->set_w(individual_w);
			for (size_t nidx = idx + 1; nidx < rv->size(); ++nidx)
				rv->at(nidx)->set_x(rv->at(nidx)->x() + individual_w);
		}
	} else {
		// Take last elements style in this line and check horizontal alignment
		if (!rv->empty() && (*rv->rbegin())->halign() != HALIGN_LEFT) {
			if ((*rv->rbegin())->halign() == HALIGN_CENTER) {
				rem_space /= 2;  // Otherwise, we align right
			}
			for (RenderNode* node : *rv)  {
				node->set_x(node->x() + rem_space);
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
uint16_t Layout::fit_nodes(vector<RenderNode*>& rv, uint16_t w, Borders p) {
	assert(rv.empty());
	m_h = p.top;

	uint16_t max_line_width = 0;
	while (m_idx < m_all_nodes.size()) {
		vector<RenderNode*> nodes_in_line;
		size_t m_idx_before_iteration = m_idx;
		uint16_t biggest_hotspot = m_fit_line(w, p, &nodes_in_line);

		int line_height = 0;
		for (RenderNode* n : nodes_in_line) {
			line_height = max(line_height, biggest_hotspot - n->hotspot_y() + n->height());
			n->set_y(m_h + biggest_hotspot - n->hotspot_y());
			max_line_width = max<int>(max_line_width, n->x() + n->width() + p.right);
		}

		// Go over again and adjust position for VALIGN
		for (RenderNode* n : nodes_in_line) {
			uint16_t space = line_height - n->height();
			if (!space || n->valign() == VALIGN_BOTTOM)
				continue;
			if (n->valign() == VALIGN_CENTER)
				space /= 2;
			n->set_y(n->y() - space);
		}
		rv.insert(rv.end(), nodes_in_line.begin(), nodes_in_line.end());

		m_h += line_height;
		while (! m_constraint_changes.empty() && m_constraint_changes.top().at_y <= m_h) {
			const ConstraintChange& top = m_constraint_changes.top();
			w += top.delta_w;
			p.left += top.delta_offset_x;
			m_constraint_changes.pop();
		}

		if ((m_idx < m_all_nodes.size()) && m_all_nodes[m_idx]->get_floating()) {
			RenderNode* n = m_all_nodes[m_idx];
			n->set_y(m_h);
			ConstraintChange cc = {m_h + n->height(), 0, 0};
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
			m_constraint_changes.push(cc);
			rv.push_back(n);
			++m_idx;
		}
		if (m_idx == m_idx_before_iteration) {
			throw WidthTooSmall("Could not fit a single render node in line. Width of an Element is too small!");
		}
	}

	m_h += p.bottom;
	return max_line_width;
}

/*
 * A word in the text.
 */
class TextNode : public RenderNode {
public:
	TextNode(IFont& font, NodeStyle&, const string& txt);
	virtual ~TextNode() {}

	uint16_t width() override {return m_w;}
	uint16_t height() override {return m_h + m_s.spacing;}
	uint16_t hotspot_y() override;
	const vector<Reference> get_references() override {
		vector<Reference> rv;
		if (!m_s.reference.empty()) {
			Reference r = {Rect(0, 0, m_w, m_h), m_s.reference};
			rv.push_back(r);
		}
		return rv;
	}

	Texture* render(TextureCache* texture_cache) override;

protected:
	uint16_t m_w, m_h;
	const string m_txt;
	NodeStyle m_s;
	IFont& m_font;
};

TextNode::TextNode(IFont& font, NodeStyle& ns, const string& txt)
	: RenderNode(ns), m_txt(txt), m_s(ns), m_font(font)
{
	m_font.dimensions(m_txt, ns.font_style, &m_w, &m_h);
}
uint16_t TextNode::hotspot_y() {
	return m_font.ascent(m_s.font_style);
}
Texture* TextNode::render(TextureCache* texture_cache) {
	std::string temp_txt = i18n::make_ligatures(m_txt.c_str());
	m_s.direction = m_s.fontset->direction();
	if (m_s.fontset->direction() == UI::FontSet::Direction::kRightToLeft) {
		if (i18n::has_rtl_character(temp_txt.c_str())) {
			temp_txt = i18n::string2bidi(temp_txt.c_str());
		} else { // If a string only contains English characters, we render LTR anyway
			m_s.direction = UI::FontSet::Direction::kLeftToRight;
		}
	}

	const Texture& img = m_font.render(temp_txt, m_s.font_color, m_s.font_style, texture_cache);
	Texture* rv = new Texture(img.width(), img.height());
	blit(Rect(0, 0, img.width(), img.height()),
	     img,
	     Rect(0, 0, img.width(), img.height()),
	     1.,
	     BlendMode::Copy,
	     rv);
	return rv;
}

/*
 * Text that might need to expand to fill the space between other elements. One
 * example are ... in a table like construction.
 */
class FillingTextNode : public TextNode {
public:
	FillingTextNode(IFont& font, NodeStyle& ns, uint16_t w, const string& txt, bool expanding = false) :
		TextNode(font, ns, txt), m_expanding(expanding) {
			m_w = w;
		}
	virtual ~FillingTextNode() {}
	Texture* render(TextureCache*) override;

	bool is_expanding() override {return m_expanding;}
	void set_w(uint16_t w) override {m_w = w;}

private:
	bool m_expanding;
};
Texture* FillingTextNode::render(TextureCache* texture_cache) {
	const Texture& t = m_font.render(m_txt, m_s.font_color, m_s.font_style, texture_cache);
	Texture* rv = new Texture(m_w, m_h);
	for (uint16_t curx = 0; curx < m_w; curx += t.width()) {
		Rect srcrect(Point(0, 0), min<int>(t.width(), m_w - curx), m_h);
		blit(Rect(curx, 0, srcrect.w, srcrect.h), t, srcrect, 1., BlendMode::Copy, rv);
	}
	return rv;
}

/*
 * The whitespace between two words. There is a debug option to make it red and therefore
 * visible.
 */
class WordSpacerNode : public TextNode {
public:
	WordSpacerNode(IFont& font, NodeStyle& ns) : TextNode(font, ns, " ") {}
	static void show_spaces(bool t) {m_show_spaces = t;}

	Texture* render(TextureCache* texture_cache) override {
		if (m_show_spaces) {
			Texture* rv = new Texture(m_w, m_h);
			fill_rect(Rect(0, 0, m_w, m_h), RGBAColor(0xff, 0, 0, 0xff), rv);
			return rv;
		}
		return TextNode::render(texture_cache);
	}
	bool is_non_mandatory_space() override {return true;}

private:
	static bool m_show_spaces;
};
bool WordSpacerNode::m_show_spaces;

/*
 * This is a forced newline that can either be inside the text from the user or
 * is inserted by Layout whenever appropriate.
 */
class NewlineNode : public RenderNode {
public:
	NewlineNode(NodeStyle& ns) : RenderNode(ns) {}
	uint16_t height() override {return 0;}
	uint16_t width() override {return INFINITE_WIDTH; }
	uint16_t hotspot_y() override {return 0;}
	Texture* render(TextureCache* /* texture_cache */) override {
		assert(false);
		throw RenderError("This should never be called. This is a bug, please submit a report.");
	}
	bool is_non_mandatory_space() override {return true;}
};

/*
 * Arbitrary whitespace or a tiled image.
 */
class SpaceNode : public RenderNode {
public:
	SpaceNode(NodeStyle& ns, uint16_t w, uint16_t h = 0, bool expanding = false) :
		RenderNode(ns), m_w(w), m_h(h), m_bg(nullptr), m_expanding(expanding) {}

	uint16_t height() override {return m_h;}
	uint16_t width() override {return m_w;}
	uint16_t hotspot_y() override {return m_h;}
	Texture* render(TextureCache* /* texture_cache */) override {
		Texture* rv = new Texture(m_w, m_h);

		// Draw background image (tiling)
		if (m_bg) {
			Rect dst;
			Rect srcrect(Point(0, 0), 1, 1);
			for (uint16_t curx = 0; curx < m_w; curx += m_bg->width()) {
				dst.x = curx;
				dst.y = 0;
				srcrect.w = dst.w = min<int>(m_bg->width(), m_w - curx);
				srcrect.h = dst.h = m_h;
				blit(dst, *m_bg, srcrect, 1., BlendMode::Copy, rv);
			}
		} else {
			fill_rect(Rect(0, 0, m_w, m_h), RGBAColor(255, 255, 255, 0), rv);
		}
		return rv;
	}
	bool is_expanding() override {return m_expanding;}
	void set_w(uint16_t w) override {m_w = w;}

	void set_background(const Image* s) {
		m_bg = s; m_h = s->height();
	}

private:
	uint16_t m_w, m_h;
	const Image* m_bg;  // not owned
	bool m_expanding;
};

/*
 * This is a sub tag node. It is also the same as a full rich text render node.
 */
class SubTagRenderNode : public RenderNode {
public:
	SubTagRenderNode(NodeStyle& ns) : RenderNode(ns),
		m_bg_clr(0, 0, 0), m_bg_clr_set(false), m_bg_img(nullptr) {
	}
	virtual ~SubTagRenderNode() {
		for (RenderNode* n : m_nodes_to_render) {
			delete n;
		}
		m_nodes_to_render.clear();
	}

	uint16_t width() override {return m_w + m_margin.left + m_margin.right;}
	uint16_t height() override {return m_h + m_margin.top + m_margin.bottom;}
	uint16_t hotspot_y() override {return height();}
	Texture* render(TextureCache* texture_cache) override {
		Texture* rv = new Texture(width(), height());
		fill_rect(Rect(0, 0, rv->width(), rv->height()), RGBAColor(255, 255, 255, 0), rv);

		// Draw Solid background Color
		bool set_alpha = true;
		if (m_bg_clr_set) {
			fill_rect(Rect(Point(m_margin.left, m_margin.top), m_w, m_h), m_bg_clr, rv);
			set_alpha = false;
		}

		// Draw background image (tiling)
		if (m_bg_img) {
			Rect dst;
			Rect src(0, 0, 0, 0);

			for (uint16_t cury = m_margin.top; cury < m_h + m_margin.top; cury += m_bg_img->height()) {
				for (uint16_t curx = m_margin.left; curx < m_w + m_margin.left; curx += m_bg_img->width()) {
					dst.x = curx;
					dst.y = cury;
					src.w = dst.w = min<int>(m_bg_img->width(), m_w + m_margin.left - curx);
					src.h = dst.h = min<int>(m_bg_img->height(), m_h + m_margin.top - cury);
					blit(dst, *m_bg_img, src, 1., BlendMode::Copy, rv);
				}
			}
			set_alpha = false;
		}

		for (RenderNode* n : m_nodes_to_render) {
			Texture* node_texture = n->render(texture_cache);
			if (node_texture) {
				Rect dst = Rect(n->x() + m_margin.left,
				                n->y() + m_margin.top,
				                node_texture->width(),
				                node_texture->height());
				Rect src = Rect(0, 0, node_texture->width(), node_texture->height());

				blit(dst, *node_texture, src, 1., set_alpha ? BlendMode::Copy : BlendMode::UseAlpha, rv);
				delete node_texture;
			}
			delete n;
		}

		m_nodes_to_render.clear();

		return rv;
	}
	const vector<Reference> get_references() override {return m_refs;}
	void set_dimensions(uint16_t inner_w, uint16_t inner_h, Borders margin) {
		m_w = inner_w; m_h = inner_h; m_margin = margin;
	}
	void set_background(RGBColor clr) {
		m_bg_clr = clr;
		m_bg_clr_set = true;
	}
	void set_background(const Image* img) {m_bg_img = img;}
	void set_nodes_to_render(vector<RenderNode*>& n) {m_nodes_to_render = n;}
	void add_reference(int16_t gx, int16_t gy, uint16_t w, uint16_t h, const string& s) {
		Reference r = {Rect(gx, gy, w, h), s};
		m_refs.push_back(r);
	}

private:
	uint16_t m_w, m_h;
	vector<RenderNode*> m_nodes_to_render;
	Borders m_margin;
	RGBColor m_bg_clr;
	bool m_bg_clr_set;
	const Image* m_bg_img; // Not owned.
	vector<Reference> m_refs;
};

class ImgRenderNode : public RenderNode {
public:
	ImgRenderNode(NodeStyle& ns, const Image& image) : RenderNode(ns), m_image(image) {
	}

	uint16_t width() override {return m_image.width();}
	uint16_t height() override {return m_image.height();}
	uint16_t hotspot_y() override {return m_image.height();}
	Texture* render(TextureCache* texture_cache) override;

private:
	const Image& m_image;
};

Texture* ImgRenderNode::render(TextureCache* /* texture_cache */) {
	Texture* rv = new Texture(m_image.width(), m_image.height());
	blit(Rect(0, 0, m_image.width(), m_image.height()),
	         m_image,
	         Rect(0, 0, m_image.width(), m_image.height()),
				1.,
	         BlendMode::Copy, rv);
	return rv;
}
// End: Helper Stuff

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

	FontMap m_fontmap;

	DISALLOW_COPY_AND_ASSIGN(FontCache);
};

FontCache::~FontCache() {
	for (FontMap::reference& entry : m_fontmap) {
		delete entry.second;
	}
}

IFont& FontCache::get_font(NodeStyle* ns) {
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
	FontMap::iterator i = m_fontmap.find(fd);
	if (i != m_fontmap.end())
		return *i->second;

	std::unique_ptr<IFont> font;
	try {
		font.reset(load_font(ns->font_face, font_size));
	} catch (FileNotFoundError& e) {
		log("Font file not found. Falling back to serif: %s\n%s\n", ns->font_face.c_str(), e.what());
		font.reset(load_font(ns->fontset->serif(), font_size));
	}
	assert(font != nullptr);

	return *m_fontmap.insert(std::make_pair(fd, font.release())).first->second;
}

class TagHandler;
TagHandler* create_taghandler(Tag& tag, FontCache& fc, NodeStyle& ns, ImageCache* image_cache);

class TagHandler {
public:
	TagHandler(Tag& tag, FontCache& fc, NodeStyle ns, ImageCache* image_cache) :
		m_tag(tag), font_cache_(fc), m_ns(ns), image_cache_(image_cache) {}
	virtual ~TagHandler() {}

	virtual void enter() {}
	virtual void emit(vector<RenderNode*>&);

private:
	void m_make_text_nodes(const string& txt, vector<RenderNode*>& nodes, NodeStyle& ns);

protected:
	Tag& m_tag;
	FontCache& font_cache_;
	NodeStyle m_ns;
	ImageCache* image_cache_;  // Not owned
};

void TagHandler::m_make_text_nodes(const string& txt, vector<RenderNode*>& nodes, NodeStyle& ns) {
	TextStream ts(txt);
	std::string word;
	//log("Rendering %s\n", txt.c_str()); // NOCOM
	// NOCOM(GunChleoc): Too much space between text nodes.

	if (ns.fontset->direction() == UI::FontSet::Direction::kRightToLeft
		 && i18n::has_rtl_character(txt.c_str())) {
		std::string previous_word;
		std::vector<RenderNode*>::iterator it = nodes.begin();

		while (ts.pos() < txt.size()) {
			std::size_t cpos = ts.pos();
			ts.skip_ws();

			// We only know if the spacer goes to the left or right after having a look at the current word.
			bool add_spacer = ts.pos() != cpos;

			word = ts.till_any_or_end(" \t\n\r");
			if (!word.empty()) {
				if (i18n::has_rtl_character(word.c_str()) ||
					 i18n::has_rtl_character(previous_word.c_str())) {
					if (add_spacer) {
						//log("Insert spacer at front: %s\n", word.c_str()); // NOCOM
						it = nodes.insert(nodes.begin(), new WordSpacerNode(font_cache_.get_font(&ns), ns));
					}
					it = nodes.insert(nodes.begin(), new TextNode(font_cache_.get_font(&ns), ns, word));
				} else { // Sequences of Latin words go to the right from current position
					if (it < nodes.end()) {
						++it;
					}
					if (add_spacer) {
						//log("Insert spacer at back: %s\n", word.c_str()); // NOCOM
						it = nodes.insert(it, new WordSpacerNode(font_cache_.get_font(&ns), ns));
						if (it < nodes.end()) {
							++it;
						}
					}
					it = nodes.insert(it, new TextNode(font_cache_.get_font(&ns), ns, word));
				}
			}
			previous_word = word;
		}
	} else {  // LTR
		while (ts.pos() < txt.size()) {
			std::size_t cpos = ts.pos();
			ts.skip_ws();
			if (ts.pos() != cpos) {
				nodes.push_back(new WordSpacerNode(font_cache_.get_font(&ns), ns));
			}
			word = ts.till_any_or_end(" \t\n\r");
			if (!word.empty()) {
				nodes.push_back(new TextNode(font_cache_.get_font(&ns), ns, word));
			}
		}
	}
}

void TagHandler::emit(vector<RenderNode*>& nodes) {
	for (Child* c : m_tag.childs()) {
		if (c->tag) {
			std::unique_ptr<TagHandler> th(create_taghandler(*c->tag, font_cache_, m_ns, image_cache_));
			th->enter();
			th->emit(nodes);
		} else
			m_make_text_nodes(c->text, nodes, m_ns);
	}
}

class FontTagHandler : public TagHandler {
public:
	FontTagHandler(Tag& tag, FontCache& fc, NodeStyle ns, ImageCache* image_cache)
		: TagHandler(tag, fc, ns, image_cache) {}

	void enter() override {
		const AttrMap& a = m_tag.attrs();
		if (a.has("color")) m_ns.font_color = a["color"].get_color();
		if (a.has("size")) m_ns.font_size = a["size"].get_int();
		if (a.has("face")) m_ns.font_face = a["face"].get_string();
		if (a.has("bold")) m_ns.font_style |= a["bold"].get_bool() ? IFont::BOLD : 0;
		if (a.has("italic")) m_ns.font_style |= a["italic"].get_bool() ? IFont::ITALIC : 0;
		if (a.has("underline")) m_ns.font_style |= a["underline"].get_bool() ? IFont::UNDERLINE : 0;
		if (a.has("shadow")) m_ns.font_style |= a["shadow"].get_bool() ? IFont::SHADOW : 0;
		if (a.has("ref")) m_ns.reference = a["ref"].get_string();
	}
};

class PTagHandler : public TagHandler {
public:
	PTagHandler(Tag& tag, FontCache& fc, NodeStyle ns, ImageCache* image_cache)
		: TagHandler(tag, fc, ns, image_cache), m_indent(0) {
	}

	void enter() override {
		const AttrMap& a = m_tag.attrs();
		if (a.has("indent")) m_indent = a["indent"].get_int();
		if (a.has("align")) {
			const std::string align = a["align"].get_string();
			if (align == "right") {
				if (m_ns.direction == UI::FontSet::Direction::kLeftToRight) {
					m_ns.halign = HALIGN_RIGHT;
				} else {
					m_ns.halign = HALIGN_LEFT;
				}
			} else if (align == "center" || align == "middle") {
				m_ns.halign = HALIGN_CENTER;
			} else {
				if (m_ns.direction == UI::FontSet::Direction::kLeftToRight) {
					m_ns.halign = HALIGN_LEFT;
				} else {
					m_ns.halign = HALIGN_RIGHT;
				}
			}
		}
		if (a.has("valign")) {
			const string align = a["valign"].get_string();
			if (align == "top") m_ns.valign = VALIGN_TOP;
			else if (align == "bottom") m_ns.valign = VALIGN_BOTTOM;
			else if (align == "center" || align == "middle") m_ns.valign = VALIGN_CENTER;
		}
		if (a.has("spacing"))
			m_ns.spacing = a["spacing"].get_int();
	}
	void emit(vector<RenderNode*>& nodes) override {
		if (m_indent) {
			nodes.push_back(new SpaceNode(m_ns, m_indent));
		}
		TagHandler::emit(nodes);

		nodes.push_back(new NewlineNode(m_ns));
	}

private:
	uint16_t m_indent;
};

class ImgTagHandler : public TagHandler {
public:
	ImgTagHandler(Tag& tag, FontCache& fc, NodeStyle ns, ImageCache* image_cache) :
		TagHandler(tag, fc, ns, image_cache), m_rn(nullptr) {
	}

	void enter() override {
		const AttrMap& a = m_tag.attrs();
		m_rn = new ImgRenderNode(m_ns, *image_cache_->get(a["src"].get_string()));
	}
	void emit(vector<RenderNode*>& nodes) override {
		nodes.push_back(m_rn);
	}

private:
	ImgRenderNode* m_rn;
};

class VspaceTagHandler : public TagHandler {
public:
	VspaceTagHandler(Tag& tag, FontCache& fc, NodeStyle ns, ImageCache* image_cache) :
		TagHandler(tag, fc, ns, image_cache), m_space(0) {}

	void enter() override {
		const AttrMap& a = m_tag.attrs();

		m_space = a["gap"].get_int();
	}
	void emit(vector<RenderNode*>& nodes) override {
		nodes.push_back(new SpaceNode(m_ns, 0, m_space));
		nodes.push_back(new NewlineNode(m_ns));
	}

private:
	uint16_t m_space;
};

class HspaceTagHandler : public TagHandler {
public:
	HspaceTagHandler(Tag& tag, FontCache& fc, NodeStyle ns, ImageCache* image_cache) :
		TagHandler(tag, fc, ns, image_cache), m_bg(nullptr), m_space(0) {}

	void enter() override {
		const AttrMap& a = m_tag.attrs();

		if (a.has("gap"))
			m_space = a["gap"].get_int();
		else
			m_space = INFINITE_WIDTH;

		if (a.has("fill")) {
			m_fill_text = a["fill"].get_string();
			try {
				m_bg = image_cache_->get(m_fill_text);
				m_fill_text = "";
			}
			catch (ImageNotFound&) {
			}
		}
	}

	void emit(vector<RenderNode*>& nodes) override {
		RenderNode* rn = nullptr;
		if (!m_fill_text.empty()) {
			if (m_space < INFINITE_WIDTH)
				rn = new FillingTextNode(font_cache_.get_font(&m_ns), m_ns, m_space, m_fill_text);
			else
				rn = new FillingTextNode(font_cache_.get_font(&m_ns), m_ns, 0, m_fill_text, true);
		} else {
			SpaceNode* sn;
			if (m_space < INFINITE_WIDTH)
				sn = new SpaceNode(m_ns, m_space, 0);
			else
				sn = new SpaceNode(m_ns, 0, 0, true);

			if (m_bg)
				sn->set_background(m_bg);
			rn = sn;
		}
		nodes.push_back(rn);
	}

private:
	string m_fill_text;
	const Image* m_bg;
	uint16_t m_space;
};

class BrTagHandler : public TagHandler {
public:
	BrTagHandler(Tag& tag, FontCache& fc, NodeStyle ns, ImageCache* image_cache) :
		TagHandler(tag, fc, ns, image_cache) {
	}

	void emit(vector<RenderNode*>& nodes) override {
		nodes.push_back(new NewlineNode(m_ns));
	}
};


class SubTagHandler : public TagHandler {
public:
	SubTagHandler
		(Tag& tag, FontCache& fc, NodeStyle ns, ImageCache* image_cache,
		 uint16_t max_w = 0, bool shrink_to_fit = false)
		:
			TagHandler(tag, fc, ns, image_cache),
			shrink_to_fit_(shrink_to_fit),
			m_w(max_w),
			m_rn(new SubTagRenderNode(ns))
	{
	}

	void enter() override {
		Borders padding, margin;

		handle_unique_attributes();
		const AttrMap& a = m_tag.attrs();
		if (a.has("background")) {
			RGBColor clr;
			try {
				clr = a["background"].get_color();
				m_rn->set_background(clr);
			} catch (InvalidColor&) {
				m_rn->set_background(image_cache_->get(a["background"].get_string()));
			}
		}
		if (a.has("padding")) {
			uint8_t p = a["padding"].get_int();
			padding.left = padding.top = padding.right = padding.bottom = p;
		}
		if (a.has("padding_r")) padding.right = a["padding_r"].get_int();
		if (a.has("padding_b")) padding.bottom = a["padding_b"].get_int();
		if (a.has("padding_l")) padding.left = a["padding_l"].get_int();
		if (a.has("padding_t")) padding.top = a["padding_t"].get_int();
		if (a.has("margin")) {
			uint8_t p = a["margin"].get_int();
			margin.left = margin.top = margin.right = margin.bottom = p;
		}

		vector<RenderNode*> subnodes;
		TagHandler::emit(subnodes);

		if (! m_w) { // Determine the width by the width of the widest subnode
			for (RenderNode* n : subnodes) {
				if (n->width() >= INFINITE_WIDTH)
					continue;
				m_w = max<int>(m_w, n->width() + padding.left + padding.right);
			}
		}

		// Layout takes ownership of subnodes
		Layout layout(subnodes);
		vector<RenderNode*> nodes_to_render;
		uint16_t max_line_width = layout.fit_nodes(nodes_to_render, m_w, padding);
		if (shrink_to_fit_) {
			m_w = min(m_w, max_line_width);
		}

		// Collect all tags from children
		for (RenderNode* rn : nodes_to_render) {
			for (const Reference& r : rn->get_references()) {
			m_rn->add_reference(rn->x() + r.dim.x, rn->y() + r.dim.y, r.dim.w, r.dim.h, r.ref);
		}
	}

		m_rn->set_dimensions(m_w, layout.height(), margin);
		m_rn->set_nodes_to_render(nodes_to_render);
	}
	void emit(vector<RenderNode*>& nodes) override {
		nodes.push_back(m_rn);
	}

	// Handle attributes that are in sub, but not in rt.
	virtual void handle_unique_attributes() {
		const AttrMap& a = m_tag.attrs();
		if (a.has("width")) {
			m_w = a["width"].get_int();
			shrink_to_fit_ = false;
		}
		if (a.has("float")) {
			const string s = a["float"].get_string();
			if (s == "right") m_rn->set_floating(RenderNode::FLOAT_RIGHT);
			else if (s == "left") m_rn->set_floating(RenderNode::FLOAT_LEFT);
		}
		if (a.has("valign")) {
			const string align = a["valign"].get_string();
			if (align == "top") m_rn->set_valign(VALIGN_TOP);
			else if (align == "bottom") m_rn->set_valign(VALIGN_BOTTOM);
			else if (align == "center" || align == "middle") m_rn->set_valign(VALIGN_CENTER);
		}
	}

private:
	bool shrink_to_fit_;
	uint16_t m_w;
	SubTagRenderNode* m_rn;
};

class RTTagHandler : public SubTagHandler {
public:
	RTTagHandler(Tag& tag, FontCache& fc, NodeStyle ns, ImageCache* image_cache, uint16_t w) :
		SubTagHandler(tag, fc, ns, image_cache, w, true) {
	}

	// Handle attributes that are in rt, but not in sub.
	void handle_unique_attributes() override {
		const AttrMap& a = m_tag.attrs();
		WordSpacerNode::show_spaces(a.has("db_show_spaces") ? a["db_show_spaces"].get_bool() : 0);
	}
};

template<typename T> TagHandler* create_taghandler
	(Tag& tag, FontCache& fc, NodeStyle& ns, ImageCache* image_cache)
{
	return new T(tag, fc, ns, image_cache);
}
using TagHandlerMap = map<const string, TagHandler* (*)
	(Tag& tag, FontCache& fc, NodeStyle& ns, ImageCache* image_cache)>;

TagHandler* create_taghandler(Tag& tag, FontCache& fc, NodeStyle& ns, ImageCache* image_cache) {
	static TagHandlerMap map;
	if (map.empty()) {
		map["br"] = &create_taghandler<BrTagHandler>;
		map["font"] = &create_taghandler<FontTagHandler>;
		map["sub"] = &create_taghandler<SubTagHandler>;
		map["p"] = &create_taghandler<PTagHandler>;
		map["img"] = &create_taghandler<ImgTagHandler>;
		map["vspace"] = &create_taghandler<VspaceTagHandler>;
		map["space"] = &create_taghandler<HspaceTagHandler>;
	}
	TagHandlerMap::iterator i = map.find(tag.name());
	if (i == map.end())
		throw RenderError
			((boost::format("No Tag handler for %s. This is a bug, please submit a report.")
			  % tag.name()).str());
	return i->second(tag, fc, ns, image_cache);
}

Renderer::Renderer(ImageCache* image_cache, TextureCache* texture_cache, UI::FontSet* fontset) :
	font_cache_(new FontCache()), parser_(new Parser()),
	image_cache_(image_cache), texture_cache_(texture_cache), fontset_(fontset) {

	// NOCOM testing BIDI
	//i18n::string2bidi(i18n::make_ligatures("1 العربية - حاله اللعبه2\n23Català,456\n.עברית:789日本語12\n3ქართული4"
	//				"\n56한국어78\n9मराठी12\n3Lietuvių4\n56မြန်မာ7\n89Русский123සිංහල45\n6Tiếng Việt789"));
}

Renderer::~Renderer() {
}

RenderNode* Renderer::layout_(const string& text, uint16_t width, const TagSet& allowed_tags) {
	std::unique_ptr<Tag> rt(parser_->parse(text, allowed_tags));

	NodeStyle default_style = {
		fontset_, fontset_->serif(), 16,
		RGBColor(0, 0, 0), IFont::DEFAULT, fontset_->direction(), 0, HALIGN_LEFT, VALIGN_BOTTOM,
		""
	};

	if (!width)
		width = INFINITE_WIDTH;

	RTTagHandler rtrn(*rt, *font_cache_, default_style, image_cache_, width);
	vector<RenderNode*> nodes;
	rtrn.enter();
	rtrn.emit(nodes);

	assert(nodes.size() == 1);
	assert(nodes[0]);
	return nodes[0];
}

Texture* Renderer::render(const string& text, uint16_t width, const TagSet& allowed_tags) {
	std::unique_ptr<RenderNode> node(layout_(text, width, allowed_tags));

	return node->render(texture_cache_);
}

IRefMap* Renderer::make_reference_map(const string& text, uint16_t width, const TagSet& allowed_tags) {
	std::unique_ptr<RenderNode> node(layout_(text, width, allowed_tags));
	return new RefMap(node->get_references());
}

}
