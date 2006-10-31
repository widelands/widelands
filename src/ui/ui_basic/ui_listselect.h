/*
 * Copyright (C) 2002, 2006 by the Widelands Development Team
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


#ifndef __S__LISTSELECT_H
#define __S__LISTSELECT_H

#include <vector>
#include "compile_assert.h"
#include "font_handler.h"
#include "ui_panel.h"
#include "ui_signal.h"
#include <limits>

class UIScrollbar;

/**
 * This class defines a list-select box.
 */
template <typename T> struct UIListselect {
	UIListselect(UIPanel *parent, int x, int y, uint w, uint h, Align align = Align_Left, bool show_check = false);
	~UIListselect();

	UISignal1<uint> selected;
   UISignal1<uint> double_clicked;

	void clear();
	void sort(const int start = -1, const int end=-1);
	void add_entry
		(const char * const name,
		 T value,
		 const bool select_this = false,
		 const int picid = -1);
	void remove_entry(const uint i) const throw ();
	void remove_entry(const char * const name);

	void switch_entries(const uint, const uint);

	void set_entry_color(const uint n, const RGBColor) throw ();
	void set_align(const Align);

	uint get_nr_entries() const throw ();
	T get_entry(const uint i) const throw ();
	uint no_selection_index() const throw ();
	uint get_selection_index() const throw ();

	void select(const uint i);
	bool has_selection() const throw ();
	struct No_Selection {};
	T get_selection() const throw (No_Selection);
	void remove_selection() const throw (No_Selection);

	int get_lineheight() const throw ();
	uint get_eff_w() const throw ();

	// Drawing and event handling
	void draw(RenderTarget* dst);
	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);
};

template <> struct UIListselect<void *> : public UIPanel {
	UIListselect(UIPanel *parent, int x, int y, uint w, uint h, Align align = Align_Left, bool show_check = false);
	~UIListselect();

	UISignal1<uint> selected;
	UISignal1<uint> double_clicked;

	void clear();
	void sort(const int start = -1, const int end=-1);
	void add_entry
		(const char * const name,
		 void * value,
		 const bool select_this = false,
		 const int picid = -1);
	void remove_entry(const uint i);
	void remove_entry(const char * const name);

	void switch_entries(const uint, const uint);

	void set_entry_color(const uint n, const RGBColor col) throw () {
		assert( n < m_entries.size() );
		m_entries[n]->use_clr = true;
		m_entries[n]->clr = col;
	}

	void set_align(const Align);

	uint get_nr_entries() const throw () {return m_entries.size();}
	void * get_entry(const uint i) const throw ()
	{assert(i < m_entries.size()); return m_entries[i]->value;}
	uint no_selection_index() const throw ()
	{return std::numeric_limits<uint>::max();}
	uint get_selection_index() const throw () {return m_selection;}

	void select(const uint i);
	bool has_selection() const throw ()
	{return m_selection != no_selection_index();}
	struct No_Selection {};
	void * get_selection() const {
		if (m_selection == no_selection_index()) throw No_Selection();
		return m_entries[m_selection]->value;
	}
	void remove_selection() throw (No_Selection) {
		if (m_selection == no_selection_index()) throw No_Selection();
		remove_entry(m_selection);
	}

	int get_lineheight() const throw ();
	uint get_eff_w() const throw () {return get_w();}

	// Drawing and event handling
	void draw(RenderTarget* dst);
	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);

private:
   static const int DOUBLE_CLICK_INTERVAL=500; // half a second
	void set_scrollpos(const int);

private:
	static const int ms_darken_value=-20;

   struct Entry {
		void*		value;
      bool     use_clr;
      RGBColor clr;
      int      picid;
		char		name[1];
	};

	uint                  m_max_pic_width;
	uint                  m_lineheight;
	Align						m_align;
	std::vector<Entry*>	m_entries;
	UIScrollbar *         m_scrollbar;
	uint                  m_scrollpos;	// in pixels
	uint                  m_selection;
   int                  m_last_click_time;
	uint                  m_last_selection;  // for double clicks
	bool						m_show_check;	//show a green arrow left of selected element
	int						m_check_picid;
};

template <typename T> struct UIListselect<const T * const> : public UIListselect<void *> {
	typedef UIListselect<void *> Base;
	UIListselect
		(UIPanel * parent,
		 int x, int y,
		 uint w, uint h,
		 Align align = Align_Left,
		 bool show_check = false)
		:
		Base(parent, x, y, w, h, align, show_check) {}

	void add_entry
		(const char * const name,
		 const T * const value,
		 const bool select_this = false,
		 const int picid = -1)
	{Base::add_entry(name, const_cast<T * const>(value), select_this, picid);}
	const T * get_entry(const uint i) const throw ()
	{return static_cast<const T * const>(Base::get_entry(i));}
	const T * get_selection() const
	{return static_cast<const T * const>(Base::get_selection());}
};

template <class T> struct UIListselect<T * const> : public UIListselect<void *> {
	typedef UIListselect<void *> Base;
	UIListselect
		(UIPanel * parent,
		 int x, int y,
		 uint w, uint h,
		 Align align = Align_Left,
		 bool show_check = false)
		:
		Base(parent, x, y, w, h, align, show_check) {}

	T * get_entry(const uint i) const throw ()
	{return static_cast<T * const>(Base::get_entry(i));}
	T * get_selection() const
	{return static_cast<T * const>(Base::get_selection());}
};

template <class T> struct UIListselect<T &> : public UIListselect<void *> {
	typedef UIListselect<void *> Base;
	UIListselect
		(UIPanel * parent,
		 int x, int y,
		 uint w, uint h,
		 Align align = Align_Left,
		 bool show_check = false)
		:
		Base(parent, x, y, w, h, align, show_check) {}

	void add_entry
		(const char * const name,
		 T & value,
		 const bool select_this = false,
		 const int picid = -1)
	{Base::add_entry(name, &value, select_this, picid);}
	T & get_entry(const uint i) const throw ()
	{return *static_cast<T * const>(Base::get_entry(i));}
	T & get_selection() const
	{return *static_cast<T * const>(Base::get_selection());}
};

compile_assert(sizeof(void *) == sizeof(uint));
template <> struct UIListselect<uint> : public UIListselect<void *> {
	typedef UIListselect<void *> Base;
	UIListselect
		(UIPanel * parent,
		 int x, int y,
		 uint w, uint h,
		 Align align = Align_Left,
		 bool show_check = false)
		:
		Base(parent, x, y, w, h, align, show_check) {}

	void add_entry
		(const char * const name,
		 const uint value,
		 const bool select_this = false,
		 const int picid = -1)
	{
		Base::add_entry
			(name, reinterpret_cast<void * const>(value), select_this, picid);
	}
	uint get_entry(const uint i) const throw ()
	{return reinterpret_cast<const uint>(Base::get_entry(i));}
	uint get_selection() const
	{return reinterpret_cast<const uint>(Base::get_selection());}
};

#endif
