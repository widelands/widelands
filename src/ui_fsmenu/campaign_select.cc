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

#include "ui_fsmenu/campaign_select.h"

#include <memory>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/wexception.h"
#include "graphic/graphic.h"
#include "graphic/text_constants.h"
#include "logic/campaign_visibility.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"

/*
 * UI 1 - Selection of Campaign
 *
 */

/**
 * CampaignSelect UI
 * Loads a list of all visible campaigns
 */
FullscreenMenuCampaignSelect::FullscreenMenuCampaignSelect()
   : FullscreenMenuLoadMapOrGame(),
     table_(this, tablex_, tabley_, tablew_, tableh_, false),

     // Main Title
     title_(this, get_w() / 2, tabley_ / 3, _("Choose a campaign"), UI::Align::kHCenter),

     // Campaign description
     label_campname_(this, right_column_x_, tabley_, "", UI::Align::kLeft),
     ta_campname_(this,
                  right_column_x_ + indent_,
                  get_y_from_preceding(label_campname_) + padding_,
                  get_right_column_w(right_column_x_) - indent_,
                  label_height_),

     label_tribename_(this,
                      right_column_x_,
                      get_y_from_preceding(ta_campname_) + 2 * padding_,
                      "",
                      UI::Align::kLeft),
     ta_tribename_(this,
                   right_column_x_ + indent_,
                   get_y_from_preceding(label_tribename_) + padding_,
                   get_right_column_w(right_column_x_ + indent_),
                   label_height_),

     label_difficulty_(this,
                       right_column_x_,
                       get_y_from_preceding(ta_tribename_) + 2 * padding_,
                       "",
                       UI::Align::kLeft),
     ta_difficulty_(this,
                    right_column_x_ + indent_,
                    get_y_from_preceding(label_difficulty_) + padding_,
                    get_right_column_w(right_column_x_ + indent_),
                    2 * label_height_ - padding_),

     label_description_(this,
                        right_column_x_,
                        get_y_from_preceding(ta_difficulty_) + 2 * padding_,
                        _("Description:"),
                        UI::Align::kLeft),
     ta_description_(this,
                     right_column_x_ + indent_,
                     get_y_from_preceding(label_description_) + padding_,
                     get_right_column_w(right_column_x_ + indent_),
                     buty_ - get_y_from_preceding(label_description_) - 4 * padding_) {
	title_.set_fontsize(UI_FONT_SIZE_BIG);
	back_.set_tooltip(_("Return to the main menu"));
	ok_.set_tooltip(_("Play this campaign"));
	ta_campname_.set_tooltip(_("The name of this campaign"));
	ta_tribename_.set_tooltip(_("The tribe you will be playing"));
	ta_difficulty_.set_tooltip(_("The difficulty of this campaign"));

	ok_.sigclicked.connect(
	   boost::bind(&FullscreenMenuCampaignSelect::clicked_ok, boost::ref(*this)));
	back_.sigclicked.connect(
	   boost::bind(&FullscreenMenuCampaignSelect::clicked_back, boost::ref(*this)));
	table_.selected.connect(boost::bind(&FullscreenMenuCampaignSelect::entry_selected, this));
	table_.double_clicked.connect(
	   boost::bind(&FullscreenMenuCampaignSelect::clicked_ok, boost::ref(*this)));

	/** TRANSLATORS: Campaign difficulty table header */
	table_.add_column(45, _("Diff."), _("Difficulty"), UI::Align::kLeft);
	table_.add_column(100, _("Tribe"), _("Tribe Name"), UI::Align::kLeft);
	table_.add_column(
	   table_.get_w() - 100 - 45, _("Campaign Name"), _("Campaign Name"), UI::Align::kLeft);
	table_.set_column_compare(
	   0, boost::bind(&FullscreenMenuCampaignSelect::compare_difficulty, this, _1, _2));
	table_.set_sort_column(0);
	table_.focus();
	fill_table();
}

void FullscreenMenuCampaignSelect::fit_to_screen() {
	// NOCOM implement
}

/**
 * OK was clicked, after an entry of campaignlist got selected.
 */
void FullscreenMenuCampaignSelect::clicked_ok() {
	if (!table_.has_selection()) {
		return;
	}
	get_campaign();
	end_modal<FullscreenMenuBase::MenuTarget>(FullscreenMenuBase::MenuTarget::kOk);
}

int32_t FullscreenMenuCampaignSelect::get_campaign() {
	return campaign;
}

