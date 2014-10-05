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
		(this,
		 get_w() / 2, m_maplisty / 3,
		 _("Choose a campaign"),
		 UI::Align_HCenter),

	// Campaign description
	m_label_mapname
		(this,
		 m_butx, m_maplisty,
		 _("Campaign:"),
		 UI::Align_Left),
	m_ta_mapname(this, m_butx + m_indent, m_label_mapname.get_y() + m_label_mapname.get_h() + m_padding,
					get_w() - m_butx - m_indent - m_margin_right, 2 * m_label_height - m_padding),

	m_label_difficulty
		(this,
		 m_butx, m_ta_mapname.get_y() + m_ta_mapname.get_h() + 3 * m_padding,
		 _("Difficulty:"),
		 UI::Align_Left),
	m_ta_difficulty(this, m_butx + m_indent,
						 m_label_difficulty.get_y() + m_label_difficulty.get_h() + m_padding,
						 get_w() - m_butx - m_indent - m_margin_right, 2 * m_label_height - m_padding),

	m_label_description
		(this,
		 m_butx, m_ta_difficulty.get_y() + m_ta_difficulty.get_h() + 3 * m_padding,
		 _("Description:"),
		 UI::Align_Left),
	m_ta_description
		(this,
		 m_butx + m_indent,
		 m_label_description.get_y() + m_label_description.get_h() + m_padding,
		 get_w() - m_butx - m_indent - m_margin_right,
		 m_buty - m_label_description.get_y() - m_label_description.get_h()  - 4 * m_padding),

	// Campaign list
	m_list(this, m_maplistx, m_maplisty, m_maplistw, m_maplisth)
{
	m_back.set_tooltip(_("Return to the main menu"));
	m_ok.set_tooltip(_("Play this campaign"));
	m_ta_mapname.set_tooltip(_("The name of this campaign"));
	m_ta_difficulty.set_tooltip(_("The difficulty of this campaign"));
	m_ta_description.set_tooltip(_("Story and hints"));

	m_ok.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuCampaignSelect::clicked_ok, boost::ref(*this)));
	m_back.sigclicked.connect
		(boost::bind
			 (&FullscreenMenuCampaignSelect::end_modal, boost::ref(*this), 0));

	m_title.set_textstyle(ts_big());

	m_list.selected.connect
		(boost::bind(&FullscreenMenuCampaignSelect::campaign_selected, this, _1));
	m_list.double_clicked.connect
		(boost::bind(&FullscreenMenuCampaignSelect::double_clicked, this, _1));
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
void FullscreenMenuCampaignSelect::campaign_selected(uint32_t const i)
{
	if (m_list.get_selected()) { //  false if the selected entry has no value
		campaign = i;

		// enable OK button
		m_ok.set_enabled(true);

		Profile prof("campaigns/cconfig", nullptr, "maps");
		Section & s = prof.get_safe_section("global");

		const std::string cname = (boost::format("campname%u") % i).str();
		const std::string cdifficulty = (boost::format("campdiff%u") % i).str();
		const std::string cdif_descr = (boost::format("campdiffdescr%u") % i).str();
		const std::string cdescription = (boost::format("campdesc%u") % i).str();

		s.get_natural(cdifficulty.c_str());

		std::string dif_description = s.get_string
			(cdif_descr.c_str(), _("[No value found]"));

		m_ta_mapname .set_text(s.get_string(cname.c_str(), _("[No value found]")));
		m_ta_difficulty.set_text(dif_description.c_str());
		m_ta_description.set_text(s.get_string(cdescription.c_str(), _("[No value found]")));

	} else { // normally never here
		m_ok.set_enabled(false);
		m_ta_mapname  .set_text(_("[Invalid entry]"));
		m_ta_difficulty.set_text("");
		m_ta_description .set_text("");
	}
}


/**
 * listbox got double clicked
 */
void FullscreenMenuCampaignSelect::double_clicked(uint32_t)
{
	clicked_ok();
}


/**
 * fill the campaign list
 */
