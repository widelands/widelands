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

#ifndef __S__EDITOR_VARIABLES_MENU_H
#define __S__EDITOR_VARIABLES_MENU_H

#include <stdint.h>
#include "ui_button.h"
#include "ui_table.h"
#include "ui_unique_window.h"

class Editor_Interactive;
class MapVariable;

/*
=============================

class Editor_Variables_Menu

=============================
*/
struct Editor_Variables_Menu : public UI::UniqueWindow {
      Editor_Variables_Menu(Editor_Interactive*, UI::UniqueWindow::Registry*);
      virtual ~Editor_Variables_Menu();

private:
      Editor_Interactive *m_parent;
	UI::Table<MapVariable &>          m_table;
	UI::Button<Editor_Variables_Menu> m_button_new;
	UI::Button<Editor_Variables_Menu> m_button_edit;
	UI::Button<Editor_Variables_Menu> m_button_del;

private:
	void insert_variable(MapVariable &);
	void clicked_new ();
	void clicked_edit();
	void clicked_del ();
	void table_selected(uint32_t);
	void table_dblclicked(uint32_t);
};


#endif
