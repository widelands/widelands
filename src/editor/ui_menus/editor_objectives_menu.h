/*
 * Copyright (C) 2002-2006, 2008 by the Widelands Development Team
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

#ifndef EDITOR_OBJECTIVES_MENU_H
#define EDITOR_OBJECTIVES_MENU_H

#include "ui_basic/table.h"
#include "ui_basic/unique_window.h"

struct Editor_Interactive;
namespace Widelands {struct Objective;}
namespace UI {
template <typename T> struct Callback_Button;
template <typename T> struct Table;
struct Textarea;
};

struct Editor_Objectives_Menu : public UI::UniqueWindow {
	Editor_Objectives_Menu(Editor_Interactive *, UI::UniqueWindow::Registry *);

private:
	Editor_Interactive                 * m_parent;
	UI::Table<Widelands::Objective &> m_table;
	UI::Callback_Button<Editor_Objectives_Menu> * m_edit_button;
	UI::Callback_Button<Editor_Objectives_Menu> * m_delete_button;
	UI::Textarea                       * m_trigger;

	void insert_objective(Widelands::Objective &);
	void clicked_ok  ();
	void clicked_new ();
	void clicked_edit();
	void clicked_del ();
	void table_selected  (uint32_t);
	void table_dblclicked(uint32_t);
};


#endif
