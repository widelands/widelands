/*
 * Copyright (C) 2002-2012 by the Widelands Development Team
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
#include "logic/campaign_visibility.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "wui/text_constants.h"


/*
 * UI 1 - Selection of Campaign
 *
 */

/**
 * CampaignSelect UI
 * Loads a list of all visible campaigns
 */
FullscreenMenuCampaignSelect::FullscreenMenuCampaignSelect() :
	FullscreenMenuLoadMapOrGame(),

	// Main Title
	m_title
		(this, get_w() / 2, m_maplisty / 3,
		 _("Choose a campaign"),
		 UI::Align_HCenter),

	// Campaign description
	m_label_campname
		(this, m_right_column_x, m_maplisty,
		 _("Campaign Name:"),
		 UI::Align_Left),
	m_ta_campname(this,
					  m_right_column_x + m_indent, get_y_from_preceding(m_label_campname) + m_padding,
					  get_right_column_w(m_right_column_x) - m_indent, m_label_height),

	m_label_tribename
		(this, m_right_column_x, get_y_from_preceding(m_ta_campname) + 2 * m_padding,
		 _("Tribe:"),
		 UI::Align_Left),
	m_ta_tribename(this,
						 m_right_column_x + m_indent, get_y_from_preceding(m_label_tribename) + m_padding,
						 get_right_column_w(m_right_column_x + m_indent), m_label_height),

	m_label_difficulty
		(this, m_right_column_x, get_y_from_preceding(m_ta_tribename) + 2 * m_padding,
		 _("Difficulty:"),
		 UI::Align_Left),
	m_ta_difficulty(this,
						 m_right_column_x + m_indent, get_y_from_preceding(m_label_difficulty) + m_padding,
						 get_right_column_w(m_right_column_x + m_indent), 2 * m_label_height - m_padding),

	m_label_description
		(this, m_right_column_x, get_y_from_preceding(m_ta_difficulty) + 2 * m_padding,
		 _("Description:"),
		 UI::Align_Left),
	m_ta_description
		(this,
		 m_right_column_x + m_indent,
		 get_y_from_preceding(m_label_description) + m_padding,
		 get_right_column_w(m_right_column_x + m_indent),
		 m_buty - get_y_from_preceding(m_label_description) - 4 * m_padding),

	// Campaign list
	m_table(this, m_maplistx, m_maplisty, m_maplistw, m_maplisth)
{
	m_title.set_textstyle(ts_big());
	m_back.set_tooltip(_("Return to the main menu"));
	m_ok.set_tooltip(_("Play this campaign"));
	m_ta_campname.set_tooltip(_("The name of this campaign"));
	m_ta_tribename.set_tooltip(_("The tribe you will be playing"));
	m_ta_difficulty.set_tooltip(_("The difficulty of this campaign"));
	m_ta_description.set_tooltip(_("Story and hints"));

	m_ok.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuCampaignSelect::clicked_ok, boost::ref(*this)));
	m_back.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuCampaignSelect::clicked_back, boost::ref(*this)));
	m_table.selected.connect
		(boost::bind(&FullscreenMenuCampaignSelect::campaign_selected, this, _1));
	m_table.double_clicked.connect
		(boost::bind(&FullscreenMenuCampaignSelect::clicked_ok, boost::ref(*this)));

	/** TRANSLATORS: Campaign difficulty table header */
	m_table.add_column(45, _("Diff."), _("Difficulty"), UI::Align_Left);
	m_table.add_column(100, _("Tribe"), _("Tribe Name"), UI::Align_Left);
	m_table.add_column(m_table.get_w() - 100 - 45, _("Campaign Name"), _("Campaign Name"), UI::Align_Left);
	m_table.set_column_compare
			(0,
			 boost::bind(&FullscreenMenuCampaignSelect::compare_difficulty, this, _1, _2));
	m_table.set_sort_column(0);
	m_table.focus();
	fill_list();
}


/**
 * OK was clicked, after an entry of campaignlist got selected.
 */
void FullscreenMenuCampaignSelect::clicked_ok()
{
	get_campaign();
	end_modal(1);
}

int32_t FullscreenMenuCampaignSelect::get_campaign()
{
	return campaign;
}

/// Pictorial descriptions of difficulty levels.
static char const * const difficulty_picture_filenames[] = {
	"pics/novalue.png",
	"pics/easy.png",
	"pics/challenging.png",
	"pics/hard.png"
};

/**
 * an entry of campaignlist got selected.
 */
