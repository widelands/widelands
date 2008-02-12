/*
 * Copyright (C) 2002-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "fullscreen_menu_campaign_select.h"
#include "campvis.h"
#include "constants.h"
#include "graphic.h"
#include "i18n.h"
#include "profile.h"


/*
 * UI 1 - Selection of Campaign
 *
 */

/**
 * CampaignSelect UI
 * Loads a list of all visible campaigns
 */
Fullscreen_Menu_CampaignSelect::Fullscreen_Menu_CampaignSelect() :
	Fullscreen_Menu_Base("choosemapmenu.jpg"),

	// Text
	title (this, MENU_XRES / 2, 90, _("Select a campaign"), Align_HCenter),

	// List of visible, available campaigns
	list(this, 15, 205, 455, 365),

	// Info fields
	label_campname   (this, 480, 205, _("Campaign:")),
	tacampname       (this, 490, 225, ""),
	label_difficulty (this, 480, 255, _("Difficulty:")),
	tadifficulty     (this, 490, 275, ""),
	label_campdescr  (this, 480, 305, _("Description:")),
	tacampdescr      (this, 490, 330, 290, 190, ""),

b_ok
(this, 490, 540, 142, 26, 2,
 &Fullscreen_Menu_CampaignSelect::clicked_ok, this,
 _("OK"), std::string(), false),

back
(this, 637, 540, 143, 26, 2,
 &Fullscreen_Menu_CampaignSelect::end_modal, this, 0,
 _("Back"))

{
	title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	list.selected.set(this, &Fullscreen_Menu_CampaignSelect::campaign_selected);
	list.double_clicked.set(this, &Fullscreen_Menu_CampaignSelect::double_clicked);
	fill_list();
}


/**
 * OK was clicked, after an entry of campaignlist got selected.
 */
void Fullscreen_Menu_CampaignSelect::clicked_ok()
{
	get_campaign();
	end_modal(1);
}

int32_t Fullscreen_Menu_CampaignSelect::get_campaign()
{
	return campaign;
}


/**
 * an entry of campaignlist got selected.
 */
void Fullscreen_Menu_CampaignSelect::campaign_selected(uint32_t i)
{
	if (list.get_selected()) { //gets false, if the selected entry has no value.
		campaign=i;

		// enable OK button
		b_ok.set_enabled(true);

		// predefine the used variables
		char cname[12];
		char cdifficulty[12];
		char cdescription[12];

		// Load maps textdomain to translate the strings from cconfig
		i18n::grab_textdomain("maps");

		// read in the campaign config
		Profile prof("campaigns/cconfig");
		Section *s;
		s = prof.get_section("global");

		// Release maps texdoamin
		i18n::release_textdomain();

		// add I to basic section name
		sprintf(cname, "campname%i", i);
		sprintf(cdifficulty, "campdiff%i", i);
		sprintf(cdescription, "campdesc%i", i);

		// Convert difficulty level to something understandable
		static const std::string dif_descriptions[] = {
			_("[No value found]"),
			_("Easy living"),
			_("Be vigilant"),
			_("Hard struggle")
		};

		uint32_t dif = s->get_int(cdifficulty);
		if (sizeof(dif_descriptions) / sizeof(*dif_descriptions) <= dif)
			dif = 0;

		// Print informations
		tacampname .set_text(s->get_string(cname,        _("[No value found]")));
		tadifficulty.set_text(dif_descriptions[dif]);
		tacampdescr.set_text(s->get_string(cdescription, _("[No value found]")));

	} else { // normally never here
		b_ok.set_enabled(false);
		tacampname  .set_text(_("[Invalid entry]"));
		tadifficulty.set_text("");
		tacampdescr .set_text("");
	}
}


/**
 * listbox got double clicked
 */
void Fullscreen_Menu_CampaignSelect::double_clicked(uint32_t)
{
	clicked_ok();
}


/**
 * fill the campaign list
 */
void Fullscreen_Menu_CampaignSelect::fill_list()
{
	// Load maps textdomain to translate the strings from cconfig
	i18n::grab_textdomain("maps");

	// Read in the campaign config
	Profile prof("campaigns/cconfig");
	Section *s;
	s = prof.get_section("global");

	// Release maps texdoamin
	i18n::release_textdomain();

	// Read in campvis-file
	Campaign_visiblity_save cvs;
	Profile campvis(cvs.get_path().c_str());
	Section *c;
	c = campvis.get_section("campaigns");

	int32_t i = 0;

	// predefine variables, used in while-loop
	char cname[12];
	char csection[12];
	char cdifficulty[12];
	std::string difficulty;

	sprintf(csection, "campsect%i", i);
	while (s->get_string(csection)) {
		// add i to the other strings the UI will search for
		sprintf(cname, "campname%i", i);
		sprintf(cdifficulty, "campdiff%i", i);

		// Only list visible campaigns
		if (c->get_bool(csection)) {

			// convert difficulty level to the fitting picture
			static const char * const dif_picture_filenames[] = {
			"pics/novalue.png",
			"pics/big.png",
			"pics/medium.png",
			"pics/small.png"
			};

			uint32_t dif = s->get_int(cdifficulty);
			if (sizeof(dif_picture_filenames) / sizeof(*dif_picture_filenames) <= dif)
				dif = 0;
			difficulty = dif_picture_filenames[dif];

			list.add
				(s->get_string(cname, _("[No value found]")),
				 s->get_string(csection),
				 g_gr->get_picture(PicMod_Game, difficulty.c_str()));

		}

		++i;

		// increase csection
		sprintf(csection, "campsect%i", i);
	} // while (s->get_string(csection))
	if (list.size())
		list.select(0);
}


