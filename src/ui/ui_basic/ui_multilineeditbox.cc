/*
 * Copyright (C) 2002-2004 by the Widelands Development Team
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

#include "font_handler.h"
#include "types.h"
#include "ui_multilineeditbox.h"
#include "ui_multilinetextarea.h"
#include "ui_scrollbar.h"
#include "constants.h"
#include "system.h"
#include "font_handler.h"
#include "keycodes.h"
#include "rendertarget.h"
#include "error.h"

/**
Initialize a edibox that supports multiline strings.
*/
UIMultiline_Editbox::UIMultiline_Editbox(UIPanel *parent, int x, int y, uint w, uint h,
                                       const char *text)
   : UIMultiline_Textarea(parent, x, y, w, h, text, Align_Left, true) {
   m_maxchars=0xffff;

   set_scrollmode(ScrollLog);

   m_needs_update=false;
   m_cur_pos=get_text().size();

   set_handle_mouse(true);
   set_can_focus(true);
	set_think(false);
}


/**
Free allocated resources
*/
UIMultiline_Editbox::~UIMultiline_Editbox() {
   changed.call();
}

/**
a key event must be handled
*/
bool UIMultiline_Editbox::handle_key(bool down, int code, char c) {

   m_needs_update=true;

   if(down) {
      std::string m_text=get_text();
      switch(code) {
         case KEY_BACKSPACE:
            if(m_text.size() && m_cur_pos) {
               m_cur_pos--;
            } else {
               break;
            }
            // Fallthrough

         case KEY_DELETE:
            if(m_text.size() && m_cur_pos<m_text.size()) {
               m_text.erase(m_text.begin() + m_cur_pos);
               UIMultiline_Textarea::set_text(m_text.c_str());
            }
            break;

         case KEY_LEFT:
            m_cur_pos-=1;
            if(static_cast<int>(m_cur_pos)<0) m_cur_pos=0;
            break;

         case KEY_RIGHT:
            m_cur_pos+=1;
            if(m_cur_pos>=m_text.size()) m_cur_pos=m_text.size();
            break;

         case KEY_DOWN:
            if(m_cur_pos<m_text.size()-1) {
               uint begin_of_line=m_cur_pos;
               if(m_text[begin_of_line]=='\n') --begin_of_line;
               while(begin_of_line>0 && m_text[begin_of_line]!='\n') --begin_of_line;
               if(begin_of_line!=0) ++begin_of_line;
               uint begin_of_next_line=m_cur_pos;
               while(m_text[begin_of_next_line]!='\n' && begin_of_next_line<m_text.size())
                  ++begin_of_next_line;
               if(begin_of_next_line==m_text.size())
                  --begin_of_next_line;
                else
                  ++begin_of_next_line;
               uint end_of_next_line=begin_of_next_line;
               while(m_text[end_of_next_line]!='\n' && end_of_next_line<m_text.size())
                  ++end_of_next_line;
               if(begin_of_next_line+m_cur_pos-begin_of_line > end_of_next_line)
                  m_cur_pos=end_of_next_line;
               else
                  m_cur_pos=begin_of_next_line+m_cur_pos-begin_of_line;
            }
            break;

         case KEY_UP:
            if(m_cur_pos>0) {
               uint begin_of_line=m_cur_pos;
               if(m_text[begin_of_line]=='\n') --begin_of_line;
               while(begin_of_line>0 && m_text[begin_of_line]!='\n') --begin_of_line;
               if(begin_of_line!=0) ++begin_of_line;
               uint end_of_last_line=begin_of_line;
               if(begin_of_line!=0) --end_of_last_line;
               uint begin_of_lastline=end_of_last_line;
               if(m_text[begin_of_lastline]=='\n') --begin_of_lastline;
               while(begin_of_lastline>0 && m_text[begin_of_lastline]!='\n') --begin_of_lastline;
               if(begin_of_lastline!=0) ++begin_of_lastline;
               if(begin_of_lastline+(m_cur_pos-begin_of_line) > end_of_last_line)
                  m_cur_pos=end_of_last_line;
               else
                  m_cur_pos=begin_of_lastline+(m_cur_pos-begin_of_line);
            }
            break;

         case KEY_RETURN:
            c='\n';
            // fallthrough
         default:
            if(c && m_text.size()<m_maxchars) {
               m_text.insert(m_cur_pos,1,c);
               m_cur_pos++;
            }
            UIMultiline_Textarea::set_text(m_text.c_str());
            break;
      }

      UIMultiline_Textarea::set_text(m_text.c_str());
      changed.call();
      return true;
   }

   return false;
}

/*
 * handle mousclicks
 */
bool UIMultiline_Editbox::handle_mouseclick(uint btn, bool down, int x, int y) {
   if(!down) return false;
   if(btn==MOUSE_LEFT && !has_focus()) {
      focus();
      return true;
   }
   return UIMultiline_Textarea::handle_mouseclick(btn,down,x,y);
}

/**
Redraw the Editbox
*/
void UIMultiline_Editbox::draw(RenderTarget* dst)
{
   // make the whole area a bit darker
   dst->brighten_rect(0,0,get_w(),get_h(),ms_darken_value);

   std::string m_text=get_text();
   m_text.append(1,' ');
   if (m_text.size() )
   {
      //TODO: Implement in new font handler
		// Let the font handler worry about all the complicated stuff..
      //if(has_focus())
      //   g_fh->draw_string(dst, get_font_name(), get_font_size(), get_font_clr(), RGBColor(0,0,0), 0, 0 - get_m_textpos(), m_text.c_str(), Align_Left, get_eff_w(),m_cur_pos,-ms_darken_value*4);
      //else
         g_fh->draw_string(dst, get_font_name(), get_font_size(), get_font_clr(), RGBColor(0,0,0), 0, 0 - get_m_textpos(), m_text.c_str(), Align_Left, get_eff_w());
   }

   // now draw the textarea
   UIMultiline_Textarea::draw(dst);
}

/*
 * Set text function needs to take care of the current
 * position
 */
void UIMultiline_Editbox::set_text(const char* str) {
   if(strlen(str))
      m_cur_pos=strlen(str);
   else
      m_cur_pos=0;

   UIMultiline_Textarea::set_text(str);

}

