/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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

#include "ui/editor/main_menu_load_map.h"

#include <memory>

#include "base/i18n.h"
#include "base/string.h"
#include "ui/editor/editorinteractive.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "map_io/widelands_map_loader.h"
#include "ui/shared/mapdetails.h"
#include "ui/shared/maptable.h"

/**
 * Create all the buttons etc...
 */
MainMenuLoadMap::MainMenuLoadMap(EditorInteractive& parent, UI::UniqueWindow::Registry& registry)
   : MainMenuLoadOrSaveMap(parent, registry, "load_map_menu", _("Load Map"), true) {

	table_.selected.connect([this](unsigned /* value */) { entry_selected(); });
	table_.double_clicked.connect([this](unsigned /* value */) { clicked_ok(); });

	ok_.sigclicked.connect([this]() { clicked_ok(); });
	cancel_.sigclicked.connect([this]() { die(); });

	navigate_directory(curdir_, kMapsDir);
	layout();

	initialization_complete();
}

void MainMenuLoadMap::clicked_ok() {
	if (!ok_.enabled() || !table_.has_selection()) {
		return;
	}
	const MapData& mapdata = table_.get_selected_data();
	assert(!mapdata.filenames.empty());
	if (g_fs->is_directory(mapdata.filenames.at(0)) &&
	    !Widelands::WidelandsMapLoader::is_widelands_map(mapdata.filenames.at(0))) {
		navigate_directory(mapdata.filenames, mapdata.localized_name);
	} else {
		assert(mapdata.filenames.size() == 1);
		// Prevent description notes from reaching a subscriber
		// other than the one they're meant for
		egbase_.delete_world_and_tribes();

		EditorInteractive& eia = dynamic_cast<EditorInteractive&>(*get_parent());
		eia.egbase().create_loader_ui({"editor"}, true, "", kEditorSplashImage, false);
		eia.load(mapdata.filenames.at(0));
		// load() will delete us.
		eia.egbase().remove_loader_ui();
	}
}

void MainMenuLoadMap::set_current_directory(const std::vector<std::string>& filenames) {
	assert(!filenames.empty());
	curdir_ = filenames;

	std::string display_dir = curdir_.at(0);
	if (starts_with(display_dir, basedir_)) {
		replace_first(display_dir, basedir_, "");
	} else if (starts_with(display_dir, kAddOnDir)) {
		std::vector<std::string> result;
		split(result, display_dir, {'/'});
		assert(result.size() > 2);

		/* translate directory names */
		std::string& addon = result.at(1);
		std::unique_ptr<i18n::GenericTextdomain> td(AddOns::create_textdomain_for_addon(addon));
		std::string profilepath = kAddOnDir;
		profilepath += FileSystem::file_separator();
		profilepath += addon;
		profilepath += FileSystem::file_separator();
		profilepath += "dirnames";
		Profile p(profilepath.c_str());

		/* strip away addons/<addon-name>/ */
		result.erase(result.begin(), result.begin() + 2);

		if (Section* s = p.get_section("global")) {
			for (auto& fname : result) {
				if (s->has_val(fname.c_str())) {
					fname = s->get_safe_string(fname);
				}
			}
		}
		display_dir = join(result, "/");
	}

	if (starts_with(display_dir, "/")) {
		display_dir = display_dir.substr(1);
	}

	if (starts_with(display_dir, kMyMapsDir)) {
		replace_first(display_dir, kMyMapsDir, _("My Maps"));
	} else if (starts_with(display_dir, kMultiPlayerScenarioDir)) {
		replace_first(display_dir, kMultiPlayerScenarioDir, _("Multiplayer Scenarios"));
	} else if (starts_with(display_dir, kSinglePlayerScenarioDir)) {
		replace_first(display_dir, kSinglePlayerScenarioDir, _("Singleplayer Scenarios"));
	} else if (starts_with(display_dir, kDownloadedMapsDir)) {
		replace_first(display_dir, kDownloadedMapsDir, _("Downloaded Maps"));
	}
	/** TRANSLATORS: The folder that a file will be saved to. */
	directory_info_.set_text(format(_("Current directory: %s"), display_dir));
}

/**
 * Called when a entry is selected
 */
void MainMenuLoadMap::entry_selected() {
	if (set_has_selection()) {
		ok_.set_enabled(map_details_.update(
		   table_.get_selected_data(),
		   display_mode_.get_selected() == MapData::DisplayType::kMapnamesLocalized, true));
	}
}
