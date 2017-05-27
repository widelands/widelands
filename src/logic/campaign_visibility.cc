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

#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>

#include <sys/stat.h>

#include "base/log.h" // NOCOM
#include "base/wexception.h"
#include "io/filesystem/filesystem.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

namespace {
constexpr int kCurrentVersion = 8;
}

/**
 * Get the path of campaign visibility save-file
 */
std::string CampaignVisibilitySave::get_path() {
	std::string savepath = "save";
	g_fs->ensure_directory_exists(savepath);  // Make sure save directory exists
	savepath += "/campvis";                   // add the name of save-file

	// Make sure that campaigns visibility-save is up to date and well-formed
	update_campvis(savepath);
	return savepath;
}


/**
 * Update the campaign visibility save-file of the user
 */
void CampaignVisibilitySave::update_campvis(const std::string& savepath) {
	// Prepare campvis
	if (!(g_fs->file_exists(savepath))) {
		// There is no campvis file - create one.
		Profile campvis(savepath.c_str());
		campvis.pull_section("global");
		campvis.pull_section("campaigns");
		campvis.pull_section("scenarios");
		campvis.write(savepath.c_str(), true);
	}

	Profile campvis(savepath.c_str());


	// TODO(GunChleoc): Remove compatibility code after Build 21.
	std::map<std::string, std::string> legacy_scenarios;
	bool is_legacy = false;
	const int campvis_version = campvis.pull_section("global").get_int("version");
	if (campvis_version > 0 && campvis_version < 8) {
		is_legacy = true;
		legacy_scenarios = {
		      {"bar01", "barbariantut00"},
		      {"bar02", "barbariantut01"},
		      {"emp01", "empiretut00"},
		      {"emp02", "empiretut01"},
		      {"emp03", "empiretut02"},
		      {"emp04", "empiretut03"},
		      {"atl01", "atlanteans00"},
		      {"atl02", "atlanteans01"},
		};
	}

	Section& campvis_campaigns = campvis.get_safe_section("campaigns");
	Section& campvis_scenarios = is_legacy ? campvis.get_safe_section("campmaps") : campvis.get_safe_section("scenarios");

	// Prepare campaigns.lua
	LuaInterface lua;
	std::unique_ptr<LuaTable> table(lua.run_script("campaigns/campaigns.lua"));
	table->do_not_warn_about_unaccessed_keys();
	std::unique_ptr<LuaTable> campaigns_table(table->get_table("campaigns"));
	campaigns_table->do_not_warn_about_unaccessed_keys();

	// Collect all information about campaigns and scenarios
	std::map<std::string, bool> campaigns;
	std::map<std::string, bool> scenarios;
	for (const auto& campaign : campaigns_table->array_entries<std::unique_ptr<LuaTable>>()) {
		campaign->do_not_warn_about_unaccessed_keys();
		const std::string campaign_name = campaign->get_string("name");
		if (campaigns.count(campaign_name) != 1) {
			campaigns[campaign_name] = false;
		}
		campaigns[campaign_name] = campaigns[campaign_name] || campaign->get_bool("visible") || campvis_campaigns.get_bool(campaign_name.c_str());

		std::unique_ptr<LuaTable> scenarios_table(campaign->get_table("scenarios"));
		scenarios_table->do_not_warn_about_unaccessed_keys();
		for (const auto& scenario : scenarios_table->array_entries<std::unique_ptr<LuaTable>>()) {
			scenario->do_not_warn_about_unaccessed_keys();
			const std::string scenario_name = scenario->get_string("name");
			scenarios[scenario_name] = is_legacy ? campvis_scenarios.get_bool(legacy_scenarios[scenario_name].c_str()) : campvis_scenarios.get_bool(scenario_name.c_str());

			// If a scenario is visible, this campaign is visible too.
			if (scenarios[scenario_name]) {
				campaigns[campaign_name] = true;
			}
		}

		// A campaign can also make sure that scenarios of a previous campaign are visible
		if (campaigns[campaign_name] && campaign->has_key<std::string>("reveal_scenarios")) {
			for (const auto& scenario : campaign->get_table("reveal_scenarios")->array_entries<std::string>()) {
				scenarios[scenario] = true;
			}
		}
	}

	// Make sure that all visible campaigns have their first scenario visible.
	for (const auto& campaign : campaigns_table->array_entries<std::unique_ptr<LuaTable>>()) {
		campaign->do_not_warn_about_unaccessed_keys();
		const std::string campaign_name = campaign->get_string("name");
		if (campaigns[campaign_name]) {
			std::unique_ptr<LuaTable> scenarios_table(campaign->get_table("scenarios"));
			scenarios_table->do_not_warn_about_unaccessed_keys();
			const auto& scenario = scenarios_table->get_table(1);
			scenario->do_not_warn_about_unaccessed_keys();
			scenarios[scenario->get_string("name")] = true;
		}
	}

	// Now write everything
	Profile write_campvis(savepath.c_str());
	write_campvis.pull_section("global").set_int("version", kCurrentVersion);

	Section& write_campaigns = write_campvis.pull_section("campaigns");
	for (const auto& campaign: campaigns) {
		write_campaigns.set_bool(campaign.first.c_str(), campaign.second);
	}

	Section& write_scenarios = write_campvis.pull_section("scenarios");
	for (const auto& scenario: scenarios) {
		write_scenarios.set_bool(scenario.first.c_str(), scenario.second);
	}

	write_campvis.write(savepath.c_str(), true);
}

