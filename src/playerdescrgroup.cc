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

#include "playerdescrgroup.h"

#include "constants.h"
#include "gamesettings.h"
#include "i18n.h"
#include "player.h"
#include "profile.h"
#include "tribe.h"
#include "wexception.h"

#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_textarea.h"


struct PlayerDescriptionGroupImpl {
	GameSettingsProvider* settings;
	uint32_t plnum;

	UI::Textarea* plr_name;
	UI::Checkbox* btnEnablePlayer;
	UI::Basic_Button* btnPlayerType;
	UI::Basic_Button* btnPlayerTribe;
	UI::Basic_Button* btnPlayerInit;
};

PlayerDescriptionGroup::PlayerDescriptionGroup
	(UI::Panel            * const parent,
	 int32_t const x, int32_t const y, int32_t const w, int32_t const h,
	 GameSettingsProvider * const settings,
	 uint32_t               const plnum,
	 std::string const & fname, uint32_t const fsize)
:
UI::Panel(parent, x, y, w, h),
d(new PlayerDescriptionGroupImpl)
{
	d->settings = settings;
	d->plnum = plnum;

	d->plr_name = new UI::Textarea
		(this, 0, 0, w * 11 / 50, h, std::string(), Align_Left);
	d->plr_name->set_font(fname, fsize, UI_FONT_CLR_FG);
	d->btnEnablePlayer = new UI::Checkbox(this, Point(w / 5, 0));
	d->btnEnablePlayer->changedto.set
		(this, &PlayerDescriptionGroup::enable_player);
	d->btnPlayerType = new UI::Button<PlayerDescriptionGroup>
		(this,
		 w * 29 / 125, 0, w * 38 / 200, h,
		 1,
		 &PlayerDescriptionGroup::toggle_playertype, this,
		 "",
		 std::string(),
		 true, false,
		 fname, fsize);
	d->btnPlayerTribe = new UI::Button<PlayerDescriptionGroup>
		(this,
		 w * 43 / 100, 0, w * 48 / 200, h,
		 1,
		 &PlayerDescriptionGroup::toggle_playertribe, this,
		 "",
		 std::string(),
		 true, false,
		 fname, fsize);
	d->btnPlayerInit = new UI::Button<PlayerDescriptionGroup>
		(this,
		 w * 135 / 200, 0, w * 64 / 200, h,
		 1,
		 &PlayerDescriptionGroup::toggle_playerinit, this,
		 "",
		 _("Initialization"),
		 true, false,
		 fname, fsize);

	refresh();
}


PlayerDescriptionGroup::~PlayerDescriptionGroup()
{
	delete d;
	d = 0;
}


/**
 * Update display and enabled buttons based on current settings.
 */
void PlayerDescriptionGroup::refresh()
{
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size()) {
		set_visible(false);
		return;
	}

	set_visible(true);

	const PlayerSettings& player = settings.players[d->plnum];
	bool stateaccess = d->settings->canChangePlayerState(d->plnum);
	bool tribeaccess = d->settings->canChangePlayerTribe(d->plnum);
	bool const initaccess  = d->settings->canChangePlayerInit(d->plnum);
	log("PlayerDescriptionGroup::refresh: initaccess = %u\n", initaccess);

	d->btnEnablePlayer->set_enabled(stateaccess);

	if (player.state == PlayerSettings::stateClosed) {
		d->btnEnablePlayer->set_state(false);
		d->btnPlayerType->set_visible(false);
		d->btnPlayerType->set_enabled(false);
		d->btnPlayerTribe->set_visible(false);
		d->btnPlayerTribe->set_enabled(false);
		d->btnPlayerInit->set_visible(false);
		d->btnPlayerInit->set_enabled(false);
		d->plr_name->set_text(std::string());
	} else {
		d->btnEnablePlayer->set_state(true);
		d->btnPlayerType->set_visible(true);
		d->btnPlayerType->set_enabled(stateaccess);

		if (player.state == PlayerSettings::stateOpen) {
			d->btnPlayerType->set_title(_("Open"));
			d->btnPlayerTribe->set_visible(false);
			d->btnPlayerInit ->set_visible(false);
			d->btnPlayerTribe->set_enabled(false);
			d->btnPlayerInit ->set_enabled(false);
			d->plr_name->set_text(std::string());
		} else {
			std::string title;

			if (player.state == PlayerSettings::stateComputer) {
				if (player.ai.size() == 0)
					title = _("Computer");
				else
					title = _("AI: ") + player.ai;
			} else
				title = _("Human");
			// get translated tribesname
			std::string tribepath("tribes/" + player.tribe);
			i18n::grab_textdomain(tribepath);
			Profile prof((tribepath + "/conf").c_str());
			Section & global = prof.get_safe_section("tribe");
			std::string tribe(global.get_safe_string("name"));
			i18n::release_textdomain();

			d->btnPlayerType->set_title(title);
			d->btnPlayerTribe->set_title(tribe);
			for
				(struct {
				 	std::vector<TribeBasicInfo>::const_iterator       current;
				 	std::vector<TribeBasicInfo>::const_iterator const end;
				 } i = {settings.tribes.begin(), settings.tribes.end()};;
				 ++i.current)
			{
				assert(i.current < i.end);
				if (i.current->name == player.tribe) {
					d->btnPlayerInit->set_title
						(i.current->initializations.at(player.initialization_index)
						 .second);
					break;
				}
			}
			d->plr_name->set_text(player.name);

			d->btnPlayerTribe->set_visible(true);
			d->btnPlayerInit ->set_visible(true);
			d->btnPlayerTribe->set_enabled(tribeaccess);
			d->btnPlayerInit ->set_enabled(initaccess);
			log("called btnPlayerInit(%u)\n", initaccess);
		}
	}
}


