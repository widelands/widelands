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

#ifndef __S__EDITOR_INCREASE_HEIGHT_TOOL_H
#define __S__EDITOR_INCREASE_HEIGHT_TOOL_H

#include "editor_tool.h"
#include "editor_increase_height_tool.h"
#include "editor_decrease_height_tool.h"

/*
=============================
class Editor_Increase_Height_Tool

this increases the height of a field by a value
=============================
*/
class Editor_Increase_Height_Tool : public Editor_Tool {
   public:
      Editor_Increase_Height_Tool(Editor_Decrease_Height_Tool* dht, Editor_Set_Height_Tool* sht)
        : Editor_Tool(dht, sht) { m_changed_by=1; m_dht=dht; m_sht=sht; }
      virtual ~Editor_Increase_Height_Tool() {  }

      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);

      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_increase_height.png"; }

      inline int get_changed_by(void) { return m_changed_by; }
      inline void set_changed_by(int n) { m_changed_by=n; }

      Editor_Decrease_Height_Tool* get_dht(void) { return m_dht; }
      Editor_Set_Height_Tool* get_sht(void) { return m_sht; }

   private:
      Editor_Decrease_Height_Tool* m_dht;
      Editor_Set_Height_Tool* m_sht;
      int m_changed_by;
};

#endif