/*
 * UI 2 - Selection of a map
 *
 */

/**
 * CampaignMapSelect UI
 * Loads a list of all visible maps of selected campaign and let's the user choose one.
 */
Fullscreen_Menu_CampaignMapSelect::Fullscreen_Menu_CampaignMapSelect() :
	Fullscreen_Menu_Base("choosemapmenu.jpg"),

	// Text
	title
	(this, MENU_XRES / 2, 90, _("Choose your map!"), Align_HCenter),

	// List of visible, available campaigns
	list(this, 15, 205, 455, 365),

	// Info fields
	label_mapname   (this, 480, 205, _("Name:")),
	tamapname       (this, 490, 225, ""),
	label_author    (this, 480, 255, _("Author:")),
	taauthor        (this, 490, 275, ""),
	label_mapdescr  (this, 480, 305, _("Description:")),
	tamapdescr      (this, 490, 330, 290, 190, ""),

b_ok
(this, 490, 540, 142, 26, 2,
 &Fullscreen_Menu_CampaignMapSelect::clicked_ok, this,
 _("OK"), std::string(), false),

back
(this, 637, 540, 143, 26, 2,
 &Fullscreen_Menu_CampaignMapSelect::end_modal, this, 0,
 _("Back"))

{
	title.set_font(UI_FONT_BIG, UI_FONT_CLR_FG);
	list.selected.set(this, &Fullscreen_Menu_CampaignMapSelect::map_selected);
	list.double_clicked.set(this, &Fullscreen_Menu_CampaignMapSelect::double_clicked);
}


/**
 * OK was clicked, after an entry of maplist got selected.
 */
void Fullscreen_Menu_CampaignMapSelect::clicked_ok()
{
	end_modal(1);
}


std::string Fullscreen_Menu_CampaignMapSelect::get_map()
{
	return campmapfile;
}


//telling this class what campaign we have and since we know what campaign we have, fill it.
void Fullscreen_Menu_CampaignMapSelect::set_campaign(uint32_t const i) {
	campaign = i;
	fill_list();
}

/**
 * an entry of the maplist got selected.
 */
void Fullscreen_Menu_CampaignMapSelect::map_selected(uint32_t const i) {
	if (list.get_selected()) { //gets false, if the selected entry has no value.
		// Load maps textdomain to translate the strings from cconfig
		i18n::grab_textdomain("maps");

		// read in the campaign config
		Profile prof("campaigns/cconfig");
		Section *s;
		s = prof.get_section("global");

		// Release maps texdoamin
		i18n::release_textdomain();

		// Get section of campaign-maps
		char csection[12];
		sprintf(csection, "campsect%i", campaign);
		std::string campsection = s->get_string(csection);
		std::string mapsection;
		char number[4];

		// Create the entry we use to load the section of the map
		mapsection = campsection;
		sprintf(number, "%02i", i);
		mapsection += number;

		// Load the section of the map
		s = prof.get_section(mapsection.c_str());

		// Put the path to the map into campmapfile
		campmapfile=s->get_string("path");

		// enable OK button
		b_ok.set_enabled(true);

		// Print informations
		tamapname .set_text(s->get_string("name",   _("[No value found]")));
		taauthor  .set_text(s->get_string("author", _("[No value found]")));
		tamapdescr.set_text(s->get_string("descr",  _("[No value found]")));

	} else { // normally never here
		b_ok.set_enabled(false);
		tamapname .set_text(_("[Invalid entry]"));
		taauthor  .set_text("");
		tamapdescr.set_text("");
	}
}


/**
 * listbox got double clicked
 */
void Fullscreen_Menu_CampaignMapSelect::double_clicked(uint32_t)
{
	clicked_ok();
}


/**
 * fill the campaign-map list
 */
void Fullscreen_Menu_CampaignMapSelect::fill_list()
{
	// Load maps textdomain to translate the strings from cconfig
	i18n::grab_textdomain("maps");

	// read in the campaign config
	Profile prof("campaigns/cconfig");
	Section *s;
	s = prof.get_section("global");

	// Release maps texdoamin
	i18n::release_textdomain();

	// Read in campvis-file
	Campaign_visiblity_save cvs;
	Profile campvis(cvs.get_path().c_str());
	Section *c;
	c = campvis.get_section("campmaps");

	// Set title of the page
	char cname[12];
	sprintf(cname, "campname%i", campaign);
	title.set_text(s->get_string(cname));

	// Get section of campaign-maps
	char csection[12];
	sprintf(csection, "campsect%i", campaign);
	std::string campsection = s->get_string(csection);
	std::string mapsection;
	int32_t i = 0;
	char number[4];

	// Create the entry we use to load the section of the map
	mapsection = campsection;
	sprintf(number, "%02i", i);
	mapsection += number;

	// Add all visible entries to the list.
	while ((s = prof.get_section(mapsection.c_str()))) {
		if (c->get_bool(mapsection.c_str())) {
			list.add
				(s->get_string("name", _("[No value found]")),
				 s->get_string("path"),
				 g_gr->get_picture(PicMod_Game, "pics/ls_wlmap.png"));
		}

		++i;

		// increase mapsection
		mapsection = campsection;
		sprintf(number, "%02i", i);
		mapsection += number;
	}
	if (list.size())
		list.select(0);
}
