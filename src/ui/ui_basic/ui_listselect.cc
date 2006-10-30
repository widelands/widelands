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

#include "constants.h"
#include "graphic.h"
#include "rendertarget.h"
#include "types.h"
#include "ui_listselect.h"
#include "wlapplication.h"

/**
Initialize a list select panel

Args: parent	parent panel
      x		coordinates of the UIListselect
      y
      w		dimensions, in pixels, of the UIListselect
      h
      align	alignment of text inside the UIListselect
*/
UIListselect<void *>::UIListselect
(UIPanel *parent, int x, int y, uint w, uint h, Align align, bool show_check)
:
UIPanel(parent, x, y, w, h),
m_lineheight(g_fh->get_fontheight(UI_FONT_SMALL)),
m_scrollbar     (parent, x + get_w() - 24, y, 24, h, false),
m_scrollpos     (0),
m_selection     (no_selection_index()),
m_last_click_time(-10000),
m_last_selection(no_selection_index()),
m_show_check(show_check)
{
	set_think(false);

	set_align(align);

	m_scrollbar.moved.set(this, &UIListselect::set_scrollpos);
	m_scrollbar.set_pagesize(h - 2*g_fh->get_fontheight(UI_FONT_SMALL));
	m_scrollbar.set_steps(1);

	if (show_check) {
		uint pic_h;
		m_check_picid = g_gr->get_picture( PicMod_Game,  "pics/list_selected.png" );
		g_gr->get_picture_size(m_check_picid, m_max_pic_width, pic_h);
		if (pic_h > m_lineheight) m_lineheight = pic_h;
	}
	else {
		m_max_pic_width=0;
	}

}


/**
Free allocated resources
*/
UIListselect<void *>::~UIListselect() {clear();}


/**
Remove all entries from the listselect
*/
void UIListselect<void *>::clear() {
	for(uint i = 0; i < m_entries.size(); i++)
		free(m_entries[i]);
	m_entries.clear();

	m_scrollbar.set_steps(1);
	m_scrollpos = 0;
	m_selection = no_selection_index();
	m_last_click_time = -10000;
   m_last_selection = no_selection_index();
}


/**
Add a new entry to the listselect.

Args: name	name that will be displayed
      value	value returned by get_select()
      select if true, directly select the new entry
*/
void UIListselect<void *>::add_entry
(const char * const name,
 void * value,
 const bool select,
 const int picid)
{
	Entry & e =
		*static_cast<Entry * const>(malloc(sizeof(Entry) + strlen(name)));

	e.value = value;
	e.picid = picid;
	e.use_clr = false;
	strcpy(e.name, name);

	uint entry_height = 0;
   if(picid==-1) {
      entry_height=g_fh->get_fontheight(UI_FONT_SMALL);
   } else {
		uint w, h;
		g_gr->get_picture_size(picid, w, h);
      entry_height= (h >= g_fh->get_fontheight(UI_FONT_SMALL)) ? h : g_fh->get_fontheight(UI_FONT_SMALL);
	   if (m_max_pic_width < w) m_max_pic_width = w;
   }
   if(entry_height>m_lineheight) m_lineheight=entry_height;

	m_entries.push_back(&e);

	m_scrollbar.set_steps(m_entries.size() * get_lineheight() - get_h());

	update(0, 0, get_eff_w(), get_h());
   if(select) {
      UIListselect::select( m_entries.size() - 1);
	}
}

/*
 * Switch two entries
 */
void UIListselect<void *>::switch_entries(const uint m, const uint n) {
	assert(m < get_nr_entries());
	assert(n < get_nr_entries());

	std::swap(m_entries[m], m_entries[n]);

	if (m_selection == m) {
		m_selection = n;
		selected.call(n);
	} else if (m_selection == n) {
		m_selection = m;
		selected.call(m);
	}
}

/*
 * Sort the listbox alphabetically. make sure that the current selection stays
 * valid (though it might scroll out of visibility).
 * start and end defines the beginning and the end of a subarea to
 * sort, for example you might want to sort directorys for themselves at the
 * top of list and files at the bottom.
 */