/// Pictorial descriptions of difficulty levels.
static char const* const difficulty_picture_filenames[] = {
   "images/novalue.png", "images/ui_fsmenu/easy.png", "images/ui_fsmenu/challenging.png",
   "images/ui_fsmenu/hard.png"};

bool FullscreenMenuCampaignSelect::set_has_selection() {
	bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);

	if (!has_selection) {
		label_campname_.set_text(std::string());
		label_tribename_.set_text(std::string());
		label_difficulty_.set_text(std::string());
		label_description_.set_text(std::string());

		ta_campname_.set_text(std::string());
		ta_tribename_.set_text(std::string());
		ta_difficulty_.set_text(std::string());
		ta_description_.set_text(std::string());

	} else {
		label_campname_.set_text(_("Campaign Name:"));
		label_tribename_.set_text(_("Tribe:"));
		label_difficulty_.set_text(_("Difficulty:"));
		label_description_.set_text(_("Description:"));
	}
	return has_selection;
}

void FullscreenMenuCampaignSelect::entry_selected() {
	if (set_has_selection()) {
		const CampaignListData& campaign_data = campaigns_data_[table_.get_selected()];
		campaign = campaign_data.index;

		ta_campname_.set_text(campaign_data.name);
		ta_tribename_.set_text(campaign_data.tribename);
		ta_difficulty_.set_text(campaign_data.difficulty_description);
		ta_description_.set_text(campaign_data.description);
	}
	ta_description_.scroll_to_top();
}

/**
 * fill the campaign list
 */
void FullscreenMenuCampaignSelect::fill_table() {
	campaigns_data_.clear();
	table_.clear();

	// Read in the campaign config
	Profile prof("campaigns/campaigns.conf", nullptr, "maps");
	Section& s = prof.get_safe_section("global");

	// Read in campvis-file
	CampaignVisibilitySave cvs;
	Profile campvis(cvs.get_path().c_str());
	Section& c = campvis.get_safe_section("campaigns");

	// Predefine variables, used in while-loop
	uint32_t i = 0;
	std::string csection = (boost::format("campsect%u") % i).str();
	std::string cname;
	std::string ctribename;
	std::string cdifficulty;
	std::string cdiff_descr;
	std::string cdescription;

	while (s.get_string(csection.c_str())) {

		cname = (boost::format("campname%u") % i).str();
		ctribename = (boost::format("camptribe%u") % i).str();
		cdifficulty = (boost::format("campdiff%u") % i).str();
		cdiff_descr = (boost::format("campdiffdescr%u") % i).str();
		cdescription = (boost::format("campdesc%u") % i).str();

		// Only list visible campaigns
		if (c.get_bool(csection.c_str())) {

			uint32_t difficulty = s.get_int(cdifficulty.c_str());
			if (sizeof(difficulty_picture_filenames) / sizeof(*difficulty_picture_filenames) <=
			    difficulty) {
				difficulty = 0;
			}

			CampaignListData campaign_data;

			campaign_data.index = i;

			{
				i18n::Textdomain td("maps");
				campaign_data.name = _(s.get_string(cname.c_str(), ""));
				campaign_data.tribename = _(s.get_string(ctribename.c_str(), ""));
				campaign_data.difficulty = difficulty;
				campaign_data.difficulty_description = _(s.get_string(cdiff_descr.c_str(), ""));
				campaign_data.description = _(s.get_string(cdescription.c_str(), ""));
			}

			campaigns_data_.push_back(campaign_data);

			UI::Table<uintptr_t>::EntryRecord& tableEntry = table_.add(i);
			tableEntry.set_picture(0, g_gr->images().get(difficulty_picture_filenames[difficulty]));
			tableEntry.set_string(1, campaign_data.tribename);
			tableEntry.set_string(2, campaign_data.name);
		}

		// Increase counter & csection
		++i;
		csection = (boost::format("campsect%u") % i).str();

	}  // while (s.get_string(csection.c_str()))
	table_.sort();

	if (table_.size()) {
		table_.select(0);
	}
	set_has_selection();
}

bool FullscreenMenuCampaignSelect::compare_difficulty(uint32_t rowa, uint32_t rowb) {
	const CampaignListData& r1 = campaigns_data_[table_[rowa]];
	const CampaignListData& r2 = campaigns_data_[table_[rowb]];

	if (r1.difficulty < r2.difficulty) {
		return true;
	}
	return r1.index < r2.index;
}

