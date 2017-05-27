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

#include "ui_fsmenu/campaign_select.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "logic/campaign_visibility.h"
#include "profile/profile.h"
#include "scripting/lua_interface.h"
#include "scripting/lua_table.h"

/**
 * CampaignSelect UI
 * Loads a list of all visible campaigns
 */
FullscreenMenuCampaignSelect::FullscreenMenuCampaignSelect()
   : FullscreenMenuLoadMapOrGame(),
     table_(this, 0, 0, 0, 0),

     // Main Title
     title_(this, 0, 0, _("Choose a campaign"), UI::Align::kCenter),

     // Campaign description
     campaign_details_(this) {
	title_.set_fontsize(UI_FONT_SIZE_BIG);
	back_.set_tooltip(_("Return to the main menu"));
	ok_.set_tooltip(_("Play this campaign"));

	ok_.sigclicked.connect(
	   boost::bind(&FullscreenMenuCampaignSelect::clicked_ok, boost::ref(*this)));
	back_.sigclicked.connect(
	   boost::bind(&FullscreenMenuCampaignSelect::clicked_back, boost::ref(*this)));
	table_.selected.connect(boost::bind(&FullscreenMenuCampaignSelect::entry_selected, this));
	table_.double_clicked.connect(
	   boost::bind(&FullscreenMenuCampaignSelect::clicked_ok, boost::ref(*this)));

	/** TRANSLATORS: Campaign difficulty table header */
	table_.add_column(45, _("Diff."), _("Difficulty"));
	table_.add_column(100, _("Tribe"), _("Tribe Name"));
	table_.add_column(
	   0, _("Campaign Name"), _("Campaign Name"), UI::Align::kLeft, UI::TableColumnType::kFlexible);
	table_.set_column_compare(
	   0, boost::bind(&FullscreenMenuCampaignSelect::compare_difficulty, this, _1, _2));
	table_.set_sort_column(0);
	table_.focus();
	fill_table();
	layout();
}

void FullscreenMenuCampaignSelect::layout() {
	FullscreenMenuLoadMapOrGame::layout();
	title_.set_pos(Vector2i(0, tabley_ / 3));
	title_.set_size(get_w(), title_.get_h());
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
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
}

std::string FullscreenMenuCampaignSelect::get_campaign() const {
	return campaign_;
}

/// Pictorial descriptions of difficulty levels.
static char const* const difficulty_picture_filenames[] = {
   "images/novalue.png", "images/ui_fsmenu/easy.png", "images/ui_fsmenu/challenging.png",
   "images/ui_fsmenu/hard.png"};

bool FullscreenMenuCampaignSelect::set_has_selection() {
	bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);
	return has_selection;
}

void FullscreenMenuCampaignSelect::entry_selected() {
	if (set_has_selection()) {
		const CampaignData& campaign_data = campaigns_data_[table_.get_selected()];
		campaign_ = campaign_data.name;
		campaign_details_.update(campaign_data);
	}
}

/**
 * fill the campaign list
 */
void FullscreenMenuCampaignSelect::fill_table() {
	campaigns_data_.clear();
	table_.clear();

	// Read in the campaign configuration for the currently selected campaign
	LuaInterface lua;
	std::unique_ptr<LuaTable> table(lua.run_script("campaigns/campaigns.lua"));
	table->do_not_warn_about_unaccessed_keys();
	std::unique_ptr<LuaTable> campaigns_table(table->get_table("campaigns"));
	campaigns_table->do_not_warn_about_unaccessed_keys();

	// Read in campvis-file
	CampaignVisibilitySave cvs;
	Profile campvis(cvs.get_path().c_str());
	Section& campaign_visibility = campvis.get_safe_section("campaigns");

	uint32_t counter = 0;
	for (const auto& campaign : campaigns_table->array_entries<std::unique_ptr<LuaTable>>()) {
		campaign->do_not_warn_about_unaccessed_keys();
		CampaignData campaign_data;
		campaign_data.index = counter;
		campaign_data.name = campaign->get_string("name");

		// Only list visible campaigns
		if (campaign_visibility.get_bool(campaign_data.name.c_str())) {

			i18n::Textdomain td("maps");

			campaign_data.descname = _(campaign->get_string("descname"));
			campaign_data.tribename = _(campaign->get_string("tribe"));
			campaign_data.description = _(campaign->get_string("description"));

			std::unique_ptr<LuaTable> difficulty(campaign->get_table("difficulty"));
			campaign_data.difficulty = difficulty->get_int("value");
			if (sizeof(difficulty_picture_filenames) / sizeof(*difficulty_picture_filenames) <=
				 campaign_data.difficulty) {
				campaign_data.difficulty = 0;
			}

			campaign_data.difficulty_description = _(difficulty->get_string("description"));
			campaigns_data_.push_back(campaign_data);

			UI::Table<uintptr_t>::EntryRecord& tableEntry = table_.add(campaign_data.index);
			tableEntry.set_picture(
				0, g_gr->images().get(difficulty_picture_filenames[campaign_data.difficulty]));
			tableEntry.set_string(1, campaign_data.tribename);
			tableEntry.set_string(2, campaign_data.descname);
			++counter;
		}
	}

	table_.sort();

	if (table_.size()) {
		table_.select(0);
	}
	set_has_selection();
}

bool FullscreenMenuCampaignSelect::compare_difficulty(uint32_t rowa, uint32_t rowb) {
	const CampaignData& r1 = campaigns_data_[table_[rowa]];
	const CampaignData& r2 = campaigns_data_[table_[rowb]];

	if (r1.difficulty < r2.difficulty) {
		return true;
	}
	return r1.index < r2.index;
}
