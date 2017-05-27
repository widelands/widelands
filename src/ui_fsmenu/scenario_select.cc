/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "ui_fsmenu/scenario_select.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "logic/campaign_visibility.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"
#include "ui_basic/scrollbar.h"

/**
 * FullscreenMenuScenarioSelect UI.
 *
 * Loads a list of all visible maps of selected campaign or all tutorials and
 * lets the user choose one.
 */
FullscreenMenuScenarioSelect::FullscreenMenuScenarioSelect(bool is_tutorial)
   : FullscreenMenuLoadMapOrGame(),
     table_(this, tablex_, tabley_, tablew_, tableh_),
     header_box_(this, 0, 0, UI::Box::Vertical),

     // Main title
     title_(&header_box_,
            0,
            0,
            is_tutorial ? _("Choose a tutorial") : _("Choose a scenario"),
            UI::Align::kCenter),
     subtitle_(&header_box_,
               0,
               0,
               UI::Scrollbar::kSize,
               0,
               "",
               UI::Align::kCenter,
               g_gr->images().get("images/ui_basic/but3.png"),
               UI::MultilineTextarea::ScrollMode::kNoScrolling),
     scenario_details_(this),
     is_tutorial_(is_tutorial) {
	title_.set_fontsize(UI_FONT_SIZE_BIG);

	header_box_.add_inf_space();
	header_box_.add_inf_space();
	header_box_.add_inf_space();
	header_box_.add(&title_, UI::Box::Resizing::kFullSize);
	header_box_.add_inf_space();
	header_box_.add(&subtitle_, UI::Box::Resizing::kFullSize);
	header_box_.add_inf_space();
	header_box_.add_inf_space();
	header_box_.add_inf_space();

	back_.set_tooltip(is_tutorial_ ? _("Return to the main menu") :
	                                 _("Return to campaign selection"));
	ok_.set_tooltip(is_tutorial_ ? _("Play this tutorial") : _("Play this scenario"));

	ok_.sigclicked.connect(
	   boost::bind(&FullscreenMenuScenarioSelect::clicked_ok, boost::ref(*this)));
	back_.sigclicked.connect(
	   boost::bind(&FullscreenMenuScenarioSelect::clicked_back, boost::ref(*this)));
	table_.selected.connect(boost::bind(&FullscreenMenuScenarioSelect::entry_selected, this));
	table_.double_clicked.connect(
	   boost::bind(&FullscreenMenuScenarioSelect::clicked_ok, boost::ref(*this)));

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
	layout();
}

void FullscreenMenuScenarioSelect::layout() {
	FullscreenMenuLoadMapOrGame::layout();
	header_box_.set_size(get_w(), tabley_);
	table_.set_size(tablew_, tableh_);
	table_.set_pos(Vector2i(tablex_, tabley_));
	scenario_details_.set_size(get_right_column_w(right_column_x_), tableh_ - buth_ - 4 * padding_);
	scenario_details_.set_pos(Vector2i(right_column_x_, tabley_));
}

std::string FullscreenMenuScenarioSelect::get_map() {
	return campmapfile;
}

// Telling this class what campaign we have and since we know what campaign we have, fill it.
void FullscreenMenuScenarioSelect::set_campaign(const std::string& campaign_name) {
	campaign_name_ = campaign_name;
	fill_table();
}

bool FullscreenMenuScenarioSelect::set_has_selection() {
	bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);
	return has_selection;
}

void FullscreenMenuScenarioSelect::clicked_ok() {
	if (!table_.has_selection()) {
		return;
	}
	const ScenarioTableData& scenario_data = scenarios_data_[table_.get_selected()];
	if (!scenario_data.visible) {
		return;
	}
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
}

