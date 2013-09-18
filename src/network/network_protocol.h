/*
 * Copyright (C) 2008, 2010-2011 by the Widelands Development Team
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

#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

/// How many bytes will (maximal) be send as file part
#define NETFILEPARTSIZE 511

enum {
	/**
	 * The current version of the in-game network protocol. Client and host
	 * protocol versions must match.
	 */
	NETWORK_PROTOCOL_VERSION = 22,

	/**
	 * The default interval (in milliseconds) in which the host issues
	 * \ref NETCMD_SYNCREQUEST commands.
	 *
	 * This is a tunable for the host, and clients must not rely on its value.
	 */
	SYNCREPORT_INTERVAL = 1000,

	/**
	 * During the game, clients must send a \ref NETCMD_TIME command in this
	 * interval (in milliseconds). If the host does not receive such commands,
	 * it will enter a waiting state (see \ref NETCMD_WAIT).
	 *
	 * Note that a client may replace explicit \ref NETCMD_TIME commands by
	 * \ref NETCMD_PLAYERCOMMAND commands which imply a time update.
	 */
	CLIENT_TIMESTAMP_INTERVAL = 500,

	/**
	 * The default interval (in milliseconds) in which the host sends
	 * \ref NETCMD_TIME commands to update the game time to which clients
	 * may advance.
	 */
	SERVER_TIMESTAMP_INTERVAL = 100,
};


/**
 * These are the command codes used in the in-game network protocol.
 *
 * The in-game network protocol is responsible for the game setup phase, i.e.
 * setting up the map, choosing tribes and so on, as well as for the actual
 * running game itself. It is \em not responsible for game discovery and
 * other "lobby" functionality.
 *
 * The network stream of the in-game network protocol is split up into
 * packets (see \ref Deserializer, \ref RecvPacket, \ref SendPacket).
 * Every packet starts with a single-byte command code.
 */
enum {
	NETCMD_UNUSED = 0,

	/**
	 * Initiate a connection.
	 *
	 * The first communication across the network stream is a HELLO command
	 * sent by the client, with the following payload:
	 * \li Unsigned8: protocol version
	 * \li String: player name
	 * \li String: build_id of the client
	 *
	 * If the host accepts, it replies with a HELLO command with the following
	 * payload:
	 * \li Unsigned8: protocol version
	 * \li Unsigned32: 0-based user number for the client
	 *
	 * \note The host may override the client's chosen name in a subsequent
	 * \ref NETCMD_SETTING_ALLPLAYERS or \ref NETCMD_SETTING_PLAYER packet.
	 *
	 * \note For historical reasons, the HELLO command code should not be 0.
	 */
	NETCMD_HELLO = 1,

	/**
	 * Bidirectional command: Game time update. This command has the following
	 * payload:
	 * \li Signed32: game time
	 *
	 * If sent by the host, this command advises the client that it may
	 * simulate the game up to the given game time. The host guarantees that
	 * all player command with a duetime up to the given time have been sent.
	 *
	 * The client uses this command to inform the host how far its simulation
	 * has advanced.
	 */
	NETCMD_TIME = 2,

	/**
	 * Bidirectional command: Player command. This command has the payload:
	 * \li Signed32: game time
	 * \li serialized \ref Widelands::PlayerCommand
	 *
	 * If sent by the host, the client must enqueue the given \ref PlayerCommand
	 * with the sent game time as duetime. The host can send PlayerCommands for
	 * very player in the game.
	 *
	 * This command is sent by the client to request the execution of a
	 * \ref PlayerCommand. The client is only allowed to send PlayerCommands
	 * for its own player.
	 *
	 * In both cases, this command implicitly acts like a \ref NETCMD_TIME
	 * command.
	 *
	 * \note The client must never enqueue a \ref PlayerCommand that it issued
	 * itself. It must not assume that the host will accept a \ref PlayerCommand
	 * that it sends. Instead, it must rely on the host echoing back the command.
	 *
	 */
	NETCMD_PLAYERCOMMAND = 3,

	/**
	 * Bidirectional command: Terminate the connection with a given reason.
	 * Payload is:
	 * \li Unsigned8: number of attached strings
	 * \li String:    reason for disconnect
	 * \li String:    ...
	 *
	 * Both host and client can send this command, followed by immediately
	 * closing the connection. The receiver of this command should just close
	 * the connection.
	 *
	 * Note that either party is allowed to close the connection without
	 * sending a \ref NETCMD_DISCONNECT command first (in any case, this can
	 * happen when the program crashes or network connection is lost).
	 *
	 * \note If you want to change the payload of this command, change it only
	 * by appending new items. The reason is that this is the only command
	 * that can be sent by the server even when the protocol versions differ.
	 */
	NETCMD_DISCONNECT = 4,

	/**
	 * During game setup, this command is sent by the host to advise clients of a map change.
	 * Or by a client on a dedicated server to advise a map change.
	 *
	 * Payload is:
	 * \li String: human readable mapname
	 * \li String: map filename
	 * \li bool:   is_savegame
	 * \li bool:   is_scenario
	 */
	NETCMD_SETTING_MAP = 5,

