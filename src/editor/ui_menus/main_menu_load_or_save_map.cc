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
 *
 */

#include "editor/ui_menus/main_menu_load_or_save_map.h"

#include <memory>

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "graphic/font_handler.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/addons.h"
#include "map_io/widelands_map_loader.h"

MainMenuLoadOrSaveMap::MainMenuLoadOrSaveMap(EditorInteractive& parent,
                                             Registry& registry,
                                             const std::string& name,
                                             const std::string& title,
                                             bool addons,
                                             bool show_empty_dirs,
                                             const std::string& basedir)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, name, &registry, parent.get_w(), parent.get_h(), title),

     show_empty_dirs_(show_empty_dirs),

     main_box_(this, UI::PanelStyle::kWui, padding_, padding_, UI::Box::Vertical, 0, 0, padding_),

     table_and_details_box_(
        &main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal, 0, 0, padding_),
     table_box_(
        &table_and_details_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical, 0, 0, padding_),

     table_(&table_box_, 0, 0, 200, 200, UI::PanelStyle::kWui),
     egbase_(nullptr),
     map_details_box_(
        &table_and_details_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical, 0, 0, padding_),
     map_details_(&map_details_box_, 0, 0, 100, 100, UI::PanelStyle::kWui, egbase_),

     display_mode_(&table_box_,
                   "display_mode",
                   0,
                   0,
                   100,
                   4,
                   24,
                   /** TRANSLATORS: "Display: Original/Localized map/file names" */
                   _("Display"),
                   UI::DropdownType::kTextual,
                   UI::PanelStyle::kWui,
                   UI::ButtonStyle::kWuiSecondary),

     table_footer_box_(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal, 0, 0, padding_),

     directory_info_(&main_box_, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, 0, 0, 0, 0),

     // Bottom button row
     button_box_(&main_box_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal, 0, 0, padding_),
     ok_(&button_box_, "ok", 0, 0, 0, 0, UI::ButtonStyle::kWuiPrimary, _("OK")),
     cancel_(&button_box_, "cancel", 0, 0, 0, 0, UI::ButtonStyle::kWuiSecondary, _("Cancel")),

     // Options
     basedir_(basedir),
     include_addon_maps_(addons) {

	g_fs->ensure_directory_exists(basedir_);
	curdir_ = {basedir_};

	main_box_.add(&table_and_details_box_, UI::Box::Resizing::kExpandBoth);
	main_box_.add_space(padding_);
	main_box_.add(&table_footer_box_, UI::Box::Resizing::kFullSize);
	main_box_.add(&directory_info_, UI::Box::Resizing::kFullSize);
	main_box_.add_space(padding_);
	main_box_.add(&button_box_, UI::Box::Resizing::kFullSize);

	table_.set_column_compare(0, [this](uint32_t a, uint32_t b) { return compare_players(a, b); });
	table_.set_column_compare(1, [this](uint32_t a, uint32_t b) { return compare_mapnames(a, b); });
	table_.set_column_compare(2, [this](uint32_t a, uint32_t b) { return compare_size(a, b); });

	table_box_.add(&display_mode_, UI::Box::Resizing::kFullSize);
	table_box_.add(&table_, UI::Box::Resizing::kExpandBoth);
	table_and_details_box_.add(&table_box_, UI::Box::Resizing::kExpandBoth);
	table_and_details_box_.add_space(0);
	table_and_details_box_.add(&map_details_box_, UI::Box::Resizing::kFullSize);
	map_details_box_.add(&map_details_, UI::Box::Resizing::kExpandBoth);

	const bool locale_is_en = i18n::get_locale() == "en" || starts_with(i18n::get_locale(), "en_");
	display_mode_.add(_("File names"), MapData::DisplayType::kFilenames);
	display_mode_.add(locale_is_en ? _("Map names") : _("Original map names"),
	                  MapData::DisplayType::kMapnames, nullptr, locale_is_en);
	if (!locale_is_en) {
		display_mode_.add(
		   _("Translated map names"), MapData::DisplayType::kMapnamesLocalized, nullptr, true);
	}

	table_.focus();

	button_box_.add_inf_space();
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &ok_ : &cancel_, UI::Box::Resizing::kExpandBoth);
	button_box_.add_space(padding_);
	button_box_.add(UI::g_fh->fontset()->is_rtl() ? &cancel_ : &ok_, UI::Box::Resizing::kExpandBoth);
	button_box_.add_inf_space();

	display_mode_.selected.connect([this]() { fill_table(); });
	table_.cancel.connect([this]() { die(); });

	set_z(UI::Panel::ZOrder::kFullscreenWindow);
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

