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

#include "campvis.h"
#include "constants.h"
#include "graphic/graphic.h"
#include "i18n.h"
#include "map_io/widelands_map_loader.h"
#include "profile/profile.h"
#include "wexception.h"


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
	m_butw (get_w() / 4),
	m_buth (get_h() * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Text labels
	title
		(this,
		 get_w() / 2, get_h() * 9 / 50,
		 _("Select a campaign"), UI::Align_HCenter),
	label_campname
		(this, get_w() *  3 /   5, get_h() * 17 / 50, _("Campaign:")),
	tacampname
		(this, get_w() * 61 / 100, get_h() *  3 /  8, ""),
	label_difficulty
		(this, get_w() *  3 /   5, get_h() * 17 / 40, _("Difficulty:")),
	tadifficulty
		(this, get_w() * 61 / 100, get_h() * 23 / 50, get_w() * 9 / 25, get_h() * 3 / 50, ""),
	label_campdescr
		(this, get_w() *  3 /   5, get_h() * 17 / 32, _("Description:")),
	tacampdescr
		(this,
		 get_w() * 61 / 100, get_h() * 45 / 80, get_w() * 9 / 25, get_h() * 7 / 25,
		 ""),

// Buttons
	b_ok
		(this, "ok",
		 get_w() * 71 / 100, get_h() * 9 / 10, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("OK"), std::string(), false, false),
	back
		(this, "back",
		 get_w() * 71 / 100, get_h() * 17 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),

// Campaign list
	m_list
		(this,
		 get_w() *  47 / 2500, get_h() * 3417 / 10000,
		 get_w() * 711 / 1250, get_h() * 6083 / 10000)
{
	b_ok.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_CampaignSelect::clicked_ok, boost::ref(*this)));
	back.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_CampaignSelect::end_modal, boost::ref(*this), 0));

	back.set_font(font_small());
	b_ok.set_font(font_small());

	title           .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	label_campname  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tacampname      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	label_difficulty.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tadifficulty    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	label_campdescr .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tacampdescr     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_list.set_font(m_fn, m_fs);
	m_list.selected.connect
		(boost::bind(&Fullscreen_Menu_CampaignSelect::campaign_selected, this, _1));
	m_list.double_clicked.connect
		(boost::bind(&Fullscreen_Menu_CampaignSelect::double_clicked, this, _1));
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

/// Pictorial descriptions of difficulty levels.
static char const * const dif_picture_filenames[] = {
	"pics/novalue.png",
	"pics/easy.png",
	"pics/challenging.png",
	"pics/hard.png"
};

/**
 * an entry of campaignlist got selected.
 */
