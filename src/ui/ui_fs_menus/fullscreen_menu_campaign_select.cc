/*
 * Copyright (C) 2002-2009 by the Widelands Development Team
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
#include "wexception.h"
#include "widelands_map_loader.h"


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

// Values for alignment and size
m_butw
	(m_xres / 4),
m_buth
	(m_yres * 9 / 200),
m_fs
	(fs_small()),
m_fn
	(ui_fn()),

// Text labels
title
	(this, m_xres / 2, m_yres * 9 / 50, _("Select a campaign"), Align_HCenter),
label_campname   (this, m_xres * 3 / 5,  m_yres * 17 / 50, _("Campaign:")),
tacampname       (this, m_xres * 61 / 100, m_yres * 3 / 8, ""),
label_difficulty (this, m_xres * 3 / 5,  m_yres * 17 / 40, _("Difficulty:")),
tadifficulty     (this, m_xres * 61 / 100, m_yres * 23 / 50, ""),
label_campdescr  (this, m_xres * 3 / 5,  m_yres * 51 / 100, _("Description:")),
tacampdescr      (this, m_xres * 61 / 100, m_yres * 11 / 20, m_xres * 9 / 25, m_yres * 7 / 25, ""),

// Buttons
b_ok
	(this,
	 m_xres * 71 / 100, m_yres * 9 / 10, m_butw, m_buth,
	 2,
	 &Fullscreen_Menu_CampaignSelect::clicked_ok, this,
	 _("OK"), std::string(), false, false,
	 m_fn, m_fs),
back
	(this,
	 m_xres * 71 / 100, m_yres * 17 / 20, m_butw, m_buth,
	 0,
	 &Fullscreen_Menu_CampaignSelect::end_modal, this, 0,
	 _("Back"), std::string(), true, false,
	 m_fn, m_fs),

// Campaign list
m_list(this, m_xres * 47 / 2500, m_yres * 3417 / 10000, m_xres * 711 / 1250, m_yres * 6083 / 10000)
{
	title           .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	label_campname  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tacampname      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	label_difficulty.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tadifficulty    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	label_campdescr .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tacampdescr     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_list.set_font(m_fn, m_fs);
	m_list.selected.set(this, &Fullscreen_Menu_CampaignSelect::campaign_selected);
	m_list.double_clicked.set(this, &Fullscreen_Menu_CampaignSelect::double_clicked);
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


/// Textual descriptions of difficulty levels.
static char const * const dif_descriptions[] = {
	_("[No value found]"),
	_("Easy living"),
	_("Be vigilant"),
	_("Hard struggle")
};

/**
 * an entry of campaignlist got selected.
 */
