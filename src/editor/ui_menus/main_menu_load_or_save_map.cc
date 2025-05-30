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

#include "editor/ui_menus/main_menu_load_or_save_map.h"

#include <memory>

#include "base/i18n.h"
#include "base/wexception.h"
#include "editor/editorinteractive.h"
#include "graphic/font_handler.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"

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

     main_box_(this,
               UI::PanelStyle::kWui,
               "main_box",
               padding_,
               padding_,
               UI::Box::Vertical,
               0,
               0,
               padding_),

     table_and_details_box_(&main_box_,
                            UI::PanelStyle::kWui,
                            "table_details_box",
                            0,
                            0,
                            UI::Box::Horizontal,
                            0,
                            0,
                            padding_),
     table_box_(&table_and_details_box_,
                UI::PanelStyle::kWui,
                "table_box",
                0,
                0,
                UI::Box::Vertical,
                0,
                0,
                padding_),

     table_(&table_box_, 0, 0, 200, 200, UI::PanelStyle::kWui),
     egbase_(nullptr),
     map_details_box_(&table_and_details_box_,
                      UI::PanelStyle::kWui,
                      "map_details_box",
                      0,
                      0,
                      UI::Box::Vertical,
                      0,
                      0,
                      padding_),
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

     table_footer_box_(
        &main_box_, UI::PanelStyle::kWui, "footer_box", 0, 0, UI::Box::Horizontal, 0, 0, padding_),

     directory_info_(&main_box_,
                     UI::PanelStyle::kWui,
                     "label_directory_info",
                     UI::FontStyle::kWuiLabel,
                     0,
                     0,
                     0,
                     0),

     // Bottom button row
     button_box_(
        &main_box_, UI::PanelStyle::kWui, "buttons_box", 0, 0, UI::Box::Horizontal, 0, 0, padding_),
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

void MainMenuLoadOrSaveMap::layout() {
	main_box_.set_size(get_inner_w() - 2 * padding_, get_inner_h() - 2 * padding_);

	// Set the width. Height is controlled by expanding into the outer box
	map_details_box_.set_desired_size(main_box_.get_w() / 3, 100);

	center_to_parent();
}

bool MainMenuLoadOrSaveMap::set_has_selection() {
	bool has_selection = table_.has_selection();

	if (!has_selection) {
		ok_.set_enabled(false);
		map_details_.clear();
	}
	return has_selection;
}

void MainMenuLoadOrSaveMap::navigate_directory(const std::vector<std::string>& filenames,
                                               const std::string& localized_name) {
	set_current_directory(filenames);
	if (localized_name == MapData::parent_name()) {
		table_.update_table(display_mode_.get_selected(), true);
	} else {
		fill_table();
	}
	set_has_selection();
}

/**
 * fill the file list
 */
void MainMenuLoadOrSaveMap::fill_table() {
	Widelands::Map::ScenarioTypes scenario_types =
	   Widelands::Map::MP_SCENARIO | Widelands::Map::SP_SCENARIO;
	/* No filtering */
	MapTable::FilterFn filter = [](MapData&) { return true; };

	table_.fill(curdir_, basedir_, display_mode_.get_selected(), scenario_types, filter,
	            include_addon_maps_, show_empty_dirs_);
}