	/**
	 * During game setup, this command is sent by the host to inform clients
	 * about the names of the tribes they may choose.
	 *
	 * \see NetClient::handle_network
	 */
	NETCMD_SETTING_TRIBES = 6,

	/**
	 * During game setup, this command contains complete information about all
	 * player slots (independent of their state).
	 *
	 * \see NetClient::handle_network
	 */
	NETCMD_SETTING_ALLPLAYERS = 7,

	/**
	 * During game setup, this command updates the information associated to
	 * one player slot.
	 *
	 * The client sends this command to toggle the player type. This is only available if the server is
	 * dedicated and the client was granted access.
	 * Payload in that case is:
	 * \li Unsigned8: number of the player
	 *
	 * \see NetClient::handle_network
	 */
	NETCMD_SETTING_PLAYER = 8,

	/**
	 * During game setup, this command contains complete information about all
	 * users.
	 *
	 * \see NetClient::handle_network
	 */
	NETCMD_SETTING_ALLUSERS = 9,

	/**
	 * During game setup, this command updates the information associated to
	 * one user.
	 *
	 * \see NetClient::handle_network
	 */
	NETCMD_SETTING_USER = 10,

	/**
	 * Sent by the host with no payload. The client must reply with a
	 * \ref NETCMD_PONG command.
	 */
	NETCMD_PING = 11,

	/**
	 * Sent by the host during game setup to indicate that the game starts.
	 * Alternatively sent by the client to start a dedicated server.
	 *
	 * The client must load the map and setup the game. As soon as the game
	 * is fully loaded, it must behave as if a \ref NETCMD_WAIT command had
	 * been sent.
	 */
	NETCMD_LAUNCH = 12,

	/**
	 * Sent by both the host and the client to indicate speed change.
	 * The payload is:
	 * \li Unsigned16 game speed in milliseconds per second
	 *
	 * If sent by the host, this command advises the client of how fast
	 * it should simulate the game to avoid a choppy "stop-and-go"
	 * simulation. However, the client must never simulate beyond the
	 * game time allowed by (explicit or implicit) \ref NETCMD_TIME commands.
	 *
	 * If sent by the client, it indicates the speed at which the client's
	 * player would like to play. It is up to the host to decide whether
	 * the simulation speed should be changed or not.
	 */
	NETCMD_SETSPEED = 13,

	/**
	 * Sent by the host when it suspects that a client has died or cannot
	 * catch up with the simulation.
	 *
	 * The client must continue simulating until it reaches the latest
	 * gametime received from the host via (explicit or implicit)
	 * \ref NETCMD_TIME commands and then send a \ref NETCMD_TIME to
	 * confirm that it is up to speed (in a sense, this time reply acts
	 * as a pong).
	 */
	NETCMD_WAIT = 14,

	/**
	 * Sent by the host to request an MD5 synchronization hash. Payload is:
	 * \li Signed32: game time at which the hash must be taken
	 *
	 * The client must reply with a \ref NETCMD_SYNCREPORT command as soon
	 * as the given game time has been reached on the client.
	 *
	 * \see Widelands::Game::get_sync_hash
	 */
	NETCMD_SYNCREQUEST = 15,

	/**
	 * Reply to a \ref NETCMD_PING command, with no payload.
	 */
	NETCMD_PONG = 16,

	/**
	 * During game setup, this is sent by the client to request a change
	 * to a different tribe. Payload is
	 * \li Unsigned8: player number
	 * \li String:    name of tribe
	 * \li bool:      random_tribe
	 *
	 * The client must not assume that the host will accept this request.
	 * The host may or may not send a \ref NETCMD_SETTING_ALLPLAYERS or
	 * \ref NETCMD_SETTING_PLAYER command reflecting the changed tribe.
	 * If no such command is received, then the tribe is not changed.
	 */
	NETCMD_SETTING_CHANGETRIBE = 17,

	/**
	 * During game setup, this is sent by the client to request a change
	 * to a different starting position. Payload is
	 * \li Unsigned8: number of the starting point
	 *
	 * The client must not assume that the host will accept this request.
	 * The host may or may not send a \ref NETCMD_SETTING_ALLPLAYERS or
	 * \ref NETCMD_SETTING_PLAYER command reflecting the changed position.
	 * If no such command is received, the position is not changed.
	 */
	NETCMD_SETTING_CHANGEPOSITION = 18,

	/**
	 * During game setup, this is sent by the host to a specific player.
	 * It is the answer on NETCMD_SETTING_CHANGETRIBE, if the player
	 * starting position was successfully changed. Payload is
	 * \li Signed32: new playernumber
	 */
	NETCMD_SET_PLAYERNUMBER = 19,

	/**
	 * Sent by the client to reply to a \ref NETCMD_SYNCREQUEST command,
	 * with the following payload:
	 * \li Signed32: game time at which the hash was taken
	 * \li 16 bytes: MD5 hash
	 *
	 * It is solely the host's responsibility to act when desyncs are
	 * detected.
	 */
	NETCMD_SYNCREPORT = 20,