void FullscreenMenuCampaignSelect::fill_list()
{
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
	std::string cdifficulty;

	while (s.get_string(csection.c_str())) {

		cname = (boost::format("campname%u") % i).str();
		cdifficulty = (boost::format("campdiff%u") % i).str();

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

			m_list.add
				(s.get_string(cname.c_str(), _("[No value found]")),
				 s.get_string(csection.c_str()),
				 g_gr->images().get(difficulty_picture_filenames[difficulty]));
		}

		// Increase counter & csection
		++i;
		csection = (boost::format("campsect%u") % i).str();

	} // while (s.get_string(csection.c_str()))
	if (m_list.size()) {
		m_list.select(0);
	}
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
		(this,
		 get_w() / 2, m_maplisty / 3,
		 _("Choose a scenario"),
		 UI::Align_HCenter),
	m_subtitle
		(this,
		 get_w() / 2, m_maplisty / 3 + 50,
		 "",
		 UI::Align_HCenter),

	// Map description
	m_label_mapname
		(this,
		 m_butx, m_maplisty,
		 _("Scenario:"),
		 UI::Align_Left),
	m_ta_mapname(this, m_butx + m_indent, m_label_mapname.get_y() + m_label_mapname.get_h() + m_padding,
					get_w() - m_butx - m_indent - m_margin_right, 2 * m_label_height - m_padding),

	m_label_author
		(this,
		 m_butx, m_ta_mapname.get_y() + m_ta_mapname.get_h() + 3 * m_padding,
		 _("Authors:"),
		 UI::Align_Left),
	m_ta_author(this, m_butx + m_indent, m_label_author.get_y() + m_label_author.get_h() + m_padding,
				get_w() - m_butx - m_indent - m_margin_right, m_label_height),

	m_label_description
		(this,
		 m_butx, m_ta_author.get_y() + m_ta_author.get_h() + 3 * m_padding,
		 _("Description:"),
		 UI::Align_Left),
	m_ta_description
		(this,
		 m_butx + m_indent,
		 m_label_description.get_y() + m_label_description.get_h() + m_padding,
		 get_w() - m_butx - m_indent - m_margin_right,
		 m_buty - m_label_description.get_y() - m_label_description.get_h()  - 4 * m_padding),

	// Campaign map list
	m_list(this, m_maplistx, m_maplisty, m_maplistw, m_maplisth)
{
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
			 (&FullscreenMenuCampaignMapSelect::end_modal, boost::ref(*this), 0));

	m_title.set_textstyle(ts_big());

	m_list.selected.connect(boost::bind(&FullscreenMenuCampaignMapSelect::map_selected, this, _1));
	m_list.double_clicked.connect
		(boost::bind(&FullscreenMenuCampaignMapSelect::double_clicked, this, _1));
}


/**
 * OK was clicked, after an entry of maplist got selected.
 */
void FullscreenMenuCampaignMapSelect::clicked_ok()
{
	end_modal(1);
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
	campmapfile = m_list.get_selected();
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

	// enable OK button
	m_ok.set_enabled(true);
}


/**
 * listbox got double clicked
 */
void FullscreenMenuCampaignMapSelect::double_clicked(uint32_t)
{
	clicked_ok();
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

	// Set title of the page
	m_subtitle.set_text(global_s.get_string((boost::format("campname%u") % campaign).str().c_str()));

	// Get section of campaign-maps
	std::string campsection = global_s.get_string((boost::format("campsect%u") % campaign).str().c_str());

	// Create the entry we use to load the section of the map
	uint32_t i = 0;
	std::string mapsection = campsection + (boost::format("%02i") % i).str();

	// Add all visible entries to the list.
	while (Section * const s = prof.get_section(mapsection.c_str())) {
		if (c.get_bool(mapsection.c_str())) {
			m_list.add
				(s->get_string("name", _("[No value found]")),
				 s->get_string("path"),
				 g_gr->images().get("pics/ls_wlmap.png"));
		}

		// Increase counter & mapsection
		++i;
		mapsection = campsection + (boost::format("%02i") % i).str();
	}
	if (m_list.size()) {
		m_list.select(0);
	}
}
