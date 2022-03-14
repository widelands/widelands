/*
 * Copyright (C) 2007-2022 by the Widelands Development Team
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

#include "ui_fsmenu/campaigns.h"

#include <memory>

#include "base/log.h"
#include "graphic/image_cache.h"
#include "io/filesystem/filesystem.h"
#include "io/profile.h"
#include "logic/addons.h"
#include "logic/filesystem_constants.h"
#include "logic/map_objects/tribes/tribe_basic_info.h"
#include "scripting/lua_interface.h"

namespace {
constexpr const char* const kCampVisFileLegacy = "save/campvis";
}  // namespace

Campaigns::Campaigns() {
	// Load solved scenarios
	std::unique_ptr<Profile> campvis;
	g_fs->ensure_directory_exists(kSaveDir);
	if (!(g_fs->file_exists(kCampVisFile))) {
		// There is no campaigns.conf file - create one.
		campvis.reset(new Profile(kCampVisFile.c_str()));
		campvis->pull_section("scenarios");
		campvis->write(kCampVisFile.c_str(), true);
		if (g_fs->file_exists(kCampVisFileLegacy)) {
			update_legacy_campvis();
		}
	}
	campvis.reset(new Profile(kCampVisFile.c_str()));
	Section& campvis_scenarios = campvis->get_safe_section("scenarios");

	/** TRANSLATORS: A campaign difficulty */
	const std::string default_difficulty_name = _("Default");
	i18n::Textdomain td("maps");

	// Now load the campaign info
	Widelands::AllTribes all_tribes = Widelands::get_all_tribeinfos(nullptr);
	LuaInterface lua;
	std::vector<std::string> campaign_config_scripts = {"campaigns/campaigns.lua"};
	for (const auto& pair : AddOns::g_addons) {
		if (pair.first->category == AddOns::AddOnCategory::kCampaign && pair.second) {
			campaign_config_scripts.push_back(kAddOnDir + FileSystem::file_separator() +
			                                  pair.first->internal_name +
			                                  FileSystem::file_separator() + "campaigns.lua");
		}
	}
	for (const std::string& script : campaign_config_scripts) {
		std::unique_ptr<LuaTable> table(lua.run_script(script));

		// Read difficulty images
		std::unique_ptr<LuaTable> difficulties_table(table->get_table("difficulties"));
		std::vector<std::pair<const std::string, const Image*>> difficulty_levels;
		for (const auto& difficulty_level_table :
		     difficulties_table->array_entries<std::unique_ptr<LuaTable>>()) {
			difficulty_levels.emplace_back(
			   _(difficulty_level_table->get_string("descname")),
			   g_image_cache->get(difficulty_level_table->get_string("image")));
		}

		// Read the campaigns themselves
		std::unique_ptr<LuaTable> campaigns_table(table->get_table("campaigns"));

		for (const auto& campaign_table :
		     campaigns_table->array_entries<std::unique_ptr<LuaTable>>()) {
			CampaignData* campaign_data = new CampaignData();
			campaign_data->descname = _(campaign_table->get_string("descname"));
			campaign_data->tribename =
			   Widelands::get_tribeinfo(campaign_table->get_string("tribe"), all_tribes).descname;
			campaign_data->description = _(campaign_table->get_string("description"));
			if (campaign_table->has_key("prerequisites")) {
				for (const std::string& prerequisite :
				     campaign_table->get_table("prerequisites")->array_entries<std::string>()) {
					campaign_data->prerequisites.insert(prerequisite);
				}
			}
			if (campaign_table->has_key("difficulties")) {
				for (const std::string& d :
				     campaign_table->get_table("difficulties")->array_entries<std::string>()) {
					campaign_data->difficulties.push_back(d);
				}
				assert(campaign_table->has_key("default_difficulty"));
				campaign_data->default_difficulty =
				   get_positive_int(*campaign_table, "default_difficulty");
			} else {
				assert(!campaign_table->has_key("default_difficulty"));
				campaign_data->difficulties.push_back(default_difficulty_name);
				campaign_data->default_difficulty = 1;
			}

			campaign_data->visible = false;

			// Collect difficulty information
			std::unique_ptr<LuaTable> difficulty_table(campaign_table->get_table("difficulty"));
			campaign_data->difficulty_level = difficulty_table->get_int("level");
			campaign_data->difficulty_image =
			   difficulty_levels.at(campaign_data->difficulty_level - 1).second;
			campaign_data->difficulty_description =
			   difficulty_levels.at(campaign_data->difficulty_level - 1).first;
			const std::string difficulty_description = _(difficulty_table->get_string("description"));
			if (!difficulty_description.empty()) {
				campaign_data->difficulty_description =
				   i18n::join_sentences(campaign_data->difficulty_description, difficulty_description);
			}

			// Scenarios
			std::unique_ptr<LuaTable> scenarios_table(campaign_table->get_table("scenarios"));
			for (const std::string& path : scenarios_table->array_entries<std::string>()) {
				ScenarioData* scenario_data = new ScenarioData();
				scenario_data->path = path;
				if (campvis_scenarios.get_bool(scenario_data->path.c_str(), false)) {
					solved_scenarios_.insert(scenario_data->path);
				}

				scenario_data->is_tutorial = false;
				scenario_data->playable = scenario_data->path != "dummy.wmf";
				scenario_data->visible = false;
				campaign_data->scenarios.push_back(std::unique_ptr<ScenarioData>(scenario_data));
			}

			campaigns_.push_back(std::unique_ptr<CampaignData>(campaign_data));
		}
	}

	// Finally, calculate the visibility
	update_visibility_info();
}

