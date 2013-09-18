/*
 * Copyright (C) 2007-2010 by the Widelands Development Team
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

#include "campvis.h"

#include <cstdio>
#include <cstdlib>

#include <sys/stat.h>

#include "io/filesystem/filesystem.h"
#include "profile/profile.h"
#include "wexception.h"

/**
 * Get the path of campaign visibility save-file
 */
std::string Campaign_visibility_save::get_path()
{
	std::string savepath = "save";
	g_fs->EnsureDirectoryExists(savepath); // Make sure save directory exists
	savepath += "/campvis"; // add the name of save-file

	// check if campaigns visibility-save is available
	if (!(g_fs->FileExists(savepath)))
		make_campvis(savepath);

	// check if campaigns visibility-save is up to date
	Profile ca(savepath.c_str());

	//  1st version of campvis had no global section
	if (!ca.get_section("global"))
		update_campvis(savepath);
	else {
		Section & ca_s = ca.get_safe_section("global");
		Profile cc("campaigns/cconfig");
		Section & cc_s = cc.get_safe_section("global");
		if (cc_s.get_int("version") > ca_s.get_int("version"))
			update_campvis(savepath);
	}

	return savepath;
}


/**
 * Create the campaign visibility save-file of the user
 */
void Campaign_visibility_save::make_campvis(const std::string & savepath)
{
	// Only prepare campvis-file -> data will be written via update_campvis
	Profile campvis(savepath.c_str());
	campvis.pull_section("global");
	campvis.pull_section("campaigns");
	campvis.pull_section("campmaps");
	campvis.write(savepath.c_str(), true);

	update_campvis(savepath);
}


/**
 * Update the campaign visibility save-file of the user
 */
void Campaign_visibility_save::update_campvis(const std::string & savepath)
{
	// Variable declaration
	int32_t i = 0;
	int32_t imap = 0;
	char csection[12];
	char number[4];
	std::string mapsection;
	std::string cms;

	// Prepare cconfig and campvis
	Profile cconfig("campaigns/cconfig");
	Section & cconf_s = cconfig.get_safe_section("global");
	Profile campvisr(savepath.c_str());
	Profile campvisw(savepath.c_str());

	// Write down global section
	campvisw.pull_section("global").set_int("version", cconf_s.get_int("version", 1));

	// Write down visibility of campaigns
	Section & campv_c = campvisr.get_safe_section("campaigns");
	Section & campv_m = campvisr.get_safe_section("campmaps");
	{
		Section & vis = campvisw.pull_section("campaigns");
		sprintf(csection, "campsect%i", i);
		char cvisible[12];
		char cnewvisi[12];
		while (cconf_s.get_string(csection)) {
			sprintf(cvisible, "campvisi%i", i);
			sprintf(cnewvisi, "cnewvisi%i", i);
			bool visible = cconf_s.get_bool(cvisible) || campv_c.get_bool(csection);
			if (!visible) {
				const char * newvisi = cconf_s.get_string(cnewvisi, "");
				if (sizeof(newvisi) > 1) {
					visible = campv_m.get_bool(newvisi, false) || campv_c.get_bool(newvisi, false);
				}
			}
			vis.set_bool(csection, visible);
			++i;
			sprintf(csection, "campsect%i", i);
		}
	}

	// Write down visibility of campaign maps
	Section & vis = campvisw.pull_section("campmaps");
	i = 0;

	sprintf(csection, "campsect%i", i);
	while (cconf_s.get_string(csection)) {
		mapsection = cconf_s.get_string(csection);

		cms = mapsection;
		sprintf(number, "%02i", imap);
		cms += number;

		while (Section * const s = cconfig.get_section(cms.c_str())) {
			bool visible = s->get_bool("visible") || campv_m.get_bool(cms.c_str());
			if (!visible) {
				const char * newvisi = s->get_string("newvisi", "");
				if (sizeof(newvisi) > 1) {
					visible = campv_m.get_bool(newvisi, false) || campv_c.get_bool(newvisi, false);
				}
			}
			vis.set_bool (cms.c_str(), visible);

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
void Campaign_visibility_save::set_campaign_visibility
	(const std::string & entry, bool visible)
{
	std::string savepath = get_path();
	Profile campvis(savepath.c_str());

	campvis.pull_section("campaigns").set_bool(entry.c_str(), visible);

	campvis.write(savepath.c_str(), false);
}


/**
 * Set an campaignmap entry in campvis visible or invisible.
 * If it doesn't exist, create it.
 * \param entry entry to be changed
 * \param visible should the map be visible?
 */
void Campaign_visibility_save::set_map_visibility
	(const std::string & entry, bool visible)
{
	std::string savepath = get_path();
	Profile campvis(savepath.c_str());

	campvis.pull_section("campmaps").set_bool(entry.c_str(), visible);

	campvis.write(savepath.c_str(), false);
}
