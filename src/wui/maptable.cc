/*
 * Copyright (C) 2002-2026 by the Widelands Development Team
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

	set_column_compare(0, [this](uint32_t a, uint32_t b) { return compare_players(a, b); });
	set_column_compare(1, [this](uint32_t a, uint32_t b) { return compare_mapnames(a, b); });
	set_column_compare(2, [this](uint32_t a, uint32_t b) { return compare_size(a, b); });

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

/**
 * Shared code to fill the list with maps that can be opened.
 * FsMenu::MapSelect, editor/ui_menus/MainMenuLoadOrSaveMap *
 *
 * At first, only the subdirectories are added to the list, then the normal
 * files follow. This is done to make navigation easier.
 *
 * To make things more difficult, we have to support compressed and uncompressed
 * map files here - the former are files, the latter are directories. Care must
 * be taken to sort uncompressed maps (which look like and really are
 * directories) with the files.
 *
 * The search starts in \ref basedir ("..../maps") and there is no possibility
 * to move further up. If the user moves down into subdirectories, we insert an
 * entry to move back up.
 *
 * \ref display_type: choose localized or unlocalized map names
 *
 * Filter settings:
 * \ref scenario_types:     visible scenarios
 * \ref filter:             function to filter on MapData per map, e.g. for tags
 * \ref include_addon_maps: show folder with maps from add-ons
 * \ref show_empty_dirs:    show folders without maps
 *
 */
void MapTable::fill(const std::vector<std::string>& directories,
                    const std::string& basedir,
                    MapData::DisplayType display_type,
                    Widelands::Map::ScenarioTypes scenario_types,
                    FilterFn filter,
                    bool include_addon_maps,
                    bool show_empty_dirs) {
	clear();
	parent_data_.push_front(maps_data_);
	maps_data_.clear();

	//  Fill it with all files we find.
	assert(!directories.empty());
	FilenameSet files;
	for (const std::string& dir : directories) {
		FilenameSet f = g_fs->list_directory(dir);
		files.insert(f.begin(), f.end());
	}

	// If we are not at the top of the map directory hierarchy (we're not talking
	// about the absolute filesystem top!) we manually add ".."
	if (directories.at(0) != basedir) {
		maps_data_.push_back(MapData::create_parent_dir(directories.at(0)));
	} else {
		if (files.empty()) {
			maps_data_.push_back(MapData::create_empty_dir(directories.at(0)));
		}
		// In the toplevel directory we also need to include add-on maps –
		// but only in the load screen, not in the save screen!
		if (include_addon_maps) {
			for (const auto& addon : AddOns::g_addons) {
				if (addon.first->category == AddOns::AddOnCategory::kMaps && addon.second) {
					for (const std::string& mapname : g_fs->list_directory(
					        kAddOnDir + FileSystem::file_separator() + addon.first->internal_name)) {
						files.insert(mapname);
					}
				}
			}
		}
	}

	Widelands::Map map;  //  MapLoader needs a place to put its preload data

	for (const std::string& mapfilename : files) {
		// Add map file (compressed) or map directory (uncompressed)
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(mapfilename);
		if (ml != nullptr) {
			try {
				ml->preload_map(true, nullptr);

				if ((map.get_width() == 0) || (map.get_height() == 0)) {
					continue;
				}

				MapData::MapType maptype;

				if ((map.scenario_types() & scenario_types) != 0u) {
					maptype = MapData::MapType::kScenario;
				} else if (dynamic_cast<Widelands::WidelandsMapLoader*>(ml.get()) != nullptr) {
					maptype = MapData::MapType::kNormal;
				} else {
					maptype = MapData::MapType::kSettlers2;
				}

				MapData mapdata(map, mapfilename, maptype, display_type);
				if (!filter(mapdata)) {
					continue;
				}

				maps_data_.push_back(mapdata);
			} catch (const std::exception& e) {
				log_warn("Map list: Skip %s due to preload error: %s\n", mapfilename.c_str(), e.what());
			}  //  we simply skip illegal entries
		} else if (g_fs->is_directory(mapfilename) &&
		           (show_empty_dirs || !g_fs->list_directory(mapfilename).empty())) {
			// Add subdirectory to the list
			const char* fs_filename = FileSystem::fs_filename(mapfilename.c_str());
			if ((strcmp(fs_filename, ".") == 0) || (strcmp(fs_filename, "..") == 0)) {
				continue;
			}

			MapData new_md = MapData::create_directory(mapfilename);

			auto found =
			   std::find_if(maps_data_.begin(), maps_data_.end(), [&new_md](const MapData& md) {
				   return md.maptype == MapData::MapType::kDirectory &&
				          md.localized_name == new_md.localized_name;
			   });

			if (found != maps_data_.end()) {
				found->add(new_md);
			} else {
				maps_data_.push_back(new_md);
			}
		}
	}

	update_table(display_type);
}

void MapTable::update_table(MapData::DisplayType display_type, bool to_parent) {
	if (to_parent) {
		clear();
		maps_data_.clear();
		maps_data_ = std::move(parent_data_.at(0));
		parent_data_.pop_front();
	}
	fill(maps_data_, display_type);
	if (!empty()) {
		select(0);
	}
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