void FullscreenMenuCampaignSelect::campaign_selected(uint32_t i)
{

	if (i < m_campaigns_data.size()) { // Don't test for get_selected(), because we need index 0

		const CampaignListData& campaign_data = m_campaigns_data[i];
		campaign = campaign_data.index;

		// enable OK button
		m_ok.set_enabled(true);
		m_ta_campname.set_text(campaign_data.name);
		m_ta_tribename.set_text(campaign_data.tribename);
		m_ta_difficulty.set_text(campaign_data.difficulty_description);
		m_ta_description.set_text(campaign_data.description);

	} else { // normally never here
		m_ok.set_enabled(false);
		m_ta_campname.set_text("");
		m_ta_tribename.set_text("");
		m_ta_difficulty.set_text("");
		m_ta_description .set_text("");
	}
	m_ta_description.scroll_to_top();
}


/**
 * fill the campaign list
 */
void FullscreenMenuCampaignSelect::fill_list()
{
	m_campaigns_data.clear();
	m_table.clear();

	// Read in the campaign config
	Profile prof("campaigns/cconfig", nullptr, "maps");
	Section & s = prof.get_safe_section("global");

	// Read in campvis-file
	CampaignVisibilitySave cvs;
	Profile campvis(cvs.get_path().c_str());
	Section & c = campvis.get_safe_section("campaigns");

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
			if
				(sizeof (difficulty_picture_filenames)
				 /
				 sizeof(*difficulty_picture_filenames)
				 <=
				 difficulty) {
				difficulty = 0;
			}

			CampaignListData campaign_data;

			campaign_data.index = i;
			campaign_data.name = s.get_string(cname.c_str(), "");
			campaign_data.tribename = s.get_string(ctribename.c_str(), "");
			campaign_data.difficulty = difficulty;
			campaign_data.difficulty_description = s.get_string(cdiff_descr.c_str(), "");
			campaign_data.description = s.get_string(cdescription.c_str(), "");
			m_campaigns_data.push_back(campaign_data);

			UI::Table<uintptr_t>::EntryRecord& tableEntry = m_table.add(i);
			tableEntry.set_picture(0, g_gr->images().get(difficulty_picture_filenames[difficulty]));
			tableEntry.set_string(1, campaign_data.tribename);
			tableEntry.set_string(2, campaign_data.name);
		}

		// Increase counter & csection
		++i;
		csection = (boost::format("campsect%u") % i).str();

	} // while (s.get_string(csection.c_str()))

	m_table.sort();
	if (m_table.size()) {
		m_table.select(0);
	}
}

bool FullscreenMenuCampaignSelect::compare_difficulty
	(uint32_t rowa, uint32_t rowb)
{
	const CampaignListData& r1 = m_campaigns_data[m_table[rowa]];
	const CampaignListData& r2 = m_campaigns_data[m_table[rowb]];

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
FullscreenMenuCampaignMapSelect::FullscreenMenuCampaignMapSelect() :
	FullscreenMenuLoadMapOrGame(),

	// Main title
	m_title
		(this, get_w() / 2, m_maplisty / 3,
		 _("Choose a scenario"),
		 UI::Align_HCenter),
	m_subtitle
		(this, get_w() / 2, get_y_from_preceding(m_title) + 6 * m_padding,
		 "",
		 UI::Align_HCenter),

	// Map description
	m_label_mapname
		(this, m_right_column_x, m_maplisty,
		 _("Scenario:"),
		 UI::Align_Left),
	m_ta_mapname(this,
					 m_right_column_x + m_indent, get_y_from_preceding(m_label_mapname) + m_padding,
					 get_right_column_w(m_right_column_x + m_indent), m_label_height),

	m_label_author
		(this,
		 m_right_column_x, get_y_from_preceding(m_ta_mapname) + 2 * m_padding,
		 _("Authors:"),
		 UI::Align_Left),
	m_ta_author(this,
					m_right_column_x + m_indent, get_y_from_preceding(m_label_author) + m_padding,
					get_right_column_w(m_right_column_x + m_indent), m_label_height),

	m_label_description
		(this, m_right_column_x, get_y_from_preceding(m_ta_author) + 2 * m_padding,
		 _("Description:"),
		 UI::Align_Left),
	m_ta_description
		(this,
		 m_right_column_x + m_indent,
		 get_y_from_preceding(m_label_description) + m_padding,
		 get_right_column_w(m_right_column_x + m_indent),
		 m_buty - get_y_from_preceding(m_label_description) - 4 * m_padding),

	// Campaign map list
	m_table(this, m_maplistx, m_maplisty, m_maplistw, m_maplisth)
{
	m_title.set_textstyle(ts_big());
	m_back.set_tooltip(_("Return to the main menu"));
	m_ok.set_tooltip(_("Play this scenario"));
	m_ta_mapname.set_tooltip(_("The name of this scenario"));
	m_ta_author.set_tooltip(_("The designers of this scenario"));
	m_ta_description.set_tooltip(_("Story and hints"));

	m_ok.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuCampaignMapSelect::clicked_ok, boost::ref(*this)));
	m_back.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuCampaignMapSelect::clicked_back, boost::ref(*this)));
	m_table.selected.connect(boost::bind(&FullscreenMenuCampaignMapSelect::map_selected, this, _1));
	m_table.double_clicked.connect
		(boost::bind(&FullscreenMenuCampaignMapSelect::clicked_ok, boost::ref(*this)));

	/** TRANSLATORS: Campaign scenario number table header */
	m_table.add_column(35, _("#"), _("The number of this scenario in the campaign"), UI::Align_Left);
	m_table.add_column(m_table.get_w() - 35, _("Scenario Name"), _("Scenario Name"), UI::Align_Left);
	m_table.set_sort_column(0);

	m_table.focus();
}


