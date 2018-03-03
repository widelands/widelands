/*
 * Copyright (C) 2007-2017 by the Widelands Development Team
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

#include "logic/campaign_visibility.h"

#include <map>
#include <memory>

#include "base/log.h"
#include "io/filesystem/filesystem.h"
#include "logic/filesystem_constants.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"

CampaignVisibility::CampaignVisibility() {
	g_fs->ensure_directory_exists(kSaveDir);

	// Get the current version of the campaign config
	LuaInterface lua;
	std::unique_ptr<LuaTable> table(lua.run_script("campaigns/campaigns.lua"));
	const int current_version = table->get_int("version");
	std::unique_ptr<Profile> campvis;

	// Create a new campvis file if there wasn't one
	if (!(g_fs->file_exists(kCampVisFile))) {
		// There is no campvis file - create one.
		campvis.reset(new Profile(kCampVisFile.c_str()));
		campvis->pull_section("global");
		campvis->get_safe_section("global").set_int("version", current_version - 1);
		campvis->pull_section("campaigns");
		log("NOCOM ensure_campvis_file_exists\n");
		campvis->pull_section("scenarios");
		campvis->write(kCampVisFile.c_str(), true);
	}

	// Handle legacy campvis file
	// TODO(GunChleoc): Remove after Build 21
	campvis.reset(new Profile(kCampVisFile.c_str()));
	if (campvis->get_section("campmaps") != nullptr) {
		update_legacy_campvis(current_version);
	}

	// Read solved scenarios
	campvis.reset(new Profile(kCampVisFile.c_str()));
	Section& campvis_scenarios = campvis->get_safe_section("scenarios");

	// Now read all the data from campaign config
	std::unique_ptr<LuaTable> campaigns_table(table->get_table("campaigns"));
	i18n::Textdomain td("maps");

	for (const auto& campaign : campaigns_table->array_entries<std::unique_ptr<LuaTable>>()) {
		CampaignData* campaign_data = new CampaignData();
		// NOCOM campaign_data.index = counter;
		campaign_data->name = campaign->get_string("name");
		campaign_data->descname = _(campaign->get_string("descname"));
		campaign_data->tribename = _(campaign->get_string("tribe"));
		campaign_data->description = _(campaign->get_string("description"));
		campaign_data->prerequisite = campaign->get_string("prerequisite");

		std::unique_ptr<LuaTable> difficulty(campaign->get_table("difficulty"));
		campaign_data->difficulty = difficulty->get_int("value");
		campaign_data->difficulty_description = _(difficulty->get_string("description"));

		// Scenarios
		std::unique_ptr<LuaTable> scenarios_table(campaigns_table->get_table("scenarios"));
		for (const auto& scenario : scenarios_table->array_entries<std::unique_ptr<LuaTable>>()) {
			ScenarioData* scenario_data = new ScenarioData();
			// NOCOM scenario_data.index = counter + 1;
			scenario_data->path = scenario->get_string("path");
			if (campvis_scenarios.get_bool(scenario_data->path.c_str(), false)) {
				solved_scenarios.insert(scenario_data->path);
			}

			scenario_data->descname = _(scenario->get_string("descname")); // NOCOM get this from the map later
			scenario_data->is_tutorial = false;
			scenario_data->playable = scenario_data->path != "dummy.wmf";
			scenario_data->visible = false; // NOCOM
			campaign_data->scenarios.push_back(std::unique_ptr<ScenarioData>(std::move(scenario_data)));
		}

		campaigns_.push_back(std::unique_ptr<CampaignData>(std::move(campaign_data)));
	}

	// Now calculate the visibility
	update_visibility_info();
}

void CampaignVisibility::update_visibility_info() {
	for (auto& campaign : campaigns_) {
		if (campaign->prerequisite.empty() || solved_scenarios.count(campaign->prerequisite) == 1) {
			campaign->visible = true;
			// Only a visible campaign can contain visible scenarios
			for (size_t i = 0; i < campaign->scenarios.size(); ++i) {
				auto& scenario = campaign->scenarios.at(i);
				if (i == 0) {
					// The first scenario in a visible campaign is always visible
					scenario->visible = true;
				} else {
					// A scenario is visible if its predecessor was solved
					scenario->visible = solved_scenarios.count(campaign->scenarios.at(i-1)->path) == 1;
				}
				if (!scenario->visible) {
					// If a scenario is invisible, subsequent scenarios are also invisible
					break;
				}
			}
		}
	}
}

/**
 * Handle legacy campvis file
 */
// TODO(GunChleoc): Remove after Build 21
void CampaignVisibility::update_legacy_campvis(int version) {
	log("Converting legacy campvis\n");

	std::map<std::string, std::string> legacy_scenarios = {
		{"fri02.wmf", "frisians01"},
		{"fri01.wmf", "frisians00"},
		{"atl02.wmf", "atlanteans01"},
		{"atl01.wmf", "atlanteans00"},
		{"emp04.wmf", "empiretut03"},
		{"emp03.wmf", "empiretut02"},
		{"emp02.wmf", "empiretut01"},
		{"emp01.wmf", "empiretut00"},
		{"bar02.wmf", "barbariantut01"},
		{"bar01.wmf", "barbariantut00"},
	};

	Profile read_campvis(kCampVisFile.c_str());
	Section& campvis_scenarios = read_campvis.get_safe_section("campmaps");
	bool set_solved = false;
	std::set<std::string> solved_legacy_scenarios;
	for (const auto& legacy_scenario : legacy_scenarios) {
		if (set_solved) {
			solved_legacy_scenarios.insert(legacy_scenario.first);
		}
		set_solved = campvis_scenarios.get_bool(legacy_scenario.second.c_str(), false);
	}

	// Now write everything
	Profile write_campvis(kCampVisFile.c_str());
	write_campvis.pull_section("global").set_int("version", version);

	Section& write_scenarios = write_campvis.pull_section("scenarios");
	for (const auto& scenario : solved_legacy_scenarios) {
		write_scenarios.set_bool(scenario.c_str(), true);
	}

	write_campvis.write(kCampVisFile.c_str(), true);
}

/**
 * Marks the scenario with the given file 'path' as solved.
 */
void CampaignVisibility::mark_scenario_as_solved(const std::string& path) {
	assert(g_fs->file_exists(kCampVisFile));
	// NOCOM evaluate whether we need to update here
	solved_scenarios.insert(path);
	update_visibility_info();

	// Write the campvis
	Profile campvis(kCampVisFile.c_str());
	campvis.pull_section("scenarios").set_bool(path.c_str(), true);
	campvis.write(kCampVisFile.c_str(), false);
}
