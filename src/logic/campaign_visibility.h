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

#ifndef WL_LOGIC_CAMPAIGN_VISIBILITY_H
#define WL_LOGIC_CAMPAIGN_VISIBILITY_H

#include <memory>
#include <set> // NOCOM unordered?
#include <string>
#include <vector>

#include "scripting/lua_table.h"
#include "wui/mapauthordata.h" // NOCOM move this

/**
 * Data about a campaign or tutorial scenario that we're interested in.
 */
struct ScenarioData {
	uint32_t index; // NOCOM
	std::string path;
	std::string descname;
	std::string description;
	MapAuthorData authors;
	std::string campaign;
	bool is_tutorial;
	bool playable;
	bool visible;

	ScenarioData(const std::string& init_authors) : authors(init_authors) {}
	// We start with empty authors, because those will come from the map
	ScenarioData() : ScenarioData("") {}
};

/**
 * Data about a campaign that we're interested in.
 */
struct CampaignData {
	uint32_t index; // NOCOM
	std::string name;
	std::string descname;
	std::string tribename;
	uint32_t difficulty;
	std::string difficulty_description;
	std::string description;
	std::string prerequisite;
	bool visible;
	std::vector<std::unique_ptr<ScenarioData>> scenarios;

	CampaignData() = default;
};

struct CampaignVisibility {
	CampaignVisibility();
	void mark_scenario_as_solved(const std::string& path);

private:
	void update_visibility_info();
	static void update_legacy_campvis(int version);

	std::vector<std::unique_ptr<CampaignData>> campaigns_;
	ScenarioData scenarios_;
	std::set<std::string> solved_scenarios;
};

#endif  // end of include guard: WL_LOGIC_CAMPAIGN_VISIBILITY_H
