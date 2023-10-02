/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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
 */

#include "wui/maptable.h"

#include <memory>

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "map_io/widelands_map_loader.h"
#include "wui/mapdata.h"

MapTable::MapTable(
   UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style)
   : UI::Table<uintptr_t>(parent, "maps_table", x, y, w, h, style) {

	/** TRANSLATORS: Column title for number of players in map list */
	add_column(35, _("Pl."), _("Number of players"), UI::Align::kCenter);
	add_column(0, _("Filename"), _("The name of the map or scenario"), UI::Align::kLeft,
	           UI::TableColumnType::kFlexible);
	add_column(90, _("Size"), _("The size of the map (Width x Height)"));
	set_sort_column(0);
}

void MapTable::fill(const std::vector<MapData>& entries, MapData::DisplayType type) {
	clear();

	for (size_t i = 0; i < entries.size(); ++i) {
		const MapData& mapdata = entries[i];
		UI::Table<uintptr_t const>::EntryRecord& te = add(i);

		if (mapdata.maptype == MapData::MapType::kDirectory) {
			te.set_string(0, "");
			te.set_picture(
			   1, g_image_cache->get("images/ui_basic/ls_dir.png"), mapdata.localized_name);
			te.set_string(2, "");
		} else {
			te.set_string(0, format("(%i)", mapdata.nrplayers));

			std::string picture = "images/ui_basic/ls_wlmap.png";
			if (mapdata.maptype == MapData::MapType::kScenario) {
				picture = "images/ui_basic/ls_wlscenario.png";
			} else if (mapdata.maptype == MapData::MapType::kSettlers2) {
				picture = "images/ui_basic/ls_s2map.png";
			}

			if (type == MapData::DisplayType::kFilenames) {
				set_column_title(1, _("Filename"));
				te.set_picture(1, g_image_cache->get(picture),
				               FileSystem::filename_without_ext(mapdata.filenames.at(0).c_str()));
			} else {
				set_column_title(1, _("Map Name"));
				if (type == MapData::DisplayType::kMapnames) {
					te.set_picture(1, g_image_cache->get(picture), mapdata.name);
				} else {
					te.set_picture(1, g_image_cache->get(picture), mapdata.localized_name);
				}
			}

			te.set_string(2, format("%u x %u", mapdata.width, mapdata.height));
		}
	}
	sort();
	layout();
}

void find_maps(const std::string& directory, std::vector<MapEntry>& results) {
	for (const std::string& file : g_fs->list_directory(directory)) {
		Widelands::Map map;
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(file);
		if (ml != nullptr) {
			try {
				map.set_filename(file);
				ml->preload_map(true, nullptr);
				if (map.version().map_version_timestamp > 0) {
					MapData::MapType type = map.scenario_types() == Widelands::Map::SP_SCENARIO ?
                                          MapData::MapType::kScenario :
                                          MapData::MapType::kNormal;
					results.emplace_back(MapEntry(
					   {MapData(map, file, type, MapData::DisplayType::kFilenames), map.version()}));
				}
			} catch (...) {
				// invalid file - silently ignore
			}
		} else if (g_fs->is_directory(file)) {
			find_maps(file, results);
		}
	}
}

std::optional<MapData> newest_edited_map() {
	std::vector<MapEntry> all_maps;
	find_maps(kMyMapsDirFull, all_maps);
	MapEntry* last_edited = nullptr;
	for (MapEntry& m : all_maps) {
		if (last_edited == nullptr ||
		    m.version.map_version_timestamp > last_edited->version.map_version_timestamp) {
			last_edited = &m;
		}
	}
	if (last_edited == nullptr) {
		return std::nullopt;
	}
	return last_edited->data;
}