	/**
	 * Sent by both host and client to exchange chat messages, though with
	 * different payloads.
	 *
	 * The client sends this message to the host with the following payload:
	 * \li String: the message
	 * The host will echo the message if the client is allowed to send
	 * chat messages.
	 *
	 * The host sends this message with the following payload:
	 * \li Signed16: playernumber - only used for colorization of messages.
	 * \li String: sender (may be empty to indicate system messages)
	 * \li String: the message
	 * \li Unsigned8: whether this is a personal message (0 / 1)
	 * \li String: the recipient (only filled as personal message)
	 */
	NETCMD_CHAT = 21,

	/**
	 * Sent by the host to indicate that a desync has been detected. This command
	 * has no payload.
	 *
	 * The only purpose of this command is to instruct clients to store data that
	 * may be helpful for debugging. In particular, clients should not disconnect
	 * when receiving this packet. It is the host's job to decide how to proceed
	 * after a desync has been detected.
	 */
	NETCMD_INFO_DESYNC = 22,

	/**
	 * Sent by the host to tell a client that a file transfer will start soon.
	 *
	 * The host sends all needed data to prepare for the file transfer:
	 * \li String: filename (path relative to standard path of filetype)
	 * \li Unsigned32: how many bytes will be send?
	 * \li String: md5sum
	 *
	 * Sent by the client as answer on the same message of the host as request.
	 */
	NETCMD_NEW_FILE_AVAILABLE = 23,

	/**
	 * Sent by the host to transfer a part of the file.
	 *
	 * Attached data is:
	 * \li Unsigned32: part number
	 * \li Unsigned32: length of data (needed because last part might be shorter)
	 * \li void[length of data]: data
	 *
	 * Sent by the client to request the next part from the host.
	 * \li Unsigned32: number of the last received part
	 * \li String: md5sum - to ensure client and host are talking about the same
	 */
	NETCMD_FILE_PART = 24,

	/**
	* Sent by the host (or by the client if access was granted to dedicated server)
	* to change the win condition.
	*
	* If sent by the host, attached data is:
	* \li String: name of the win condition
	*
	* If sent by the client, no data is attached, as it is only a request to toggle
	*/
	NETCMD_WIN_CONDITION = 25,

	/**
	 * During game setup, this is sent by the client to indicate that the
	 * client wants to change a team number.
	 *
	 * \li Unsigned8: number of the player
	 * \li Unsigned8: new desired team number
	 *
	 * \note The client must not assume that the host will accept this
	 * request. Change of team number only becomes effective when/if the host
	 * replies with a \ref NETCMD_SETTING_PLAYER or
	 *  \ref NETCMD_SETTING_ALLPLAYERS indicating the changed team.
	 */
	NETCMD_SETTING_CHANGETEAM = 26,

	/**
	 * During game setup, this is sent by the client to indicate that the
	 * client wants to change a shared player.
	 *
	 * \li Unsigned8: number of the player
	 * \li Unsigned8: new shared player
	 *
	 * \note The client must not assume that the host will accept this
	 * request. Change of team number only becomes effective when/if the host
	 * replies with a \ref NETCMD_SETTING_PLAYER or
	 *  \ref NETCMD_SETTING_ALLPLAYERS indicating the changed team.
	 */
	NETCMD_SETTING_CHANGESHARED = 27,

	/**
	 * During game setup, this is sent by the client to indicate that the
	 * client wants to change a player's initialisation.
	 *
	 * \li Unsigned8: number of the player
	 *
	 * \note The client must not assume that the host will accept this
	 * request. Change of team number only becomes effective when/if the host
	 * replies with a \ref NETCMD_SETTING_PLAYER or
	 *  \ref NETCMD_SETTING_ALLPLAYERS indicating the changed team.
	 */
	NETCMD_SETTING_CHANGEINIT = 28,

	/**
	 * This is sent by the server to grant access to the settings (as well as to acknowledge the correct
	 * password if the server is password protected)
	 */
	NETCMD_DEDICATED_ACCESS = 29,

	/**
	 * This is sent by the dedicated server to inform the client about the available maps on the dedicated
	 * server. Payload is:
	 *
	 * \li String:    Path to the map file
	 * \li Unsigned8: Number of maximum players
	 * \li Bool:      Whether this map can be played as multiplayer scenario
	 */
	NETCMD_DEDICATED_MAPS = 30,

	/**
	 * This is sent by the dedicated server to inform the client about the available saved games on the
	 * dedicated server. Payload is:
	 *
	 * \li String:    Path to the map file
	 * \li Unsigned8: Number of maximum players
	 */
	NETCMD_DEDICATED_SAVED_GAMES = 31,

	/**
	 * This is sent by the dedicated server to generate a clientsided translated system chat message.
	 * Payload is:
	 *
	 * \li String:    Message code \see NetworkGamingMessages::fill_map()
	 * \li String:    First attached string
	 * \li String:    Second attached string
	 * \li String:    Third attached string
	 */
	NETCMD_SYSTEM_MESSAGE_CODE = 32,

	/**
	 * Sent by the metaserver to a freshly opened game to check connectability
	 */
	NETCMD_METASERVER_PING = 64
};

#endif
