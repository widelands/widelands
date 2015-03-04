/*
 * Copyright (C) 2002, 2006-2013 by the Widelands Development Team
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

#include <string>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"

MapTable::MapTable
		(UI::Panel * parent,
		 int32_t x, int32_t y, uint32_t w, uint32_t h, Type type,
		 const bool descending) :
	UI::Table<uintptr_t>(parent, x, y, w, h, descending),
	type_(type),
	localize_map_names_(true) {

	/** TRANSLATORS: Column title for number of players in map list */
	add_column(35, _("Pl."), _("Number of players"), UI::Align_HCenter);
	if (type_ == MapTable::Type::kFilenames) {
		add_column(get_w() - 35 - 115, _("Filename"), _("The filename of the map or scenario"),
					  UI::Align_Left);
	} else {
		add_column(get_w() - 35 - 115, _("Map Name"), _("The name of the map or scenario"),
					  UI::Align_Left);
	}
	add_column(115, _("Size"), _("The size of the map (Width x Height)"), UI::Align_Left);
	set_column_compare
		(0,
		 boost::bind(&MapTable::compare_players, this, _1, _2));
	set_column_compare
		(1,
		 boost::bind(&MapTable::compare_mapnames, this, _1, _2));
	set_column_compare
		(2,
		 boost::bind(&MapTable::compare_size, this, _1, _2));
	set_sort_column(0);
}

// NOCOM fix column sorting

bool MapTable::compare_players(uint32_t rowa, uint32_t rowb)
{
	const MapData & r1 = maps_data_[rowa];
	const MapData & r2 = maps_data_[rowb];

	if (r1.nrplayers == r2.nrplayers) {
		return compare_mapnames(rowa, rowb);
	}
	return r1.nrplayers < r2.nrplayers;
}


bool MapTable::compare_mapnames(uint32_t rowa, uint32_t rowb)
{
	const MapData & r1 = maps_data_[rowa];
	const MapData & r2 = maps_data_[rowb];

	if (!r1.width && !r2.width) {
		return r1.name < r2.name;
	} else if (!r1.width && r2.width) {
		return true;
	} else if (r1.width && !r2.width) {
		return false;
	} else if (!localize_map_names_) {
		return r1.name < r2.name;
	}
	return r1.localized_name < r2.localized_name;
}


bool MapTable::compare_size(uint32_t rowa, uint32_t rowb)
{
	const MapData & r1 = maps_data_[rowa];
	const MapData & r2 = maps_data_[rowb];

	if (r1.width != r2.width) {
		return r1.width < r2.width;
	} else if (r1.height == r2.height) {
		return compare_mapnames(rowa, rowb);
	}
	return r1.height < r2.height;
}

const MapData* MapTable::get_map() const {
	if (!has_selection()) {
		return nullptr;
	}
	return &maps_data_[get_selected()];
}


void MapTable::fill(std::vector<MapData> entries, bool localize_map_names)
{
	localize_map_names_ = localize_map_names;
	uint8_t col_players = 0;
	uint8_t col_name = 1;
	uint8_t col_size = 2;

	maps_data_ = entries;
	clear();

	for (size_t i = 0; i < maps_data_.size(); ++i) {
		const MapData& mapdata = maps_data_[i];
		UI::Table<uintptr_t const>::EntryRecord& te = add(i);

		if (mapdata.maptype == MapData::MapType::kDirectory) {
			te.set_string(col_players, "");
			te.set_picture
				(col_name,  g_gr->images().get("pics/ls_dir.png"),
				mapdata.localized_name);
			te.set_string(col_size, "");
		} else {
			te.set_string(col_players, (boost::format("(%i)") % mapdata.nrplayers).str());

			std::string picture = "pics/ls_wlmap.png";
			if (mapdata.maptype == MapData::MapType::kScenario) {
				picture = "pics/ls_wlscenario.png";
			} else if (mapdata.maptype == MapData::MapType::kSettlers2) {
				picture = "pics/ls_s2map.png";
			}

			if (type_ == MapTable::Type::kFilenames) {
				te.set_picture(
							col_name,
							g_gr->images().get(picture),
							FileSystem::filename_without_ext(mapdata.filename.c_str()));
			} else {
				te.set_picture(
							col_name,
							g_gr->images().get(picture),
							localize_map_names_ ? mapdata.localized_name : mapdata.name);
			}

			te.set_string(col_size, (boost::format("%u x %u") % mapdata.width % mapdata.height).str());
		}
	}
	sort();
}
