/*
 * Copyright (C) 2002-2004, 2006-2012 by the Widelands Development Team
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

#include "editor/ui_menus/editor_main_menu_load_map.h"

#include <cstdio>
#include <memory>


#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/map.h"
#include "map_io/map_loader.h"
#include "map_io/widelands_map_loader.h"

using Widelands::WidelandsMapLoader;

/**
 * Create all the buttons etc...
*/
MainMenuLoadMap::MainMenuLoadMap(EditorInteractive & parent)
	: UI::Window(&parent, "load_map_menu",
					 0, 0, parent.get_inner_w() - 80, parent.get_inner_h() - 80,
					 _("Load Map")),

	  // Values for alignment and size
	  padding_(4),
	  butw_(get_inner_w() / 4 - 1.5 * padding_),
	  buth_(20),
	  tablex_(padding_),
	  tabley_(padding_),
	  tablew_(get_inner_w() * 2 / 3 - 2 * padding_),
	  tableh_(get_inner_h() - buth_ - 4 * padding_),
	  right_column_x_(tablew_ + 2 * padding_),
	  table_(this, tablex_, tabley_, tablew_, tableh_, false),
	  map_details_(
		  this, right_column_x_, tabley_,
		  get_inner_w() - right_column_x_ - padding_,
		  tableh_),
	  ok_(
		  this, "ok",
		  get_inner_w() - butw_ - padding_, get_inner_h() - padding_ - buth_,
		  butw_, buth_,
		  g_gr->images().get("pics/but0.png"),
		  _("OK")),
	  cancel_(
		  this, "cancel",
		  get_inner_w() - 2 * butw_ - 2 * padding_, get_inner_h() - padding_ - buth_,
		  butw_, buth_,
		  g_gr->images().get("pics/but1.png"),
		  _("Cancel")),
	  basedir_("maps") {
	curdir_ = basedir_;

	table_.selected.connect(boost::bind(&MainMenuLoadMap::selected, this));
	table_.double_clicked.connect(boost::bind(&MainMenuLoadMap::clicked_ok, boost::ref(*this)));
	table_.focus();
	fill_table();

	ok_.sigclicked.connect(boost::bind(&MainMenuLoadMap::clicked_ok, this));
	cancel_.sigclicked.connect(boost::bind(&MainMenuLoadMap::die, this));

	center_to_parent();
	move_to_top();
}


void MainMenuLoadMap::clicked_ok() {
	assert(table_.has_selection());
	const MapData& mapdata = *table_.get_map();
	if (g_fs->is_directory(mapdata.filename) && !WidelandsMapLoader::is_widelands_map(mapdata.filename)) {
		curdir_ = mapdata.filename;
		fill_table();
	} else {
		dynamic_cast<EditorInteractive&>(*get_parent()).load(mapdata.filename);
		die();
	}
}

bool MainMenuLoadMap::set_has_selection()
{
	bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);

	if (!has_selection) {
		map_details_.clear();
	}
	return has_selection;
}

/**
 * Called when a entry is selected
 */
void MainMenuLoadMap::selected() {
	if (set_has_selection()) {
		map_details_.update(*table_.get_map(), false); // NOCOM localize?
	}
}


/**
 * fill the file list
 */
void MainMenuLoadMap::fill_table() {
	std::vector<MapData> maps_data;
	table_.clear();

	//  Fill it with all files we find.
	FilenameSet files = g_fs->list_directory(curdir_);

	//If we are not at the top of the map directory hierarchy (we're not talking
	//about the absolute filesystem top!) we manually add ".."
	if (curdir_ != basedir_) {
		MapData mapdata;
#ifndef _WIN32
		mapdata.filename = curdir_.substr(0, curdir_.rfind('/'));
#else
		mapdata.filename = curdir_.substr(0, curdir_.rfind('\\'));
#endif
		mapdata.localized_name = (boost::format("\\<%s\\>") % _("parent")).str();
		mapdata.maptype = MapData::MapType::kDirectory;
		maps_data.push_back(mapdata);
	}

	//Add subdirectories to the list (except for uncompressed maps)
	for (const std::string& mapfilename : files) {
		char const * const name = mapfilename.c_str();
		if (!strcmp(FileSystem::fs_filename(name), "."))
			continue;
		// Upsy, appeared again. ignore
		if (!strcmp(FileSystem::fs_filename(name), ".."))
			continue;
		if (!g_fs->is_directory(name))
			continue;
		if (WidelandsMapLoader::is_widelands_map(name))
			continue;

		MapData mapdata;
		mapdata.filename = name;
		if (strcmp (name, "maps/MP Scenarios") == 0) {
			/** TRANSLATORS: Directory name for MP Scenarios in map selection */
			mapdata.localized_name = _("Multiplayer Scenarios");
		} else {
			mapdata.localized_name = FileSystem::fs_filename(name);
		}
		mapdata.maptype = MapData::MapType::kDirectory;
		maps_data.push_back(mapdata);
	}

	//Add map files(compressed maps) and directories(uncompressed)
	Widelands::Map map;

	for (const std::string& mapfilename : files) {
		char const * const name = mapfilename.c_str();

		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(name);
		if (ml.get() != nullptr) {
			try {
				ml->preload_map(true);

				i18n::Textdomain td("maps");

				MapData mapdata;
				mapdata.filename       = mapfilename;
				mapdata.name           = map.get_name();
				mapdata.localized_name = mapdata.name.empty() ? "" : _(mapdata.name);
				mapdata.authors.parse(map.get_author());
				mapdata.description    = map.get_description().empty() ? "" : _(map.get_description());
				mapdata.hint           = map.get_hint().empty() ? "" : _(map.get_hint());
				mapdata.nrplayers      = map.get_nrplayers();
				mapdata.width          = map.get_width();
				mapdata.height         = map.get_height();
				mapdata.suggested_teams = map.get_suggested_teams();

				if (map.scenario_types() & Widelands::Map::MP_SCENARIO ||
					 map.scenario_types() & Widelands::Map::SP_SCENARIO) {
					mapdata.maptype = MapData::MapType::kScenario;
					} else if (dynamic_cast<WidelandsMapLoader*>(ml.get())) {
					mapdata.maptype = MapData::MapType::kNormal;
				} else {
					mapdata.maptype = MapData::MapType::kSettlers2;
				}

				if (!mapdata.width || !mapdata.height) {
					continue;
				}
				maps_data.push_back(mapdata);

			} catch (const WException &) {} //  we simply skip illegal entries
		}
	}

	table_.fill(maps_data, false); // NOCOM(GunChleoc): Do we want to localize the map names here?
	set_has_selection();
}
