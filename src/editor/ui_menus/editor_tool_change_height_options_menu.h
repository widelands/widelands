/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef EDITOR_TOOL_CHANGE_HEIGHT_OPTIONS_MENU_H
#define EDITOR_TOOL_CHANGE_HEIGHT_OPTIONS_MENU_H

#include "editor_tool_options_menu.h"

#include "ui_button.h"
#include "ui_textarea.h"

struct Editor_Interactive;
struct Editor_Increase_Height_Tool;

struct Editor_Tool_Change_Height_Options_Menu : public Editor_Tool_Options_Menu {
	Editor_Tool_Change_Height_Options_Menu
		(Editor_Interactive          &,
		 Editor_Increase_Height_Tool &,
		 UI::UniqueWindow::Registry  &);

private:
	Editor_Increase_Height_Tool & m_increase_tool;
	UI::Textarea                  m_change_by_label;
	UI::Button<Editor_Tool_Change_Height_Options_Menu>
		m_change_by_increase, m_change_by_decrease;
	UI::Textarea                  m_change_by_value;
	UI::Textarea                  m_setto_label;
	UI::Button<Editor_Tool_Change_Height_Options_Menu>
		m_setto_increase,     m_setto_decrease;
	UI::Textarea                  m_setto_value;

	void clicked_change_by_decrement();
	void clicked_change_by_increment();
	void clicked_setto_decrement    ();
	void clicked_setto_increment    ();
	void update();
};

#endif
