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

#ifndef __S__EDITOR_SET_HEIGHT_TOOL_H
#define __S__EDITOR_SET_HEIGHT_TOOL_H

#include "editor_tool.h"
#include "editor_decrease_height_tool.h"
#include "editor_increase_height_tool.h"

/*
=============================
class Editor_Set_Height_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Set_Height_Tool : public Editor_Tool {
   public:
      Editor_Set_Height_Tool() : Editor_Tool(this,this) { m_set_to=10; }
      virtual ~Editor_Set_Height_Tool() { }

      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);

      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_set_height.png"; }

      inline int get_set_to(void) { return m_set_to; }
      inline void set_set_to(int n) { m_set_to=n; }

   private:
      int m_set_to;
};

#endif
