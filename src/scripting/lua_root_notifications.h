/*
 * Copyright (C) 2025-2026 by the Widelands Development Team
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

#ifndef WL_SCRIPTING_LUA_ROOT_NOTIFICATIONS_H
#define WL_SCRIPTING_LUA_ROOT_NOTIFICATIONS_H

#include <deque>
#include <memory>

#include "scripting/lua_root.h"

namespace LuaRoot::LuaNotifications {

struct PersistenceInfo {
	std::string type;
	uint32_t serial{0U};

	static constexpr const char kMapViewJump[] = "mapview_jump";
	static constexpr const char kMapViewChangeview[] = "mapview_changeview";
	static constexpr const char kMapViewFieldClicked[] = "mapview_field_clicked";
	static constexpr const char kMapViewTrackSelection[] = "mapview_track_selection";
	static constexpr const char kMapObjectRemoved[] = "mapobject_removed";
	static constexpr const char kBuildingMuted[] = "building_muted";

	static constexpr const char kBuilding[] = "building";
	static constexpr const char kChatMessage[] = "chat_message";
	static constexpr const char kEconomy[] = "economy";
	static constexpr const char kEconomyProfile[] = "economy_profile";
	static constexpr const char kExpeditionCancelled[] = "expedition_cancelled";
	static constexpr const char kFieldPossession[] = "field_possession";
	static constexpr const char kFieldTerrainChanged[] = "field_terrain_changed";
	static constexpr const char kGraphicResolutionChanged[] = "graphic_resolution_changed";
	static constexpr const char kImmovable[] = "immovable";
	static constexpr const char kPinnedNoteMoved[] = "pinned_note_moved";
	static constexpr const char kPlayerDetailsEvent[] = "player_details";
	static constexpr const char kProductionSiteOutOfResources[] = "production_site_out_of_resources";
	static constexpr const char kQuicknavChangedEvent[] = "quicknav_changed";
	static constexpr const char kShip[] = "ship";
	static constexpr const char kTradeChanged[] = "trade_changed";
	static constexpr const char kTrainingSiteSoldierTrained[] = "training_site_soldier_trained";
};

struct Wrapper {
	virtual ~Wrapper() = default;

	LuaSubscriber* owner{nullptr};
	PersistenceInfo persistence;
};

Wrapper* create_mapview_jump(lua_State* L);
Wrapper* create_mapview_changeview(lua_State* L);
Wrapper* create_mapview_field_clicked(lua_State* L);
Wrapper* create_mapview_track_selection(lua_State* L);
Wrapper* create_map_object_removed(const Widelands::MapObject& obj);
Wrapper* create_building_muted(const Widelands::Building& obj);

Wrapper* create(const std::string& type);
Wrapper* create_building();
Wrapper* create_chat_message();
Wrapper* create_economy();
Wrapper* create_economy_profile();
Wrapper* create_expedition_cancelled();
Wrapper* create_field_possession();
Wrapper* create_field_terrain_changed();
Wrapper* create_graphic_resolution_changed();
Wrapper* create_immovable();
Wrapper* create_pinned_note_moved();
Wrapper* create_player_details();
Wrapper* create_production_site_out_of_resources();
Wrapper* create_quicknav_changed();
Wrapper* create_ship();
Wrapper* create_trade_changed();
Wrapper* create_training_site_soldier_trained();

}  // namespace LuaRoot::LuaNotifications

#endif  // end of include guard: WL_SCRIPTING_LUA_ROOT_NOTIFICATIONS_H
