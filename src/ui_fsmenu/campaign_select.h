/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_CAMPAIGN_SELECT_H
#define WL_UI_FSMENU_CAMPAIGN_SELECT_H

#include "ui_basic/button.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/base.h"
#include "ui_fsmenu/load_map_or_game.h"

/*
 * Fullscreen Menu for all Campaigns
 */

/*
 * UI 1 - Selection of Campaign
 *
 */
class FullscreenMenuCampaignSelect : public FullscreenMenuLoadMapOrGame {
public:
	FullscreenMenuCampaignSelect();

	int32_t get_campaign();

protected:
	void clicked_ok() override;
	void entry_selected() override;
	void fill_table() override;

private:
	void layout() override;

	/// Updates buttons and text labels and returns whether a table entry is selected.
	bool set_has_selection();

	/**
	 * Data about a campaign that we're interested in.
	 */
	struct CampaignListData {
		uint32_t index;
		std::string name;
		std::string tribename;
		uint32_t difficulty;
		std::string difficulty_description;
		std::string description;

		CampaignListData() : index(0), difficulty(0) {
		}
	};

	bool compare_difficulty(uint32_t, uint32_t);

	UI::Table<uintptr_t const> table_;

	UI::Textarea title_;
	UI::Textarea label_campname_;
	UI::MultilineTextarea ta_campname_;
	UI::Textarea label_tribename_;
	UI::MultilineTextarea ta_tribename_;
	UI::Textarea label_difficulty_;
	UI::MultilineTextarea ta_difficulty_;
	UI::Textarea label_description_;
	UI::MultilineTextarea ta_description_;

	std::vector<CampaignListData> campaigns_data_;

	/// Variables used for exchange between the two Campaign UIs and
	/// Game::run_campaign
	int32_t campaign;
};
/*
 * UI 2 - Selection of a map
 *
 */
class FullscreenMenuCampaignMapSelect : public FullscreenMenuLoadMapOrGame {
public:
	FullscreenMenuCampaignMapSelect(bool is_tutorial = false);

	std::string get_map();
	void set_campaign(uint32_t);

protected:
	void entry_selected() override;
	void fill_table() override;

private:
	void layout() override;

	/// Updates buttons and text labels and returns whether a table entry is selected.
	bool set_has_selection();
	/**
	 * Data about a campaign scenario that we're interested in.
	 */
	struct CampaignScenarioData {
		uint32_t index;
		std::string name;
		std::string path;

		CampaignScenarioData() : index(0) {
		}
	};

	UI::Table<uintptr_t const> table_;

	UI::Textarea title_;
	UI::MultilineTextarea subtitle_;
	UI::Textarea label_mapname_;
	UI::MultilineTextarea ta_mapname_;
	UI::Textarea label_author_;
	UI::MultilineTextarea ta_author_;
	UI::Textarea label_description_;
	UI::MultilineTextarea ta_description_;

	uint32_t campaign;
	std::string campmapfile;

	std::vector<CampaignScenarioData> scenarios_data_;

	bool is_tutorial_;
};

#endif  // end of include guard: WL_UI_FSMENU_CAMPAIGN_SELECT_H
