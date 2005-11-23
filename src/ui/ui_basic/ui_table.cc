/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "constants.h"
#include "graphic.h"
#include "rendertarget.h"
#include "system.h"
#include "types.h"
#include "ui_button.h"
#include "ui_scrollbar.h"
#include "ui_table.h"

/**
Initialize a panel

Args: parent	parent panel
      x		coordinates of the UITable
      y
      w		dimensions, in pixels, of the UITable
      h
      align	alignment of text inside the UITable
*/
UITable::UITable(UIPanel *parent, int x, int y, uint w, uint h, Align align, Dir def_dir)
	: UIPanel(parent, x, y, w, h)
{
	set_think(false);

	set_align(align);

	m_scrollpos = 0;
	m_selection = -1;

	m_scrollbar = 0;

   m_lineheight=g_fh->get_fontheight(UI_FONT_SMALL); 

   m_max_pic_width=0;
   m_last_click_time=-10000;
   m_last_selection=-1;

   m_default_sort_dir = def_dir;
   m_sort_direction = m_default_sort_dir;
   m_sort_column = 0;

}


/**
Free allocated resources
*/
UITable::~UITable()
{
	m_scrollbar = 0;
   clear();
}

/*
 * Add a new colum to this table 
 */
void UITable::add_column(const char* name, Type type, int w) {
   uint i=0;
   int complete_width=0;
   for(i=0; i<m_columns.size(); i++) {
      assert(m_columns[i].btn);
      complete_width+=m_columns[i].btn->get_w();
   }
   UIButton* btn=new UIButton(this, complete_width, 0, w, 15, 3, m_columns.size());
   btn->clickedid.set(this, &UITable::header_button_clicked);
   btn->set_title(name);

   Column c = {
      name, type, btn
   };
   m_columns.push_back(c);
   if(!m_scrollbar) {
      m_scrollbar=new UIScrollbar(get_parent(), get_x()+get_w()-24, get_y()+m_columns[0].btn->get_h(), 24, get_h()-m_columns[0].btn->get_h(), false);
      m_scrollbar->moved.set(this, &UITable::set_scrollpos);
      m_scrollbar->set_steps(1);
   }

}

UITable_Entry* UITable::find_entry (const void* userdata)
{
    unsigned int i;
    
    for (i=0; i<m_entries.size(); i++)
	if (m_entries[i]->get_user_data()==userdata)
	    return m_entries[i];

    return 0;
}

/*
 * A header button has been clicked
 */
void UITable::header_button_clicked(int n) {
   if(get_sort_colum()==n) {
      // Change sort direction
      if(get_sort_direction()==UP) 
         set_sort_direction(DOWN);
      else 
         set_sort_direction(UP);
      sort();
      return;
   } 
   
   set_sort_column(n);
   set_sort_direction(m_default_sort_dir);
   sort();
   return;
}

/**
Remove all entries from the table
*/
void UITable::clear()
{
	for(uint i = 0; i < m_entries.size(); i++)
		delete m_entries[i];
   m_entries.clear();

	if (m_scrollbar)
		m_scrollbar->set_steps(1);
	m_scrollpos = 0;
	m_selection = -1;
	m_last_click_time = -10000; 
   m_last_selection = -1;
}

/**
Redraw the table 
*/
void UITable::draw(RenderTarget* dst)
{
   // draw text lines
   int lineheight = get_lineheight();
   int idx = m_scrollpos / lineheight;
   int y = 1 + idx*lineheight - m_scrollpos + m_columns[0].btn->get_h();

   dst->brighten_rect(0,0,get_w(),get_h(),ms_darken_value);

   while(idx < (int)m_entries.size())
   {
      if (y >= get_h())
         return;

      UITable_Entry* e = m_entries[idx];

      if (idx == m_selection) {
         // dst->fill_rect(1, y, get_eff_w()-2, g_font->get_fontheight(), m_selcolor);
         dst->brighten_rect(1, y, get_eff_w()-2, m_lineheight, -ms_darken_value);
      }

      // First draw pictures
      if(e->get_picid()!=-1) {
         int w,h;
         g_gr->get_picture_size(e->get_picid(), &w, &h);
         dst->blit(1, y + (get_lineheight()-h)/2, e->get_picid());
      }

      RGBColor col = UI_FONT_CLR_FG;
      if( e->use_color() ) 
         col = e->get_color();
      
      int i=0;
      int curx=0;
      int curw;
      for(i=0; i<get_nr_columns(); i++) {
         curw=m_columns[i].btn->get_w(); 
         int x;
         if (m_align & Align_Right)
            x = curx + (curw - 1);
         else if (m_align & Align_HCenter)
            x = curx + (curw>>1);
         else {
            // Pictures are always left aligned, leave some space here
            if(m_max_pic_width && i==0)
               x= curx + m_max_pic_width + 10;
            else 
               x= curx + 1;
         }

         // Horizontal center the string
         g_fh->draw_string(dst, UI_FONT_SMALL, col, RGBColor(107,87,55), x, y + (get_lineheight()-g_fh->get_fontheight(UI_FONT_SMALL))/2, e->get_string(i), m_align, -1);
         
         curx+=curw; 
      }

      y += lineheight;
      idx++;
   }
}

