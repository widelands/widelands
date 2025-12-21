/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#ifndef WL_UI_SHARED_MAPTABLE_H
#define WL_UI_SHARED_MAPTABLE_H

#include <optional>
#include <string>
#include <vector>

#include "logic/map_revision.h"
#include "ui/basic/table.h"
#include "ui/shared/mapdata.h"

/**
 * A table listing all the available maps for saveloading.
 * This contains a UI model only; the callig classes have to define the data model
 * and bind the compare functions.
 */
class MapTable : public UI::Table<uintptr_t> {
public:
	MapTable(UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style);

	/// Fill the table with maps and directories.
	void fill(const std::vector<MapData>& entries, MapData::DisplayType type);

	using FilterFn = std::function<bool(MapData&)>;
	/// Fill the table by searching the file system
	void fill(const std::vector<std::string>& directories,
	          const std::string& basedir,
	          MapData::DisplayType type,
	          Widelands::Map::ScenarioTypes scenario_types,
	          FilterFn filter,
	          bool include_addon_maps = false,
	          bool show_empty_dirs = false);
	void update_table(MapData::DisplayType type, bool to_parent = false);

	[[nodiscard]] const MapData& get_selected_data() const {
		return maps_data_[get_selected()];
	}

private:
	[[nodiscard]] bool compare_players(uint32_t rowa, uint32_t rowb) const {
		return maps_data_[get(get_record(rowa))].compare_players(maps_data_[get(get_record(rowb))]);
	}

	[[nodiscard]] bool compare_mapnames(uint32_t rowa, uint32_t rowb) const {
		return maps_data_[get(get_record(rowa))].compare_names(maps_data_[get(get_record(rowb))]);
	}

	[[nodiscard]] bool compare_size(uint32_t rowa, uint32_t rowb) const {
		return maps_data_[get(get_record(rowa))].compare_size(maps_data_[get(get_record(rowb))]);
	}

	// Store directory hierarchy
	std::deque<std::vector<MapData>> parent_data_;
	std::vector<MapData> maps_data_;
};

struct MapEntry {
	MapData data;
	Widelands::MapVersion version;
};
void find_maps(const std::string& directory, std::vector<MapEntry>& results);
std::optional<MapData> newest_edited_map();

#endif  // end of include guard: WL_UI_SHARED_MAPTABLE_H
