/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 */

#include "ui_fsmenu/mapselect.h"

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/wexception.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "map_io/widelands_map_loader.h"
#include "ui_fsmenu/launch_mpg.h"
#include "ui_fsmenu/launch_spg.h"
#include "wui/map_tags.h"

namespace FsMenu {

// TODO(GunChleoc): Arabic: line height broken for descriptions for Arabic.
// Fix align for table headings & entries and for wordwrap.

constexpr int checkbox_space_ = 20;

using Widelands::WidelandsMapLoader;

MapSelect::MapSelect(MenuCapsule& m,
                     LaunchMPG* mpg,
                     GameSettingsProvider* const settings,
                     GameController* const ctrl,
                     Widelands::Game& g)
   : TwoColumnsFullNavigationMenu(m, _("Choose Map")),
     parent_screen_(mpg),
     checkboxes_(
        &header_box_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical, 0, 0, 2 * kPadding),
     table_(&left_column_box_, 0, 0, 0, 0, UI::PanelStyle::kFsMenu),
     map_details_(&right_column_content_box_, 0, 0, 0, 0, UI::PanelStyle::kFsMenu, g),

     scenario_types_(settings->settings().multiplayer ? Map::MP_SCENARIO : Map::SP_SCENARIO),
     basedir_(kMapsDir),
     game_(g),
     settings_(settings),
     ctrl_(ctrl),
     has_translated_mapname_(false),
     unspecified_balancing_found_(false),
     update_map_details_(false) {
	curdir_ = basedir_;
	if (settings_->settings().multiplayer) {
		back_.set_tooltip(_("Return to the multiplayer game setup"));
	} else {
		back_.set_tooltip(_("Return to the single player menu"));
	}

	table_.selected.connect([this](uint32_t /* value */) { entry_selected(); });
	table_.double_clicked.connect([this](uint32_t /* value */) { clicked_ok(); });
	table_.set_column_compare(0, [this](uint32_t a, uint32_t b) { return compare_players(a, b); });
	table_.set_column_compare(1, [this](uint32_t a, uint32_t b) { return compare_mapnames(a, b); });
	table_.set_column_compare(2, [this](uint32_t a, uint32_t b) { return compare_size(a, b); });

	UI::Box* hbox = new UI::Box(
	   &checkboxes_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, checkbox_space_, get_w());

	show_all_maps_ = new UI::Button(
	   hbox, "show_all_maps", 0, 0, 0, 0, UI::ButtonStyle::kFsMenuSecondary, _("Show all maps"));
	cb_dont_localize_mapnames_ = new UI::Checkbox(
	   hbox, UI::PanelStyle::kFsMenu, Vector2i::zero(), _("Show original map names"));
	cb_dont_localize_mapnames_->set_state(false);

	hbox->add(show_all_maps_, UI::Box::Resizing::kFullSize);
	hbox->add_space(checkbox_space_);
	hbox->add(cb_dont_localize_mapnames_, UI::Box::Resizing::kFullSize);
	hbox->add_inf_space();
	checkboxes_.add(hbox, UI::Box::Resizing::kFullSize);

	// Row with dropdowns

	hbox = new UI::Box(
	   &checkboxes_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, checkbox_space_, get_w());

	official_tags_dropdown_ = new UI::Dropdown<std::string>(
	   hbox, "dropdown_official_tags", 0, 0, 200, 50, 24, "", UI::DropdownType::kTextual,
	   UI::PanelStyle::kFsMenu, UI::ButtonStyle::kFsMenuMenu);
	official_tags_dropdown_->set_autoexpand_display_button();
	official_tags_dropdown_->add(_("Official & Unofficial"), "");
	official_tags_dropdown_->add(localize_tag("official"), "official");
	official_tags_dropdown_->add(localize_tag("unofficial"), "unofficial");

	hbox->add(official_tags_dropdown_, UI::Box::Resizing::kFullSize);

	hbox->add_space(checkbox_space_);

	team_tags_dropdown_ = new UI::Dropdown<std::string>(
	   hbox, "dropdown_team_tags", 0, 0, 200, 50, 24, "", UI::DropdownType::kTextual,
	   UI::PanelStyle::kFsMenu, UI::ButtonStyle::kFsMenuMenu);
	team_tags_dropdown_->set_autoexpand_display_button();
	/** TRANSLATORS: Filter entry in map selection. Other entries are "Free for all"", "Teams of 2"
	 * etc. */
	team_tags_dropdown_->add(_("Any Teams"), "");
	team_tags_dropdown_->add(localize_tag("ffa"), "ffa");
	team_tags_dropdown_->add(localize_tag("1v1"), "1v1");
	team_tags_dropdown_->add(localize_tag("2teams"), "2teams");
	team_tags_dropdown_->add(localize_tag("3teams"), "3teams");
	team_tags_dropdown_->add(localize_tag("4teams"), "4teams");

	hbox->add(team_tags_dropdown_, UI::Box::Resizing::kFullSize);

	hbox->add_space(checkbox_space_);

	balancing_tags_dropdown_ = new UI::Dropdown<std::string>(
	   hbox, "dropdown_balancing", 0, 0, 200, 50, 24, "", UI::DropdownType::kTextual,
	   UI::PanelStyle::kFsMenu, UI::ButtonStyle::kFsMenuMenu);
	balancing_tags_dropdown_->set_autoexpand_display_button();
	rebuild_balancing_dropdown();

	hbox->add(balancing_tags_dropdown_, UI::Box::Resizing::kFullSize);

	checkboxes_.add(hbox, UI::Box::Resizing::kFullSize);

	// Row with checkboxes

	hbox = new UI::Box(
	   &checkboxes_, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Horizontal, checkbox_space_, get_w());
	add_tag_checkbox(hbox, "seafaring", localize_tag("seafaring"));
	add_tag_checkbox(hbox, "ferries", localize_tag("ferries"));
	add_tag_checkbox(hbox, "artifacts", localize_tag("artifacts"));
	add_tag_checkbox(hbox, "scenario", localize_tag("scenario"));
	hbox->add_inf_space();
	checkboxes_.add(hbox, UI::Box::Resizing::kFullSize);

	table_.focus();
	clear_filter();

	// We don't need the unlocalizing option if there is nothing to unlocalize.
	// We know this after the list is filled.
	cb_dont_localize_mapnames_->set_visible(has_translated_mapname_);

	cb_dont_localize_mapnames_->changedto.connect([this](unsigned /* value */) { fill_table(); });

	for (size_t i = 0; i < tags_checkboxes_.size(); ++i) {
		tags_checkboxes_.at(i)->changedto.connect([this, i](bool b) { tagbox_changed(i, b); });
	}

	balancing_tags_dropdown_->selected.connect([this] { fill_table(); });
	official_tags_dropdown_->selected.connect([this] { fill_table(); });
	team_tags_dropdown_->selected.connect([this] { fill_table(); });
	show_all_maps_->sigclicked.connect([this] { clear_filter(); });

	header_box_.add(&checkboxes_, UI::Box::Resizing::kExpandBoth);
	header_box_.add_space(2 * kPadding);
	left_column_box_.add(&table_, UI::Box::Resizing::kExpandBoth);
	right_column_content_box_.add(&map_details_, UI::Box::Resizing::kExpandBoth);

	layout();

	initialization_complete();
}

MapSelect::~MapSelect() {
	if (!parent_screen_) {
		game_.cleanup_objects();
		delete &game_;
		delete settings_;
	}
}

void MapSelect::think() {
	TwoColumnsFullNavigationMenu::think();

	if (ctrl_) {
		ctrl_->think();
	}

	if (update_map_details_) {
		// Call performance heavy draw_minimap function only during think
		update_map_details_ = false;
		bool loadable = map_details_.update(
		   maps_data_[table_.get_selected()], !cb_dont_localize_mapnames_->get_state(), true);
		ok_.set_enabled(loadable && maps_data_.at(table_.get_selected()).nrplayers > 0);
	}
}

bool MapSelect::compare_players(uint32_t rowa, uint32_t rowb) {
	return maps_data_[table_[rowa]].compare_players(maps_data_[table_[rowb]]);
}

bool MapSelect::compare_mapnames(uint32_t rowa, uint32_t rowb) {
	return maps_data_[table_[rowa]].compare_names(maps_data_[table_[rowb]]);
}

bool MapSelect::compare_size(uint32_t rowa, uint32_t rowb) {
	return maps_data_[table_[rowa]].compare_size(maps_data_[table_[rowb]]);
}

MapData const* MapSelect::get_map() const {
	if (!table_.has_selection()) {
		return nullptr;
	}
	return &maps_data_[table_.get_selected()];
}

void MapSelect::clicked_back() {
	if (parent_screen_) {
		parent_screen_->clicked_select_map_callback(nullptr, false);
	}
	die();
}

void MapSelect::clicked_ok() {
	if (!table_.has_selection()) {
		return;
	}
	const MapData& mapdata = maps_data_[table_.get_selected()];

	if (!mapdata.width) {
		curdir_ = mapdata.filename;
		fill_table();
	} else if (!ok_.enabled()) {
		return;
	} else if (parent_screen_) {
		parent_screen_->clicked_select_map_callback(
		   get_map(), maps_data_[table_.get_selected()].maptype == MapData::MapType::kScenario);
		die();
	} else {
		new LaunchSPG(get_capsule(), *settings_, game_, get_map(),
		              maps_data_[table_.get_selected()].maptype == MapData::MapType::kScenario);
	}
}

bool MapSelect::set_has_selection() {
	bool has_selection = table_.has_selection();

	if (!has_selection) {
		ok_.set_enabled(false);
		map_details_.clear();
	}
	return has_selection;
}

void MapSelect::entry_selected() {
	if (set_has_selection()) {
		// Update during think() instead of every keypress
		update_map_details_ = true;
	}
}

/**
 * Fill the list with maps that can be opened.
 *
 *
 * At first, only the subdirectories are added to the list, then the normal
 * files follow. This is done to make navigation easier.
 *
 * To make things more difficult, we have to support compressed and uncompressed
 * map files here - the former are files, the latter are directories. Care must
 * be taken to sort uncompressed maps (which look like and really are
 * directories) with the files.
 *
 * The search starts in \ref curdir_ ("..../maps") and there is no possibility
 * to move further up. If the user moves down into subdirectories, we insert an
 * entry to move back up.
 */
void MapSelect::fill_table() {
	has_translated_mapname_ = false;
	bool unspecified_balancing_found = false;

	maps_data_.clear();

	MapData::DisplayType display_type;
	if (cb_dont_localize_mapnames_->get_state()) {
		display_type = MapData::DisplayType::kMapnames;
	} else {
		display_type = MapData::DisplayType::kMapnamesLocalized;
	}

	// This is the normal case

	//  Fill it with all files we find in all directories.
	FilenameSet files = g_fs->list_directory(curdir_);

	// If we are not at the top of the map directory hierarchy (we're not talking
	// about the absolute filesystem top!) we manually add ".."
	if (curdir_ != basedir_) {
		maps_data_.push_back(MapData::create_parent_dir(curdir_));
	} else {
		// In the toplevel directory we also need to include add-on maps
		for (auto& addon : AddOns::g_addons) {
			if (addon.first->category == AddOns::AddOnCategory::kMaps && addon.second) {
				for (const std::string& mapname : g_fs->list_directory(
				        kAddOnDir + FileSystem::file_separator() + addon.first->internal_name)) {
					files.insert(mapname);
				}
			}
		}
	}

	Widelands::Map map;  //  MapLoader needs a place to put its preload data

	for (const std::string& mapfilename : files) {
		// Add map file (compressed) or map directory (uncompressed)
		std::unique_ptr<Widelands::MapLoader> ml = map.get_correct_loader(mapfilename);
		if (ml != nullptr) {
			try {
				map.set_filename(mapfilename);
				ml->preload_map(true, nullptr);

				if (!map.get_width() || !map.get_height()) {
					continue;
				}

				MapData::MapType maptype;
				if (map.scenario_types() & scenario_types_) {
					maptype = MapData::MapType::kScenario;
				} else if (dynamic_cast<WidelandsMapLoader*>(ml.get())) {
					maptype = MapData::MapType::kNormal;
				} else {
					maptype = MapData::MapType::kSettlers2;
				}

				MapData mapdata(map, mapfilename, maptype, display_type);

				has_translated_mapname_ =
				   has_translated_mapname_ || (mapdata.name != mapdata.localized_name);

				bool has_all_tags = true;
				if (team_tags_dropdown_->has_selection()) {
					const std::string selected_tag = team_tags_dropdown_->get_selected();
					if (!selected_tag.empty()) {
						has_all_tags &= mapdata.tags.count(selected_tag) > 0;
					}
				}
				if (official_tags_dropdown_->has_selection()) {
					const std::string selected_tag = official_tags_dropdown_->get_selected();
					if (!selected_tag.empty()) {
						if (selected_tag == "official") {
							has_all_tags &= mapdata.tags.count("official") > 0;
						} else {
							has_all_tags &= !mapdata.tags.count("official");
						}
					}
				}
				if (balancing_tags_dropdown_->has_selection()) {
					const std::string selected_tag = balancing_tags_dropdown_->get_selected();
					if (!selected_tag.empty()) {
						if (selected_tag == "unspecified") {
							has_all_tags &= !mapdata.tags.count("balanced");
							has_all_tags &= !mapdata.tags.count("unbalanced");
						} else {
							has_all_tags &= mapdata.tags.count(selected_tag) > 0;
						}
					}
				}
				// Backwards compatibility
				if (!mapdata.tags.count("balanced") && !mapdata.tags.count("unbalanced")) {
					unspecified_balancing_found = true;
				} else if (mapdata.tags.count("balanced") && mapdata.tags.count("unbalanced")) {
					log_warn("Map '%s' is both balanced and unbalanced - please fix the 'elemental' "
					         "packet\n",
					         mapfilename.c_str());
				}

				for (uint32_t tag : req_tags_) {
					has_all_tags &= mapdata.tags.count(tags_ordered_[tag]) > 0;
				}

				if (!has_all_tags) {
					continue;
				}
				maps_data_.push_back(mapdata);
			} catch (const std::exception& e) {
				log_warn(
				   "Mapselect: Skip %s due to preload error: %s\n", mapfilename.c_str(), e.what());
			} catch (...) {
				log_warn("Mapselect: Skip %s due to unknown exception\n", mapfilename.c_str());
			}
		} else if (g_fs->is_directory(mapfilename) && !g_fs->list_directory(mapfilename).empty()) {
			// Add subdirectory to the list
			const char* fs_filename = FileSystem::fs_filename(mapfilename.c_str());
			if (!strcmp(fs_filename, ".") || !strcmp(fs_filename, "..")) {
				continue;
			}
			maps_data_.push_back(MapData::create_directory(mapfilename));
		}
	}

	table_.fill(maps_data_, display_type);
	if (!table_.empty()) {
		table_.select(0);
	}
	set_has_selection();
	table_.cancel.connect([this]() { clicked_back(); });

	if (unspecified_balancing_found != unspecified_balancing_found_) {
		unspecified_balancing_found_ = unspecified_balancing_found;
		rebuild_balancing_dropdown();
	}
}

/*
 * Add a tag to the checkboxes
 */
UI::Checkbox*
MapSelect::add_tag_checkbox(UI::Box* box, const std::string& tag, const std::string& displ_name) {
	tags_ordered_.push_back(tag);

	UI::Checkbox* cb = new UI::Checkbox(box, UI::PanelStyle::kFsMenu, Vector2i::zero(), displ_name);

	box->add(cb, UI::Box::Resizing::kFullSize);
	box->add_space(checkbox_space_);
	tags_checkboxes_.push_back(cb);

	return cb;
}

/*
 * One of the tagboxes has changed
 */
void MapSelect::tagbox_changed(int32_t id, bool to) {
	if (to) {
		req_tags_.insert(id);
	} else {
		req_tags_.erase(id);
	}

	fill_table();
}

void MapSelect::clear_filter() {
	req_tags_.clear();
	for (UI::Checkbox* checkbox : tags_checkboxes_) {
		checkbox->set_state(false);
	}

	balancing_tags_dropdown_->select("");
	official_tags_dropdown_->select("");
	team_tags_dropdown_->select("");
	fill_table();
}

void MapSelect::rebuild_balancing_dropdown() {
	const std::string selected =
	   balancing_tags_dropdown_->has_selection() ? balancing_tags_dropdown_->get_selected() : "";
	balancing_tags_dropdown_->clear();
	balancing_tags_dropdown_->add(_("Balanced & Unbalanced"), "");
	balancing_tags_dropdown_->add(localize_tag("balanced"), "balanced");
	balancing_tags_dropdown_->add(localize_tag("unbalanced"), "unbalanced");
	if (unspecified_balancing_found_) {
		// Backwards compatibility with old maps
		balancing_tags_dropdown_->add(pgettext("balancing", "Unspecified"), "unspecified");
		balancing_tags_dropdown_->select(selected);
	} else {
		balancing_tags_dropdown_->select(selected == "unspecified" ? "" : selected);
		fill_table();
	}
}
}  // namespace FsMenu
