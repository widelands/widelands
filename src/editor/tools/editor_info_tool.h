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

#ifndef __S__EDITOR_INFO_TOOL_H
#define __S__EDITOR_INFO_TOOL_H

#include "editor_tool.h"

/*
=============================
class Editor_Info_Tool

this is a simple tool to show information about the clicked field
=============================
*/
class Editor_Info_Tool : public Editor_Tool {
   public:
      Editor_Info_Tool() : Editor_Tool(this,this) { }
      virtual ~Editor_Info_Tool() { }

      virtual int handle_click_impl(const Coords*, Field*, Map*, Editor_Interactive*);
      virtual const char* get_fsel_impl(void) { return "pics/fsel_editor_info.png"; }
};

#endif
