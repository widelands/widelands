/*
 * Copyright (C) 2002-2004, 2008 by the Widelands Development Team
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

#ifndef GAME_SERVER_PROTO_H
#define GAME_SERVER_PROTO_H

#include <stdint.h>

/*
 * This file delivers all informations needed to understand
 * and implement the general game server protocol.
 *
 * The protocol consist of packages. Each package passes four
 * stages:
 *   1) senden by client or server, containing data of what to do
 *   2) executet by receiver
 *   3) receiver sends answer package
 *   4) sender executes answer package
 * Each package consits at least of the following data
 *
 *  <2B packetsize><2B  id><2B INDEX><2B FLAGS><... packet data...>
 *                    a)      b)
 * The id defines which packet follows, the index is a simple index increasing with each
 * send packet. The Client starts with id 0, the server with id 0xffff + 1. Each wrap
 * before they get into the other ones id space.
 * Flags are currently only if this is a reply to a packet.
 *
 * When a client connects to
 * a server it sends a hello package. This package also contains the ability of the client
 * (see the hello package source for more informations)
 *
 * All strings are unicode!!
 */

/*
 * First the package ids
 *
 * (General Game Server Protocol Packet)
 */
enum {
	GGSPP_CONNECT       =  1,
	GGSPP_HELLO         =  2,
	GGSPP_USERENTERED   =  3,
	GGSPP_GETROOMINFO   =  4,
	GGSPP_GETUSERINFO   =  5,

	GGSPP_CHATMESSAGE   =  6,
	GGSPP_PING          =  7,




	GGSPP_GAMELIST      =  8,

	GGSPP_GAMESTARTED   =  9,
	GGSPP_GAMEENDED     = 10,

	GGSPP_SERVERMESSAGE = 11,

};

/*
 * The indexs
 */
const static uint32_t FIRST_CLIENT_PACKET_INDEX = 0;
const static uint32_t LAST_CLIENT_PACKET_INDEX = 0xffff - 1;
const static uint32_t FIRST_SERVER_PACKET_INDEX = 0xffff0001;
const static uint32_t LAST_SERVER_PACKET_INDEX = 0xffffffff - 1;

/*
 * Standart port
 */
const static uint16_t GAME_SERVER_PORT = 8128;

/*
 * Flags
 */
enum GSP_Flags {
	GSP_ANSWER = 1
};

/*
 * Flag decoding
 */
#define IS_ANSWER(flag) ((flag) & GSP_ANSWER)

/*
 * Protocol version: currently 1.0
 */
const static uint8_t GSP_MAJOR_VERSION = 1;
const static uint8_t GSP_MINOR_VERSION = 0;

/*
 * Connect package
 *
 * Request (Client):
 *  <2B id><2B index><2B FLAGS> <2B Version> <Str Game Name>
 * Answer (Server):
 *  <2B id> <2B index> <2B FLAGS> <1B Answer> <2B Server Protocol Version>
 */
enum CP_Answer {
	WELCOME,
	PROTOCOL_TO_OLD,
	SERVER_FULL,
	GAME_NOT_SERVED
};

/*
 * HELLO PACKET
 *
 * Request (Client):
 *  <2B id><2B index><2B FLAGS><1B Feature List><User Name>
 * Answer (Server):
 *  <2B id><2B index><2B FLAGS><1B return code><User Name to use><User Group><Initial room><Welcome message (MOTD)>
 *   if the return code is a failure, the other data might not contain
 *   really usefull data.
 */
enum HP_Features {
	FEATURES_NONE        = 0,
	FEATURES_FONT_FORMAT = 1,  // This client has font formating support (colors, size)
};
enum HP_RetVal {
	USER_WELCOME = 1, //  you are on board
};

/*
 * USER ENTERED a room packet
 *
 * Request (Server):
 *  <2B id><2B index><2B FLags><Username><Room it enters><1b enters or leaves>
 * Answer (Client):
 *  <2B id><2B index><2B FLags><1B ack = 1>
 */
const uint8_t UEP_ACK = 1;

/*
 * ROOM INFO : send informations about the current room.
 * Request (Client):
 *  <2B id><2B index><2B Flags><Room name>
 * Answer (Server):
 *  <2B id><2B index><2B Flags><Answer Flags><2B nr_users><nr_users* Usernames>
 */
enum RI_AnswerFlags {
	RI_ACK = 1,
	RI_NONEXISTANT
};

/*
 * USER INFO : send informations about a looged in user.
 * Request (Client):
 *  <2B id><2B index><2B Flags><User name>
 * Answer (Server):
 *  <2B id><2B index><2B Flags><Answer Flags><Game Name><Room Name>
 */
enum UI_AnswerFlags {
	UI_ACK = 1,
	UI_UNKNOWN
};

/*
 * CHAT MESSAGE: a simple chaat message
 * Request (Both):
 *    <2B id><2B index><2B Flags><Flags><User><Msg>
 * Answer (Both):
 *    <2B id><2B index><1B ACK>
 */
enum CM_Flags {
	CM_NONE,
	CM_ACTION = 1
};
const uint8_t CM_ACK = 1;

/*
 * PING: keep alive ping
 * Request (Server):
 *    <2B id><2B index><2B Flags>
 * Answer (Client):
 *    <2B id><2B index><2B Flags><1B ACK>
 */
const uint8_t PING_ACK = 1;

#endif
