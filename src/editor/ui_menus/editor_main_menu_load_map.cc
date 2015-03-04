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
	  tabley_(buth_ + 2 * padding_),
	  tablew_(get_inner_w() * 2 / 3 - 2 * padding_),
	  tableh_(get_inner_h() - tabley_ - buth_ - 4 * padding_),
	  right_column_x_(tablew_ + 2 * padding_),
	  table_(this, tablex_, tabley_, tablew_, tableh_, MapTable::Type::kMapnames, false),
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
	  basedir_("maps"),
	  has_translated_mapname_(false) {
	curdir_ = basedir_;

	UI::Box* vbox = new UI::Box(this, tablex_, padding_,
										 UI::Box::Horizontal, padding_, get_w());
	cb_dont_localize_mapnames_ = new UI::Checkbox(vbox, Point(0, 0));
	cb_dont_localize_mapnames_->set_state(false);
	cb_dont_localize_mapnames_->changedto.connect
			(boost::bind(&MainMenuLoadMap::fill_table, boost::ref(*this)));
	vbox->add(cb_dont_localize_mapnames_, UI::Box::AlignLeft, true);
	UI::Textarea * ta_dont_localize_mapnames =
			/** TRANSLATORS: Checkbox title. If this checkbox is enabled, map names aren't translated. */
			new UI::Textarea(vbox, _("Show original map names"), UI::Align_CenterLeft);
	vbox->add_space(padding_);
	vbox->add(ta_dont_localize_mapnames, UI::Box::AlignLeft);
	vbox->set_size(get_inner_w(), buth_);

	table_.selected.connect(boost::bind(&MainMenuLoadMap::entry_selected, this));
	table_.double_clicked.connect(boost::bind(&MainMenuLoadMap::clicked_ok, boost::ref(*this)));
	table_.focus();
	fill_table();

	ok_.sigclicked.connect(boost::bind(&MainMenuLoadMap::clicked_ok, this));
	cancel_.sigclicked.connect(boost::bind(&MainMenuLoadMap::die, this));

	// We don't need the unlocalizing option if there is nothing to unlocalize.
	// We know this after the list is filled.
	cb_dont_localize_mapnames_->set_visible(has_translated_mapname_);
	ta_dont_localize_mapnames->set_visible(has_translated_mapname_);

	center_to_parent();
	move_to_top();
}


void MainMenuLoadMap::clicked_ok() {
	assert(table_.has_selection());
	const MapData& mapdata = *table_.get_map();
	if (g_fs->is_directory(mapdata.filename) && !Widelands::WidelandsMapLoader::is_widelands_map(mapdata.filename)) {
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
void MainMenuLoadMap::entry_selected() {
	if (set_has_selection()) {
		map_details_.update(*table_.get_map(), !cb_dont_localize_mapnames_->get_state());
	}
}


/**
 * fill the file list
 */
void MainMenuLoadMap::fill_table() {
	std::vector<MapData> maps_data;
	table_.clear();
	has_translated_mapname_ = false;

	//  Fill it with all files we find.
	FilenameSet files = g_fs->list_directory(curdir_);

	//If we are not at the top of the map directory hierarchy (we're not talking
	//about the absolute filesystem top!) we manually add ".."
	if (curdir_ != basedir_) {
		maps_data.push_back(MapData::create_parent_dir(curdir_));
	}

	Widelands::Map map;

	for (const std::string& mapfilename : files) {

		// Add map file (compressed) or map directory (uncompressed)
		if (Widelands::WidelandsMapLoader::is_widelands_map(mapfilename)) {
			std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(mapfilename);
			if (ml.get() != nullptr) {
				try {
					ml->preload_map(true);

					if (!map.get_width() || !map.get_height()) {
						continue;
					}

					MapData::MapType maptype;

					if (map.scenario_types() & Widelands::Map::MP_SCENARIO ||
						 map.scenario_types() & Widelands::Map::SP_SCENARIO) {
						maptype = MapData::MapType::kScenario;
						} else if (dynamic_cast<Widelands::WidelandsMapLoader*>(ml.get())) {
						maptype = MapData::MapType::kNormal;
					} else {
						maptype = MapData::MapType::kSettlers2;
					}

					MapData mapdata(map, mapfilename, maptype);

					has_translated_mapname_ =
							has_translated_mapname_ || (mapdata.name != mapdata.localized_name);

					maps_data.push_back(mapdata);

				} catch (const WException &) {} //  we simply skip illegal entries
			}
		} else if (g_fs->is_directory(mapfilename)) {
			// Add subdirectory to the list
			const char* fs_filename = FileSystem::fs_filename(mapfilename.c_str());
			if (!strcmp(fs_filename, ".") || !strcmp(fs_filename, ".."))
				continue;
			maps_data.push_back(MapData::create_directory(mapfilename));
		}
	}

	table_.fill(maps_data, !cb_dont_localize_mapnames_->get_state());
	set_has_selection();
}
