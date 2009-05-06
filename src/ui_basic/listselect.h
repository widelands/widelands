/*
 * Copyright (C) 2002, 2006, 2008-2009 by the Widelands Development Team
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


#ifndef UI_LISTSELECT_H
#define UI_LISTSELECT_H

#include "panel.h"
#include "scrollbar.h"
#include "m_signal.h"

#include "font_handler.h"

#include "compile_assert.h"

#include <limits>

namespace UI {
struct Scrollbar;

/**
 * This class defines a list-select box whose entries are defined by a name
 * and an associated numeric ID.
 *
 * Use the \ref Listselect template to use arbitrary IDs.
 */
struct BaseListselect : public Panel {
	BaseListselect
		(Panel *parent,
		 int32_t x,
		 int32_t y,
		 uint32_t w,
		 uint32_t h,
		 Align align = Align_Left,
		 bool show_check = false);
	~BaseListselect();

	Signal1<uint32_t> selected;
	Signal1<uint32_t> clicked;
	Signal1<uint32_t> double_clicked;

	void clear();
	void sort
		(const uint32_t Begin = 0,
		 uint32_t End = std::numeric_limits<uint32_t>::max());
	void add
		(const char * const name,
		 uint32_t value,
		 const int32_t picid = -1,
		 const bool select_this = false);
	void remove(uint32_t);
	void remove(const char * name);

	void switch_entries(uint32_t, uint32_t);

	void set_entry_color(uint32_t, RGBColor) throw ();
	void set_align(Align);
	void set_font(std::string const & fontname, int32_t fontsize) {
		m_fontname = fontname;
		m_fontsize = fontsize;
	}

	uint32_t size() const throw ()
	{
		return m_entry_records.size();
	}

	uint32_t operator[](const uint32_t i) const throw ()
	{
		assert(i < m_entry_records.size());
		return m_entry_records[i]->m_entry;
	}

	static uint32_t no_selection_index() throw ()
	{
		return std::numeric_limits<uint32_t>::max();
	}

	uint32_t selection_index() const throw ()
	{
		return m_selection;
	}

	void select(uint32_t i);
	bool has_selection() const throw ();

	struct No_Selection {};
	uint32_t get_selected() const throw (No_Selection);
	void remove_selected() throw (No_Selection);

	///  Return the total height (text + spacing) occupied by a single line.
	uint32_t get_lineheight() const throw ();

	uint32_t get_eff_w     () const throw ();

	// Drawing and event handling
	void draw(RenderTarget &);
	bool handle_mousepress  (Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(Uint8 btn, int32_t x, int32_t y);

private:
	static const int32_t DOUBLE_CLICK_INTERVAL = 500; // half a second

	void set_scrollpos(int32_t);

private:
	static const int32_t ms_darken_value = -20;

	struct Entry_Record {
		uint32_t m_entry;
		bool use_clr;
		RGBColor clr;
		int32_t picid;
		char name[1];
	};
	typedef std::vector<Entry_Record *> Entry_Record_vector;

	uint32_t m_max_pic_width;
	uint32_t m_lineheight;
	Align m_align;
	Entry_Record_vector m_entry_records;
	Scrollbar m_scrollbar;
	uint32_t m_scrollpos;         //  in pixels
	uint32_t m_selection;
	int32_t m_last_click_time;
	uint32_t m_last_selection;  // for double clicks
	bool m_show_check; //  show a green arrow left of selected element
	int32_t m_check_picid;

	std::string m_fontname;
	uint32_t    m_fontsize;
};

template<typename Entry>
struct Listselect : public BaseListselect {
	Listselect
		(Panel * parent,
		 int32_t x, int32_t y,
		 uint32_t w, uint32_t h,
		 Align align = Align_Left,
		 bool show_check = false)
		: BaseListselect(parent, x, y, w, h, align, show_check)
	{}

	void add
		(const char * const name,
		 Entry value,
		 const int32_t picid = -1,
		 const bool select_this = false)
	{
		m_entry_cache.push_back(value);
		BaseListselect::add(name, m_entry_cache.size() - 1, picid, select_this);
	}

	Entry const & operator[](uint32_t const i) const throw ()
	{
		return m_entry_cache[BaseListselect::operator[](i)];
	}

	Entry const & get_selected() const
	{
		return m_entry_cache[BaseListselect::get_selected()];
	}

private:
	std::vector<Entry> m_entry_cache;
};

/**
 * This template specialization is for backwards compatibility and convenience
 * only. Allowing references as template parameter is not a good idea
 * (e.g. STL containers don't allow it), you should really use pointers instead
 * because they are more explicit, and that's what this specialization does
 * internally.
 */
template<typename Entry>
struct Listselect<Entry &> : public Listselect<Entry *> {
	typedef Listselect<Entry *> Base;

	Listselect
		(Panel * parent,
		 int32_t x, int32_t y,
		 uint32_t w, uint32_t h,
		 Align align = Align_Left,
		 bool show_check = false)
		: Base(parent, x, y, w, h, align, show_check)
	{}

	void add
		(const char * const name,
		 Entry      &       value,
		 const int32_t picid = -1,
		 const bool select_this = false)
	{
		Base::add(name, &value, picid, select_this);
	}

	Entry & operator[](uint32_t const i) const throw ()
	{
		return *Base::operator[](i);
	}

	Entry & get_selected() const
	{
		return *Base::get_selected();
	}
};

};

#endif
