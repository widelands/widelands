/*
 * Copyright (C) 2003 by The Widelands Development Team
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

#ifndef __S__EDITOR_TOOLS_H
#define __S__EDITOR_TOOLS_H

class Editor_Interactive;
#include "map.h"
#include "ui.h"

#define MAX_TOOL_AREA 6

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
      Editor_Tool() { } ;
      virtual ~Editor_Tool() { } ;

      virtual int handle_click(const Coords*, Field* field, Map* m, Editor_Interactive* parent) = 0;
      virtual int tool_options_dialog(Editor_Interactive* parent) { return 0; } // not needed by every tool
      virtual bool has_options(void) { return false; }
      virtual const char* get_name(void) = 0;
};

/*
=============================
class Editor_Info_Tool

this is a simple tool to show information about the clicked field
=============================
*/
class Editor_Info_Tool : public Editor_Tool {
   public:
      Editor_Info_Tool() { }
      virtual ~Editor_Info_Tool() { }

      virtual int handle_click(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual const char* get_name(void) { return "Field Informations"; }
};

/*
=============================
class Editor_Increase_Height_Tool

this increases the height of a field by a value
=============================
*/
class Editor_Increase_Height_Tool : public Editor_Tool {
   public:
      Editor_Increase_Height_Tool() { m_increase_by=1; }
      virtual ~Editor_Increase_Height_Tool() { }
  
      virtual int handle_click(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual int tool_options_dialog(Editor_Interactive* parent);
      virtual bool has_options(void) { return true; }
      
      virtual const char* get_name(void) { return "Increase Field Height"; }
      
   private:
      UniqueWindow m_w;
      int m_increase_by;
};

/*
=============================
class Editor_Decrease_Height_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Decrease_Height_Tool : public Editor_Tool {
   public:
      Editor_Decrease_Height_Tool() { m_decrease_by=1; }
      virtual ~Editor_Decrease_Height_Tool() { }
  
      virtual int handle_click(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual int tool_options_dialog(Editor_Interactive* parent);
      virtual bool has_options(void) { return true; }
      
      virtual const char* get_name(void) { return "Decrease Field Height"; }
      
   private:
      UniqueWindow m_w;
      int m_decrease_by;
};

/*
=============================
class Editor_Set_Height_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Set_Height_Tool : public Editor_Tool {
   public:
      Editor_Set_Height_Tool() { m_set_to=10; }
      virtual ~Editor_Set_Height_Tool() { }
  
      virtual int handle_click(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual int tool_options_dialog(Editor_Interactive* parent);
      virtual bool has_options(void) { return true; }
      
      virtual const char* get_name(void) { return "Set Field Height"; }
      
   private:
      UniqueWindow m_w;
      int m_set_to;
};

/*
=============================
class Editor_Noise_Height_Tool

this decreases the height of a field by a value
=============================
*/
class Editor_Noise_Height_Tool : public Editor_Tool {
   public:
      Editor_Noise_Height_Tool() { m_upper_value=MAX_FIELD_HEIGHT/2; m_lower_value=10; }
      virtual ~Editor_Noise_Height_Tool() { }
  
      virtual int handle_click(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual int tool_options_dialog(Editor_Interactive* parent);
      virtual bool has_options(void) { return true; }
      
      virtual const char* get_name(void) { return "Noise Field Height"; }
      
   private:
      UniqueWindow m_w;
      int m_upper_value;
      int m_lower_value;
};




#endif // __S__EDITOR_TOOLS_H
