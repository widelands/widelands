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
#include "ui_listselect.h"
#include "ui_scrollbar.h"

/**
Initialize a list select panel

Args: parent	parent panel
      x		coordinates of the UIListselect
      y
      w		dimensions, in pixels, of the UIListselect
      h
      align	alignment of text inside the UIListselect
*/
UIListselect::UIListselect(UIPanel *parent, int x, int y, uint w, uint h, Align align)
	: UIPanel(parent, x, y, w, h)
{
	set_think(false);

	set_align(align);

	m_scrollpos = 0;
	m_selection = -1;

	m_scrollbar = new UIScrollbar(parent, x+get_w()-24, y, 24, h, false);
	m_scrollbar->moved.set(this, &UIListselect::set_scrollpos);

	m_scrollbar->set_pagesize(h - 2*g_fh->get_fontheight(UI_FONT_SMALL));
	m_scrollbar->set_steps(1);

   m_lineheight=g_fh->get_fontheight(UI_FONT_SMALL); 

   m_max_pic_width=0;
   m_last_click_time=-10000;
   m_last_selection=-1;
}


/**
Free allocated resources
*/
UIListselect::~UIListselect()
{
	m_scrollbar = 0;
   clear();
}


/**
Remove all entries from the listselect
*/
void UIListselect::clear()
{
	for(uint i = 0; i < m_entries.size(); i++)
		free(m_entries[i]);
	m_entries.clear();

	if (m_scrollbar)
		m_scrollbar->set_steps(1);
	m_scrollpos = 0;
	m_selection = -1;
	m_last_click_time = -10000; 
   m_last_selection = -1;
}


/**
Add a new entry to the listselect.

Args: name	name that will be displayed
      value	value returned by get_select()
      select if true, directly select the new entry
*/
void UIListselect::add_entry(const char *name, void* value, bool select, int picid)
{
	Entry *e = (Entry *)malloc(sizeof(Entry) + strlen(name));

	e->value = value;
   e->picid = picid;
	strcpy(e->name, name);

   int entry_height=0;
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

	m_scrollbar->set_steps(m_entries.size() * get_lineheight() - get_h());

	update(0, 0, get_eff_w(), get_h());
   if(select)
      m_selection=m_entries.size()-1;
}

/*
 * Sort the listbox alphabetically. make sure that the current selection stays
 * valid (though it might scroll out of visibility). 
 * start and end defines the beginning and the end of a subarea to 
 * sort, for example you might want to sort directorys for themselves at the
 * top of list and files at the bottom.
 */
void UIListselect::sort(int gstart, int gend) {
   uint start=gstart; 
   uint stop=gend;
   if(gstart==-1) start=0;
   if(gend==-1) stop=m_entries.size();
   
   Entry *ei, *ej;
   for(uint i=start; i<stop; i++)
      for(uint j=i; j<stop; j++) {
         ei=m_entries[i];
         ej=m_entries[j];
         if(strcmp(ei->name, ej->name) > 0)  {
            if(m_selection==((int)i))
               m_selection=j;
            else if(m_selection==((int)j))
               m_selection=i;
            m_entries[i]=ej;
            m_entries[j]=ei;
         }
      }
}

/**
Set the list alignment (only horizontal alignment works)
*/
void UIListselect::set_align(Align align)
{
	m_align = (Align)(align & Align_Horizontal);
}


/**
Scroll to the given position, in pixels.
*/
void UIListselect::set_scrollpos(int i)
{
	m_scrollpos = i;

	update(0, 0, get_eff_w(), get_h());
}


/**
 *
 * Change the currently selected entry
 *
 * Args: i	the entry to select
 */
void UIListselect::select(int i)
{
	if (m_selection == i)
		return;

	m_selection = i;

	selected.call(m_selection);
	update(0, 0, get_eff_w(), get_h());
}


/**
Return the total height (text + spacing) occupied by a single line
*/
int UIListselect::get_lineheight()
{
	return m_lineheight+2; 
}


/**
Redraw the listselect box
*/
void UIListselect::draw(RenderTarget* dst)
{
	// draw text lines
	int lineheight = get_lineheight();
	int idx = m_scrollpos / lineheight;
	int y = 1 + idx*lineheight - m_scrollpos;

   dst->brighten_rect(0,0,get_w(),get_h(),ms_darken_value);

	while(idx < (int)m_entries.size())
		{
		if (y >= get_h())
			return;

		Entry* e = m_entries[idx];

		if (idx == m_selection) {
			// dst->fill_rect(1, y, get_eff_w()-2, g_font->get_fontheight(), m_selcolor);
			dst->brighten_rect(1, y, get_eff_w()-2, m_lineheight, -ms_darken_value);
      }

		int x;
		if (m_align & Align_Right)
			x = get_eff_w() - 1;
		else if (m_align & Align_HCenter)
			x = get_eff_w()>>1;
		else {
         // Pictures are always left aligned, leave some space here
			if(m_max_pic_width)
            x= m_max_pic_width + 10;
         else 
            x=1;
      }

      // Horizontal center the string
		g_fh->draw_string(dst, UI_FONT_SMALL, UI_FONT_SMALL_CLR, x, y + (get_lineheight()-g_fh->get_fontheight(UI_FONT_SMALL))/2, e->name, m_align, -1);

      // Now draw pictures
      if(e->picid!=-1) {
         int w,h;
         g_gr->get_picture_size(e->picid, &w, &h);
         dst->blit(1, y + (get_lineheight()-h)/2, e->picid);
      }
		y += lineheight;
		idx++;
	}
}


/**
 * Handle mouse clicks: select the appropriate entry
 */
bool UIListselect::handle_mouseclick(uint btn, bool down, int x, int y)
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

      y = (y + m_scrollpos) / get_lineheight();
      if (y >= 0 && y < (int)m_entries.size())
         select(y);
     
      // check if doubleclicked
      if(time-real_last_click_time < DOUBLE_CLICK_INTERVAL && m_last_selection==m_selection && m_selection!=-1) 
         double_clicked.call(m_selection);

   }

	return true;
}

/*
 * Remove entry
 */
void UIListselect::remove_entry(int i) {
   if(i<0 || ((uint)i)>=m_entries.size()) return;

   free(m_entries[i]);
   m_entries.erase(m_entries.begin() + i);
   if(m_selection==i)
      m_selection=-1;
}

