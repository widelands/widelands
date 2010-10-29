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

#include "ui_basic/button.h"
#include "ui_basic/checkbox.h"
#include "ui_basic/scrollbar.h"
#include "ui_basic/textarea.h"

struct MultiPlayerClientGroup : public UI::Box {
	MultiPlayerClientGroup
		(UI::Panel            * const parent, uint8_t id,
		 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
		 GameSettingsProvider * const settings,
		 std::string const & fname, uint32_t const fsize)
		 :
		 UI::Box(parent, 0, 0, UI::Box::Horizontal, w, h),
		 s(settings),
		 m_id(id)
	{
		set_size(w, h);
		name = new UI::Textarea
			(this, 0, 0, w - h - UI::Scrollbar::Size * 11 / 5, h);
		name->set_font(fname, fsize, UI_FONT_CLR_FG);
		name->set_text("Test");
		add(name, 1);
		type = new UI::Callback_Button
			(this, "client_type",
			 0, 0, h, h,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 boost::bind
				 (&MultiPlayerClientGroup::toggle_type, boost::ref(*this)),
			 std::string(), std::string(), true, false, fname, fsize);
		add(type, 0);
	}

	void toggle_type() {
	}

	void toggle_ready(bool ready) {}

	UI::Textarea         * name;
	UI::Button           * type;
	GameSettingsProvider * const s;
	uint8_t                const m_id;
};

MultiPlayerSetupGroup::MultiPlayerSetupGroup
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 GameSettingsProvider * const settings,
	 uint32_t butw, uint32_t buth,
	 std::string const & fname, uint32_t const fsize)
:
UI::Panel(parent, x, y, w, h),
s(settings),
clientbox(this, 0, buth, UI::Box::Vertical, w / 3, h - buth)
{
	clientbox.set_size(w / 3, h - buth);
	clientbox.set_scrolling(true);
	for (uint32_t i = 0; i < 64; ++i) {
		c[i] =
			new MultiPlayerClientGroup
				(&clientbox, i,
				 0, 0, w / 3, buth,
				 settings, fname, fsize);
		clientbox.add(&*c[i], 1);
	}

	refresh();
}


MultiPlayerSetupGroup::~MultiPlayerSetupGroup()
{

}


/**
 * Update display and enabled buttons based on current settings.
 */
void MultiPlayerSetupGroup::refresh()
{
	//GameSettings const & settings = s->settings();

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