/**
 * Enables/Disables a playerposition completely
 * This is useful for multi player savegames, where specific player positions
 * were not used in the saved game.
 */
void PlayerDescriptionGroup::enable_pdg(bool enable)
{
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	enable_player(enable);
	d->btnEnablePlayer->set_enabled(enable);
	d->btnEnablePlayer->set_visible(enable);
	d->btnPlayerType  ->set_enabled(enable);
	d->btnPlayerType  ->set_visible(enable);
	d->btnPlayerTribe ->set_visible(enable);
	d->btnPlayerTribe ->set_enabled(enable);
	d->btnPlayerInit  ->set_visible(enable);
	d->btnPlayerInit  ->set_enabled(enable);
	if (!enable)
		d->plr_name->set_text(std::string());
}


/**
 * Show/hide player's tribe button
 * This is useful for multi player savegames, as the tribes can't be changed
 */
void PlayerDescriptionGroup::show_tribe_button(bool show)
{
	d->btnPlayerTribe ->set_visible(show);
	d->btnPlayerTribe ->set_enabled(show);
}


/**
 * The checkbox to open/close a player position has been pressed.
 */
void PlayerDescriptionGroup::enable_player(bool on)
{
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	if (on) {
		if (settings.players[d->plnum].state == PlayerSettings::stateClosed)
			d->settings->nextPlayerState(d->plnum);
	} else {
		if (settings.players[d->plnum].state != PlayerSettings::stateClosed)
			d->settings->setPlayerState(d->plnum, PlayerSettings::stateClosed);
	}
}


void PlayerDescriptionGroup::toggle_playertype()
{
	d->settings->nextPlayerState(d->plnum);
}

/**
 * Cycle through available tribes for the player.
 */
void PlayerDescriptionGroup::toggle_playertribe()
{
	const GameSettings& settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	const std::string& currenttribe = settings.players[d->plnum].tribe;
	std::string nexttribe = settings.tribes.at(0).name;

	for (uint32_t i = 0; i < settings.tribes.size()-1; ++i) {
		if (settings.tribes[i].name == currenttribe) {
			nexttribe = settings.tribes.at(i + 1).name;
			break;
		}
	}

	d->settings->setPlayerTribe(d->plnum, nexttribe);
}


/// Cycle through available initializations for the player's tribe.
void PlayerDescriptionGroup::toggle_playerinit()
{
	GameSettings const & settings = d->settings->settings();

	if (d->plnum >= settings.players.size())
		return;

	PlayerSettings const & player = settings.players[d->plnum];
	container_iterate_const(std::vector<TribeBasicInfo>, settings.tribes, j)
		if (j.current->name == player.tribe)
			return
				d->settings->setPlayerInit
					(d->plnum,
					 (player.initialization_index + 1)
					 %
					 j.current->initializations.size());
	assert(false);
}
