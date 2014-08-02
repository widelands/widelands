/*
 * Copyright (C) 2002-2004, 2006-2009, 2013 by the Widelands Development Team
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

#include "game_io/game_preload_data_packet.h"

#include <ctime>
#include <memory>

#include <boost/format.hpp>

#include "graphic/graphic.h"
#include "graphic/in_memory_image.h"
#include "graphic/render/minimaprenderer.h"
#include "graphic/surface.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/playersmanager.h"
#include "profile/profile.h"
#include "scripting/lua_table.h"
#include "scripting/scripting.h"
#include "wui/interactive_player.h"
#include "wui/mapviewpixelconstants.h"
#include "wui/mapviewpixelfunctions.h"
#include "wui/minimap.h"


namespace Widelands {

#define CURRENT_PACKET_VERSION 5
#define PLAYERS_AMOUNT_KEY_V4 "player_amount"
#define MINIMAP_FILENAME "minimap.png"


void Game_Preload_Data_Packet::Read
	(FileSystem & fs, Game &, MapMapObjectLoader * const)
{
	try {
		Profile prof;
		prof.read("preload", nullptr, fs);
		Section & s = prof.get_safe_section("global");
		int32_t const packet_version = s.get_int("packet_version");

		if (packet_version == CURRENT_PACKET_VERSION) {
			m_gametime = s.get_safe_int("gametime");
			m_mapname = s.get_safe_string("mapname");

			m_background = s.get_safe_string("background");
			m_player_nr = s.get_safe_int("player_nr");
			m_win_condition = s.get_safe_string("win_condition");
			m_number_of_players = s.get_safe_int(PLAYERS_AMOUNT_KEY_V4);
			if (fs.FileExists(MINIMAP_FILENAME)) {
				m_minimap_path = MINIMAP_FILENAME;
			}
			m_saveyear = s.get_int("saveyear");
			m_savemonth = s.get_int("savemonth");
			m_saveday = s.get_int("saveday");
			m_gametype = static_cast<GameController::GameType>(s.get_natural("gametype"));
		} else {
			throw game_data_error
				("unknown/unhandled version %i", packet_version);
		}
	} catch (const _wexception & e) {
		throw game_data_error("preload: %s", e.what());
	}
}


void Game_Preload_Data_Packet::Write
	(FileSystem & fs, Game & game, MapMapObjectSaver * const)
{

	Profile prof;
	Section & s = prof.create_section("global");

	Interactive_Player const * const ipl = game.get_ipl();

	s.set_int   ("packet_version", CURRENT_PACKET_VERSION);

	//  save some kind of header.
	s.set_int   ("gametime",       game.get_gametime());
	const Map & map = game.map();
	s.set_string("mapname",        map.get_name());  // Name of map

	if (ipl) {
		// player that saved the game.
		s.set_int("player_nr", ipl->player_number());
	} else {
		// Pretend that the first player saved the game
		for (Widelands::Player_Number p = 1; p <= map.get_nrplayers(); ++p) {
			if (game.get_player(p)) {
				s.set_int("player_nr", p);
				break;
			}
		}
	}
	s.set_int(PLAYERS_AMOUNT_KEY_V4, game.player_manager()->get_number_of_players());

	s.set_string("background", map.get_background());
	s.set_string("win_condition", game.get_win_condition_displayname());
	prof.write("preload", false, fs);

	// Write minimap image
	if (!game.is_loaded()) {
		return;
	}
	if (ipl != nullptr) {
		const MiniMapLayer flags = MiniMapLayer::Owner | MiniMapLayer::Building | MiniMapLayer::Terrain;
		const Point& vp = ipl->get_viewpoint();
		std::unique_ptr< ::StreamWrite> sw(fs.OpenStreamWrite(MINIMAP_FILENAME));
		if (sw.get() != nullptr) {
			write_minimap_image(game, &ipl->player(), vp, flags, sw.get());
			sw->Flush();
		}
	}

	time_t t;
	time(&t);
	struct tm * datetime  = localtime(&t);
	s.set_int("saveyear", 1900 + datetime->tm_year); //  years start at 1900
	s.set_int("savemonth", 1 + datetime->tm_mon); //  months start at 0
	s.set_int("saveday", datetime->tm_mday);
	s.set_int("gametype", static_cast<int32_t>(game.gameController()->getGameType()));
}


namespace  {
std::string localize_month(int8_t month) {
	switch (month) {
		case 1:
			/** TRANSLATORS: January */
			return _("Jan");
		case 2:
			/** TRANSLATORS: February */
			return _("Feb");
		case 3:
			/** TRANSLATORS: March */
			return _("Mar");
		case 4:
			/** TRANSLATORS: April */
			return _("Apr");
		case 5:
			/** TRANSLATORS: May */
			return _("May");
		case 6:
			/** TRANSLATORS: June */
			return _("Jun");
		case 7:
			/** TRANSLATORS: July */
			return _("Jul");
		case 8:
			/** TRANSLATORS: August */
			return _("Aug");
		case 9:
			/** TRANSLATORS: September */
			return _("Sep");
		case 10:
			/** TRANSLATORS: October */
			return _("Oct");
		case 11:
			/** TRANSLATORS: November */
			return _("Nov");
		case 12:
			/** TRANSLATORS: December */
			return _("Dec");
		default:
			return std::to_string(month);
	}
}
}

// NOCOM why are all the variables empty?
const std::string Game_Preload_Data_Packet::get_localized_display_title() {
	std::string result;
	if (m_saveyear > 0 && m_savemonth > 0 && m_saveday > 0) {
		result = (boost::format(_("%1$u %2$s %3$u"))
					 % static_cast<unsigned int>(m_saveyear)
					 % localize_month(m_savemonth)
					 % static_cast<unsigned int>(m_saveday)).str();
	}
	if(m_gametype == GameController::GameType::SINGLEPLAYER) {
		/** TRANSLATORS: Used in filenames for loading games */
		/** TRANSLATORS: %1% is a formatted date/time string */
		result = (boost::format(_("%1% Single Player")) % result).str();
	}
	else if(m_gametype == GameController::GameType::NETHOST) {
		/** TRANSLATORS: Used in filenames for loading games */
		/** TRANSLATORS: %1% is a formatted date/time string */
		/** TRANSLATORS: %2% is the number of the player */
		result = (boost::format(_("%1% Multiplayer (Player %2%, Host)"))
					 % result % static_cast<unsigned int>(get_player_nr())).str();
	}
	else if(m_gametype == GameController::GameType::NETCLIENT) {
		/** TRANSLATORS: Used in filenames for loading games */
		/** TRANSLATORS: %1% is a formatted date/time string */
		/** TRANSLATORS: %2% is the number of the player */
		result = (boost::format(_("%1% Multiplayer (Player %2%)"))
							% result % static_cast<unsigned int>(get_player_nr())).str();
	}
	return result;
}

}