void Fullscreen_Menu_CampaignSelect::campaign_selected(uint32_t const i)
{
	if (m_list.get_selected()) { //gets false, if the selected entry has no value.
		campaign = i;

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
		Section & s = prof.get_safe_section("global");

		// Release maps texdoamin
		i18n::release_textdomain();

		// add I to basic section name
		sprintf(cname, "campname%i", i);
		sprintf(cdifficulty, "campdiff%i", i);
		sprintf(cdescription, "campdesc%i", i);

		uint32_t dif = s.get_int(cdifficulty);
		if (sizeof(dif_descriptions) / sizeof(*dif_descriptions) <= dif)
			dif = 0;

		tacampname .set_text(s.get_string(cname,        _("[No value found]")));
		tadifficulty.set_text(i18n::translate(dif_descriptions[dif]));
		tacampdescr.set_text(s.get_string(cdescription, _("[No value found]")));

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
	Section & s = prof.get_safe_section("global");

	// Release maps texdoamin
	i18n::release_textdomain();

	// Read in campvis-file
	Campaign_visibility_save cvs;
	Profile campvis(cvs.get_path().c_str());
	Section & c = campvis.get_safe_section("campaigns");

	int32_t i = 0;

	// predefine variables, used in while-loop
	char cname[12];
	char csection[12];
	char cdifficulty[12];
	std::string difficulty;

	sprintf(csection, "campsect%i", i);
	while (s.get_string(csection)) {
		// add i to the other strings the UI will search for
		sprintf(cname, "campname%i", i);
		sprintf(cdifficulty, "campdiff%i", i);

		// Only list visible campaigns
		if (c.get_bool(csection)) {

			// convert difficulty level to the fitting picture
			static const char * const dif_picture_filenames[] = {
			"pics/novalue.png",
			"pics/big.png",
			"pics/medium.png",
			"pics/small.png"
			};

			uint32_t dif = s.get_int(cdifficulty);
			if (sizeof(dif_picture_filenames) / sizeof(*dif_picture_filenames) <= dif)
				dif = 0;
			difficulty = dif_picture_filenames[dif];

			m_list.add
				(s.get_string(cname, _("[No value found]")),
				 s.get_string(csection),
				 g_gr->get_picture(PicMod_Game, difficulty.c_str()));

		}

		++i;

		// increase csection
		sprintf(csection, "campsect%i", i);
	} // while (s->get_string(csection))
	if (m_list.size())
		m_list.select(0);
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

// Values for alignment and size
m_xres
	(gr_x()),
m_yres
	(gr_y()),
m_butw
	(m_xres / 4),
m_buth
	(m_yres * 9 / 200),
m_fs
	(fs_small()),
m_fn
	(ui_fn()),

// Text labels
title(this, m_xres / 2, m_yres * 9 / 50, _("Choose your map!"), Align_HCenter),
label_mapname   (this, m_xres * 3 / 5,  m_yres * 17 / 50, _("Name:")),
tamapname       (this, m_xres * 61 / 100, m_yres * 3 / 8, ""),
label_author    (this, m_xres * 3 / 5,  m_yres * 17 / 40, _("Author:")),
taauthor        (this, m_xres * 61 / 100, m_yres * 23 / 50, ""),
label_mapdescr  (this, m_xres * 3 / 5,  m_yres * 51 / 100, _("Description:")),
tamapdescr
	(this,
	 m_xres * 61 / 100, m_yres * 11 / 20, m_xres * 9 / 25, m_yres * 7 / 25,
	 ""),

// Buttons
b_ok
	(this,
	 m_xres * 71 / 100, m_yres * 9 / 10, m_butw, m_buth,
	 2,
	 &Fullscreen_Menu_CampaignMapSelect::clicked_ok, this,
	 _("OK"), std::string(), false, false,
	 m_fn, m_fs),
back
	(this,
	 m_xres * 71 / 100, m_yres * 17 / 20, m_butw, m_buth,
	 0,
	 &Fullscreen_Menu_CampaignMapSelect::end_modal, this, 0,
	 _("Back"), std::string(), true, false,
	 m_fn, m_fs),

// Campaign map list
m_list(this, m_xres * 47 / 2500, m_yres * 3417 / 10000, m_xres * 711 / 1250, m_yres * 6083 / 10000)
{
	title         .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	label_mapname .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tamapname     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	label_author  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	taauthor      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	label_mapdescr.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tamapdescr    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_list.set_font(m_fn, m_fs);
	m_list.selected.set(this, &Fullscreen_Menu_CampaignMapSelect::map_selected);
	m_list.double_clicked.set(this, &Fullscreen_Menu_CampaignMapSelect::double_clicked);
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
void Fullscreen_Menu_CampaignMapSelect::map_selected(uint32_t) {
	campmapfile = m_list.get_selected();

	// Load the maps textdomain to translate the strings from map
	i18n::Textdomain td(campmapfile.c_str());

	Widelands::Map map;

	Widelands::Map_Loader *const ml
		= map.get_correct_loader(campmapfile.c_str());
	if (!ml) {
		i18n::release_textdomain();
		throw wexception
			(_("Invalid path to file in cconfig: %s"), campmapfile.c_str());
	}

	map.set_filename(campmapfile.c_str());
	ml->preload_map(true);

	tamapname .set_text(map.get_name());
	taauthor  .set_text(map.get_author());
	tamapdescr.set_text(map.get_description());

	// enable OK button
	b_ok.set_enabled(true);
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
	Section & global_s = prof.get_safe_section("global");

	// Release maps textdomain
	i18n::release_textdomain();

	// Read in campvis-file
	Campaign_visibility_save cvs;
	Profile campvis(cvs.get_path().c_str());
	Section & c = campvis.get_safe_section("campmaps");

	// Set title of the page
	char cname[12];
	sprintf(cname, "campname%i", campaign);
	title.set_text(global_s.get_string(cname));

	// Get section of campaign-maps
	char csection[12];
	sprintf(csection, "campsect%i", campaign);
	std::string campsection = global_s.get_string(csection);
	std::string mapsection;
	int32_t i = 0;
	char number[4];

	// Create the entry we use to load the section of the map
	mapsection = campsection;
	sprintf(number, "%02i", i);
	mapsection += number;

	// Add all visible entries to the list.
	while (Section * const s = prof.get_section(mapsection.c_str())) {
		if (c.get_bool(mapsection.c_str())) {
			m_list.add
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
	if (m_list.size())
		m_list.select(0);
}
