/*
 * Copyright (C) 2002-2004, 2006-2015 by the Widelands Development Team
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

#include "editor/ui_menus/main_menu_load_or_save_map.h"

#include <cstdio>
#include <memory>

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "map_io/widelands_map_loader.h"

MainMenuLoadOrSaveMap::MainMenuLoadOrSaveMap(EditorInteractive& parent,
                                             const std::string& name,
                                             const std::string& title,
                                             const std::string& basedir)
   : UI::Window(&parent, name, 0, 0, parent.get_inner_w() - 40, parent.get_inner_h() - 40, title),

     // Values for alignment and size
     padding_(4),
     buth_(20),
     tablex_(padding_),
     tabley_(buth_ + 2 * padding_),
     tablew_(get_inner_w() * 7 / 12),
     tableh_(get_inner_h() - tabley_ - 3 * buth_ - 2 * padding_),
     right_column_x_(tablew_ + 2 * padding_),
     butw_((get_inner_w() - right_column_x_ - 2 * padding_) / 2),

	  table_(this, tablex_, tabley_, tablew_, tableh_, false),
	  map_details_(
		  this, right_column_x_, tabley_, get_inner_w() - right_column_x_ - padding_, tableh_,
		  MapDetails::Style::kWui),
     ok_(this,
         "ok",
			UI::g_fh1->fontset()->is_rtl() ? get_inner_w() / 2 - butw_ - padding_ : get_inner_w() / 2 + padding_,
         get_inner_h() - padding_ - buth_,
         butw_,
         buth_,
			g_gr->images().get("images/ui_basic/but5.png"),
         _("OK")),
     cancel_(this,
             "cancel",
				 UI::g_fh1->fontset()->is_rtl() ?
					 get_inner_w() / 2 + padding_ :
					 get_inner_w() / 2 - butw_ - padding_,
             get_inner_h() - padding_ - buth_,
             butw_,
             buth_,
				 g_gr->images().get("images/ui_basic/but1.png"),
             _("Cancel")),
     basedir_(basedir),
     has_translated_mapname_(false),
     showing_mapames_(false) {
	g_fs->ensure_directory_exists(basedir_);
	curdir_ = basedir_;

	UI::Box* vbox = new UI::Box(this, tablex_, padding_, UI::Box::Horizontal, padding_, get_w());
	show_mapnames_ = new UI::Button(vbox,
	                                "show_mapnames",
	                                0,
	                                0,
	                                butw_,
	                                buth_,
											  g_gr->images().get("images/ui_basic/but1.png"),
	                                _("Show Map Names"));
	vbox->add(show_mapnames_, UI::Align::kLeft, true);

	/** TRANSLATORS: Checkbox title. If this checkbox is enabled, map names aren't translated. */
	cb_dont_localize_mapnames_ = new UI::Checkbox(vbox, Point(0, 0), _("Show original map names"));
	cb_dont_localize_mapnames_->set_state(false);
	vbox->add_space(2 * padding_);
	vbox->add(cb_dont_localize_mapnames_, UI::Align::kLeft, true);
	vbox->set_size(get_inner_w(), buth_);

	table_.set_column_compare(0, boost::bind(&MainMenuLoadOrSaveMap::compare_players, this, _1, _2));
	table_.set_column_compare(1, boost::bind(&MainMenuLoadOrSaveMap::compare_mapnames, this, _1, _2));
	table_.set_column_compare(2, boost::bind(&MainMenuLoadOrSaveMap::compare_size, this, _1, _2));


	table_.focus();
	fill_table();

	// We don't need the unlocalizing option if there is nothing to unlocalize.
	// We know this after the list is filled.
	cb_dont_localize_mapnames_->set_visible(has_translated_mapname_);
	cb_dont_localize_mapnames_->changedto.connect(
	   boost::bind(&MainMenuLoadOrSaveMap::fill_table, boost::ref(*this)));
	show_mapnames_->sigclicked.connect(
	   boost::bind(&MainMenuLoadOrSaveMap::toggle_mapnames, boost::ref(*this)));

	center_to_parent();
	move_to_top();
}


bool MainMenuLoadOrSaveMap::compare_players(uint32_t rowa, uint32_t rowb)
{
	return maps_data_[table_[rowa]].compare_players(maps_data_[table_[rowb]]);
}


bool MainMenuLoadOrSaveMap::compare_mapnames(uint32_t rowa, uint32_t rowb)
{
	return maps_data_[table_[rowa]].compare_names(maps_data_[table_[rowb]]);
}


bool MainMenuLoadOrSaveMap::compare_size(uint32_t rowa, uint32_t rowb)
{
	return maps_data_[table_[rowa]].compare_size(maps_data_[table_[rowb]]);
}


void MainMenuLoadOrSaveMap::toggle_mapnames() {
	if (showing_mapames_) {
		show_mapnames_->set_title(_("Show Map Names"));
	} else {
		show_mapnames_->set_title(_("Show Filenames"));
	}
	showing_mapames_ = !showing_mapames_;
	fill_table();
}

/**
 * fill the file list
 */
void MainMenuLoadOrSaveMap::fill_table() {
	table_.clear();
	maps_data_.clear();
	has_translated_mapname_ = false;

	//  Fill it with all files we find.
	FilenameSet files = g_fs->list_directory(curdir_);


	// If we are not at the top of the map directory hierarchy (we're not talking
	// about the absolute filesystem top!) we manually add ".."
	if (curdir_ != basedir_) {
		maps_data_.push_back(MapData::create_parent_dir(curdir_));
	} else if (files.empty()) {
		maps_data_.push_back(MapData::create_empty_dir(curdir_));
	}

	MapData::DisplayType display_type;
	if (!showing_mapames_) {
		display_type = MapData::DisplayType::kFilenames;
	} else if (cb_dont_localize_mapnames_->get_state()) {
		display_type = MapData::DisplayType::kMapnames;
	} else {
		display_type = MapData::DisplayType::kMapnamesLocalized;
	}

	Widelands::Map map;

	for (const std::string& mapfilename : files) {
		// Add map file (compressed) or map directory (uncompressed)
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(mapfilename);
		if (ml != nullptr) {
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

				MapData mapdata(map, mapfilename, maptype, display_type);

				has_translated_mapname_ =
					has_translated_mapname_ || (mapdata.name != mapdata.localized_name);

				maps_data_.push_back(mapdata);

			} catch (const WException&) {
			}  //  we simply skip illegal entries
		} else if (g_fs->is_directory(mapfilename)) {
			// Add subdirectory to the list
			const char* fs_filename = FileSystem::fs_filename(mapfilename.c_str());
			if (!strcmp(fs_filename, ".") || !strcmp(fs_filename, ".."))
				continue;
			maps_data_.push_back(MapData::create_directory(mapfilename));
		}
	}

	table_.fill(maps_data_, display_type);
	if (!table_.empty()) {
		table_.select(0);
	} else {
		ok_.set_enabled(false);
	}
}