/*
 * UI 2 - Selection of a map
 *
 */

/**
 * CampaignMapSelect UI
 * Loads a list of all visible maps of selected campaign and let's the user
 * choose one.
 */
FullscreenMenuCampaignMapSelect::FullscreenMenuCampaignMapSelect(bool is_tutorial)
   : FullscreenMenuLoadMapOrGame(),
     table_(this, tablex_, tabley_, tablew_, tableh_, false),

     // Main title
     title_(this,
            get_w() / 2,
            tabley_ / 3,
            is_tutorial ? _("Choose a tutorial") : _("Choose a scenario"),
            UI::Align::kHCenter),
     subtitle_(this,
               get_w() / 6,
               get_y_from_preceding(title_) + 6 * padding_,
               get_w() * 2 / 3,
               4 * label_height_,
               "",
               UI::Align::kHCenter),

     // Map description
     label_mapname_(this, right_column_x_, tabley_, "", UI::Align::kLeft),
     ta_mapname_(this,
                 right_column_x_ + indent_,
                 get_y_from_preceding(label_mapname_) + padding_,
                 get_right_column_w(right_column_x_ + indent_),
                 label_height_),

     label_author_(this,
                   right_column_x_,
                   get_y_from_preceding(ta_mapname_) + 2 * padding_,
                   "",
                   UI::Align::kLeft),
     ta_author_(this,
                right_column_x_ + indent_,
                get_y_from_preceding(label_author_) + padding_,
                get_right_column_w(right_column_x_ + indent_),
                2 * label_height_),

     label_description_(
        this, right_column_x_, get_y_from_preceding(ta_author_) + padding_, "", UI::Align::kLeft),
     ta_description_(this,
                     right_column_x_ + indent_,
                     get_y_from_preceding(label_description_) + padding_,
                     get_right_column_w(right_column_x_ + indent_),
                     buty_ - get_y_from_preceding(label_description_) - 4 * padding_),

     is_tutorial_(is_tutorial) {
	title_.set_fontsize(UI_FONT_SIZE_BIG);
	back_.set_tooltip(_("Return to the main menu"));
	if (is_tutorial_) {
		ok_.set_tooltip(_("Play this tutorial"));
		ta_mapname_.set_tooltip(_("The name of this tutorial"));
		ta_description_.set_tooltip(_("What you will learn in this tutorial"));
	} else {
		ok_.set_tooltip(_("Play this scenario"));
		ta_mapname_.set_tooltip(_("The name of this scenario"));
	}

	ok_.sigclicked.connect(
	   boost::bind(&FullscreenMenuCampaignMapSelect::clicked_ok, boost::ref(*this)));
	back_.sigclicked.connect(
	   boost::bind(&FullscreenMenuCampaignMapSelect::clicked_back, boost::ref(*this)));
	table_.selected.connect(boost::bind(&FullscreenMenuCampaignMapSelect::entry_selected, this));
	table_.double_clicked.connect(
	   boost::bind(&FullscreenMenuCampaignMapSelect::clicked_ok, boost::ref(*this)));

	std::string number_tooltip;
	std::string name_tooltip;
	if (is_tutorial_) {
		number_tooltip = _("The order in which the tutorials should be played");
		name_tooltip = _("Tutorial Name");
	} else {
		number_tooltip = _("The number of this scenario in the campaign");
		name_tooltip = _("Scenario Name");
	}

	/** TRANSLATORS: Campaign scenario number table header */
	table_.add_column(35, _("#"), number_tooltip, UI::Align::kLeft);
	table_.add_column(table_.get_w() - 35, name_tooltip, name_tooltip, UI::Align::kLeft);
	table_.set_sort_column(0);

	table_.focus();
}

void FullscreenMenuCampaignMapSelect::fit_to_screen() {
	// NOCOM implement
}

std::string FullscreenMenuCampaignMapSelect::get_map() {
	return campmapfile;
}

// Telling this class what campaign we have and since we know what campaign we have, fill it.
void FullscreenMenuCampaignMapSelect::set_campaign(uint32_t const i) {
	campaign = i;
	fill_table();
}

bool FullscreenMenuCampaignMapSelect::set_has_selection() {
	bool has_selection = table_.has_selection();
	ok_.set_enabled(has_selection);

	if (!has_selection) {
		label_mapname_.set_text(std::string());
		label_author_.set_text(std::string());
		label_description_.set_text(std::string());

		ta_mapname_.set_text(std::string());
		ta_author_.set_text(std::string());
		ta_description_.set_text(std::string());

	} else {
		is_tutorial_ ? label_mapname_.set_text(_("Tutorial:")) :
		               label_mapname_.set_text(_("Scenario:"));
		label_description_.set_text(_("Description:"));
	}
	return has_selection;
}