std::string FullscreenMenuCampaignMapSelect::get_map()
{
	return campmapfile;
}


//telling this class what campaign we have and since we know what campaign we
//have, fill it.
void FullscreenMenuCampaignMapSelect::set_campaign(uint32_t const i) {
	campaign = i;
	fill_list();
}

/**
 * an entry of the maplist got selected.
 */
void FullscreenMenuCampaignMapSelect::map_selected(uint32_t) {
	const CampaignScenarioData& scenario_data = m_scenarios_data[m_table.get_selected()];
	campmapfile = scenario_data.path;
	Widelands::Map map;

	std::unique_ptr<Widelands::MapLoader> ml(map.get_correct_loader(campmapfile));
	if (!ml) {
		throw wexception
			(_("Invalid path to file in cconfig: %s"), campmapfile.c_str());
	}

	map.set_filename(campmapfile);
	ml->preload_map(true);

	i18n::Textdomain td("maps");
	m_ta_mapname.set_text(_(map.get_name()));
	m_ta_author.set_text(map.get_author());
	m_ta_description.set_text(_(map.get_description()));
	m_ta_description.scroll_to_top();

	// enable OK button
	m_ok.set_enabled(true);
}


/**
 * fill the campaign-map list
 */
void FullscreenMenuCampaignMapSelect::fill_list()
{
	// read in the campaign config
	Profile prof("campaigns/cconfig", nullptr, "maps");
	Section & global_s = prof.get_safe_section("global");

	// Read in campvis-file
	CampaignVisibilitySave cvs;
	Profile campvis(cvs.get_path().c_str());
	Section & c = campvis.get_safe_section("campmaps");

	// Set subtitle of the page
	const std::string campaign_name =
			global_s.get_string((boost::format("campname%u") % campaign).str().c_str());
	const std::string campaign_tribe =
			global_s.get_string((boost::format("camptribe%u") % campaign).str().c_str());
	m_subtitle.set_text((boost::format("%s — %s") % campaign_tribe % campaign_name).str().c_str());

	// Get section of campaign-maps
	std::string campsection = global_s.get_string((boost::format("campsect%u") % campaign).str().c_str());

	// Create the entry we use to load the section of the map
	uint32_t i = 0;
	std::string mapsection = campsection + (boost::format("%02i") % i).str();

	// Add all visible entries to the list.
	while (Section * const s = prof.get_section(mapsection.c_str())) {
		if (c.get_bool(mapsection.c_str())) {

			CampaignScenarioData scenario_data;
			scenario_data.index = i + 1;
			scenario_data.name = s->get_string("name", "");
			scenario_data.path = s->get_string("path");
			m_scenarios_data.push_back(scenario_data);

			UI::Table<uintptr_t>::EntryRecord& tableEntry = m_table.add(i);
			tableEntry.set_string(0, (boost::format("%u") % scenario_data.index).str());
			tableEntry.set_picture(1, g_gr->images().get("pics/ls_wlmap.png"), scenario_data.name);
		}

		// Increase counter & mapsection
		++i;
		mapsection = campsection + (boost::format("%02i") % i).str();
	}
	m_table.sort();
	if (m_table.size()) {
		m_table.select(0);
	}
}
