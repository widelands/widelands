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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_UI_FSMENU_CAMPAIGNS_H
#define WL_UI_FSMENU_CAMPAIGNS_H

#include <memory>
#include <unordered_set>

#include "graphic/image.h"
#include "wui/mapauthordata.h"

/**
 * Data about a campaign or tutorial scenario that we're interested in.
 */
struct ScenarioData {
	std::string path;
	std::string descname;
	std::string description;
	MapAuthorData authors;
	bool is_tutorial;
	bool playable;
	bool visible;

	ScenarioData() = default;
};

/**
 * Data about a campaign that we're interested in.
 */
struct CampaignData {
	std::string descname;
	std::string tribename;
	uint32_t difficulty_level;
	const Image* difficulty_image;
	std::string difficulty_description;
	std::string description;
	std::set<std::string> prerequisites;
	bool visible;
	std::vector<std::unique_ptr<ScenarioData>> scenarios;
	std::vector<std::string> difficulties;
	uint32_t default_difficulty;

	CampaignData() = default;
};

struct Campaigns {
	Campaigns();

	size_t no_of_campaigns() const {
		return campaigns_.size();
	}

	CampaignData* get_campaign(size_t campaign_index) const {
		assert(campaign_index < campaigns_.size());
		return campaigns_.at(campaign_index).get();
	}

private:
	void update_visibility_info();
	static void update_legacy_campvis();

	std::vector<std::unique_ptr<CampaignData>> campaigns_;
	std::unordered_set<std::string> solved_scenarios_;
};

#endif  // end of include guard: WL_UI_FSMENU_CAMPAIGNS_H