void FullscreenMenuCampaignMapSelect::entry_selected() {
	if (set_has_selection()) {
		const CampaignScenarioData& scenario_data = scenarios_data_[table_.get_selected()];
		campmapfile = scenario_data.path;
		Widelands::Map map;

		std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(campmapfile));
		if (!ml) {
			throw wexception(_("Invalid path to file in campaigns.conf: %s"), campmapfile.c_str());
		}

		map.set_filename(campmapfile);
		ml->preload_map(true);

		// Localizing this, because some author fields now have "edited by" text.
		MapAuthorData authors(_(map.get_author()));

		ta_author_.set_text(authors.get_names());
		if (is_tutorial_) {
			ta_author_.set_tooltip(ngettext("The designer of this tutorial",
			                                "The designers of this tutorial", authors.get_number()));
		} else {
			ta_author_.set_tooltip(ngettext("The designer of this scenario",
			                                "The designers of this scenario", authors.get_number()));
		}
		label_author_.set_text(ngettext("Author:", "Authors:", authors.get_number()));

		{
			i18n::Textdomain td("maps");
			ta_mapname_.set_text(_(map.get_name()));
			ta_description_.set_text(_(map.get_description()));
		}
		ta_description_.scroll_to_top();

		// The dummy scenario can't be played, so we disable the OK button.
		if (campmapfile == "campaigns/dummy.wmf") {
			ok_.set_enabled(false);
		}
	}
}

/**
 * fill the campaign-map list
 */
void FullscreenMenuCampaignMapSelect::fill_table() {
	// read in the campaign config
	Profile* prof;
	std::string campsection;
	if (is_tutorial_) {
		prof = new Profile("campaigns/tutorials.conf", nullptr, "maps");

		// Set subtitle of the page
		const std::string subtitle1 = _("Pick a tutorial from the list, then hit \"OK\".");
		const std::string subtitle2 =
		   _("You can see a description of the currently selected tutorial on the right.");
		subtitle_.set_text((boost::format("%s\n%s") % subtitle1 % subtitle2).str());

		// Get section of campaign-maps
		campsection = "tutorials";

	} else {
		prof = new Profile("campaigns/campaigns.conf", nullptr, "maps");

		Section& global_s = prof->get_safe_section("global");

		// Set subtitle of the page
		const char* campaign_tribe =
		   _(global_s.get_string((boost::format("camptribe%u") % campaign).str().c_str()));
		const char* campaign_name;
		{
			i18n::Textdomain td("maps");
			campaign_name =
			   _(global_s.get_string((boost::format("campname%u") % campaign).str().c_str()));
		}
		subtitle_.set_text((boost::format("%s — %s") % campaign_tribe % campaign_name).str());

		// Get section of campaign-maps
		campsection = global_s.get_string((boost::format("campsect%u") % campaign).str().c_str());
	}

	// Create the entry we use to load the section of the map
	uint32_t i = 0;
	std::string mapsection = campsection + (boost::format("%02i") % i).str();

	// Read in campvis-file
	CampaignVisibilitySave cvs;
	Profile campvis(cvs.get_path().c_str());
	Section& c = campvis.get_safe_section("campmaps");

	// Add all visible entries to the list.
	while (Section* const s = prof->get_section(mapsection)) {
		if (is_tutorial_ || c.get_bool(mapsection.c_str())) {

			CampaignScenarioData scenario_data;
			scenario_data.index = i + 1;
			scenario_data.name = s->get_string("name", "");
			scenario_data.path = s->get_string("path");
			scenarios_data_.push_back(scenario_data);

			UI::Table<uintptr_t>::EntryRecord& tableEntry = table_.add(i);
			tableEntry.set_string(0, (boost::format("%u") % scenario_data.index).str());
			tableEntry.set_picture(
			   1, g_gr->images().get("images/ui_basic/ls_wlmap.png"), scenario_data.name);
		}

		// Increase counter & mapsection
		++i;
		mapsection = campsection + (boost::format("%02i") % i).str();
	}
	table_.sort();

	if (table_.size()) {
		table_.select(0);
	}
	set_has_selection();
}
