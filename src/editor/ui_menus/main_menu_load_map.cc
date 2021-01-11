/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "editor/ui_menus/main_menu_load_map.h"

#include "base/i18n.h"
#include "editor/editorinteractive.h"
#include "io/filesystem/layered_filesystem.h"
#include "map_io/widelands_map_loader.h"
#include "wui/mapdetails.h"
#include "wui/maptable.h"

/**
 * Create all the buttons etc...
 */
MainMenuLoadMap::MainMenuLoadMap(EditorInteractive& parent, UI::UniqueWindow::Registry& registry)
   : MainMenuLoadOrSaveMap(parent, registry, "load_map_menu", _("Load Map")) {
	set_current_directory(curdir_);

	table_.selected.connect([this](unsigned) { entry_selected(); });
	table_.double_clicked.connect([this](unsigned) { clicked_ok(); });

	ok_.sigclicked.connect([this]() { clicked_ok(); });
	cancel_.sigclicked.connect([this]() { die(); });

	fill_table();
	layout();
}

void MainMenuLoadMap::clicked_ok() {
	assert(ok_.enabled());
	assert(table_.has_selection());
	const MapData& mapdata = maps_data_[table_.get_selected()];
	if (g_fs->is_directory(mapdata.filename) &&
	    !Widelands::WidelandsMapLoader::is_widelands_map(mapdata.filename)) {
		set_current_directory(mapdata.filename);
		fill_table();
	} else {
		EditorInteractive& eia = dynamic_cast<EditorInteractive&>(*get_parent());
		eia.egbase().create_loader_ui({"editor"}, true, "", kEditorSplashImage);
		eia.load(mapdata.filename);
		// load() will delete us.
		eia.egbase().remove_loader_ui();
	}
}

void MainMenuLoadMap::set_current_directory(const std::string& filename) {
	curdir_ = filename;

	std::string display_dir = curdir_.substr(basedir_.size());
	if (boost::starts_with(display_dir, "/")) {
		display_dir = display_dir.substr(1);
	}
	if (boost::starts_with(display_dir, "My_Maps")) {
		boost::replace_first(display_dir, "My_Maps", _("My Maps"));
	} else if (boost::starts_with(display_dir, "MP_Scenarios")) {
		boost::replace_first(display_dir, "MP_Scenarios", _("Multiplayer Scenarios"));
	} else if (boost::starts_with(display_dir, "Downloaded")) {
		boost::replace_first(display_dir, "Downloaded", _("Downloaded Maps"));
	}
	/** TRANSLATORS: The folder that a file will be saved to. */
	directory_info_.set_text((boost::format(_("Current directory: %s")) % display_dir).str());
}

/**
 * Called when a entry is selected
 */
void MainMenuLoadMap::entry_selected() {
	bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);
	if (!has_selection) {
		map_details_.clear();
	} else {
		map_details_.update(maps_data_[table_.get_selected()],
		                    display_mode_.get_selected() == MapData::DisplayType::kMapnamesLocalized,
		                    true);
	}
}
