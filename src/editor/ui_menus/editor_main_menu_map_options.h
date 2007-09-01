/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#ifndef __S__EDITOR_MAIN_MENU_MAP_OPTIONS_H
#define __S__EDITOR_MAIN_MENU_MAP_OPTIONS_H

#include "ui_window.h"

class Editor_Interactive;
namespace UI {
class Edit_Box;
class Multiline_Editbox;
struct Textarea;
};

/*
=================================================

class Main_Menu_Map_Options

this is the Main Options Menu. Here, informations
about the current map are displayed and you can change
author, name and description

=================================================
*/
struct Main_Menu_Map_Options : public UI::Window {
      Main_Menu_Map_Options(Editor_Interactive*);
      virtual ~Main_Menu_Map_Options();

private:
      void changed(int);
      void editbox_changed();
      Editor_Interactive  *m_parent;
      UI::Multiline_Editbox* m_descr;
      UI::Textarea* m_world, *m_nrplayers, *m_size;
      UI::Edit_Box* m_name, *m_author;
      void update();
};

#endif
