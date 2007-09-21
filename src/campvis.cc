/*
 * Copyright (C) 2007 by the Widelands Development Team
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

/**
 * Get the path of campaign visiblity save-file
 */
std::string Campaign_visiblity_save::get_path()
{
	std::string savepath("ssave");

#ifndef __WIN32__
	char *buf=getenv("HOME");

	if (buf) {
		savepath = std::string(buf) + "/.widelands/ssave";
		g_fs->EnsureDirectoryExists(savepath);
	} else {
		// If the user has NO homedirectory (singleusers-system) but can
		// start Widelands, (s)he/it hopefully has write-access to the
		// Widelands-directory, so we keep the path at [widelands]/ssave.
	}
#else
	// Until now, Widelands doesn't use home directories of the users under
	// Windows and as the Widelands-path is always writeable under Windows,
	// we just keep the path at [widelands]/ssave.
#endif

	savepath += "/campvis";

	// check if campaigns visiblity-save is available
	if (!(g_fs->FileExists(savepath))){
		make_campvis(savepath);
	}

	return savepath;
}


/**
 * Create the campaign visiblity save-file of the user
 */
void Campaign_visiblity_save::make_campvis(std::string savepath)
{
	int i = 0;
	int imap = 0;
	char csection[12];
	char cvisible[12];
	char number[4];
	std::string mapsection;
	std::string cms;

	// read in the campaign config
	Profile prof("campaigns/cconfig");
	Section *cconf;
	cconf = prof.get_section("global");

	// Write down visiblity of campaigns
	Profile campvis(savepath.c_str());
	Section *vis;
	vis=campvis.pull_section("campaigns");

	sprintf(cvisible, "campvisi%i", i);
	sprintf(csection, "campsect%i", i);
	while (cconf->get_string(csection)) {
		vis->set_bool(csection, cconf->get_bool(cvisible), "0");

		i++;
		sprintf(cvisible, "campvisi%i", i);
		sprintf(csection, "campsect%i", i);
	}

	// Write down visiblity of campaign maps
	vis = campvis.pull_section("campmaps");
	i = 0;

	sprintf(csection, "campsect%i", i);
	while (cconf->get_string(csection)) {
		mapsection = cconf->get_string(csection);

		cms = mapsection;
		sprintf(number, "%02i", imap);
		cms += number;

		while ((cconf = prof.get_section(cms.c_str()))) {
			vis->set_bool(cms.c_str(), cconf->get_bool("visible"), "0");

			imap++;
			cms = mapsection;
			sprintf(number, "%02i", imap);
			cms += number;
		}

		i++;
		sprintf(csection, "campsect%i", i);
		cconf = prof.get_section("global");
		imap = 0;
	}

	campvis.write(savepath.c_str(), false);
}


/**
 * Set an entry in campvis visible or invisible.
 * If it doesn't exist, create it.
 * \param entry entry to be changed
 * \param vcase what kind of change is this
 *   0 = make campaign visible
 *   1 = make campaign invisible
 *   2 = make campaign map visible
 *   3 = make campaign map invisible
 */
void Campaign_visiblity_save::set_visiblity(std::string entry, uint32_t vcase)
{
	std::string savepath = get_path();
	Profile campvis(savepath.c_str());
	Section *vis;

	if(vcase <= 1) {
		vis=campvis.pull_section("campaigns");

		if(vcase == 0) {
			vis->set_bool(entry.c_str(), true);
		}
		if(vcase == 1) {
			vis->set_bool(entry.c_str(), false);
		}
	}

	if(vcase >= 2) {
		vis=campvis.pull_section("campmaps");

		if(vcase == 2) {
			vis->set_bool(entry.c_str(), true);
		}
		if(vcase == 3) {
			vis->set_bool(entry.c_str(), false);

		}
	}

	if(vcase >= 4) {
		throw wexception("Wrong vcase (%i) for entry \"%s\" of campvis", vcase, entry.c_str());
	}

	campvis.write(savepath.c_str(), false);
}
