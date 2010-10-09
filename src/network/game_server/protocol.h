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

/**
* @file network/game_server/protocol.h
* @brief in this header file enumerations for the widelands metasever
* communication protocol are defined. This file is included by the widelands
* game and the ggz game server module.
*/

#ifndef WIDELANDS_PROTOCOL_H
#define WIDELANDS_PROTOCOL_H

#define WIDELANDS_PROTOCOL 1
#define WIDELANDS_PROTOCOL_EXT_MAJOR 1
#define WIDELANDS_PROTOCOL_EXT_MINOR 0

#define WLGGZ_OLD_OPCODE(x) ((x > 0 and x <= 7) or x == 99)
#define WLGGZ_NEW_OPCODE(x) (x > 0 and not WLGGZ_OLD_OPCODE(x))

enum WLGGZNetworkOpcodes
{
	// The first six opcodes are "old ones"

	/// Server sends greeting after client connects.
	/// One ggz string "widelands server" and one integer (1) follows this opcode
	/// @notice This opcode does bot hava a null terminated parameter list
	op_greeting = 1,
	/// @notice This opcode does bot hava a null terminated parameter list
	op_request_ip = 2,
	/// @notice This opcode does bot hava a null terminated parameter list
	op_reply_ip = 3,
	/// @notice This opcode does bot hava a null terminated parameter list
	op_broadcast_ip = 4,
	/// @notice This opcode does bot hava a null terminated parameter list
	op_state_playing = 5,
	/// @notice This opcode does bot hava a null terminated parameter list
	op_state_done = 6,

	/** This opcode is part of the new protocol. It has no parameter and
	 *  is ignored by old metaserver versions. The server replies with
	 *  @ref op_reply_protocol_ext to this opcode. The server must not use
	 *  new opcodes before getting this opcode. The client must not get new
	 *  opcodes before getting a op_reply_protocol_ext.
	 *  @notice This opcode does bot hava a null terminated parameter list
	 */
	op_request_protocol_ext = 7,

	// All following opcodes have a null terminated parameter list. This allows
	// the server to read the stream correctly without knowing the opcodes.

	/** This is the answer to the @ref op_request_protocol_ext opcode. It returns
	 *  the version of the extension protocol in two integers. First the major
	 *  then minor version. A minor version change indicates a extension of
	 *  the opcode set. A major version change indicates a change in the opcode
	 *  meaning or dropping opcodes.
	 *  @notice The parameters are send as a null terminated parameter list.
	 */
	op_reply_protocol_ext = 8,

	/**
	 * Opcode to transmit statistics and game results tu the server
	 * For each argument ther comes first integer to indicate which data
	 * it is (GameInfo) then a second integer to indicate the dataype
	 * and then the argument. A argument type of 0 indicates the end of
	 * the argument list.
	 */
	op_game_statistics = 9,

	/// Similiar to the above op_game_statistics just for GameInfo
	op_game_information = 10,

	op_set_debug = 11,
	op_debug_string = 12,

	/**
	 * This is used to transmit a part of a file. For example a map or a
	 * savegame.
	 * opcode is followed by a parameter list.
	 * one integer - file id
	 * one integer - number of packets
	 * one integer - packet number
	 * one raw field - up to 10KB file dataype
	 */
	op_file_packet = 13,
	
	/** 
	 * 
	 * one integer - file id
	 * one integer - 
	 */
	op_file_packet_recv = 14,
	
	op_send_savegame = 15,
	
	op_savegame_received = 16,

	op_unreachable = 99
};

/// Data types for the null terminated parameter list for opcodes.
enum WLGGZDataType
{
	ggzdatatype_null = 0,
	ggzdatatype_integer = 1,
	ggzdatatype_char = 2,
	ggzdatatype_string = 3,
	ggzdatatype_boolean = 4,
	ggzdatatype_list = 5,
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
	// one int: the number of the players team
	gameinfo_teamnumber = 9,
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
	gametype_tribes_together = 4,
	gametype_territorial_lord = 5,
	gametype_wood_gnome = 6
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
 * game stats: max value + history
 */

#endif
