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


#ifndef __S__TABLE_H
#define __S__TABLE_H

#include <vector>
#include "font_handler.h"
#include "ui_panel.h"
#include "ui_signal.h"
#include <limits>

#include "compile_assert.h"

namespace UI {
struct Scrollbar;
template <typename T, typename ID> struct IDButton;

/**
 * A table whith columns and lines. The entriess can be sorted by colums by
 * clicking on the column header button.
 *
 * Entry can be
 * 1. a reference type,
 * 2. a pointer type or
 * 3. uintptr_t.
 */
template<typename Entry> struct Table {

	struct Entry_Record {
		Entry_Record(Entry entry, int picid = -1);

      // Data Functions
		void set_string(uint column, const std::string &);
		std::string & get_string(uint column) const;
		int  get_picid() const throw ();
		Entry entry() const throw ();
		void set_color(const RGBColor);

		bool     use_color() const throw ();
		RGBColor get_color() const throw ();

	};

	Table(Panel *parent, int x, int y, uint w, uint h, Align align = Align_Left, const bool up = false);
	~Table();

	Signal1<uint> selected;
	Signal1<uint> double_clicked;

	void add_column(const std::string & name, const uint width);

	uint get_nr_columns() const throw ();

   void clear();
	void set_sort_column(const uint col) throw ();
	uint get_sort_colum() const throw ();
	bool get_sort_descending() const throw ();

	void sort(const uint Begin = 0, uint End = std::numeric_limits<uint>::max());
   void remove(const uint i);

	void set_align(Align align);

	Entry_Record & add
		(void * const entry,
		 const int picid = -1,
		 const bool select_this = false);

	uint size() const throw ();
	Entry operator[](const uint i) const throw ();
	static uint no_selection_index() throw ();
	bool has_selection() const throw ();
	uint selection_index() const throw ();
	Entry_Record & get_record(const uint n) const throw ();
	static Entry get(const Entry_Record &);
	Entry_Record * find(const Entry) const throw ();

   void select(const uint);
	struct No_Selection {};
	Entry_Record & get_selected_record() const;
	Entry get_selected() const;

	///  Return the total height (text + spacing) occupied by a single line.
	uint get_lineheight() const throw ();

	uint get_eff_w     () const throw ();

	// Drawing and event handling
	void draw(RenderTarget* dst);
	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);
};

template <> struct Table<void *> : public Panel {

	struct Entry_Record {
		Entry_Record(void * const entry, int picid = -1);

      // Data Functions
		void set_string(uint column, const std::string &);
		const std::string & get_string(const uint column) const;
		int  get_picid() const throw () {return m_picid;}
		void * entry() const throw () {return m_entry;}
		void set_color(const  RGBColor c) {
			use_clr = true;
			clr = c;
		}

		bool     use_color() const throw () {return use_clr;}
		RGBColor get_color() const throw () {return clr;}

	private:
		friend struct Table<void *>;
		void *   m_entry;
		bool     use_clr;
		RGBColor clr;
		int      m_picid;
		struct _data {std::string d_string;};
      std::vector<_data> m_data;
	};

public:
	Table
		(Panel * parent,
		 int x, int y, uint w, uint h,
		 Align align = Align_Left,
		 const bool descending = false);
	~Table();

	Signal1<uint> selected;
	Signal1<uint> double_clicked;

	void add_column(const std::string & name, const uint width);

	uint get_nr_columns() const throw () {return m_columns.size();}

   void clear();
	void set_sort_column(const uint col) throw ()
	{assert(col < m_columns.size()); m_sort_column = col;}
	uint get_sort_colum() const throw () {return m_sort_column;}
	bool  get_sort_descending() const throw () {return m_sort_descending;}
	void set_sort_descending(const bool descending) throw ()
	{m_sort_descending = descending;}

	void sort(const uint Begin = 0, uint End = std::numeric_limits<uint>::max());
   void remove(const uint index);

	void set_align(Align align);

	Entry_Record & add
		(void * const entry = 0, const int picid = -1, const bool select = false);

	uint size() const throw () {return m_entry_records.size();}
	void * operator[](const uint i) const throw ()
	{assert(i < m_entry_records.size()); return m_entry_records[i]->entry();}
	static uint no_selection_index() throw ()
	{return std::numeric_limits<uint>::max();}
	bool has_selection() const throw ()
	{return m_selection != no_selection_index();}
	uint selection_index() const throw () {return m_selection;}
	Entry_Record & get_record(const uint n) const throw ()
	{assert(n < m_entry_records.size()); return *m_entry_records[n];}
	static void * get(const Entry_Record & er) throw ()
	{return er.entry();}
	Entry_Record * find(const void * entry) const throw ();

   void select(const uint);
	struct No_Selection {};
	Entry_Record & get_selected_record() const {
		if (m_selection == no_selection_index()) throw No_Selection();
		return *m_entry_records[m_selection];
	}
	void remove_selected() throw (No_Selection) {
		if (m_selection == no_selection_index()) throw No_Selection();
		remove(m_selection);
	}
	void * get_selected() const throw (No_Selection)
	{return get_selected_record().entry();};

	uint get_lineheight() const throw () {return m_lineheight + 2;}
	uint get_eff_w     () const throw () {return get_w();}

	// Drawing and event handling
	void draw(RenderTarget* dst);
	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);

