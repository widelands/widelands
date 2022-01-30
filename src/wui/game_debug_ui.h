/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */
// UI classes for real-time game debugging

#ifndef WL_WUI_GAME_DEBUG_UI_H
#define WL_WUI_GAME_DEBUG_UI_H

#include "logic/map_objects/map_object.h"
#include "logic/widelands_geometry.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/tabpanel.h"
#include "wui/interactive_base.h"

// Open debug window for the given coordinates
void show_mapobject_debug(InteractiveBase& parent, Widelands::MapObject&);
void show_field_debug(InteractiveBase& parent, const Widelands::Coords& coords);

class MapObjectDebugPanel : public UI::Panel, public Widelands::MapObject::LogSink {
public:
	MapObjectDebugPanel(UI::Panel& parent, const Widelands::EditorGameBase&, Widelands::MapObject&);
	~MapObjectDebugPanel() override;

	void log(const std::string& str) override;

private:
	const Widelands::EditorGameBase& egbase_;
	Widelands::ObjectPointer object_;

	UI::MultilineTextarea log_;
};

/*
MapObjectDebugWindow
--------------------
The map object debug window is basically just a simple container for tabs
that are provided by the map object itself via the virtual function
collect_debug_tabs().
*/
class MapObjectDebugWindow : public UI::Window {
public:
	MapObjectDebugWindow(InteractiveBase& parent, Widelands::MapObject&);

	InteractiveBase& ibase() {
		return dynamic_cast<InteractiveBase&>(*get_parent());
	}

	void think() override;

private:
	bool log_general_info_;
	Widelands::ObjectPointer object_;
	uint32_t serial_;
	UI::TabPanel tabs_;
};

class FieldDebugWindow : public UI::Window {
public:
	FieldDebugWindow(InteractiveBase& parent, Widelands::Coords);

	InteractiveBase& ibase() {
		return dynamic_cast<InteractiveBase&>(*get_parent());
	}

	void think() override;

	void open_immovable();
	void open_bob(uint32_t);

private:
	std::string text_;
	const Widelands::Map& map_;
	Widelands::FCoords const coords_;

	UI::MultilineTextarea ui_field_;
	UI::Button ui_immovable_;
	UI::Listselect<intptr_t> ui_bobs_;
};

#endif  // end of include guard: WL_WUI_GAME_DEBUG_UI_H
