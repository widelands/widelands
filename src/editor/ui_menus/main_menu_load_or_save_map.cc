/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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
#include "graphic/font_handler.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "map_io/widelands_map_loader.h"

MainMenuLoadOrSaveMap::MainMenuLoadOrSaveMap(EditorInteractive& parent,
                                             Registry& registry,
                                             const std::string& name,
                                             const std::string& title,
                                             const std::string& basedir)
   : UI::UniqueWindow(&parent, name, &registry, parent.get_w(), parent.get_h(), title),

     // Values for alignment and size
     padding_(4),

     main_box_(this, padding_, padding_, UI::Box::Vertical, 0, 0, padding_),

     show_mapnames_box_(&main_box_, 0, 0, UI::Box::Horizontal),
     show_mapnames_(&show_mapnames_box_,
                    "show_mapnames",
                    0,
                    0,
                    0,
                    0,
                    UI::ButtonStyle::kWuiSecondary,
                    _("Show Map Names")),

     table_and_details_box_(&main_box_, 0, 0, UI::Box::Horizontal, 0, 0, padding_),

     table_(&table_and_details_box_, 0, 0, 200, 200, UI::PanelStyle::kWui),
     map_details_box_(&table_and_details_box_, 0, 0, UI::Box::Vertical, 0, 0, padding_),
     map_details_(&map_details_box_, 0, 0, 100, 100, UI::PanelStyle::kWui),

     table_footer_box_(&main_box_, 0, 0, UI::Box::Horizontal, 0, 0, padding_),

     directory_info_(&main_box_, 0, 0, 0, 0),

     // Bottom button row
     button_box_(&main_box_, 0, 0, UI::Box::Horizontal, 0, 0, padding_),
     ok_(&button_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kWuiPrimary, _("OK")),
     cancel_(&button_box_, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kWuiSecondary, _("Cancel")),

     // Options
     basedir_(basedir),
     has_translated_mapname_(false),
     showing_mapnames_(false) {

	g_fs->ensure_directory_exists(basedir_);
	curdir_ = basedir_;

	main_box_.add(&show_mapnames_box_, UI::Box::Resizing::kFullSize);
	main_box_.add(&table_and_details_box_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(padding_);
	main_box_.add(&table_footer_box_, UI::Box::Resizing::kFullSize);
	main_box_.add(&directory_info_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(padding_);
	main_box_.add(&button_box_, UI::Box::Resizing::kFullSize);

	show_mapnames_box_.add(&show_mapnames_);
	cb_dont_localize_mapnames_ =
	   /** TRANSLATORS: Checkbox title. If this checkbox is enabled, map names aren't translated. */
	   new UI::Checkbox(&show_mapnames_box_, Vector2i::zero(), _("Show original map names"));
	cb_dont_localize_mapnames_->set_state(false);
	show_mapnames_box_.add_space(2 * padding_);
	show_mapnames_box_.add(cb_dont_localize_mapnames_, UI::Box::Resizing::kFullSize);
	show_mapnames_box_.add_inf_space();

	table_.set_column_compare(0, boost::bind(&MainMenuLoadOrSaveMap::compare_players, this, _1, _2));
	table_.set_column_compare(
	   1, boost::bind(&MainMenuLoadOrSaveMap::compare_mapnames, this, _1, _2));
	table_.set_column_compare(2, boost::bind(&MainMenuLoadOrSaveMap::compare_size, this, _1, _2));

	table_and_details_box_.add(&table_, UI::Box::Resizing::kExpandBoth);
	table_and_details_box_.add_space(0);
	table_and_details_box_.add(&map_details_box_, UI::Box::Resizing::kFullSize);
	map_details_box_.add(&map_details_, UI::Box::Resizing::kExpandBoth);

	table_.focus();
	fill_table();

	button_box_.add_inf_space();
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &ok_ : &cancel_, UI::Box::Resizing::kExpandBoth);
	button_box_.add_space(padding_);
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &cancel_ : &ok_, UI::Box::Resizing::kExpandBoth);
	button_box_.add_inf_space();

	// We don't need the unlocalizing option if there is nothing to unlocalize.
	// We know this after the list is filled.
	cb_dont_localize_mapnames_->set_visible(has_translated_mapname_);
	cb_dont_localize_mapnames_->changedto.connect(
	   boost::bind(&MainMenuLoadOrSaveMap::fill_table, boost::ref(*this)));
	show_mapnames_.sigclicked.connect(
	   boost::bind(&MainMenuLoadOrSaveMap::toggle_mapnames, boost::ref(*this)));

	move_to_top();
	layout();
}

bool MainMenuLoadOrSaveMap::compare_players(uint32_t rowa, uint32_t rowb) {
	return maps_data_[table_[rowa]].compare_players(maps_data_[table_[rowb]]);
}

bool MainMenuLoadOrSaveMap::compare_mapnames(uint32_t rowa, uint32_t rowb) {
	return maps_data_[table_[rowa]].compare_names(maps_data_[table_[rowb]]);
}

bool MainMenuLoadOrSaveMap::compare_size(uint32_t rowa, uint32_t rowb) {
	return maps_data_[table_[rowa]].compare_size(maps_data_[table_[rowb]]);
}

void MainMenuLoadOrSaveMap::toggle_mapnames() {
	if (showing_mapnames_) {
		show_mapnames_.set_title(_("Show Map Names"));
	} else {
		show_mapnames_.set_title(_("Show Filenames"));
	}
	showing_mapnames_ = !showing_mapnames_;
	fill_table();
}

void MainMenuLoadOrSaveMap::layout() {
	main_box_.set_size(get_inner_w() - 2 * padding_, get_inner_h() - 2 * padding_);

	// Set the width. Height is controlled by expanding into the outer box
	map_details_box_.set_desired_size(main_box_.get_w() / 3, 100);

	center_to_parent();
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
	if (!showing_mapnames_) {
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
