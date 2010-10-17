/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#include "multiplayersetupgroup.h"

#include "constants.h"
#include "gamesettings.h"
#include "i18n.h"
#include "logic/game.h"
#include "logic/player.h"
#include "logic/tribe.h"
#include "profile/profile.h"
#include "wexception.h"

#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"


struct MultiPlayerSetupGroupOptions {
	GameSettingsProvider * settings;
	int32_t usernum;
};

MultiPlayerSetupGroup::MultiPlayerSetupGroup
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 GameSettingsProvider * const settings,
	 int32_t               const usernum,
	 std::string const & fname, uint32_t const fsize)
:
UI::Panel(parent, x, y, w, h),
d(new MultiPlayerSetupGroupOptions)
{
	d->settings = settings;
	d->usernum = usernum;

	refresh();
}


MultiPlayerSetupGroup::~MultiPlayerSetupGroup()
{
	delete d;
	d = 0;
}


/**
 * Update display and enabled buttons based on current settings.
 */
void MultiPlayerSetupGroup::refresh()
{
	//GameSettings const & settings = d->settings->settings();

	// Only update the user interface for the visible tab
// 	switch(tp.active()) {
// 		case 0: { // Clients
// 			break;
// 		}
// 		case 1: { // Host
// 			if (d->usernum != 0)
// 				throw wexception("Host tab active although not host!");
// 
// 			// m_ok.set_enabled(launch);
// 			break;
// 		} 
// 		default:
// 			throw wexception("Unknown tab active!");
// 	}

}