void FullscreenMenuScenarioSelect::entry_selected() {
	if (set_has_selection()) {
		const ScenarioTableData& scenario_data = scenarios_data_[table_.get_selected()];
		campmapfile = scenario_data.path;
		Widelands::Map map;

		std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(campmapfile));
		if (!ml) {
			throw wexception(_("Invalid path to file in campaigns.lua: %s"), campmapfile.c_str());
		}

		map.set_filename(campmapfile);
		ml->preload_map(true);

		// Localizing this, because some author fields now have "edited by" text.
		ScenarioMapData scenario_map_data(_(map.get_author()));
		{
			i18n::Textdomain td("maps");
			scenario_map_data.name = _(map.get_name());
			scenario_map_data.description = _(map.get_description());
		}
		scenario_map_data.is_tutorial = is_tutorial_;
		scenario_map_data.visible = scenario_data.visible;
		scenario_details_.update(scenario_map_data);

		// The dummy scenario can't be played, so we disable the OK button.
		ok_.set_enabled(scenario_map_data.visible);
	}
}

/**
 * fill the campaign-map list
 */
void FullscreenMenuScenarioSelect::fill_table() {
	// Read in the campaign configuration for the currently selected campaign
	LuaInterface lua;
	std::unique_ptr<LuaTable> table(is_tutorial_ ? lua.run_script("campaigns/tutorials.lua") :
	                                               lua.run_script("campaigns/campaigns.lua"));
	table->do_not_warn_about_unaccessed_keys();
	std::unique_ptr<LuaTable> all_campaigns(table->get_table("campaigns"));
	all_campaigns->do_not_warn_about_unaccessed_keys();

	bool found = false;
	for (const auto& campaign_table : all_campaigns->array_entries<std::unique_ptr<LuaTable>>()) {
		campaign_table->do_not_warn_about_unaccessed_keys();
		if (found) {
			// We need to shut up the table warnings, so we can't break from the loop
			continue;
		}
		// Find the campaign by name
		if (campaign_name_ == campaign_table->get_string("name")) {
			found = true;
			// Set subtitle of the page
			if (is_tutorial_) {
				const std::string subtitle1 = _("Pick a tutorial from the list, then hit “OK”.");
				const std::string subtitle2 =
				   _("You can see a description of the currently selected tutorial on the right.");
				subtitle_.set_text((boost::format("%s\n%s") % subtitle1 % subtitle2).str());
			} else {
				const std::string campaign_tribe = _(campaign_table->get_string("tribe"));
				{
					i18n::Textdomain td("maps");
					const std::string campaign_descname = _(campaign_table->get_string("descname"));
					subtitle_.set_text(
					   (boost::format("%s — %s") % campaign_tribe % campaign_descname).str());
				}
			}

			// Create the entry we use to load the section of the map
			uint32_t counter = 0;
			// Read in campvis-file
			CampaignVisibilitySave cvs;
			Profile campvis(cvs.get_path().c_str());
			Section& scenario_visibility = campvis.get_safe_section("scenarios");

			// Add all visible entries to the list.
			std::unique_ptr<LuaTable> scenarios_table(campaign_table->get_table("scenarios"));
			scenarios_table->do_not_warn_about_unaccessed_keys();
			for (const auto& scenario : scenarios_table->array_entries<std::unique_ptr<LuaTable>>()) {
				scenario->do_not_warn_about_unaccessed_keys();
				const std::string name = scenario->get_string("name");

				if (is_tutorial_ || scenario_visibility.get_bool(name.c_str())) {
					const std::string descname = scenario->get_string("descname");
					const std::string path = scenario->get_string("path");

					ScenarioTableData scenario_data;
					scenario_data.index = counter + 1;
					scenario_data.name = descname;
					scenario_data.path = "campaigns/" + path;
					scenario_data.visible = path != "dummy.wmf";
					scenarios_data_.push_back(scenario_data);

					UI::Table<uintptr_t>::EntryRecord& te = table_.add(counter);
					te.set_string(0, (boost::format("%u") % scenario_data.index).str());
					te.set_picture(
						1, g_gr->images().get("images/ui_basic/ls_wlmap.png"), scenario_data.name);
					if (!scenario_data.visible) {
						te.set_color(UI_FONT_CLR_DISABLED);
					}
					++counter;
				}
			} // scenario
		} // campaign
	}

	table_.sort();
	if (table_.size()) {
		table_.select(0);
	}
	set_has_selection();
}
