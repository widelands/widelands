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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_UI_FSMENU_CAMPAIGN_SELECT_H
#define WL_UI_FSMENU_CAMPAIGN_SELECT_H

#include "ui_basic/table.h"
#include "ui_fsmenu/campaigndetails.h"
#include "ui_fsmenu/campaigns.h"
#include "ui_fsmenu/menu.h"

namespace FsMenu {
/*
 * Fullscreen Menu for selecting a campaign
 */
class CampaignSelect : public TwoColumnsFullNavigationMenu {
public:
	explicit CampaignSelect(MenuCapsule&);

protected:
	void clicked_ok() override;
	void entry_selected();
	void fill_table();

private:
	/// Updates buttons and text labels and returns whether a table entry is selected.
	bool set_has_selection();

	bool compare_difficulty(uint32_t, uint32_t);

	UI::Table<uintptr_t const> table_;

	CampaignDetails campaign_details_;

	Campaigns campaigns_;
};
}  // namespace FsMenu
#endif  // end of include guard: WL_UI_FSMENU_CAMPAIGN_SELECT_H
