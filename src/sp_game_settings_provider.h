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

#ifndef SP_GAME_SETTINGS_PROVIDE_H
#define SP_GAME_SETTINGS_PROVIDE_H

#include "gamesettings.h"
#include "logic/tribe.h"

// The settings provider for normal singleplayer games:
// The user can change everything, except that they are themselves human.
struct SinglePlayerGameSettingsProvider : public GameSettingsProvider {
	SinglePlayerGameSettingsProvider() {
		s.tribes = Widelands::Tribe_Descr::get_all_tribe_infos();
		s.scenario = false;
		s.multiplayer = false;
		s.playernum = 0;
	}

	virtual void setScenario(bool const set) {s.scenario = set;}

	virtual const GameSettings & settings() {return s;}

	virtual bool canChangeMap() {return true;}
	virtual bool canChangePlayerState(uint8_t number) {
		return (!s.scenario & (number != s.playernum));
	}
	virtual bool canChangePlayerTribe(uint8_t) {return !s.scenario;}
	virtual bool canChangePlayerInit (uint8_t) {return !s.scenario;}
	virtual bool canChangePlayerTeam(uint8_t) {return !s.scenario;}

	virtual bool canLaunch() {
		return s.mapname.size() != 0 && s.players.size() >= 1;
	}

	virtual std::string getMap() {
		return s.mapfilename;
	}

	virtual void setMap
		(const std::string &       mapname,
		 const std::string &       mapfilename,
		 uint32_t            const maxplayers,
		 bool                const savegame)
	{
		s.mapname = mapname;
		s.mapfilename = mapfilename;
		s.savegame = savegame;

		uint32_t oldplayers = s.players.size();
		s.players.resize(maxplayers);

		while (oldplayers < maxplayers) {
			PlayerSettings & player = s.players[oldplayers];
			player.state = (oldplayers == 0) ? PlayerSettings::stateHuman :
				PlayerSettings::stateComputer;
			player.tribe                = s.tribes.at(0).name;
			player.random_tribe         = false;
			player.initialization_index = 0;
			char buf[200];
			snprintf(buf, sizeof(buf), "%s %u", _("Player"), oldplayers + 1);
			player.name = buf;
			player.team = 0;
			// Set default computerplayer ai type
			if (player.state == PlayerSettings::stateComputer) {
				const Computer_Player::ImplementationVector & impls =
					Computer_Player::getImplementations();
				if (impls.size() > 1) {
					player.ai = impls.at(0)->name;
					player.random_ai = false;
				}
			}
			++oldplayers;
		}
	}

	virtual void setPlayerState
		(uint8_t const number, PlayerSettings::State state)
	{
		if (number == s.playernum || number >= s.players.size())
			return;

		if (state == PlayerSettings::stateOpen)
			state = PlayerSettings::stateComputer;

		s.players[number].state = state;
	}

	virtual void setPlayerAI(uint8_t const number, const std::string & ai, bool const random_ai) {
		if (number < s.players.size()) {
			s.players[number].ai = ai;
			s.players[number].random_ai = random_ai;
		}
	}

	virtual void nextPlayerState(uint8_t const number) {
		if (number == s.playernum || number >= s.players.size())
			return;

		const Computer_Player::ImplementationVector & impls =
			Computer_Player::getImplementations();
		if (impls.size() > 1) {
			Computer_Player::ImplementationVector::const_iterator it =
				impls.begin();
			do {
				++it;
				if ((*(it - 1))->name == s.players[number].ai)
					break;
			} while (it != impls.end());
			if (s.players[number].random_ai) {
				s.players[number].random_ai = false;
				it = impls.begin();
			} else if (it == impls.end()) {
				s.players[number].random_ai = true;
				do {
					uint8_t random = (std::rand() % impls.size()); // Choose a random AI
					it = impls.begin() + random;
				} while ((*it)->name == "None");
			}
			s.players[number].ai = (*it)->name;
		}

		s.players[number].state = PlayerSettings::stateComputer;
	}

	virtual void setPlayerTribe(uint8_t const number, const std::string & tribe, bool const random_tribe)
	{
		if (number >= s.players.size())
			return;

		std::string actual_tribe = tribe;
		PlayerSettings & player = s.players[number];
		player.random_tribe = random_tribe;

		if (random_tribe) {
			uint8_t num_tribes = s.tribes.size();
			uint8_t random = (std::rand() % num_tribes);
			actual_tribe = s.tribes.at(random).name;
		}

		container_iterate_const(std::vector<TribeBasicInfo>, s.tribes, i)
			if (i.current->name == player.tribe) {
				s.players[number].tribe = actual_tribe;
				if
					(i.current->initializations.size()
					 <=
					 player.initialization_index)
					player.initialization_index = 0;
			}
	}

	virtual void setPlayerInit(uint8_t const number, uint8_t const index) {
		if (number >= s.players.size())
			return;

		container_iterate_const(std::vector<TribeBasicInfo>, s.tribes, i)
			if (i.current->name == s.players[number].tribe) {
				if (index < i.current->initializations.size())
					s.players[number].initialization_index = index;
				return;
			}
		assert(false);
	}

	virtual void setPlayerTeam(uint8_t number, Widelands::TeamNumber team) {
		if (number < s.players.size())
			s.players[number].team = team;
	}

	virtual void setPlayerCloseable(uint8_t, bool) {
		// nothing to do
	}

	virtual void setPlayerShared(uint8_t, uint8_t) {
		// nothing to do
	}

	virtual void setPlayerName(uint8_t const number, const std::string & name) {
		if (number < s.players.size())
			s.players[number].name = name;
	}

	virtual void setPlayer(uint8_t const number, PlayerSettings const ps) {
		if (number < s.players.size())
			s.players[number] = ps;
	}

	virtual void setPlayerNumber(uint8_t const number) {
		if (number >= s.players.size())
			return;
		PlayerSettings const position = settings().players.at(number);
		PlayerSettings const player = settings().players.at(settings().playernum);
		if
			(number < settings().players.size() and
			 (position.state == PlayerSettings::stateOpen or
			  position.state == PlayerSettings::stateComputer))
		{
			setPlayer(number, player);
			setPlayer(settings().playernum, position);
			s.playernum = number;
		}
	}

	virtual std::string getWinCondition() {return s.win_condition;}
	virtual void setWinCondition(std::string wc) {s.win_condition = wc;}
	virtual void nextWinCondition() {assert(false);} // not implemented - feel free to do so, if you need it.

private:
	GameSettings s;
};

#endif
