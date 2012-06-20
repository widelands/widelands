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

#include <string>
#include <queue>
#include <vector>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <SDL.h>

#include "rt_render.h"
#include "rt_parse.h"
#include "textstream.h"

using namespace std;
using namespace boost;

#define foreach BOOST_FOREACH

namespace RT {

static const uint32_t INFINITE_WIDTH = 2147483647; // 2^31-1

// Helper Stuff {{{
SDL_Surface * m_make_surface(int32_t w, int32_t h) {
	 SDL_Surface *surface;
	 Uint32 rmask, gmask, bmask, amask;
	 /* SDL interprets each pixel as a 32-bit number, so our masks must depend
		 on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	 rmask = 0xff000000;
	 gmask = 0x00ff0000;
	 bmask = 0x0000ff00;
	 amask = 0x000000ff;
#else
	 rmask = 0x000000ff;
	 gmask = 0x0000ff00;
	 bmask = 0x00ff0000;
	 amask = 0xff000000;
#endif

	 surface = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32,
											  rmask, gmask, bmask, amask);
	 if (!surface)
		 throw RenderError((format("Was unable to create a Surface: %s") % SDL_GetError()).str());

	 return surface;
}

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
	Borders() {left=top=right=bottom=0;}
	uint8_t left, top, right, bottom;
};

struct NodeStyle {
	string font_face;
	uint16_t font_size;
	SDL_Color font_color;
	int font_style;

	uint8_t spacing;
	HAlign halign;
	VAlign valign;
	string reference;
};

struct Reference {
	SDL_Rect dim;
	string ref;

	inline bool contains(int16_t x, int16_t y) const {
		if (dim.x <= x and x <= dim.x + dim.w and
			 dim.y <= y and y <= dim.y + dim.h)
			return true;
		return false;
	}
};

class RefMap : public IRefMap {
public:
	RefMap(const vector<Reference> & refs) : m_refs(refs) {}
	string query(int16_t x, int16_t y) {
		// Should this linear algorithm proof to be too slow (doubtful), the
		// RefMap could also be efficiently implemented using an R-Tree
		foreach(const Reference & c, m_refs)
			if (c.contains(x, y))
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
	RenderNode(NodeStyle & ns) : m_floating(NO_FLOAT), m_halign(ns.halign), m_valign(ns.valign), m_x(0), m_y(0) {}
	virtual ~RenderNode() {};

	virtual uint32_t width() = 0;
	virtual uint32_t height() = 0;
	virtual uint32_t hotspot_y() = 0;
	virtual SDL_Surface * render() = 0;

	virtual bool is_non_mandatory_space() {return false;}
	virtual bool is_expanding() {return false;}
	virtual void set_w(uint32_t) {} // Only, when expanding

	virtual const vector<Reference> get_references() {return vector<Reference>();}

	Floating get_floating() {return m_floating;}
	void set_floating(Floating f) {m_floating = f;}
	HAlign halign() {return m_halign;}
	void set_halign(HAlign halign) {m_halign = halign;}
	VAlign valign() {return m_valign;}
	void set_valign(VAlign valign) {m_valign = valign;}
	void set_x(uint32_t nx) {m_x=nx;}
	void set_y(uint32_t ny) {m_y=ny;}
	uint32_t x() {return m_x;}
	uint32_t y() {return m_y;}

private:
	Floating m_floating;
	HAlign m_halign;
	VAlign m_valign;
	uint32_t m_x, m_y;
};

class Layout {
public:
	Layout(vector<RenderNode*> & all) : m_h(0), m_idx(0), m_all_nodes(all) {}
	virtual ~Layout() {}

	uint32_t height() {return m_h;}

	void fit_nodes(vector<RenderNode*> & rv, uint32_t w, Borders p);

private:
	struct ConstraintChange {
		uint32_t at_y;
		int32_t delta_w;
		int32_t delta_offset_x;

		bool operator<(const ConstraintChange & o) const {
			return at_y > o.at_y || (at_y == o.at_y && delta_w > o.delta_w);
		}
	};

	uint32_t m_fit_line(vector<RenderNode*> & rv, uint32_t w, const Borders &);

	uint32_t m_h;
	size_t m_idx;
	vector<RenderNode *> & m_all_nodes;
	priority_queue<ConstraintChange> m_constraint_changes;
};
uint32_t Layout::m_fit_line(vector<RenderNode*> & rv, uint32_t w, const Borders & p) {
	while (m_idx < m_all_nodes.size() and m_all_nodes[m_idx]->is_non_mandatory_space())
		delete m_all_nodes[m_idx++];

	uint32_t x = p.left;
	size_t first_idx = rv.size();
	while (m_idx < m_all_nodes.size()) {
		RenderNode * n = m_all_nodes[m_idx];
		uint32_t nw = n->width();
		if (x + nw + p.right > w or n->get_floating())
			break;

		n->set_x(x); x += nw;
		rv.push_back(n);
		++m_idx;
	}
	if (not rv.empty() and rv.back()->is_non_mandatory_space()) {
		x -= rv.back()->width();
		delete rv.back();
		rv.pop_back();
	}

	// Remaining space in this line
	uint32_t rem_space = w - p.right - x;

	// Find expanding nodes
	vector<size_t> expanding_nodes;
	for (size_t idx = first_idx; idx < rv.size(); ++idx)
		if (rv[idx]->is_expanding())
			expanding_nodes.push_back(idx);

	if (expanding_nodes.size()) { // If there are expanding nodes, we fill the space
		uint32_t individual_w = rem_space / expanding_nodes.size();
		foreach(size_t idx, expanding_nodes) {
			rv[idx]->set_w(individual_w);
			for (size_t nidx = idx + 1; nidx < rv.size(); ++nidx)
				rv[nidx]->set_x(rv[nidx]->x() + individual_w);
		}
	} else {
		// Take last elements style in this line and check horizontal alignement
		if (not rv.empty() and rv[rv.size()-1]->halign() != HALIGN_LEFT) {
			if (rv[rv.size()-1]->halign() == HALIGN_CENTER)
				rem_space /= 2; // Otherwise, we align right
			for (size_t idx = first_idx; idx < rv.size(); ++idx)
				rv[idx]->set_x(rv[idx]->x() + rem_space);
		}
	}

	// Find the biggest hotspot of the truly remaining items.
	uint32_t cur_line_hotspot = 0;
	for (size_t idx = first_idx; idx < rv.size(); ++idx)
		cur_line_hotspot = max(cur_line_hotspot, rv[idx]->hotspot_y());

	return cur_line_hotspot;
}
/*
 * Take ownership of all nodes, delete those that we do not render anyways (for
 * example unneeded spaces), append the rest to the vector
 * we got
 */
void Layout::fit_nodes(vector<RenderNode*> & rv, uint32_t w, Borders p) {
	m_h = p.top;

	while (m_idx < m_all_nodes.size()) {
		size_t first_idx = rv.size();
		uint32_t biggest_hotspot = m_fit_line(rv, w, p);
		uint32_t line_height = 0;
		for (size_t j = first_idx; j < rv.size(); ++j) {
			RenderNode * n = rv[j];
			line_height = max(line_height, biggest_hotspot - n->hotspot_y() + n->height());
			n->set_y(m_h + biggest_hotspot - n->hotspot_y());
		}

		// Go over again and adjust position for VALIGN
		for (size_t j = first_idx; j < rv.size(); ++j) {
			uint32_t space = line_height - rv[j]->height();
			if (!space or rv[j]->valign() == VALIGN_BOTTOM)
				continue;
			if (rv[j]->valign() == VALIGN_CENTER)
				space /= 2;
			rv[j]->set_y(rv[j]->y() - space);
		}

		m_h += line_height;
		while (not m_constraint_changes.empty() and m_constraint_changes.top().at_y <= m_h) {
			const ConstraintChange & top = m_constraint_changes.top();
			w += top.delta_w;
			p.left += top.delta_offset_x;
			m_constraint_changes.pop();
		}

		if ((m_idx < m_all_nodes.size()) and m_all_nodes[m_idx]->get_floating()) {
			RenderNode * n = m_all_nodes[m_idx];
			n->set_y(m_h);
			ConstraintChange cc = { m_h + n->height(), 0, 0 };
			if (n->get_floating() == RenderNode::FLOAT_LEFT) {
				n->set_x(p.left);
				p.left += n->width();
				cc.delta_offset_x = -n->width();
			} else {
				n->set_x(w - n->width());
				w -= n->width();
				cc.delta_w = n->width();
			}
			m_constraint_changes.push(cc);
			rv.push_back(n);
			++m_idx;
		}
		if (m_idx == first_idx)
			throw WidthTooSmall("Could not fit a single render node in line. Width of an Element is too small!");
	}

	m_h += p.bottom;
}

class TextNode : public RenderNode {
public:
	TextNode(IFont & font, NodeStyle &, string txt);
	virtual ~TextNode() {};

	virtual uint32_t width() {return m_w;}
	virtual uint32_t height() {return m_h + m_s.spacing;}
	virtual uint32_t hotspot_y();
	virtual const vector<Reference> get_references() {
		vector<Reference> rv;
		if (!m_s.reference.empty()) {
			Reference r = { {0, 0, m_w, m_h}, m_s.reference};
			rv.push_back(r);
		}
		return rv;
	}

	virtual SDL_Surface * render();

protected:
	uint32_t m_w, m_h;
	string m_txt;
	NodeStyle m_s;
	IFont & m_font;
};

TextNode::TextNode(IFont & font, NodeStyle & ns, string txt)
	: RenderNode(ns), m_txt(txt), m_s(ns), m_font(font)
{
	m_font.dimensions(m_txt, ns.font_style, &m_w, &m_h);
}
uint32_t TextNode::hotspot_y() {
	return m_font.ascent();
}
SDL_Surface * TextNode::render() {
	return m_font.render(m_txt, m_s.font_color, m_s.font_style);
}

class FillingTextNode : public TextNode {
public:
	FillingTextNode(IFont & font, NodeStyle & ns, uint32_t w, string txt, bool expanding = false) :
		TextNode(font, ns, txt), m_expanding(expanding) {
			m_w = w;
		};
	virtual ~FillingTextNode() {};
	virtual SDL_Surface * render();

	virtual bool is_expanding() {return m_expanding;}
	virtual void set_w(uint32_t w) {m_w = w;}

private:
	bool m_expanding;
};
SDL_Surface * FillingTextNode::render() {
	SDL_Surface * t = m_font.render(m_txt, m_s.font_color, m_s.font_style);
	SDL_Surface * rv = m_make_surface(m_w, m_h);
	SDL_SetAlpha(t, 0, SDL_ALPHA_OPAQUE);
	for (uint32_t x = 0; x < m_w; x += t->w) {
		SDL_Rect dstrect = { x, 0, 0, 0 };
		SDL_Rect srcrect = {
			0, 0,
			min(static_cast<uint32_t>(t->w), m_w - x),
			m_h
		};
		SDL_BlitSurface(t, &srcrect, rv, &dstrect);
	}
	SDL_FreeSurface(t);
	return rv;
}


class WordSpacerNode : public TextNode {
public:
	WordSpacerNode(IFont & font, NodeStyle & ns) : TextNode(font, ns, " ") {}
	static void show_spaces(bool t) {m_show_spaces = t;}

	virtual SDL_Surface * render() {
		SDL_Surface * sur = TextNode::render();
		if (m_show_spaces)
			SDL_FillRect(sur, 0, SDL_MapRGBA(sur->format, 0xff, 0, 0, SDL_ALPHA_OPAQUE));
		return sur;
	}
	virtual bool is_non_mandatory_space() {return true;}

private:
	static bool m_show_spaces;
};
bool WordSpacerNode::m_show_spaces;

class NewlineNode : public RenderNode {
public:
	NewlineNode(NodeStyle & ns) : RenderNode(ns) {}
	virtual uint32_t height() {return 0;}
	virtual uint32_t width() {return INFINITE_WIDTH; }
	virtual uint32_t hotspot_y() {return 0;}
	virtual SDL_Surface * render() {
		assert(0); // This should never be called
	}
	virtual bool is_non_mandatory_space() {return true;}

private:
	uint32_t m_w;
};

class SpaceNode : public RenderNode {
public:
	SpaceNode(NodeStyle & ns, uint32_t w, uint32_t h = 0, bool expanding = false) :
		RenderNode(ns), m_w(w), m_h(h), m_bg(0), m_expanding(expanding) {}

	virtual uint32_t height() {return m_h;}
	virtual uint32_t width() {return m_w;}
	virtual uint32_t hotspot_y() {return m_h;}
	virtual SDL_Surface * render() {
		SDL_Surface * rv = m_make_surface(m_w, m_h);

		// Draw background image (tiling)
		if (m_bg) {
			SDL_Rect dstrect;
			SDL_Rect srcrect = {0, 0, 0, 0};
			SDL_SetAlpha(m_bg, 0, SDL_ALPHA_OPAQUE);
			for (uint32_t x = 0; x < m_w; x += m_bg->w) {
				dstrect.x = x;
				dstrect.y = 0;
				srcrect.w = min(static_cast<uint32_t>(m_bg->w), m_w - x);
				srcrect.h = m_h;
				SDL_BlitSurface(m_bg, &srcrect, rv, &dstrect);
			}
		}
		return rv;
	}
	virtual bool is_expanding() {return m_expanding;}
	virtual void set_w(uint32_t w) {m_w = w;}

	void set_background(SDL_Surface * s) {
		m_bg = s; m_h = s->h;
	}

private:
	uint32_t m_w, m_h;
	SDL_Surface * m_bg;
	bool m_expanding;
};

class SubTagRenderNode : public RenderNode {
public:
	SubTagRenderNode(NodeStyle & ns) : RenderNode(ns), m_bg_img(0) {
			m_bg_clr.r = m_bg_clr.g = m_bg_clr.b = m_bg_clr.unused = 0;
	}

	virtual uint32_t width() {return m_w + m_margin.left + m_margin.right;}
	virtual uint32_t height() {return m_h + m_margin.top + m_margin.bottom;}
	virtual uint32_t hotspot_y() {return height();}
	virtual SDL_Surface * render() {
		SDL_Surface * rv = m_make_surface(width(), height());

		// Draw Solid background Color
		bool set_alpha = true;
		if (m_bg_clr.r or m_bg_clr.g or m_bg_clr.b or m_bg_clr.unused) {
			SDL_Rect fill_rect = { m_margin.left, m_margin.top, m_w, m_h };
			SDL_FillRect(rv, &fill_rect, SDL_MapRGBA(rv->format, m_bg_clr.r, m_bg_clr.g, m_bg_clr.b,
						m_bg_clr.unused));
			set_alpha = false;
		}

		// Draw background image (tiling)
		if (m_bg_img) {
			SDL_Rect dstrect;
			SDL_Rect srcrect = {0, 0, 0, 0};
			SDL_SetAlpha(m_bg_img, 0, SDL_ALPHA_OPAQUE);
			for (uint32_t y = m_margin.top; y < m_h + m_margin.top; y += m_bg_img->h) {
				for (uint32_t x = m_margin.left; x < m_w + m_margin.left; x += m_bg_img->w) {
					dstrect.x = x;
					dstrect.y = y;
					srcrect.w = min(static_cast<uint32_t>(m_bg_img->w), m_w + m_margin.left - x);
					srcrect.h = min(static_cast<uint32_t>(m_bg_img->h), m_h + m_margin.top - y);
					SDL_BlitSurface(m_bg_img, &srcrect, rv, &dstrect);
				}
			}
			set_alpha = false;
		}

		foreach(RenderNode * n, m_nodes_to_render) {
			SDL_Surface * nsur = n->render();
			if (set_alpha)
				SDL_SetAlpha(nsur, 0, SDL_ALPHA_OPAQUE);
			SDL_Rect dstrect = { n->x() + m_margin.left, n->y() + m_margin.top, 0, 0 };
			SDL_BlitSurface(nsur, 0, rv,  &dstrect);
			SDL_FreeSurface(nsur);
			delete n;
		}

		m_nodes_to_render.clear();

		return rv;
	}
	virtual const vector<Reference> get_references() {return m_refs;}
	void set_dimensions(uint32_t inner_w, uint32_t inner_h, Borders margin) {
		m_w=inner_w; m_h=inner_h; m_margin= margin;
	}
	void set_background(SDL_Color clr) {m_bg_clr = clr;}
	void set_background(SDL_Surface * img) {m_bg_img = img;}
	void set_nodes_to_render(vector<RenderNode*> & n) {m_nodes_to_render=n;}
	void add_reference(int16_t x, int16_t y, uint16_t w, uint16_t h, string s) {
		Reference r = { {x, y, w, h}, s };
		m_refs.push_back(r);
	}

private:
	uint32_t m_w, m_h;
	vector<RenderNode*> m_nodes_to_render;
	Borders m_margin;
	SDL_Color m_bg_clr;
	SDL_Surface * m_bg_img; // Owned by the image loader
	vector<Reference> m_refs;
};

class ImgRenderNode : public RenderNode {
public:
	ImgRenderNode(NodeStyle & ns, SDL_Surface * sur) : RenderNode(ns), m_sur(sur) {
	}

	virtual uint32_t width() {return m_sur->w;}
	virtual uint32_t height() {return m_sur->h;}
	virtual uint32_t hotspot_y() {return m_sur->h;}
	virtual SDL_Surface * render() {
		m_sur->refcount++; // Keep this surface around
		return m_sur;
	}

private:
	SDL_Surface * m_sur; // Note: this is owned by the image loader.
};

// End: Helper Stuff }}}

class FontCache {
public:
	FontCache(IFontLoader * fl) : m_fl(fl) {}
	virtual ~FontCache() {
		foreach(FontMapPair & pair, m_fontmap)
			delete pair.second;
		m_fontmap.clear();
		delete m_fl;
	}

	IFont & get_font(NodeStyle & style);

private:
	struct FontDescr {
		std::string face;
		uint16_t size;

		bool operator<(const FontDescr & o) const {
			return
				size < o.size ||
				(size == o.size && face < o.face);
		}
	};
	typedef std::map<FontDescr, IFont *> FontMap;
	typedef std::pair<const FontDescr, IFont *> FontMapPair;

	FontMap m_fontmap;
	IFontLoader * m_fl;
};
IFont & FontCache::get_font(NodeStyle & ns) {
	FontDescr fd = { ns.font_face, ns.font_size };
	FontMap::iterator i = m_fontmap.find(fd);
	if (i != m_fontmap.end())
		return *i->second;

	IFont * font = m_fl->load(ns.font_face, ns.font_size);
	m_fontmap[fd] = font;
	return *font;
}


class TagHandler;
TagHandler * create_taghandler(ITag & tag, FontCache & fc, NodeStyle & ns, IImageLoader & imgl);

class TagHandler {
public:
	TagHandler(ITag & tag, FontCache & fc, NodeStyle ns, IImageLoader & imgl) :
		m_tag(tag), m_fc(fc), m_ns(ns), m_imgl(imgl) {}
	virtual ~TagHandler() {};

	virtual void enter() {};
	virtual void emit(vector<RenderNode*> &);

private:
	void m_make_text_nodes(string txt, vector<RenderNode*> & nodes, NodeStyle & ns);

protected:
	ITag & m_tag;
	FontCache & m_fc;
	NodeStyle m_ns;
	IImageLoader & m_imgl;
};

void TagHandler::m_make_text_nodes(string txt, vector<RenderNode*> & nodes, NodeStyle & ns) {
	TextStream ts(txt);

	vector<string> words;
	while (ts.pos() < txt.size()) {
		size_t cpos = ts.pos();
		ts.skip_ws();
		if (ts.pos() != cpos)
			nodes.push_back(new WordSpacerNode(m_fc.get_font(ns), ns));
		string word = ts.till_any_or_end(" \t\n\r");
		if (word.size())
			nodes.push_back(new TextNode(m_fc.get_font(ns), ns, word));
	}
}

void TagHandler::emit(vector<RenderNode*> & nodes) {
	foreach(Child * c, m_tag.childs()) {
		if (c->tag) {
			TagHandler * th = create_taghandler(*c->tag, m_fc, m_ns, m_imgl);
			th->enter();
			th->emit(nodes);
			delete th;
		} else
			m_make_text_nodes(c->text, nodes, m_ns);
	}
}

class FontTagHandler : public TagHandler {
public:
	FontTagHandler(ITag & tag, FontCache & fc, NodeStyle ns, IImageLoader & imgl) : TagHandler(tag, fc, ns, imgl) {}

	void enter() {
		const IAttrMap & a = m_tag.attrs();
		if (a.has("color")) a["color"].get_color(&m_ns.font_color);
		if (a.has("size")) m_ns.font_size = a["size"].get_int();
		if (a.has("face")) m_ns.font_face = a["face"].get_string();
		if (a.has("bold")) m_ns.font_style |= a["bold"].get_bool() ? IFont::BOLD : 0;
		if (a.has("italic")) m_ns.font_style |= a["italic"].get_bool() ? IFont::ITALIC : 0;
		if (a.has("underline")) m_ns.font_style |= a["underline"].get_bool() ? IFont::UNDERLINE : 0;
		if (a.has("ref")) m_ns.reference = a["ref"].get_string();
	}
};

class PTagHandler : public TagHandler {
public:
	PTagHandler(ITag & tag, FontCache & fc, NodeStyle ns, IImageLoader & imgl)
		: TagHandler(tag, fc, ns, imgl), m_indent(0) {
	}

	void enter() {
		const IAttrMap & a = m_tag.attrs();
		if (a.has("indent")) m_indent = a["indent"].get_int();
		if (a.has("align")) {
			string align = a["align"].get_string();
			if (align=="left") m_ns.halign = HALIGN_LEFT;
			else if (align=="right") m_ns.halign = HALIGN_RIGHT;
			else if (align=="center" or align=="middle") m_ns.halign = HALIGN_CENTER;
		}
		if (a.has("valign")) {
			string align = a["valign"].get_string();
			if (align=="top") m_ns.valign = VALIGN_TOP;
			else if (align=="bottom") m_ns.valign = VALIGN_BOTTOM;
			else if (align=="center" or align=="middle") m_ns.valign = VALIGN_CENTER;
		}
		if (a.has("spacing"))
			m_ns.spacing = a["spacing"].get_int();
	}
	void emit(vector<RenderNode*> & nodes) {
		if (m_indent)
			nodes.push_back(new SpaceNode(m_ns, m_indent));
		TagHandler::emit(nodes);

		nodes.push_back(new NewlineNode(m_ns));
	}

private:
	uint32_t m_indent;
};

class ImgTagHandler : public TagHandler {
public:
	ImgTagHandler(ITag & tag, FontCache & fc, NodeStyle ns, IImageLoader & imgl) :
		TagHandler(tag, fc, ns, imgl) {
	}

	void enter() {
		const IAttrMap & a = m_tag.attrs();
		m_rn = new ImgRenderNode(m_ns, m_imgl.load(a["src"].get_string()));
	}
	void emit(vector<RenderNode*> & nodes) {
		nodes.push_back(m_rn);
	}

private:
	ImgRenderNode * m_rn;
};

class VspaceTagHandler : public TagHandler {
public:
	VspaceTagHandler(ITag & tag, FontCache & fc, NodeStyle ns, IImageLoader & imgl) :
		TagHandler(tag, fc, ns, imgl), m_space(0) {}

	void enter() {
		const IAttrMap & a = m_tag.attrs();

		m_space = a["gap"].get_int();
	}
	void emit(vector<RenderNode*> & nodes) {
		nodes.push_back(new SpaceNode(m_ns, 0, m_space));
		nodes.push_back(new NewlineNode(m_ns));
	}

private:
	uint32_t m_space;
};

class HspaceTagHandler : public TagHandler {
public:
	HspaceTagHandler(ITag & tag, FontCache & fc, NodeStyle ns, IImageLoader & imgl) :
		TagHandler(tag, fc, ns, imgl), m_bg(0), m_space(0) {}

	void enter() {
		const IAttrMap & a = m_tag.attrs();

		if (a.has("gap"))
			m_space = a["gap"].get_int();
		else
			m_space = INFINITE_WIDTH;

		if (a.has("fill")) {
			m_fill_text = a["fill"].get_string();
			try {
				m_bg = m_imgl.load(m_fill_text);
				m_fill_text = "";
			} catch(BadImage &) {
			}
		}
	}

	void emit(vector<RenderNode*> & nodes) {
		RenderNode * rn = 0;
		if (not m_fill_text.empty()) {
			if (m_space < INFINITE_WIDTH)
				rn = new FillingTextNode(m_fc.get_font(m_ns), m_ns, m_space, m_fill_text);
			else
				rn = new FillingTextNode(m_fc.get_font(m_ns), m_ns, 0, m_fill_text, true);
		} else {
			SpaceNode * sn;
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
	SDL_Surface * m_bg;
	uint32_t m_space;
};

class BrTagHandler : public TagHandler {
public:
	BrTagHandler(ITag & tag, FontCache & fc, NodeStyle ns, IImageLoader & imgl) :
		TagHandler(tag, fc, ns, imgl) {
	}

	void emit(vector<RenderNode*> & nodes) {
		nodes.push_back(new NewlineNode(m_ns));
	}
};


class SubTagHandler : public TagHandler {
public:
	SubTagHandler(ITag & tag, FontCache & fc, NodeStyle ns, IImageLoader & imgl, uint32_t gw = 0)
		: TagHandler(tag, fc, ns, imgl), m_w(gw), m_rn(new SubTagRenderNode(ns)) {
	}

	void enter() {
		Borders padding, margin;

		handle_unique_attributes();
		const IAttrMap & a = m_tag.attrs();
		if (a.has("padding")) {
			uint8_t p = a["padding"].get_int();
			padding.left = padding.top = padding.right = padding.bottom = p;
		}
		if (a.has("margin")) {
			uint8_t p = a["margin"].get_int();
			margin.left = margin.top = margin.right = margin.bottom = p;
		}

		vector<RenderNode*> subnodes, nodes_to_render;
		TagHandler::emit(subnodes);

		if (not m_w) { // Determine the width by the width of the widest subnode
			foreach(RenderNode * n, subnodes) {
				if (n->width() >= INFINITE_WIDTH)
					continue;
				m_w = max(m_w, n->width() + padding.left + padding.right);
			}
		}

		// Layout takes ownership of subnodes
		Layout layout(subnodes);
		layout.fit_nodes(nodes_to_render, m_w, padding);

		// Collect all tags from children
		foreach(RenderNode * rn, nodes_to_render)
			foreach(const Reference & r, rn->get_references()) {
				m_rn->add_reference(rn->x() + r.dim.x, rn->y() + r.dim.y, r.dim.w, r.dim.h, r.ref);
			}

		m_rn->set_dimensions(m_w, layout.height(), margin);
		m_rn->set_nodes_to_render(nodes_to_render);
	}
	void emit(vector<RenderNode*> & nodes) {
		nodes.push_back(m_rn);
	}

	virtual void handle_unique_attributes() {
		const IAttrMap & a = m_tag.attrs();
		if (a.has("width")) m_w = a["width"].get_int();
		if (a.has("background")) {
			SDL_Color clr;
			try {
				a["background"].get_color(&clr);
				m_rn->set_background(clr);
			} catch (InvalidColor &) {
				m_rn->set_background(m_imgl.load(a["background"].get_string()));
			}
		}
		if (a.has("float")) {
			string s = a["float"].get_string();
			if (s == "right") m_rn->set_floating(RenderNode::FLOAT_RIGHT);
			else if (s == "left") m_rn->set_floating(RenderNode::FLOAT_LEFT);
		}
		if (a.has("valign")) {
			string align = a["valign"].get_string();
			if (align=="top") m_rn->set_valign(VALIGN_TOP);
			else if (align=="bottom") m_rn->set_valign(VALIGN_BOTTOM);
			else if (align=="center" or align=="middle") m_rn->set_valign(VALIGN_CENTER);
		}
	}

private:
	uint32_t m_w;
	SubTagRenderNode * m_rn;
};

class RTTagHandler : public SubTagHandler {
public:
	RTTagHandler(ITag & tag, FontCache & fc, NodeStyle ns, IImageLoader & imgl, uint32_t w) :
		SubTagHandler(tag, fc, ns, imgl, w) {
	}

	virtual void handle_unique_attributes() {
		const IAttrMap & a = m_tag.attrs();
		WordSpacerNode::show_spaces(a.has("db_show_spaces") ? a["db_show_spaces"].get_bool() : 0);
	}
};

template<typename T> TagHandler * create_taghandler
	(ITag & tag, FontCache & fc, NodeStyle & ns, IImageLoader & imgl)
{
	return new T(tag, fc, ns, imgl);
}
typedef std::map<std::string, TagHandler*(*)(ITag & tag, FontCache & fc, NodeStyle & ns, IImageLoader & imgl)> TagHandlerMap;
TagHandler * create_taghandler(ITag & tag, FontCache & fc, NodeStyle & ns, IImageLoader & imgl) {
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
		throw RenderError((format("No Tag handler for %s. This is a bug, please submit a report.") % tag.name()).str());
	return i->second(tag, fc, ns, imgl);
}

class Renderer : public IRenderer {
public:
	Renderer(IFontLoader * fl, IImageLoader * imgl, IParser * p);
	virtual ~Renderer();

	virtual SDL_Surface * render(std::string, uint32_t, IRefMap **);

private:
	FontCache m_fc;
	IImageLoader * m_imgl;
	IParser * m_p;
};

Renderer::Renderer(IFontLoader * fl, IImageLoader * imgl, IParser * p) :
	IRenderer(fl), m_fc(fl), m_imgl(imgl), m_p(p) {
}

Renderer::~Renderer() {
	delete m_p;
	delete m_imgl;
}

SDL_Surface * Renderer::render(string text, uint32_t width, IRefMap ** pprm) {
	ITag * rt = m_p->parse(text);

	NodeStyle default_fs = {
		"DejaVuSerif.ttf", 16,
		{ 0, 0, 0, 0 }, IFont::DEFAULT, 0, HALIGN_LEFT, VALIGN_BOTTOM
	};

	RTTagHandler rtrn(*rt, m_fc, default_fs, *m_imgl, width);
	vector<RenderNode*> nodes;
	rtrn.enter();
	rtrn.emit(nodes);

	assert(nodes.size() == 1);
	if (pprm)
		*pprm = new RefMap(nodes[0]->get_references());
	SDL_Surface * rv = nodes[0]->render();

	delete nodes[0];
	delete rt;

	return rv;
}

IRenderer * setup_renderer(IFontLoader * fl, IImageLoader * imgl) {
	return new Renderer(fl, imgl, setup_parser());
}

};