/**
 * Handle mouse clicks: select the appropriate entry
 */
bool UITable::handle_mouseclick(uint btn, bool down, int x, int y)
{
   if (btn != 0) // only left-click
		return false;

   if (down) {
      int time=Sys_GetTime();

      // This hick hack is needed if any of the 
      // callback functions calls clear to forget the last
      // clicked time.
      int real_last_click_time=m_last_click_time; 
      
      m_last_selection=m_selection;
      m_last_click_time=time;

      y = (y + m_scrollpos - m_columns[0].btn->get_h()) / get_lineheight();
      if (y >= 0 && y < (int)m_entries.size())
         select(y);
     
      // check if doubleclicked
      if(time-real_last_click_time < DOUBLE_CLICK_INTERVAL && m_last_selection==m_selection && m_selection!=-1) 
         double_clicked.call(m_selection);

   }

	return true;
}

/**
 * Change the currently selected entry
 *
 * Args: i	the entry to select
 */
void UITable::select(int i)
{
	if (m_selection == i)
		return;

	m_selection = i;

	selected.call(m_selection);
	update(0, 0, get_eff_w(), get_h());
}

/**
Add a new entry to the table.
*/
void UITable::add_entry(UITable_Entry* e, bool do_select) { 
   int entry_height=0;
   int picid=e->get_picid();
   if(picid==-1) {
      entry_height=g_fh->get_fontheight(UI_FONT_SMALL);
   } else {
      int w,h;
      g_gr->get_picture_size(picid, &w, &h);
      entry_height= (h >= g_fh->get_fontheight(UI_FONT_SMALL)) ? h : g_fh->get_fontheight(UI_FONT_SMALL);  
      if(m_max_pic_width<w) m_max_pic_width=w;
   }
   if(entry_height>m_lineheight) m_lineheight=entry_height;

   m_entries.push_back(e);

	m_scrollbar->set_steps(m_entries.size() * get_lineheight() - (get_h() - m_columns[0].btn->get_h() - 2 )); 

   if( do_select )
      select( m_entries.size() - 1 );
	
   update(0, 0, get_eff_w(), get_h());
}

/**
Scroll to the given position, in pixels.
*/
void UITable::set_scrollpos(int i)
{
	m_scrollpos = i;

	update(0, 0, get_eff_w(), get_h());
}

/**
Set the list alignment (only horizontal alignment works)
*/
void UITable::set_align(Align align)
{
	m_align = (Align)(align & Align_Horizontal);
}


/**
Return the total height (text + spacing) occupied by a single line
*/
int UITable::get_lineheight()
{
	return m_lineheight+2; 
}

/*
 * Remove entry
 */
void UITable::remove_entry(int i) {
   if(i<0 || ((uint)i)>=m_entries.size()) return;

   delete(m_entries[i]);
   m_entries.erase(m_entries.begin() + i);
   if(m_selection==i)
      m_selection=-1;
}

/*
 * Sort the listbox alphabetically. make sure that the current selection stays
 * valid (though it might scroll out of visibility). 
 * start and end defines the beginning and the end of a subarea to 
 * sort, for example you might want to sort directorys for themselves at the
 * top of list and files at the bottom.
 */
void UITable::sort(void) {
         
   assert(m_columns[m_sort_column].type==STRING); 
  
   int start, stop, it;
   if(get_sort_direction()==DOWN) {
      start=0; 
      stop=m_entries.size();
      it=1;
   } else {
      start=m_entries.size()-1;
      stop=-1;
      it=-1;
   }

   UITable_Entry *ei, *ej;
   for(int i=start; i!=stop; i+=it)
      for(int j=i; j!=stop; j+=it) {
         ei=m_entries[i];
         ej=m_entries[j];
         // Only strings are sorted at the moment
         if(strcmp(ei->get_string(m_sort_column), ej->get_string(m_sort_column)) > 0)  {
            if(m_selection==((int)i))
               m_selection=j;
            else if(m_selection==((int)j))
               m_selection=i;
            m_entries[i]=ej;
            m_entries[j]=ei;
         }
      }
}

/*
=================================

Table Entry

=================================
*/

/*
 * constructor
 */
UITable_Entry::UITable_Entry(UITable* table, void* data, int picid, bool select) {
   m_picid=picid;
   m_user_data=data;
   m_data.resize(table->get_nr_columns());

   m_use_clr = false;

   table->add_entry(this, select);
}

/*
 * destructor
 */
UITable_Entry::~UITable_Entry(void) {
}

/*
 * Set,get string
 */
void UITable_Entry::set_string(int n, const char* str) {
   assert(((uint)n)<m_data.size());

   m_data[n].d_string=str;
}
const char* UITable_Entry::get_string(int n) {
   assert(((uint)n)<m_data.size());

   return m_data[n].d_string.c_str();
}


