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

namespace UI {
struct Scrollbar;

/**
 * This class defines a list-select box.
 *
 * Entry can be
 * 1. a reference type,
 * 2. a pointer type or
 * 3. uintptr_t.
 */
template <typename Entry> struct Listselect {
	Listselect(Panel *parent, int x, int y, uint w, uint h, Align align = Align_Left, bool show_check = false);
	~Listselect();

	Signal1<uint> selected;
   Signal1<uint> double_clicked;

	void clear();
	void sort(const uint Begin = 0, uint End = std::numeric_limits<uint>::max());
	void add
		(const char * const name,
		 Entry value,
		 const int picid = -1,
		 const bool select_this = false);
	void remove(const uint i) const throw ();
	void remove(const char * const name);

	void switch_entries(const uint, const uint);

	void set_entry_color(const uint n, const RGBColor) throw ();
	void set_align(const Align);

	uint size() const throw ();
	Entry operator[](const uint i) const throw ();
	static uint no_selection_index() throw ();
	uint selection_index() const throw ();

	void select(const uint i);
	bool has_selection() const throw ();
	struct No_Selection {};
	Entry get_selected() const throw (No_Selection);
	void remove_selected() const throw (No_Selection);

	///  Return the total height (text + spacing) occupied by a single line.
	uint get_lineheight() const throw ();

	uint get_eff_w     () const throw ();

	// Drawing and event handling
	void draw(RenderTarget* dst);
	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);
};

template <> struct Listselect<void *> : public Panel {
	Listselect(Panel *parent, int x, int y, uint w, uint h, Align align = Align_Left, bool show_check = false);
	~Listselect();

	Signal1<uint> selected;
	Signal1<uint> double_clicked;

	void clear();
	void sort
		(const uint Begin = 0, const uint End = std::numeric_limits<uint>::max());
	void add
		(const char * const name,
		 void * entry,
		 const int picid = -1,
		 const bool select_this = false);
	void remove(const uint i);
	void remove(const char * const name);

	void switch_entries(const uint, const uint);

	void set_entry_color(const uint n, const RGBColor col) throw () {
		assert( n < m_entry_records.size() );
		m_entry_records[n]->use_clr = true;
		m_entry_records[n]->clr     = col;
	}

	void set_align(const Align);

	uint size() const throw () {return m_entry_records.size();}
	void * operator[](const uint i) const throw ()
	{assert(i < m_entry_records.size()); return m_entry_records[i]->entry();}
	static uint no_selection_index() throw ()
	{return std::numeric_limits<uint>::max();}
	uint selection_index() const throw () {return m_selection;}

	void select(const uint i);
	bool has_selection() const throw ()
	{return m_selection != no_selection_index();}
	struct No_Selection {};
	void * get_selected() const {
		if (m_selection == no_selection_index()) throw No_Selection();
		return m_entry_records[m_selection]->entry();
	}
	void remove_selected() throw (No_Selection) {
		if (m_selection == no_selection_index()) throw No_Selection();
		remove(m_selection);
	}

	uint get_lineheight() const throw () {return m_lineheight + 2;}
	uint get_eff_w     () const throw () {return get_w();}

	// Drawing and event handling
	void draw(RenderTarget* dst);
	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);

private:
   static const int DOUBLE_CLICK_INTERVAL=500; // half a second
	void set_scrollpos(const int);

private:
	static const int ms_darken_value=-20;

	struct Entry_Record {
		void * entry() const throw () {return m_entry;}

		void *   m_entry;
      bool     use_clr;
      RGBColor clr;
      int      picid;
		char     name[1];
	};

	uint                  m_max_pic_width;
	uint                  m_lineheight;
	Align               m_align;
	typedef std::vector<Entry_Record *> Entry_Record_vector;
	Entry_Record_vector m_entry_records;
	Scrollbar *         m_scrollbar;
	uint                m_scrollpos;         //  in pixels
	uint                  m_selection;
   int                  m_last_click_time;
	uint                  m_last_selection;  // for double clicks
	bool m_show_check; //  show a green arrow left of selected element
	int                 m_check_picid;
};

template <typename Entry>
	struct Listselect<const Entry * const> : public Listselect<void *>
{
	typedef Listselect<void *> Base;
	Listselect
		(Panel * parent,
		 int x, int y,
		 uint w, uint h,
		 Align align = Align_Left,
		 bool show_check = false)
		: Base(parent, x, y, w, h, align, show_check)
	{}

	void add
		(const char * const name,
		 const Entry * const entry,
		 const int picid = -1,
		 const bool select_this = false)
	{Base::add(name, const_cast<Entry * const>(entry), picid, select_this);}
	const Entry * operator[](const uint i) const throw ()
	{return static_cast<const Entry * const>(Base::operator[](i));}
	const Entry * get_selected() const
	{return static_cast<const Entry * const>(Base::get_selected());}
};

template <typename Entry>
	struct Listselect<Entry * const> : public Listselect<void *>
{
	typedef Listselect<void *> Base;
	Listselect
		(Panel * parent,
		 int x, int y,
		 uint w, uint h,
		 Align align = Align_Left,
		 bool show_check = false)
		: Base(parent, x, y, w, h, align, show_check)
	{}

	Entry * operator[](const uint i) const throw ()
	{return static_cast<Entry * const>(Base::operator[](i));}
	Entry * get_selected() const
	{return static_cast<Entry * const>(Base::get_selected());}
};

template <typename Entry> struct Listselect<Entry &> : public Listselect<void *>
{
	typedef Listselect<void *> Base;
	Listselect
		(Panel * parent,
		 int x, int y,
		 uint w, uint h,
		 Align align = Align_Left,
		 bool show_check = false)
		: Base(parent, x, y, w, h, align, show_check)
	{}

	void add
		(const char * const name,
		 Entry & value,
		 const int picid = -1,
		 const bool select_this = false)
	{Base::add(name, &value, picid, select_this);}
	Entry & operator[](const uint i) const throw ()
	{return *static_cast<Entry * const>(Base::operator[](i));}
	Entry & get_selected() const
	{return *static_cast<Entry * const>(Base::get_selected());}
};

compile_assert(sizeof(void *) == sizeof(uintptr_t));
template <> struct Listselect<uintptr_t> : public Listselect<void *> {
	typedef Listselect<void *> Base;
	Listselect
		(Panel * parent,
		 int x, int y,
		 uint w, uint h,
		 Align align = Align_Left,
		 bool show_check = false)
		: Base(parent, x, y, w, h, align, show_check)
	{}

	void add
		(const char * const name,
		 const uintptr_t value,
		 const int picid = -1,
		 const bool select_this = false)
	{
		Base::add
			(name, reinterpret_cast<void * const>(value), picid, select_this);
	}
	uint operator[](const uint i) const throw ()
	{return reinterpret_cast<const uintptr_t>(Base::operator[](i));}
	uint get_selected() const
	{return reinterpret_cast<const uintptr_t>(Base::get_selected());}
};
};

#endif
