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
 *
 */

#include "ui_fsmenu/scenario_select.h"

#include <memory>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/image_cache.h"
#include "io/profile.h"
#include "logic/filesystem_constants.h"
#include "logic/game.h"
#include "map_io/widelands_map_loader.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/scrollbar.h"
#include "ui_fsmenu/campaigns.h"
#include "ui_fsmenu/main.h"
#include "wlapplication.h"

namespace FsMenu {

/**
 * ScenarioSelect UI.
 *
 * Loads a list of all visible maps of selected campaign or all tutorials and
 * lets the user choose one.
 */
ScenarioSelect::ScenarioSelect(MenuCapsule& fsmm, CampaignData* camp)
   : TwoColumnsFullNavigationMenu(fsmm, camp ? _("Choose Scenario") : _("Choose Tutorial")),
     is_tutorial_(camp == nullptr),
     table_(&left_column_box_, 0, 0, 0, 0, UI::PanelStyle::kFsMenu),

     subtitle_(&header_box_,
               0,
               0,
               UI::Scrollbar::kSize,
               0,
               UI::PanelStyle::kFsMenu,
               "",
               UI::Align::kCenter,
               UI::MultilineTextarea::ScrollMode::kNoScrolling),
     scenario_details_(&right_column_content_box_),
     scenario_difficulty_header_(&right_column_content_box_,
                                 UI::PanelStyle::kFsMenu,
                                 UI::FontStyle::kFsMenuInfoPanelHeading,
                                 0,
                                 0,
                                 0,
                                 0,
                                 is_tutorial_ ? "" : _("Difficulty"),
                                 UI::Align::kLeft),
     scenario_difficulty_(&right_column_content_box_,
                          "scenario_difficulty",
                          0,
                          0,
                          0,
                          8,
                          standard_height_,
                          "",
                          UI::DropdownType::kTextual,
                          UI::PanelStyle::kFsMenu,
                          UI::ButtonStyle::kFsMenuSecondary),
     campaign_(camp) {

	// Set subtitle of the page
	if (campaign_ == nullptr) {
		const std::string subtitle1 = _("Pick a tutorial from the list, then hit “OK”.");
		const std::string subtitle2 =
		   _("You can see a description of the currently selected tutorial on the right.");
		subtitle_.set_text(format("%s\n%s", subtitle1, subtitle2));
	} else {
		subtitle_.set_text(format("%s — %s", campaign_->tribename, campaign_->descname));
	}

	header_box_.add(&subtitle_, UI::Box::Resizing::kExpandBoth);
	header_box_.add_space(10 * kPadding);

	left_column_box_.add(&table_, UI::Box::Resizing::kExpandBoth);

	right_column_content_box_.add(&scenario_details_, UI::Box::Resizing::kExpandBoth);
	right_column_content_box_.add(
	   &scenario_difficulty_header_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	right_column_content_box_.add(&scenario_difficulty_, UI::Box::Resizing::kFullSize);

	back_.set_tooltip(is_tutorial_ ? _("Return to the main menu") :
                                    _("Return to campaign selection"));
	ok_.set_tooltip(is_tutorial_ ? _("Play this tutorial") : _("Play this scenario"));

	table_.selected.connect([this](unsigned) { entry_selected(); });
	table_.double_clicked.connect([this](unsigned) { clicked_ok(); });

	if (is_tutorial_) {
		scenario_difficulty_.set_visible(false);
	} else {
		uint32_t val = 0;
		assert(campaign_);
		assert(!campaign_->difficulties.empty());
		for (const std::string& d : campaign_->difficulties) {
			++val;  // We use values from 1 up because that's how Lua indexes arrays
			scenario_difficulty_.add(d, val, nullptr, val == campaign_->default_difficulty);
		}
		scenario_difficulty_.set_enabled(val > 1);
	}

	std::string number_tooltip;
	std::string name_tooltip;
	if (is_tutorial_) {
		number_tooltip = _("The order in which the tutorials should be played");
		name_tooltip = _("Tutorial Name");
	} else {
		number_tooltip = _("The number of this scenario in the campaign");
		name_tooltip = _("Scenario Name");
	}

	/** TRANSLATORS: Campaign scenario number table header */
	table_.add_column(35, _("#"), number_tooltip);
	table_.add_column(
	   0, name_tooltip, name_tooltip, UI::Align::kLeft, UI::TableColumnType::kFlexible);
	table_.set_sort_column(0);
	table_.focus();
	fill_table();
	layout();

	table_.cancel.connect([this]() { clicked_back(); });

	initialization_complete();
}

void ScenarioSelect::layout() {
	TwoColumnsFullNavigationMenu::layout();
	scenario_difficulty_.set_desired_size(0, standard_height_);
}

// Resolves the add-on prefix into a usable path or prefixes the path with the campaign directory
static std::string resolve_and_fix_cross_file(const std::string& path) {
	const size_t colonpos = path.find(':');
	if (colonpos == std::string::npos) {
		// normal case
		return g_fs->FileSystem::fix_cross_file(kCampaignsDir + "/" + path);
	} else {
		// add-on
		return g_fs->FileSystem::fix_cross_file(kAddOnDir + "/" + path.substr(0, colonpos) + "/" +
		                                        path.substr(colonpos + 1));
	}
}

bool ScenarioSelect::set_has_selection() {
	const bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);
	return has_selection;
}

void ScenarioSelect::clicked_ok() {
	if (!table_.has_selection()) {
		return;
	}
	const ScenarioData& selected = scenarios_data_[table_.get_selected()];
	if (!selected.playable) {
		return;
	}

	std::unique_ptr<Widelands::Game> game(capsule_.menu().create_safe_game());
	if (game == nullptr) {
		return;
	}
	capsule_.set_visible(false);
	try {
		if (scenario_difficulty_.has_selection()) {
			game->set_scenario_difficulty(scenario_difficulty_.get_selected());
		}

		std::list<std::string> next;
		bool found = false;
		auto resolve = [&next, &selected, &found](const ScenarioData& s) {
			if (s.path == selected.path) {
				assert(!found);
				found = true;
			}
			if (found && s.playable) {
				next.push_back(resolve_and_fix_cross_file(s.path));
			}
		};
		if (campaign_) {
			for (const auto& s : campaign_->scenarios) {
				resolve(*s);
			}
		} else {
			for (const ScenarioData& s : scenarios_data_) {
				resolve(s);
			}
		}

		game->run_splayer_scenario_direct(next, "");
	} catch (const std::exception& e) {
		WLApplication::emergency_save(&capsule_.menu(), *game, e.what());
	}

	return_to_main_menu();
}

void ScenarioSelect::entry_selected() {
	if (set_has_selection()) {
		const ScenarioData& scenario_data = scenarios_data_[table_.get_selected()];
		scenario_details_.update(scenario_data);

		// The dummy scenario can't be played, so we disable the OK button.
		ok_.set_enabled(scenario_data.playable);
	}
}

/**
 * fill the campaign-map list
 */
void ScenarioSelect::fill_table() {
	if (is_tutorial_) {
		// Load the tutorials
		LuaInterface lua;
		std::unique_ptr<LuaTable> table(lua.run_script("campaigns/tutorials.lua"));
		for (const std::string& path : table->array_entries<std::string>()) {
			ScenarioData scenario_data;
			scenario_data.path = path;
			scenario_data.playable = true;
			scenario_data.is_tutorial = true;
			scenarios_data_.push_back(scenario_data);
		}
	} else {
		// Load the current campaign
		for (auto& scenario_data : campaign_->scenarios) {
			if (scenario_data->visible) {
				scenario_data->is_tutorial = false;
				scenario_data->playable = scenario_data->path != "dummy.wmf";
				scenarios_data_.push_back(*scenario_data);
			} else {
				break;
			}
		}
	}

	for (size_t i = 0; i < scenarios_data_.size(); ++i) {
		// Get details info from maps
		ScenarioData* scenario_data = &scenarios_data_.at(i);
		const std::string full_path = resolve_and_fix_cross_file(scenario_data->path);
		Widelands::Map map;
		std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(full_path));
		if (!ml) {
			throw wexception(
			   _("Invalid path to file in campaigns.lua of tutorials.lua: %s"), full_path.c_str());
		}

		map.set_filename(full_path);
		ml->preload_map(true, nullptr);

		{
			std::unique_ptr<i18n::GenericTextdomain> td(AddOns::create_textdomain_for_map(full_path));
			scenario_data->authors.set_authors(map.get_author());
			scenario_data->descname = _(map.get_name());
			scenario_data->description = _(map.get_description());
		}

		// Now add to table
		UI::Table<uintptr_t>::EntryRecord& te = table_.add(i);
		te.set_string(0, format("%d", (i + 1)));
		te.set_picture(
		   1, g_image_cache->get("images/ui_basic/ls_wlmap.png"), scenario_data->descname);
		te.set_disabled(!scenario_data->playable);
	}

	if (!table_.empty()) {
		table_.select(0);
	}
	entry_selected();
}
}  // namespace FsMenu
