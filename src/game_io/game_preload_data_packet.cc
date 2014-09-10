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

#include "base/time_string.h"
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
			m_savehour = s.get_int("savehour");
			m_saveminute = s.get_int("saveminute");
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

	time_t t;
	time(&t);
	struct tm * datetime  = localtime(&t);
	s.set_int("saveyear", 1900 + datetime->tm_year); //  years start at 1900
	s.set_int("savemonth", 1 + datetime->tm_mon); //  months start at 0
	s.set_int("saveday", datetime->tm_mday);
	s.set_int("savehour", datetime->tm_hour);
	s.set_int("saveminute", datetime->tm_min);
	s.set_int("gametype", static_cast<int32_t>(game.gameController()->getGameType()));

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

}

// NOCOM(#gun): I do not like that this pulls in internationalization (which is a UI thing) into game_io. As much as possible try to handle i18n in the ui please.
// NOCOM(#sirver): WHere do you want it? The only common included header that would marginally make sense is logic/game_controller.h. Or a new h/cpp for this?
// base/time_string.h doesn't make sense either, because we would have a reference to game_controller.h in there, which is even more ugly.
std::string Game_Preload_Data_Packet::get_localized_display_title(std::string filename) {
	std::string result;

	if ((is_timestring(filename) || filename == "wl_autosave") && m_saveyear > 0 && m_savemonth > 0 && m_saveday > 0) {

		if (m_gametype == GameController::GameType::SINGLEPLAYER) {
			/** TRANSLATORS: Gametype used in filenames for loading games */
			result = _("Single Player");
		} else if (m_gametype == GameController::GameType::NETHOST) {
			/** TRANSLATORS: Gametype used in filenames for loading games */
			/** TRANSLATORS: %1% is the number of players */
			result = (boost::format(_("Multiplayer (%1%, Host)"))
						 % static_cast<unsigned int>(m_number_of_players)).str();
		} else if (m_gametype == GameController::GameType::NETCLIENT) {
			/** TRANSLATORS: Gametype used in filenames for loading games */
			/** TRANSLATORS: %1% is the number of players */
			result = (boost::format(_("Multiplayer (%1%)"))
								% static_cast<unsigned int>(m_number_of_players)).str();
		}
		/** TRANSLATORS: Filenames for loading games */
		/** TRANSLATORS: month day, year hour:minute gametype – mapname */
		/** TRANSLATORS: The mapname should always come last, because it can be longer than the space we have */
		result = (boost::format(_("%1$s %2$u, %3$u %4$u:%5$u %6$s – %7$s"))
					 % localize_month(m_savemonth)
					 % static_cast<unsigned int>(m_saveday)
					 % static_cast<unsigned int>(m_saveyear)
					 % static_cast<unsigned int>(m_savehour)
					 % static_cast<unsigned int>(m_saveminute)
					 % result
					 % m_mapname).str();
	} else {
		result = filename;
	}
	if (filename == "wl_autosave") {
		/** TRANSLATORS: Used in filenames for loading games */
		result = (boost::format(_("Autosave: %1%"))
							% result).str();
	}
	return result;
}

}
