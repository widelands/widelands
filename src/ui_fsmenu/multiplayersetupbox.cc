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

#include "ui_basic/checkbox.h"
#include "ui_basic/textarea.h"


struct MultiPlayerSetupBoxOptions {
	GameSettingsProvider * settings;
	uint32_t usernum;
};

struct MultiPlayerClientGroup {
	MultiPlayerSetupBoxOptions * d;
};

struct PlayerSettingGroup {
	MultiPlayerSetupBoxOptions * d;
};

MultiPlayerSetupBox::MultiPlayerSetupBox
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 GameSettingsProvider * const settings,
	 uint32_t               const usernum,
	 uint32_t butw, uint32_t buth,
	 std::string const & fname, uint32_t const fsize)
:
UI::Panel(parent, x, y, w, h),
d(new MultiPlayerSetupBoxOptions),
tp(this, 0, 0, w, h, g_gr->get_picture(PicMod_UI, "pics/but1.png"))
{
	d->settings = settings;
	d->usernum = usernum;

	// Add the tabs
	tp.set_size(get_w(), get_h());
	UI::Panel * tab1 = new UI::Panel(&tp, 0, 0, w, h - 38);
	UI::Textarea * t = new UI::Textarea(tab1, 20, 10, 50, 20);
	t->set_text("Test");
	uint32_t id = tp.add
		(_("Clients"), g_gr->get_picture(PicMod_UI, "pics/menu_list_workers.png"),
		 tab1, _("List of Connected clients and their settings"));
	UI::Panel * tab2 = new UI::Panel(&tp, 0, 0, w, h - 38);
	tp.add
		(_("Game Settings"),
		 g_gr->get_picture(PicMod_UI, "pics/editor_menu_toggle_tool_menu.png"),
		 tab2, _("The settings for the game"));
	if (usernum == 0) { // == host
		UI::Panel * tab3 = new UI::Panel(&tp, 0, 0, w, h - 38);
		uint32_t x = 15;
		uint32_t y = h / 7;
		m_select_map = new UI::Callback_Button<MultiPlayerSetupBox>
			(tab3, "select_map",
			 x, y, butw, buth,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 &MultiPlayerSetupBox::select_map, *this,
			 _("Select map"), std::string(), false, false,
			 fname, fsize);
		y += buth * 12 / 10;
		m_select_save = new UI::Callback_Button<MultiPlayerSetupBox>
			(tab3, "select_savegame",
			 x, y, butw, buth,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 &MultiPlayerSetupBox::select_savegame, *this,
			 _("Select Savegame"), std::string(), false, false,
			 fname, fsize);
		y += buth * 22 / 10;
		m_ok = new UI::Callback_Button<MultiPlayerSetupBox>
			(tab3, "ok",
			 x, y, butw, buth,
			 g_gr->get_picture(PicMod_UI, "pics/but1.png"),
			 &MultiPlayerSetupBox::start_clicked, *this,
			 _("Start game"), std::string(), false, false,
			 fname, fsize);
		id = tp.add
			(_("Host"), g_gr->get_picture(PicMod_UI, "pics/continue.png"),
			tab3, _("Host commands"));
	}

	tp.activate(id);
	tp.set_visible(true);
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

	// Only update the user interface for the visible tab
	switch(tp.active()) {
		case 0: { // Clients
			break;
		}
		case 1: { // Settings
			break;
		} 
		case 2: { // Host
			if (d->usernum != 0)
				throw wexception("Host tab active although not host!");

			// m_ok.set_enabled(launch);
			m_select_map ->set_visible(d->settings->canChangeMap());
			m_select_map ->set_enabled(d->settings->canChangeMap());
			m_select_save->set_visible(d->settings->canChangeMap());
			m_select_save->set_enabled(d->settings->canChangeMap());
			break;
		} 
		default:
			throw wexception("Unknown tab active!");
	}

}

void MultiPlayerSetupBox::select_map() {
}
void MultiPlayerSetupBox::select_savegame() {
}
void MultiPlayerSetupBox::start_clicked() {
}