private:
	struct Column;
	typedef std::vector<Column> Columns;
	struct Column {
      std::string name;
		IDButton<Table, Columns::size_type> * btn;
	};

	static const int ms_darken_value=-20;
	static const int DOUBLE_CLICK_INTERVAL=500; // half a second //  FIXME such values should not be defined separately for a particular widget type. They should be defined in one place for the whole UI or even better, a variable that is read from system settings (.kde/share/config/kdeglobals:DoubleClickInterval=...)

	Columns m_columns;
	uint                m_max_pic_width;
   int                  m_lineheight;
	Align              m_align;
	Scrollbar        * m_scrollbar;
	int                m_scrollpos; //  in pixels
	uint               m_selection;
   int                  m_last_click_time;
	uint                  m_last_selection;  // for double clicks
	Columns::size_type m_sort_column;
	bool               m_sort_descending;

	void header_button_clicked(const Columns::size_type);
	typedef std::vector<Entry_Record *> Entry_Record_vector;
	Entry_Record_vector m_entry_records;
	void set_scrollpos(int pos);
};

template <typename Entry>
	struct Table<const Entry * const> : public Table<void *>
{
	typedef Table<void *> Base;
	Table
		(Panel * parent,
		 int x, int y, uint w, uint h,
		 Align align = Align_Left,
		 const bool descending = false)
		: Base(parent, x, y, w, h, align, descending)
	{}

	Entry_Record & add
		(const Entry * const entry = 0,
		 const int picid = -1,
		 const bool select_this = false)
	{
		return
			Base::add(const_cast<Entry *>(entry), picid, select_this);
	}

	const Entry * operator[](const uint i) const throw ()
	{return static_cast<const Entry *>(Base::operator[](i));}

	static const Entry * const get(const Entry_Record & er)
	{return static_cast<const Entry *>(er.entry());}

	const Entry * const get_selected() const
	{return static_cast<const Entry *>(Base::get_selected());}
};

template <typename Entry> struct Table<Entry * const> : public Table<void *> {
	typedef Table<void *> Base;
	Table
		(Panel * parent,
		 int x, int y, uint w, uint h,
		 Align align = Align_Left,
		 const bool descending = false)
		: Base(parent, x, y, w, h, align, descending)
	{}

	Entry_Record & add
		(Entry * const entry = 0,
		 const int picid = -1,
		 const bool select_this = false)
	{return Base::add(entry, picid, select_this);}

	Entry * operator[](const uint i) const throw ()
	{return static_cast<Entry *>(Base::operator[](i));}

	static Entry * const get(const Entry_Record & er)
	{return static_cast<Entry *>(er.entry());}

	Entry * const get_selected() const
	{return static_cast<Entry *>(Base::get_selected());}
};

template <typename Entry> struct Table<const Entry &> : public Table<void *> {
	typedef Table<void *> Base;
	Table
		(Panel * parent,
		 int x, int y, uint w, uint h,
		 Align align = Align_Left,
		 const bool descending = false)
		: Base(parent, x, y, w, h, align, descending)
	{}

	Entry_Record & add
		(const Entry & entry,
		 const int picid = -1,
		 const bool select_this = false)
	{return Base::add(&const_cast<Entry &>(entry), picid, select_this);}

	const Entry & operator[](const uint i) const throw ()
	{return *static_cast<const Entry *>(Base::operator[](i));}

	static const Entry & get(const Entry_Record & er)
	{return *static_cast<const Entry *>(er.entry());}

	Entry_Record * find(const Entry & entry) const throw ()
	{return Base::find(&entry);}

	const Entry & get_selected() const
	{return *static_cast<const Entry *>(Base::get_selected());}
};

template <typename Entry> struct Table<Entry &> : public Table<void *> {
	typedef Table<void *> Base;
	Table
		(Panel * parent,
		 int x, int y, uint w, uint h,
		 Align align = Align_Left,
		 const bool descending = false)
		: Base(parent, x, y, w, h, align, descending)
	{}

	Entry_Record & add
		(Entry & entry,
		 const int picid = -1,
		 const bool select_this = false)
	{return Base::add(&entry, picid, select_this);}

	Entry & operator[](const uint i) const throw ()
	{return *static_cast<Entry *>(Base::operator[](i));}

	static Entry & get(const Entry_Record & er)
	{return *static_cast<Entry *>(er.entry());}

	Entry_Record * find(Entry & entry) const throw ()
	{return Base::find(&entry);}

	Entry & get_selected() const
	{return *static_cast<Entry *>(Base::get_selected());}
};

compile_assert(sizeof(void *) == sizeof(uintptr_t));
template <> struct Table<uintptr_t> : public Table<void *> {
	typedef Table<void *> Base;
	Table
		(Panel * parent,
		 int x, int y, uint w, uint h,
		 Align align = Align_Left,
		 const bool descending = false)
		: Base(parent, x, y, w, h, align, descending)
	{}

	Entry_Record & add
		(const uintptr_t entry,
		 const int picid = -1,
		 const bool select_this = false)
	{
		return Base::add
			(reinterpret_cast<void *>(entry), picid, select_this);
	}

	uintptr_t operator[](const uint i) const throw ()
	{return reinterpret_cast<uintptr_t>(Base::operator[](i));}
	static uintptr_t get(const Entry_Record & er)
	{return reinterpret_cast<uintptr_t>(er.entry());}

	Entry_Record * find(const uintptr_t entry) const throw ()
	{return Base::find(reinterpret_cast<const void *>(entry));}

	uintptr_t get_selected() const
	{return reinterpret_cast<uintptr_t>(Base::get_selected());}
};
template <> struct Table<const uintptr_t> : public Table<uintptr_t> {
	typedef Table<uintptr_t> Base;
	Table
		(Panel * parent,
		 int x, int y, uint w, uint h,
		 Align align = Align_Left,
		 const bool descending = false)
		: Table<uintptr_t>(parent, x, y, w, h, align, descending)
	{}
};
};

#endif
