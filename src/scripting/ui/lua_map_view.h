/*
 * Copyright (C) 2006-2025 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_UI_LUA_MAP_VIEW_H
#define WL_SCRIPTING_UI_LUA_MAP_VIEW_H

#include "scripting/ui/lua_panel.h"
#include "wui/interactive_base.h"

namespace LuaUi {

class LuaMapView : public LuaPanel {
public:
	LUNA_CLASS_HEAD(LuaMapView);

	LuaMapView() = default;
	explicit LuaMapView(MapView* p) : LuaPanel(p) {
	}
	explicit LuaMapView(lua_State* L);
	~LuaMapView() override = default;

	CLANG_DIAG_RESERVED_IDENTIFIER_OFF
	void __persist(lua_State*) override {
	}
	void __unpersist(lua_State* L) override;
	CLANG_DIAG_RESERVED_IDENTIFIER_ON

	/*
	 * Properties
	 */
	int get_average_fps(lua_State* L);
	int get_center_map_pixel(lua_State*);
	int get_buildhelp(lua_State* L);
	int set_buildhelp(lua_State* L);
	int get_census(lua_State* L);
	int set_census(lua_State* L);
	int get_statistics(lua_State* L);
	int set_statistics(lua_State* L);
	int get_is_building_road(lua_State* L);
	int get_auto_roadbuilding_mode(lua_State* L);
	int get_is_animating(lua_State*);
	int get_toolbar(lua_State* L);

	/*
	 * Lua Methods
	 */
	int close(lua_State* L);
	int click(lua_State* L);
	int abort_road_building(lua_State* L);
	int start_road_building(lua_State* L);
	int scroll_to_map_pixel(lua_State* L);
	int scroll_to_field(lua_State* L);
	int is_visible(lua_State* L);
	int mouse_to_field(lua_State* L);
	int mouse_to_pixel(lua_State* L);
	int add_toolbar_plugin(lua_State* L);
	int update_toolbar(lua_State* L);
	int set_keyboard_shortcut(lua_State* L);
	int set_keyboard_shortcut_release(lua_State* L);
	int add_plugin_timer(lua_State* L);
	int subscribe_to_jump(lua_State* L);
	int subscribe_to_changeview(lua_State* L);
	int subscribe_to_track_selection(lua_State* L);
	int subscribe_to_field_clicked(lua_State* L);

	/*
	 * C Methods
	 */
	InteractiveBase* get() {
		return dynamic_cast<InteractiveBase*>(panel_);
	}
};

}  // namespace LuaUi

#endif  // end of include guard: WL_SCRIPTING_UI_LUA_MAP_VIEW_H
