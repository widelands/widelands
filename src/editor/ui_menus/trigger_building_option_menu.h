/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#ifndef __S__TRIGGER_BUILDING_OPTION_MENU_H
#define __S__TRIGGER_BUILDING_OPTION_MENU_H

#include "player_area.h"
#include "building.h"

#include <stdint.h>
#include <string>
#include <vector>
#include "ui_window.h"

class Editor_Interactive;
class Trigger_Building;
namespace UI {
struct Edit_Box;
struct Textarea;
};

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
struct Trigger_Building_Option_Menu : public UI::Window {
      Trigger_Building_Option_Menu(Editor_Interactive*, Trigger_Building*);

	bool handle_mousepress  (const Uint8 btn, int32_t x, int32_t y);
	bool handle_mouserelease(const Uint8 btn, int32_t x, int32_t y);

private:
      void update();
	void clicked_ok();
      void clicked(int32_t);

      Trigger_Building* m_trigger;
      Editor_Interactive* m_parent;
      UI::Textarea *m_player_ta, *m_area_ta, *m_x_ta, *m_y_ta, *m_count_ta, *m_building_ta;
      UI::Edit_Box* m_name;
	Player_Area<Area<FCoords> > m_player_area;
	uint32_t m_count;
	//Building_Descr::Index m_building;
	//TODO: line above is type-correct, line below is signedness-correct. Merge them.
	int32_t m_building;
      std::vector<std::string> m_buildings;
};

#endif
