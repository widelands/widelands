/*
 * Copyright (C) 2017-2022 by the Widelands Development Team
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
 */

#include "ui_fsmenu/campaigndetails.h"

#include "base/i18n.h"
#include "base/string.h"
#include "graphic/text_layout.h"
#include "ui_basic/scrollbar.h"

CampaignDetails::CampaignDetails(Panel* parent)
   : UI::Box(parent, UI::PanelStyle::kFsMenu, 0, 0, UI::Box::Vertical),
     name_label_(this,
                 0,
                 0,
                 UI::Scrollbar::kSize,
                 0,
                 UI::PanelStyle::kFsMenu,
                 "",
                 UI::Align::kLeft,
                 UI::MultilineTextarea::ScrollMode::kNoScrolling),
     descr_(this, 0, 0, UI::Scrollbar::kSize, 0, UI::PanelStyle::kFsMenu) {

	constexpr int kPadding = 4;
	add(&name_label_, UI::Box::Resizing::kFullSize);
	add_space(kPadding);
	add(&descr_, UI::Box::Resizing::kExpandBoth);
}

void CampaignDetails::update(const CampaignData& campaigndata) {
	name_label_.set_text(format("<rt>%s%s</rt>",
	                            /** TRANSLATORS: Header for campaign name */
	                            as_heading(_("Campaign"), UI::PanelStyle::kFsMenu, true),
	                            as_content(campaigndata.descname, UI::PanelStyle::kFsMenu)));

	std::string description;

	if (campaigndata.visible) {
		description = format("%s%s",
		                     /** TRANSLATORS: Header for campaign tribe */
		                     as_heading(_("Tribe"), UI::PanelStyle::kFsMenu),
		                     as_content(campaigndata.tribename, UI::PanelStyle::kFsMenu));
		description = format("%s%s", description,
		                     /** TRANSLATORS: Header for campaign difficulty */
		                     as_heading(_("Difficulty"), UI::PanelStyle::kFsMenu));
		description =
		   format("%s%s", description,
		          as_content(campaigndata.difficulty_description, UI::PanelStyle::kFsMenu));

		description = format("%s%s", description,
		                     /** TRANSLATORS: Header for campaign description */
		                     as_heading(_("Description"), UI::PanelStyle::kFsMenu));
		description =
		   format("%s%s", description, as_content(campaigndata.description, UI::PanelStyle::kFsMenu));
	}

	description = format("<rt>%s</rt>", description);
	descr_.set_text(description);
	descr_.scroll_to_top();
}