void Fullscreen_Menu_CampaignSelect::campaign_selected(uint32_t const i)
{
	if (m_list.get_selected()) { //  false if the selected entry has no value
		campaign = i;

		// enable OK button
		b_ok.set_enabled(true);

		// predefine the used variables
		char cname       [sizeof("campname4294967296")];
		char cdifficulty [sizeof("campdiff4294967296")];
		char cdif_descr[sizeof("campdiffdescr4294967296")];
		char cdescription[sizeof("campdesc4294967296")];

		Profile prof("campaigns/cconfig", nullptr, "maps");
		Section & s = prof.get_safe_section("global");

		// add I to basic section name
		sprintf(cname,        "campname%u", i);
		sprintf(cdifficulty,  "campdiff%u", i);
		sprintf(cdescription, "campdesc%u", i);
		sprintf(cdif_descr, "campdiffdescr%u", i);

		s.get_natural(cdifficulty);

		std::string dif_description = s.get_string
			(cdif_descr, _("[No value found]"));

		tacampname .set_text(s.get_string(cname,   _("[No value found]")));
		tadifficulty.set_text(dif_description);
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
	// Read in the campaign config
	Profile prof("campaigns/cconfig", nullptr, "maps");
	Section & s = prof.get_safe_section("global");

	// Read in campvis-file
	Campaign_visibility_save cvs;
	Profile campvis(cvs.get_path().c_str());
	Section & c = campvis.get_safe_section("campaigns");

	uint32_t i = 0;

	// predefine variables, used in while-loop
	char cname      [sizeof("campname4294967296")];
	char csection   [sizeof("campsect4294967296")];
	char cdifficulty[sizeof("campdiff4294967296")];

	sprintf(csection, "campsect%u", i);
	while (s.get_string(csection)) {
		// add i to the other strings the UI will search for
		sprintf(cname,       "campname%u", i);
		sprintf(cdifficulty, "campdiff%u", i);

		// Only list visible campaigns
		if (c.get_bool(csection)) {

			uint32_t dif = s.get_int(cdifficulty);
			if
				(sizeof (dif_picture_filenames)
				 /
				 sizeof(*dif_picture_filenames)
				 <=
				 dif)
				dif = 0;

			m_list.add
				(s.get_string(cname, _("[No value found]")),
				 s.get_string(csection),
				 g_gr->images().get(dif_picture_filenames[dif]));

		}

		++i;

		// increase csection
		sprintf(csection, "campsect%u", i);
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
 * Loads a list of all visible maps of selected campaign and let's the user
 * choose one.
 */
Fullscreen_Menu_CampaignMapSelect::Fullscreen_Menu_CampaignMapSelect() :
	Fullscreen_Menu_Base("choosemapmenu.jpg"),

// Values for alignment and size
	m_butw (get_w() / 4),
	m_buth (get_h() * 9 / 200),
	m_fs   (fs_small()),
	m_fn   (ui_fn()),

// Text labels
	title
		(this,
		 get_w() / 2, get_h() * 9 / 50, _("Choose a map"),
		 UI::Align_HCenter),
	label_mapname (this, get_w() * 3 / 5,  get_h() * 17 / 50, _("Name:")),
	tamapname     (this, get_w() * 61 / 100, get_h() * 3 / 8, ""),
	label_author  (this, get_w() * 3 / 5,  get_h() * 17 / 40, _("Author:")),
	taauthor      (this, get_w() * 61 / 100, get_h() * 23 / 50, ""),
	label_mapdescr(this, get_w() * 3 / 5,  get_h() * 51 / 100, _("Description:")),
	tamapdescr
		(this,
		 get_w() * 61 / 100, get_h() * 11 / 20, get_w() * 9 / 25, get_h() * 7 / 25),

// Buttons
	b_ok
		(this, "ok",
		 get_w() * 71 / 100, get_h() * 9 / 10, m_butw, m_buth,
		 g_gr->images().get("pics/but2.png"),
		 _("OK"), std::string(), false, false),
	back
		(this, "back",
		 get_w() * 71 / 100, get_h() * 17 / 20, m_butw, m_buth,
		 g_gr->images().get("pics/but0.png"),
		 _("Back"), std::string(), true, false),

// Campaign map list
	m_list
		(this,
		 get_w() *  47 / 2500, get_h() * 3417 / 10000,
		 get_w() * 711 / 1250, get_h() * 6083 / 10000)
{
	b_ok.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_CampaignMapSelect::clicked_ok, boost::ref(*this)));
	back.sigclicked.connect
		(boost::bind
			 (&Fullscreen_Menu_CampaignMapSelect::end_modal, boost::ref(*this), 0));

	b_ok.set_font(font_small());
	back.set_font(font_small());

	title         .set_font(m_fn, fs_big(), UI_FONT_CLR_FG);
	label_mapname .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tamapname     .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	label_author  .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	taauthor      .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	label_mapdescr.set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	tamapdescr    .set_font(m_fn, m_fs, UI_FONT_CLR_FG);
	m_list.set_font(m_fn, m_fs);
	m_list.selected.connect(boost::bind(&Fullscreen_Menu_CampaignMapSelect::map_selected, this, _1));
	m_list.double_clicked.connect
		(boost::bind(&Fullscreen_Menu_CampaignMapSelect::double_clicked, this, _1));
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


//telling this class what campaign we have and since we know what campaign we
//have, fill it.
void Fullscreen_Menu_CampaignMapSelect::set_campaign(uint32_t const i) {
	campaign = i;
	fill_list();
}

/**
 * an entry of the maplist got selected.
 */
void Fullscreen_Menu_CampaignMapSelect::map_selected(uint32_t) {
	campmapfile = m_list.get_selected();
	Widelands::Map map;

	std::unique_ptr<Widelands::Map_Loader> ml(map.get_correct_loader(campmapfile));
	if (!ml) {
		throw wexception
			(_("Invalid path to file in cconfig: %s"), campmapfile.c_str());
	}

	map.set_filename(campmapfile.c_str());
	ml->preload_map(true);

	i18n::Textdomain td("maps");
	tamapname .set_text(_(map.get_name()));
	taauthor  .set_text(map.get_author());
	tamapdescr.set_text(_(map.get_description()));

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
	// read in the campaign config
	Profile prof("campaigns/cconfig", nullptr, "maps");
	Section & global_s = prof.get_safe_section("global");

	// Read in campvis-file
	Campaign_visibility_save cvs;
	Profile campvis(cvs.get_path().c_str());
	Section & c = campvis.get_safe_section("campmaps");

	// Set title of the page
	char cname[sizeof("campname4294967296")];
	sprintf(cname, "campname%u", campaign);
	title.set_text(global_s.get_string(cname));

	// Get section of campaign-maps
	char csection[sizeof("campsect4294967296")];
	sprintf(csection, "campsect%u", campaign);
	std::string campsection = global_s.get_string(csection);
	std::string mapsection;
	uint32_t i = 0;
	char number[sizeof("4294967296")];

	// Create the entry we use to load the section of the map
	mapsection = campsection;
	sprintf(number, "%02u", i);
	mapsection += number;

	// Add all visible entries to the list.
	while (Section * const s = prof.get_section(mapsection.c_str())) {
		if (c.get_bool(mapsection.c_str())) {
			m_list.add
				(s->get_string("name", _("[No value found]")),
				 s->get_string("path"),
				 g_gr->images().get("pics/ls_wlmap.png"));
		}

		++i;

		// increase mapsection
		mapsection = campsection;
		sprintf(number, "%02u", i);
		mapsection += number;
	}
	if (m_list.size())
		m_list.select(0);
}
