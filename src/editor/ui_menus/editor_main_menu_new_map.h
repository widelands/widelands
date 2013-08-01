/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#ifndef EDITOR_MAIN_MENU_NEW_MAP_H
#define EDITOR_MAIN_MENU_NEW_MAP_H

#include <vector>

#include "ui_basic/window.h"

struct Editor_Interactive;
namespace UI {
struct Button;
struct Textarea;
}

/**
 * This is the new map selection menu. It offers
 * the user to choose the new world and a few other
 * things like size, world ....
*/
struct Main_Menu_New_Map : public UI::Window {
	Main_Menu_New_Map(Editor_Interactive &);

private:
	UI::Textarea * m_width, * m_height;
	UI::Button * m_world;
	int32_t m_w, m_h;
	std::vector<std::string>::size_type m_currentworld;
	std::vector<std::string> m_worlds;

	void button_clicked(int32_t);
	void clicked_create_map();
};

#endif
