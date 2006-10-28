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


class UIButton;
class UIScrollbar;
class UITable;

class UITable_Entry {
   public:
      UITable_Entry(UITable*, void*, int picid=-1, bool select = false);
      ~UITable_Entry(void);

      // Data Functions
      void set_string(int, const char*);
      const char* get_string(int);
      int  get_picid(void) { return m_picid; }
      void* get_user_data(void) { return m_user_data; }

      void set_color( RGBColor col ) {
         m_use_clr = true;
         m_color = col;
      }

      inline bool use_color( void ) { return m_use_clr; }
      inline RGBColor get_color( void ) { return m_color; }

   private:
      struct _data {
         std::string d_string;
      };
      std::vector<_data> m_data;
      void*    m_user_data;
      int      m_picid;
      bool     m_use_clr;
      RGBColor m_color;
};


/**
 * This class defines a table like panel,
 * whith columns and lines and all entrys can be seleted
 * by colums by clicking on the header-button
 */
class UITable : public UIPanel {
   friend class UITable_Entry;

public: // DATA
   enum Type {
      STRING,
   };
   enum Dir {
      UP,
      DOWN
   };

public: // FUNCTIONS
	UITable(UIPanel *parent, int x, int y, uint w, uint h, Align align = Align_Left, Dir = DOWN);
	~UITable();

	UISignal1<int> selected;
   UISignal1<int> double_clicked;

   void add_column(const char*, Type, int);

   // Information functions
   inline int get_nr_columns(void) { return m_columns.size(); }

   void clear();
	void set_sort_column(const uint col) throw ()
	{assert(m_columns.size() > col); m_sort_column = col;}
   int  get_sort_colum(void) { return m_sort_column; }
   int  get_sort_direction(void) { return m_sort_direction; }
   void set_sort_direction(Dir dir) { m_sort_direction=dir; }

   void sort(void);
   void remove_entry(int i);

	void set_align(Align align);

	int get_nr_entries(void) { return m_entries.size(); }
	int get_selection_index(void) { return m_selection; }
	UITable_Entry * get_entry(const uint n) const throw ()
	{assert(m_entries.size() > n); return m_entries[n];}
	UITable_Entry* find_entry(const void*);	// find by userdata

   void select(int i);
	inline void *get_selection() {
		if (m_selection < 0) return 0;
		return m_entries[m_selection]->get_user_data();
	}

	int get_lineheight();
	inline uint get_eff_w() { return get_w(); }

	// Drawing and event handling
	void draw(RenderTarget* dst);
	bool handle_mousepress  (const Uint8 btn, int x, int y);
	bool handle_mouserelease(const Uint8 btn, int x, int y);

private: // DATA
   struct Column {
      std::string name;
      Type type;
      UIButton* btn;
   };

   struct Entry {
		void*		value;
      int      picid;
		char		name[1];
	};
	static const int ms_darken_value=-20;
   static const int DOUBLE_CLICK_INTERVAL=500; // half a second

   std::vector<Column> m_columns;
   int                  m_max_pic_width;
   int                  m_lineheight;
	Align						m_align;
	UIScrollbar*		   m_scrollbar;
	int						m_scrollpos;	// in pixels
	int						m_selection;	// -1 when nothing is selected
   int                  m_last_click_time;
   int                  m_last_selection;  // for double clicks
   int                  m_sort_direction;
   int                  m_sort_column;
   Dir                  m_default_sort_dir;

private: // FUNCTIONS
   void header_button_clicked(int);
   void add_entry(UITable_Entry* t, bool);
	std::vector<UITable_Entry*>	m_entries;
	void set_scrollpos(int pos);
};

#endif
