/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "ui_fsmenu/campaign_select.h"

#include "base/i18n.h"
#include "base/wexception.h"
#include "scripting/lua_table.h"

/**
 * CampaignSelect UI
 * Loads a list of all visible campaigns
 */
FullscreenMenuCampaignSelect::FullscreenMenuCampaignSelect(FullscreenMenuMain& fsmm,
                                                           Campaigns* campvis)
   : FullscreenMenuLoadMapOrGame(fsmm, _("Choose Campaign")),
     table_(this, 0, 0, 0, 0, UI::PanelStyle::kFsMenu),

     // Campaign description
     campaign_details_(this),
     campaigns_(campvis) {
	back_.set_tooltip(_("Return to the main menu"));
	ok_.set_tooltip(_("Play this campaign"));

	ok_.sigclicked.connect([this]() { clicked_ok(); });
	back_.sigclicked.connect([this]() { clicked_back(); });
	table_.selected.connect([this](unsigned) { entry_selected(); });
	table_.double_clicked.connect([this](unsigned) { clicked_ok(); });

	/** TRANSLATORS: Campaign difficulty table header */
	table_.add_column(45, _("Diff."), _("Difficulty"));
	table_.add_column(130, _("Tribe"), _("Tribe Name"));
	table_.add_column(
	   0, _("Campaign Name"), _("Campaign Name"), UI::Align::kLeft, UI::TableColumnType::kFlexible);
	table_.set_column_compare(
	   0, [this](uint32_t a, uint32_t b) { return compare_difficulty(a, b); });
	table_.set_sort_column(0);
	table_.focus();
	fill_table();
	layout();

	table_.cancel.connect([this]() { clicked_back(); });
}

void FullscreenMenuCampaignSelect::layout() {
	FullscreenMenuLoadMapOrGame::layout();
	table_.set_size(tablew_, tableh_);
	table_.set_pos(Vector2i(tablex_, tabley_));
	campaign_details_.set_size(get_right_column_w(right_column_x_), tableh_ - buth_ - 4 * padding_);
	campaign_details_.set_desired_size(
	   get_right_column_w(right_column_x_), tableh_ - buth_ - 4 * padding_);
	campaign_details_.set_pos(Vector2i(right_column_x_, tabley_));
}

/**
 * OK was clicked, after an entry of campaignlist got selected.
 */
void FullscreenMenuCampaignSelect::clicked_ok() {
	if (!table_.has_selection()) {
		return;
	}
	const CampaignData& campaign_data = *campaigns_->get_campaign(table_.get_selected());
	if (!campaign_data.visible) {
		return;
	}
	end_modal<MenuTarget>(MenuTarget::kOk);
}

size_t FullscreenMenuCampaignSelect::get_campaign_index() const {
	return table_.get_selected();
}

bool FullscreenMenuCampaignSelect::set_has_selection() {
	const bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);
	return has_selection;
}

void FullscreenMenuCampaignSelect::entry_selected() {
	if (set_has_selection()) {
		const CampaignData& campaign_data = *campaigns_->get_campaign(table_.get_selected());
		ok_.set_enabled(campaign_data.visible);
		campaign_details_.update(campaign_data);
	}
}

/**
 * fill the campaign list
 */
void FullscreenMenuCampaignSelect::fill_table() {
	table_.clear();

	for (size_t i = 0; i < campaigns_->no_of_campaigns(); ++i) {
		const CampaignData& campaign_data = *campaigns_->get_campaign(i);

		UI::Table<uintptr_t const>::EntryRecord& tableEntry = table_.add(i);
		tableEntry.set_picture(0, campaign_data.difficulty_image);
		tableEntry.set_string(1, campaign_data.tribename);
		tableEntry.set_string(2, campaign_data.descname);
		tableEntry.set_disabled(!campaign_data.visible);
	}

	if (table_.size()) {
		table_.sort();
		table_.select(0);
	}
	set_has_selection();
}

bool FullscreenMenuCampaignSelect::compare_difficulty(uint32_t rowa, uint32_t rowb) {
	const CampaignData& r1 = *campaigns_->get_campaign(table_[rowa]);
	const CampaignData& r2 = *campaigns_->get_campaign(table_[rowb]);

	if (r1.difficulty_level < r2.difficulty_level) {
		return true;
	}
	return table_[rowa] < table_[rowb];
}
