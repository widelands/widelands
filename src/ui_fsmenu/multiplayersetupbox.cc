/*
 * Copyright (C) 2002, 2006-2010 by the Widelands Development Team
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

#include "multiplayersetupbox.h"

#include "constants.h"
#include "gamesettings.h"
#include "i18n.h"
#include "logic/player.h"
#include "profile/profile.h"
#include "logic/tribe.h"
#include "wexception.h"

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"


struct MultiPlayerSetupBoxOptions {
	GameSettingsProvider * settings;
	uint32_t clientnr;
};

MultiPlayerSetupBox::MultiPlayerSetupBox
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 GameSettingsProvider * const settings,
	 uint32_t               const clientnr,
	 std::string const & fname, uint32_t const fsize)
:
UI::Panel(parent, x, y, w, h),
d(new MultiPlayerSetupBoxOptions)
{
	d->settings = settings;
	d->clientnr = clientnr;

	refresh();
}


MultiPlayerSetupBox::~MultiPlayerSetupBox()
{
	delete d;
	d = 0;
}


/**
 * Update display and enabled buttons based on current settings.
 */
void MultiPlayerSetupBox::refresh()
{
	//GameSettings const & settings = d->settings->settings();
}

