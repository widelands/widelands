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

#ifndef WL_UI_FSMENU_CAMPAIGN_SELECT_H
#define WL_UI_FSMENU_CAMPAIGN_SELECT_H

#include <vector>

#include "ui_basic/table.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/campaigndetails.h"
#include "ui_fsmenu/load_map_or_game.h"

/*
 * Fullscreen Menu for selecting a campaign
 */
class FullscreenMenuCampaignSelect : public FullscreenMenuLoadMapOrGame {
public:
	FullscreenMenuCampaignSelect();

	std::string get_campaign() const;

protected:
	void clicked_ok() override;
	void entry_selected() override;
	void fill_table() override;

private:
	void layout() override;

	/// Updates buttons and text labels and returns whether a table entry is selected.
	bool set_has_selection();

	bool compare_difficulty(uint32_t, uint32_t);

	UI::Table<uintptr_t const> table_;

	UI::Textarea title_;
	CampaignDetails campaign_details_;

	std::vector<CampaignData> campaigns_data_;

	/// Variables used for exchange between the two Campaign UIs and
	/// Game::run_campaign
	std::string campaign_;
};

#endif  // end of include guard: WL_UI_FSMENU_CAMPAIGN_SELECT_H