/**
 * Searches for the scenario with the given 'name' in data/campaigns/campaigns.lua,
 * and if the scenario has 'reveal_campaign' and/or 'reveal_scenario' defined, marks the respective
 * campaign/scenario as visible.
 */
void CampaignVisibilitySave::mark_scenario_as_solved(const std::string& name) {
	// Check which campaign and/or scenario to reveal
	std::string campaign_to_reveal = "";
	std::string scenario_to_reveal = "";

	LuaInterface lua;
	std::unique_ptr<LuaTable> table(lua.run_script("campaigns/campaigns.lua"));
	table->do_not_warn_about_unaccessed_keys();
	std::unique_ptr<LuaTable> campaigns_table(table->get_table("campaigns"));
	campaigns_table->do_not_warn_about_unaccessed_keys();

	bool found = false;
	for (const auto& campaign : campaigns_table->array_entries<std::unique_ptr<LuaTable>>()) {
		campaign->do_not_warn_about_unaccessed_keys();
		if (found) {
			continue;
		}
		std::unique_ptr<LuaTable> scenarios_table(campaign->get_table("scenarios"));
		scenarios_table->do_not_warn_about_unaccessed_keys();
		for (const auto& scenario : scenarios_table->array_entries<std::unique_ptr<LuaTable>>()) {
			scenario->do_not_warn_about_unaccessed_keys();
			if (name == scenario->get_string("name")) {
				if (scenario->has_key<std::string>("reveal_scenario")) {
					scenario_to_reveal = scenario->get_string("reveal_scenario");
				}
				if (scenario->has_key<std::string>("reveal_campaign")) {
					campaign_to_reveal = scenario->get_string("reveal_campaign");
				}
				// We can't break here, because we need to shut up the table warnings.
				found = true;
			}
		}
	}

	// Write the campvis
	std::string savepath = get_path();
	Profile campvis(savepath.c_str());
	if (!campaign_to_reveal.empty()) {
		campvis.pull_section("campaigns").set_bool(campaign_to_reveal.c_str(), true);

	}
	if (!scenario_to_reveal.empty()) {
		campvis.pull_section("scenarios").set_bool(scenario_to_reveal.c_str(), true);

	}
	campvis.write(savepath.c_str(), false);
}
