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

#ifndef __S__EDITOR_NOISE_HEIGHT_TOOL_H
#define __S__EDITOR_NOISE_HEIGHT_TOOL_H

#include "editor_tool.h"
#include "editor_set_height_tool.h"

/*
=============================
class Editor_Noise_Height_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Noise_Height_Tool : public Editor_Tool {
   public:
      Editor_Noise_Height_Tool(Editor_Set_Height_Tool* sht) :
        Editor_Tool(sht,sht) { m_upper_value=14; m_lower_value=10; m_sht=sht; }
      virtual ~Editor_Noise_Height_Tool() { m_third=m_second=0; } // don't delete this, somone else will care

      virtual int handle_click_impl(FCoords&, Map*, Editor_Interactive*);

      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_noise_height.png"; }

      inline Editor_Set_Height_Tool* get_sht(void) { return m_sht; }

      inline void get_values(int* a, int* b) { *a=m_lower_value; *b=m_upper_value; }
      inline void set_values(int a, int b) { m_lower_value=a; m_upper_value=b; }

   private:
      Editor_Set_Height_Tool* m_sht;
      int m_upper_value;
      int m_lower_value;
};

#endif
