/*
 * Copyright (C) 2002-2004, 2006-2008, 2012 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef EDITOR_MAKE_INFRASTRUCTURE_TOOL_H
#define EDITOR_MAKE_INFRASTRUCTURE_TOOL_H

#include "editor/tools/editor_tool.h"
#include "ui_basic/unique_window.h"

/**
 * This places immovables on the map
 * \TODO Implement undo for this tool
*/
struct Editor_Make_Infrastructure_Tool : public Editor_Tool {
	Editor_Make_Infrastructure_Tool() : Editor_Tool(*this, *this, false), m_player(0) {}

	void set_player(Widelands::Player_Number const n)
		{m_player = n;}
	Widelands::Player_Number get_player() const
		{return m_player;}

	int32_t handle_click_impl
		(Widelands::Map & map, Widelands::Node_and_Triangle<> center,
		 Editor_Interactive & parent, Editor_Action_Args & args) override;

	const char * get_sel_impl() const override
		{return "pics/fsel.png";} //  Standard sel icon, most complex tool of all

private:
	Widelands::Player_Number m_player;
	UI::UniqueWindow::Registry m_registry;
};

int32_t Editor_Make_Infrastructure_Tool_Callback
	(const Widelands::TCoords<Widelands::FCoords>& c,
	 Widelands::Editor_Game_Base& egbase, int32_t const player);

#endif
