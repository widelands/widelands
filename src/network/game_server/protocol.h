// Widelands server for GGZ
// Copyright (C) 2004 Josef Spillner <josef@ggzgamingzone.org>
// Copyright (C) 2009 The Widelands Development Team
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef WIDELANDS_PROTOCOL_H
#define WIDELANDS_PROTOCOL_H

#define WIDELANDS_PROTOCOL 1

enum WLGGZNetworkOpcodes
{
	op_greeting = 1,
	op_request_ip = 2,
	op_reply_ip = 3,
	op_broadcast_ip = 4,
	op_state_playing = 5,
	op_state_done = 6,

	/* Opcode to transmit statistics and game results tu the server
	 * For each argument ther comes first integer to indicate which data
	 * it is (GameInfo) then a second integer to indicate the dataype
	 * and then the argument. A argument type of 0 indicates the end of
	 * the argument list.
	 */
	op_game_statistics = 7,

	// Similiar to the above op_game_statistics just for GameInfo
	op_game_information = 8,

	op_unreachable = 99
};

enum WLGGZDataType
{
	ggzdatatype_null = 0,
	ggzdatatype_integer = 1,
	ggzdatatype_char = 2,
	ggzdatatype_string = 3,
	ggzdatatype_boolean = 4,
	ggzdatatype_raw = 99
};

enum WLGGZGameInfo
{
	// one integer, the widelands player number
	gameinfo_playerid = 1,
	// one string, the name of the player
	gameinfo_playername = 2,
	// one string, the name of the tribe
	gameinfo_tribe = 3,
	// one integer
	gameinfo_gametype = 4,
	// one string, the name of the map
	gameinfo_mapname = 5, 
	// two integers, the width then the height of the map
	gameinfo_mapsize = 6,
	// one integer
	gameinfo_playertype = 7,
	// two strings: version string, than buildtype 
	gameinfo_version = 8,
};

enum WLGGZPlayerType
{
	playertype_null = 0,
	playertype_human = 1,
	playertype_ai_aggressive = 2,
	playertype_ai_normal = 3,
	playertype_ai_defensive = 4,
	playertype_ai_none = 5,
	playertype_unknown = 99
};

enum WLGGZGameType
{
	gametype_endless = 1,
	gametype_defeatall = 2,
	gametype_collectors = 3,
	gametype_tribes_together =4
};

enum WLGGZGameStatsResult
{
	gamestatresult_null = 0,
	gamestatresult_winner = 1,
	gamestatresult_looser = 2,
	gamestatresult_leave = 3
};

enum WLGGZGameStats
{
	/* This codes can occur multiple times. The values are
	 * for the playerid which was send last. 
	 */
	gamestat_playernumber = 1, // integer
	gamestat_result = 2, // intger
	gamestat_points = 3, // integer
	gamestat_land = 4, // integer, land conquerored by player
	gamestat_buildings = 5, // integer, number of buildings theplayer has
	gamestat_milbuildingslost = 6, // integer
	gamestat_civbuildingslost = 7, // integer
	gamestat_buildingsdefeat = 8, // integer
	gamestat_milbuildingsconq = 9, // integer
	gamestat_economystrength = 10, // integer
	gamestat_militarystrength = 11, // integer
	gamestat_workers = 12, // integer
	gamestat_wares = 13, // integer
	gamestat_productivity = 14, // integer
	gamestat_casualties = 15, // integer
	gamestat_kills = 16, // integer
	gamestat_gametime = 17 // integer
};

/*ToDo:
 * game time
 * client version
 * 
 *
 */
  
#endif

