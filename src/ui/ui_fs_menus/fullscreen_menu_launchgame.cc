/*
 * Copyright (C) 2002, 2006-2008 by the Widelands Development Team
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

#include "fullscreen_menu_launchgame.h"

#include "fullscreen_menu_mapselect.h"
#include "game.h"
#include "gamecontroller.h"
#include "gamesettings.h"
#include "i18n.h"
#include "instances.h"
#include "player.h"
#include "network.h"
#include "map.h"
#include "playerdescrgroup.h"

#include "ui_button.h"
#include "ui_textarea.h"

struct Fullscreen_Menu_LaunchGameImpl {
	GameSettingsProvider* settings;
	GameController* ctrl; // optional

	UI::Basic_Button* ok;
	UI::Textarea* mapname;
	UI::Basic_Button* select_map;
	PlayerDescriptionGroup* players[MAX_PLAYERS];

	bool is_scenario;
};


Fullscreen_Menu_LaunchGame::Fullscreen_Menu_LaunchGame
(GameSettingsProvider* settings, GameController* ctrl)
: Fullscreen_Menu_Base("launchgamemenu.jpg"), d(new Fullscreen_Menu_LaunchGameImpl)
{
	d->settings = settings;
	d->ctrl = ctrl;

	UI::Textarea* title = new UI::Textarea(this, MENU_XRES/2, 120, _("Launch Game"), Align_HCenter);
	title->set_font(UI_FONT_BIG, UI_FONT_CLR_FG);

	new UI::Button<Fullscreen_Menu_LaunchGame>
		(this,
		 550, 450, 200, 26,
		 0,
		 &Fullscreen_Menu_LaunchGame::back_clicked, this,
		 _("Back"));

	d->ok = new UI::Button<Fullscreen_Menu_LaunchGame>
		(this,
		 550, 480, 200, 26,
		 2,
		 &Fullscreen_Menu_LaunchGame::start_clicked, this,
		 _("Start game"),
		 std::string(),
		 false);

	d->mapname = new UI::Textarea(this, 650, 250, std::string(), Align_HCenter);

	d->select_map = new UI::Button<Fullscreen_Menu_LaunchGame>
		(this,
		 550, 280, 200, 26,
		 1,
		 &Fullscreen_Menu_LaunchGame::select_map, this,
		 _("Select map"),
		 std::string(),
		 false);

	d->is_scenario = false;

	int y = 250;
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i) {
		d->players[i] = new PlayerDescriptionGroup
			(this,
			 50, y,
			 settings, i);
		y += 30;
	}
}


Fullscreen_Menu_LaunchGame::~Fullscreen_Menu_LaunchGame()
{
	// sub-panels are deleted automatically by Panel destructor
	delete d;
	d = 0;
}


/**
 * Select a map as a first step in launching a game, before
 * showing the actual setup menu.
 */
void Fullscreen_Menu_LaunchGame::start()
{
	if (d->settings->settings().mapname.size() == 0 && d->settings->canChangeMap()) {
		select_map();

		if (d->settings->settings().mapname.size() == 0)
			back_clicked();
	}

	refresh();
}


void Fullscreen_Menu_LaunchGame::think()
{
	if (d->ctrl)
		d->ctrl->think();

	refresh();
}


/**
 * back-button has been pressed
 */
void Fullscreen_Menu_LaunchGame::back_clicked()
{
	end_modal(0);
}


/**
 * start-button has been pressed
 */
void Fullscreen_Menu_LaunchGame::start_clicked()
{
	if (d->settings->canLaunch())
		end_modal(d->is_scenario?2:1);
}


void Fullscreen_Menu_LaunchGame::refresh()
{
	const GameSettings& settings = d->settings->settings();

	if (settings.mapname.size() != 0)
		d->mapname->set_text(settings.mapname);
	else
		d->mapname->set_text("(no map)");

	d->ok->set_enabled(d->settings->canLaunch());
	d->select_map->set_visible(d->settings->canChangeMap());
	d->select_map->set_enabled(d->settings->canChangeMap());

	// update the player description groups
	for (uint32_t i = 0; i < MAX_PLAYERS; ++i)
		d->players[i]->refresh();
}


void Fullscreen_Menu_LaunchGame::select_map()
{
	if (!d->settings->canChangeMap())
		return;

	Fullscreen_Menu_MapSelect msm;
	int code = msm.run();

	if (code <= 0)
		return;

	d->is_scenario = code == 2;

	const MapData* map = msm.get_map();
	d->settings->setMap(map->name, map->filename, map->nrplayers);
}
