/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__EDITOR_TOOL_H
#define __S__EDITOR_TOOL_H

#define MAX_TOOL_AREA 9

#include <vector>
#include "field.h"
#include "geometry.h"

class Coords;
class Editor_Interactive;
class Field;
class Map;

/*
=============================
class Editor_Tool

an editor tool is a tool that can be selected in the editor.
Examples are: modify height, place bob, place critter,
place building. A Tool only makes one function (like delete_building,
place building, modify building are 3 tools)
=============================
*/
class Editor_Tool {
   public:
      Editor_Tool(Editor_Tool* second, Editor_Tool* third) { m_second=second; m_third=third; if(!m_second) m_second=this; if(!m_third) m_third=this; }
      virtual ~Editor_Tool() {
         if(m_second==m_third) m_third=0;
         if(m_second && m_second!=this) { delete m_second; m_second=0; }
         if(m_third && m_third!=this) { delete m_third; m_third=0; }
      }

      int handle_click(int n, FCoords& f , Map* m, Editor_Interactive* parent) {
         if(n==0) return this->handle_click_impl(f,m,parent);
         if(n==1) return m_second->handle_click_impl(f,m,parent);
         if(n==2) return m_third->handle_click_impl(f,m,parent);
         return 0;
      }
      const char* get_fsel(int n) {
         if(n==0) return this->get_fsel_impl();
         if(n==1) return m_second->get_fsel_impl();
         if(n==2) return m_third->get_fsel_impl();
         return 0;
      }

      virtual int handle_click_impl(FCoords& f, Map* m, Editor_Interactive* parent) = 0;
      virtual const char* get_fsel_impl(void) = 0;

   protected:
      Editor_Tool* m_second, *m_third;
};

#endif

