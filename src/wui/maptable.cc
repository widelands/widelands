/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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
 */

#include "wui/maptable.h"

#include "base/i18n.h"
#include "graphic/image_cache.h"
#include "io/filesystem/filesystem.h"

MapTable::MapTable(
   UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style)
   : UI::Table<uintptr_t>(parent, x, y, w, h, style) {

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
			te.set_string(0, bformat("(%i)", mapdata.nrplayers));

			std::string picture = "images/ui_basic/ls_wlmap.png";
			if (mapdata.maptype == MapData::MapType::kScenario) {
				picture = "images/ui_basic/ls_wlscenario.png";
			} else if (mapdata.maptype == MapData::MapType::kSettlers2) {
				picture = "images/ui_basic/ls_s2map.png";
			}

			if (type == MapData::DisplayType::kFilenames) {
				set_column_title(1, _("Filename"));
				te.set_picture(1, g_image_cache->get(picture),
				               FileSystem::filename_without_ext(mapdata.filename.c_str()));
			} else {
				set_column_title(1, _("Map Name"));
				if (type == MapData::DisplayType::kMapnames) {
					te.set_picture(1, g_image_cache->get(picture), mapdata.name);
				} else {
					te.set_picture(1, g_image_cache->get(picture), mapdata.localized_name);
				}
			}

			te.set_string(2, bformat("%u x %u", mapdata.width, mapdata.height));
		}
	}
	sort();
	layout();
}
