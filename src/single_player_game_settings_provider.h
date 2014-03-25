/*
 * Copyright (C) 2014 by the Widelands Development Team
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

#ifndef SINGLE_PLAYER_GAME_SETTINGS_PROVIDE_H
#define SINGLE_PLAYER_GAME_SETTINGS_PROVIDE_H

#include "gamesettings.h"

/**
 * The settings provider for normal singleplayer games:
 * The user can change everything, except that they are themselves human.
 */
struct SinglePlayerGameSettingsProvider : public GameSettingsProvider {
	SinglePlayerGameSettingsProvider();

	virtual void setScenario(bool const set) override;

	virtual const GameSettings & settings() override;

	virtual bool canChangeMap() override;
	virtual bool canChangePlayerState(uint8_t number) override;
	virtual bool canChangePlayerTribe(uint8_t) override;
	virtual bool canChangePlayerInit (uint8_t) override;
	virtual bool canChangePlayerTeam(uint8_t) override;
	virtual bool canLaunch() override;

	virtual std::string getMap();
	virtual void setMap(const std::string & mapname, const std::string & mapfilename,
		uint32_t const maxplayers, bool const savegame) override;

	virtual void setPlayerState(uint8_t const number, PlayerSettings::State state) override;
	virtual void setPlayerAI(uint8_t const number, const std::string & ai, bool const random_ai) override;
	virtual void nextPlayerState(uint8_t const number) override;
	virtual void setPlayerTribe(uint8_t const number, const std::string & tribe, bool random_tribe) override;
	virtual void setPlayerInit(uint8_t const number, uint8_t const index) override;
	virtual void setPlayerTeam(uint8_t number, Widelands::TeamNumber team) override;
	virtual void setPlayerCloseable(uint8_t, bool) override;
	virtual void setPlayerShared(uint8_t, uint8_t) override;
	virtual void setPlayerName(uint8_t const number, const std::string & name) override;
	virtual void setPlayer(uint8_t const number, PlayerSettings const ps) override;
	virtual void setPlayerNumber(uint8_t const number) override;

	virtual std::string getWinConditionScript() override;
	virtual void setWinConditionScript(std::string wc) override;
	virtual void nextWinCondition() override;

private:
	GameSettings s;
};

#endif