void UIListselect<void *>::sort(const int gstart, const int gend) {
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
				if      (m_selection == i) m_selection = j;
				else if (m_selection == j) m_selection = i;
            m_entries[i]=ej;
            m_entries[j]=ei;
         }
      }
}

/**
Set the list alignment (only horizontal alignment works)
*/
void UIListselect<void *>::set_align(const Align align)
{m_align = static_cast<const Align>(align & Align_Horizontal);}


/**
Scroll to the given position, in pixels.
*/
void UIListselect<void *>::set_scrollpos(const int i) {
	m_scrollpos = i;

	update(0, 0, get_eff_w(), get_h());
}


/**
 *
 * Change the currently selected entry
 *
 * Args: i	the entry to select
 */
void UIListselect<void *>::select(const uint i) {
	if (m_selection == i)
		return;

	if (m_show_check) {
		if (m_selection != no_selection_index())
			m_entries[m_selection]->picid = -1;
		m_entries[i]->picid = m_check_picid;
	}
	m_selection = i;

	selected.call(m_selection);
	update(0, 0, get_eff_w(), get_h());
}


/**
Return the total height (text + spacing) occupied by a single line
*/
int UIListselect<void *>::get_lineheight() const throw ()
{return m_lineheight + 2;}


/**
Redraw the listselect box
*/
void UIListselect<void *>::draw(RenderTarget* dst) {
	// draw text lines
	const uint lineheight = get_lineheight();
	uint idx = m_scrollpos / lineheight;
	int y = 1 + idx*lineheight - m_scrollpos;

   dst->brighten_rect(0,0,get_w(),get_h(),ms_darken_value);

	while (idx < m_entries.size()) {
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

      RGBColor col = UI_FONT_CLR_FG;
      if( e->use_clr )
         col = e->clr;

      // Horizontal center the string
		g_fh->draw_string(dst, UI_FONT_SMALL, col, RGBColor(107,87,55), x, y + (get_lineheight()-g_fh->get_fontheight(UI_FONT_SMALL))/2, e->name, m_align, -1);

      // Now draw pictures
      if(e->picid!=-1) {
			uint w, h;
			g_gr->get_picture_size(e->picid, w, h);
         dst->blit(1, y + (get_lineheight()-h)/2, e->picid);
      }
		y += lineheight;
		idx++;
	}
}


/**
 * Handle mouse presses: select the appropriate entry
 */
bool UIListselect<void *>::handle_mousepress(const Uint8 btn, int, int y) {
	if (btn != SDL_BUTTON_LEFT) return false;

	   int time=WLApplication::get()->get_time();

      // This hick hack is needed if any of the
      // callback functions calls clear to forget the last
      // clicked time.
      int real_last_click_time=m_last_click_time;

      m_last_selection=m_selection;
      m_last_click_time=time;
		play_click();

      y = (y + m_scrollpos) / get_lineheight();
	if (y >= 0 and y < static_cast<const int>(m_entries.size())) select(y);

      // check if doubleclicked
	if
		(time-real_last_click_time < DOUBLE_CLICK_INTERVAL
		 and
		 m_last_selection == m_selection
		 and
		 m_selection != no_selection_index())
         double_clicked.call(m_selection);


	return true;
}
bool UIListselect<void *>::handle_mouserelease(const Uint8 btn, int, int)
{return btn == SDL_BUTTON_LEFT;}

/*
 * Remove entry
 */
void UIListselect<void *>::remove_entry(const uint i) {
	assert(i < m_entries.size());

   free(m_entries[i]);
   m_entries.erase(m_entries.begin() + i);
	if (m_selection == i) selected.call(m_selection = no_selection_index());
}

/*
 * Remove an entry by name. This only removes
 * the first entry with this name. If none is found, nothing
 * is done
 */
void UIListselect<void *>::remove_entry(const char * const str) {
   for(uint i=0; i<m_entries.size(); i++) {
      if(!strcmp(m_entries[i]->name,str)) {
         remove_entry(i);
         return;
      }
   }
}