void MainMenuLoadOrSaveMap::layout() {
	main_box_.set_size(get_inner_w() - 2 * padding_, get_inner_h() - 2 * padding_);

	// Set the width. Height is controlled by expanding into the outer box
	map_details_box_.set_desired_size(main_box_.get_w() / 3, 100);

	center_to_parent();
}

/**
 * fill the file list
 */
// TODO(Nordfriese): Code duplication with FsMenu::MapSelect::fill_table
void MainMenuLoadOrSaveMap::fill_table() {
	table_.clear();
	maps_data_.clear();

	//  Fill it with all files we find.
	assert(!curdir_.empty());
	FilenameSet files;
	for (const std::string& dir : curdir_) {
		FilenameSet f = g_fs->list_directory(dir);
		files.insert(f.begin(), f.end());
	}

	// If we are not at the top of the map directory hierarchy (we're not talking
	// about the absolute filesystem top!) we manually add ".."
	if (curdir_.at(0) != basedir_) {
		maps_data_.push_back(MapData::create_parent_dir(curdir_.at(0)));
	} else {
		if (files.empty()) {
			maps_data_.push_back(MapData::create_empty_dir(curdir_.at(0)));
		}
		// In the toplevel directory we also need to include add-on maps –
		// but only in the load screen, not in the save screen!
		if (include_addon_maps_) {
			for (auto& addon : AddOns::g_addons) {
				if (addon.first->category == AddOns::AddOnCategory::kMaps && addon.second) {
					for (const std::string& mapname : g_fs->list_directory(
					        kAddOnDir + FileSystem::file_separator() + addon.first->internal_name)) {
						files.insert(mapname);
					}
				}
			}
		}
	}

	const MapData::DisplayType display_type = display_mode_.get_selected();

	Widelands::Map map;

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

				if (((map.scenario_types() & Widelands::Map::MP_SCENARIO) != 0u) ||
				    ((map.scenario_types() & Widelands::Map::SP_SCENARIO) != 0u)) {
					maptype = MapData::MapType::kScenario;
				} else if (dynamic_cast<Widelands::WidelandsMapLoader*>(ml.get()) != nullptr) {
					maptype = MapData::MapType::kNormal;
				} else {
					maptype = MapData::MapType::kSettlers2;
				}

				maps_data_.emplace_back(map, mapfilename, maptype, display_type);
			} catch (const WException&) {
			}  //  we simply skip illegal entries
		} else if (g_fs->is_directory(mapfilename) &&
		           (show_empty_dirs_ || !g_fs->list_directory(mapfilename).empty())) {
			// Add subdirectory to the list
			const char* fs_filename = FileSystem::fs_filename(mapfilename.c_str());
			if ((strcmp(fs_filename, ".") == 0) || (strcmp(fs_filename, "..") == 0)) {
				continue;
			}

			MapData new_md = MapData::create_directory(mapfilename);
			bool found = false;
			for (MapData& md : maps_data_) {
				if (md.maptype == MapData::MapType::kDirectory &&
				    md.localized_name == new_md.localized_name) {
					found = true;
					md.add(new_md);
					break;
				}
			}
			if (!found) {
				maps_data_.push_back(new_md);
			}
		}
	}

	table_.fill(maps_data_, display_type);
	if (!table_.empty()) {
		table_.select(0);
	} else {
		ok_.set_enabled(false);
	}
}