void Campaigns::update_visibility_info() {
	for (auto& campaign : campaigns_) {
		if (campaign->prerequisites.empty()) {
			// A campaign is visible if it has no prerequisites
			campaign->visible = true;
		} else {
			// A campaign is visible if one of its prerequisites has been fulfilled
			for (const std::string& prerequisite : campaign->prerequisites) {
				if (solved_scenarios_.count(prerequisite) == 1) {
					campaign->visible = true;
					break;
				}
			}
		}
		if (!campaign->visible) {
			// A campaign is also visible if one of its scenarios has been solved
			for (size_t i = 0; i < campaign->scenarios.size(); ++i) {
				auto& scenario = campaign->scenarios.at(i);
				if (solved_scenarios_.count(scenario->path) == 1) {
					campaign->visible = true;
					break;
				}
			}
		}
		// Now set scenario visibility
		if (campaign->visible) {
			for (size_t i = 0; i < campaign->scenarios.size(); ++i) {
				auto& scenario = campaign->scenarios.at(i);
				if (i == 0) {
					// The first scenario in a visible campaign is always visible
					scenario->visible = true;
				} else {
					// A scenario is visible if its predecessor was solved
					scenario->visible =
					   solved_scenarios_.count(campaign->scenarios.at(i - 1)->path) == 1;
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
// TODO(GunChleoc): Savegame compatibility, remove after v1.0
void Campaigns::update_legacy_campvis() {
	Profile legacy_campvis(kCampVisFileLegacy);
	if (legacy_campvis.get_section("campmaps") == nullptr) {
		return;
	}

	log_info("Converting legacy campvis\n");

	using LegacyList = std::vector<std::pair<std::string, std::string>>;

	std::vector<LegacyList> legacy_scenarios;

	legacy_scenarios.push_back(
	   {{"fri02.wmf", "frisians01"}, {"fri01.wmf", "frisians00"}, {"atl01.wmf", "atlanteans00"}});

	legacy_scenarios.push_back(
	   {{"fri02.wmf", "frisians01"}, {"fri01.wmf", "frisians00"}, {"emp04.wmf", "empiretut03"}});

	legacy_scenarios.push_back({{"atl02.wmf", "atlanteans01"},
	                            {"atl01.wmf", "atlanteans00"},
	                            {"emp02.wmf", "empiretut01"},
	                            {"emp01.wmf", "empiretut00"}});

	legacy_scenarios.push_back({
	   {"emp04.wmf", "empiretut03"},
	   {"emp03.wmf", "empiretut02"},
	   {"emp02.wmf", "empiretut01"},
	   {"emp01.wmf", "empiretut00"},
	   {"bar02.wmf", "barbariantut01"},
	   {"bar01.wmf", "barbariantut00"},
	});

	Section& campvis_scenarios = legacy_campvis.get_safe_section("campmaps");
	std::set<std::string> solved_legacy_scenarios;
	for (const auto& legacy_list : legacy_scenarios) {
		bool set_solved = false;
		for (const auto& legacy_scenario : legacy_list) {
			if (set_solved) {
				solved_legacy_scenarios.insert(legacy_scenario.first);
			}
			set_solved = campvis_scenarios.get_bool(legacy_scenario.second.c_str(), false);
		}
	}

	// Now write everything
	Profile write_campvis(kCampVisFile.c_str());
	Section& write_scenarios = write_campvis.pull_section("scenarios");
	for (const auto& scenario : solved_legacy_scenarios) {
		write_scenarios.set_bool(scenario.c_str(), true);
	}

	write_campvis.write(kCampVisFile.c_str(), true);
}
