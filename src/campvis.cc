/*
 * Copyright (C) 2007-2008 by the Widelands Development Team
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

#include "campvis.h"
#include "filesystem.h"
#include "profile.h"
#include "wexception.h"

#include <sys/stat.h>
#include <stdlib.h>

/**
 * Get the path of campaign visiblity save-file
 */
std::string Campaign_visiblity_save::get_path()
{
	std::string savepath;
	if (char const * const buf = getenv("HOME")) {
		savepath  = std::string(buf);
		savepath += "/.widelands";
		g_fs->EnsureDirectoryExists(savepath);
	}
	savepath += "/ssave";
	g_fs->EnsureDirectoryExists(savepath); // Make sure ssave directory exists
	savepath += "/campvis"; // add the name of save-file

	// check if campaigns visiblity-save is available
	if (!(g_fs->FileExists(savepath)))
		make_campvis(savepath);

	// check if campaigns visiblity-save is up to date
	Profile ca(savepath.c_str());
	if(!ca.get_section("global")) // 1st version of campvis had no global section
		update_campvis(savepath);
	else {
		Section & ca_s = ca.get_safe_section("global");
		Profile cc("campaigns/cconfig");
		Section & cc_s = cc.get_safe_section("global");
		if(cc_s.get_int("version")>ca_s.get_int("version"))
			update_campvis(savepath);
	}

	return savepath;
}


/**
 * Create the campaign visiblity save-file of the user
 */
void Campaign_visiblity_save::make_campvis(std::string savepath)
{
	// Only prepare campvis-file -> data will be written via update_campvis
	Profile campvis(savepath.c_str());
	Section *vis;
	vis=campvis.pull_section("global");
	vis=campvis.pull_section("campaigns");
	vis=campvis.pull_section("campmaps");
	campvis.write(savepath.c_str(), true);

	update_campvis(savepath);
}


/**
 * Update the campaign visiblity save-file of the user
 */
void Campaign_visiblity_save::update_campvis(std::string savepath)
{
	// Variable declaration
	int32_t i = 0;
	int32_t imap = 0;
	char csection[12];
	char cvisible[12];
	char number[4];
	std::string mapsection;
	std::string cms;

	// Prepare cconfig and campvis
	Profile cconfig("campaigns/cconfig");
	Section & cconf_s = cconfig.get_safe_section("global");
	Profile campvisr(savepath.c_str());
	Profile campvisw(savepath.c_str());
	Section *vis;

	// Write down global section
	vis = campvisw.pull_section("global");
	vis->set_int("version", cconf_s.get_int("version"), 1);

	// Write down visiblity of campaigns
	Section & campv_s = campvisr.get_safe_section("campaigns");
	vis = campvisw.pull_section("campaigns");
	sprintf(cvisible, "campvisi%i", i);
	sprintf(csection, "campsect%i", i);
	while (cconf_s.get_string(csection)) {
		vis->set_bool(csection, cconf_s.get_bool(cvisible) ||
			campv_s.get_bool(csection), false);

		++i;
		sprintf(cvisible, "campvisi%i", i);
		sprintf(csection, "campsect%i", i);
	}

	// Write down visiblity of campaign maps
	campv_s = campvisr.get_safe_section("campmaps");
	vis = campvisw.pull_section("campmaps");
	i = 0;

	sprintf(csection, "campsect%i", i);
	while (cconf_s.get_string(csection)) {
		mapsection = cconf_s.get_string(csection);

		cms = mapsection;
		sprintf(number, "%02i", imap);
		cms += number;

		while (Section * const s = cconfig.get_section(cms.c_str())) {
			vis->set_bool(cms.c_str(), s->get_bool("visible") ||
				campv_s.get_bool(cms.c_str()), false);

			++imap;
			cms = mapsection;
			sprintf(number, "%02i", imap);
			cms += number;
		}

		++i;
		sprintf(csection, "campsect%i", i);
		imap = 0;
	}
	campvisw.write(savepath.c_str(), true);
}


/**
 * Set an campaign entry in campvis visible or invisible.
 * If it doesn't exist, create it.
 * \param entry entry to be changed
 * \param visible should the map be visible?
 */
void Campaign_visiblity_save::set_campaign_visiblity(std::string entry, bool visible)
{
	std::string savepath = get_path();
	Profile campvis(savepath.c_str());
	Section *vis;

	vis=campvis.pull_section("campaigns");
	vis->set_bool(entry.c_str(), visible);

	campvis.write(savepath.c_str(), false);
}


/**
 * Set an campaignmap entry in campvis visible or invisible.
 * If it doesn't exist, create it.
 * \param entry entry to be changed
 * \param visible should the map be visible?
 */
void Campaign_visiblity_save::set_map_visiblity(std::string entry, bool visible)
{
	std::string savepath = get_path();
	Profile campvis(savepath.c_str());
	Section *vis;

	vis=campvis.pull_section("campmaps");
	vis->set_bool(entry.c_str(), visible);

	campvis.write(savepath.c_str(), false);
}
