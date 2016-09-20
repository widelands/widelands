/*
 * Copyright (C) 2012 by the Widelands Development Team
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

#ifndef WL_NETWORK_INTERNET_GAMING_PROTOCOL_H
#define WL_NETWORK_INTERNET_GAMING_PROTOCOL_H

#include <string>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * BASIC SETUP VALUES                                                      *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**
 * The current version of the in-game network protocol. Client and metaserver
 * protocol versions must match.
 */
#define INTERNET_GAMING_PROTOCOL_VERSION 0

/**
 * The default timeout time after which the client tries to resend a package or even finally closes
 * the
 * connection to the metaserver, if no answer to a previous package (which requires an answer) was
 * received. In case of a login or reconnect, this is the time to wait for the metaservers answer.
 *
 * value is in milliseconds
 */
// TODO(unknown): Should this be resettable by the user?
#define INTERNET_GAMING_TIMEOUT 10  // 10 seconds

/**
 * The default timeout time after which the client tries to resend a package or even finally closes
 * the
 * connection to the metaserver, if no answer to a previous package (which requires an answer) was
 * received. In case of a login or reconnect, this is the time to wait for the metaservers answer.
 *
 * value is in milliseconds
 */
// TODO(unknown): Should this be resettable by the user?
#define INTERNET_GAMING_CLIENT_TIMEOUT 60  // 60 seconds - some time to reconnect

/**
 * The default number of retries after a timeout after which the client finally closes the
 * connection to the metaserver.
 */
// TODO(unknown): Should this be resettable by the user?
#define INTERNET_GAMING_RETRIES 3

/// Metaserver connection details
static const std::string INTERNET_GAMING_METASERVER = "widelands.org";
#define INTERNET_GAMING_PORT 7395

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CLIENT RIGHTS                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/// Client rights
static const std::string INTERNET_CLIENT_UNREGISTERED = "UNREGISTERED";
static const std::string INTERNET_CLIENT_REGISTERED = "REGISTERED";
static const std::string INTERNET_CLIENT_SUPERUSER = "SUPERUSER";
static const std::string INTERNET_CLIENT_BOT = "BOT";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * COMMUNICATION PROTOCOL BETWEEN CLIENT AND METASERVER                    *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**
 * Below are the command codes used in the internet gaming protocol.
 *
 * The internet gaming protocol is responsible for the communication between the
 * metaserver and the clients.
 *
 * The network stream of the internet gaming protocol is split up into
 * packets (see \ref Deserializer, \ref RecvPacket, \ref SendPacket).
 * Every packet starts with a single-byte command code.
 *
 * \note ALL PAYLOADS SHALL BE STRINGS - this is for easier handling and debugging of the
 * communication
 *       between metaserver and client. If an unsigned or signed value has to be send, convert it
 * with
 *       boost::lexical_cast<std::string>. Boolean values should be send in form of "true" or
 * "false".
 */

/**
 * Bidirectional command: Terminate the connection with a given reason.
 *
 * Payload is:
 * \li string: reason for disconnect in message code (see internet_gaming_messages.h)
 *
 * Both metaserver and client can send this command, followed by immediately
 * closing the connection. The receiver of this command should just close the connection.
 *
 * \note that either party is allowed to close the connection without sending a \ref
 * IGPCMD_DISCONNECT
 *       command first (in any case, this can happen when the program crashes or network connection
 * is lost).
 *
 * \note If you want to change the payload of this command, change it only by appending new items.
 * The
 *       reason is that this is the only command that can be sent by the metaserver even when the
 * protocol
 *       versions differ.
 *
 */
static const std::string IGPCMD_DISCONNECT = "DISCONNECT";

/**
 * Initiate a connection.
 *
 * The first communication across the network stream is a LOGIN command
 * sent by the client, with the following payload:
 * \li string:    protocol version
 * \li string:    client name
 * \li string:    build_id of the client
 * \li string:    whether the client wants to login in to a registered account (0 = false, 1 = true)
 * \li string:    password in clear text - only valid if previous was 1
 *
 * If the metaserver accepts, it replies with a LOGIN command with the following payload:
 * \li string:    client name (might be different to the previously chosen one, if the client did
 *                NOT login to a registered account and either the chosen is registered or already
 * used.)
 * \li string:    clients rights  (see client rights section above)
 *
 * If no answer is received in \ref INTERNET_GAMING_TIMEOUT s the client will again try to login
 * \ref INTERNET_GAMING_RETRIES times until it finally bails out something like "server does not
 * answer"
 *
 * For the case, that the metaserver does not accept the login, take a look at \ref IGPCMD_REJECTED
 */
static const std::string IGPCMD_LOGIN = "LOGIN";

/**
 * Reinitiate a connection.
 *
 * Basically the difference to \ref IGPCMD_LOGIN is, that the metaserver allows the user to go on at
 * the
 * place it was before - so if the user was in a running game and only lost connection to the
 * metaserver,
 * but not to the game itself, statistics for that game can still be send and saved.
 * To ensure everything is fine, the login information will still be checked + if a client with the
 * name
 * is still listed as online, the metaserver will ping the client and if the client does not answer
 * intime
 * it will be replaced by the user requesting the relogin
 *
 * sent by the client, with the following payload:
 * \li string:    protocol version
 * \li string:    client name - the one the metaserver replied at the first login
 * \li string:    build_id of the client
 * \li string:    whether the client wants to login in to a registered account ("false", "true")
 * \li string:    password in clear text - only valid if previous was 1
 *
 * If the metaserver accepts, it replies with a RELOGIN command without any payload.
 *
 * If no answer is received in \ref INTERNET_GAMING_TIMEOUT s the client will try to relogin
 * \ref INTERNET_GAMING_RETRIES times until it finally bails out something like "server does not
 * answer"
 *
 * For the case, that the metaserver does not accept the login, it sends a \ref IGPCMD_ERROR "LOGIN"
 */
static const std::string IGPCMD_RELOGIN = "RELOGIN";

/**
 * This command is sent by the metaserver if something went wrong.
 * At least the following payload:
 * \li string:    IGPCMD code of the message that lead to the ERROR message or ERROR
 * GARBAGE_RECEIVED if
 *                the received code was unknown.
 * \li string:    explanation code or the string that was sent, if ERROR GARBAGE_RECEIVED
 *
 * \note all this is handled in InternetGaming::handle_packet. valid explanation codes can be found
 * there.
 * \note example for not connectable game: "ERROR" "GAME_OPEN"
 */
static const std::string IGPCMD_ERROR = "ERROR";

/**
 * This is send by the metaserver to inform the client, about the metaserver time = time(0). Payload
 * \li string:    the server time
 */
static const std::string IGPCMD_TIME = "TIME";

/**
 * This is send by a superuser client to change the motd. The server has to check the permissions
 * and if those
 * allow a motd change has to change the motd and afterwards to broadcast the new motd to all
 * clients.
 * If the client has no right to change the motd, the server disconnects the client with a
 * permission denied
 * message. It should further log that try to access superuser functionality.
 * \li string:    new motd
 */
static const std::string IGPCMD_MOTD = "MOTD";

/**
 * This is send by a superuser client as announcement. The server has to check the permissions and
 * if those
 * allow an announcement, the server broadcasts the announcement as system chat to all clients.
 * If the client has no right to change the motd, the server disconnects the client with a
 * permission denied
 * message. It should further log that try to access superuser functionality.
 * \li string:    announcement message
 */
static const std::string IGPCMD_ANNOUNCEMENT = "ANNOUNCEMENT";

// in future here should the other superuser commands be

/**
 * Sent by the metaserver without payload. The client must reply with a \ref IGPCMD_PONG command.
 * If the client does not answer on a PING within \ref INTERNET_GAMING_CLIENT_TIMEOUT s it gets
 * disconnected.
 */
static const std::string IGPCMD_PING = "PING";

/**
 * Reply to a \ref IGPCMD_PING command, without payload.
 */
static const std::string IGPCMD_PONG = "PONG";

/**
 * Sent by both metaserver and client to exchange chat messages, though with different payloads.
 *
 * The client sends this message to the metaserver with the following payload:
 * \li string:    the message
 * \li string:    name of client, if private message, else empty string.
 * The metaserver will echo the message if the client is allowed to send chat messages.
 *
 * The metaserver either broadcasts a chat message to all clients or sends it to the pm recipient
 * with the
 * following payload:
 * \li string:    sender (may be empty if it is a system message)
 * \li string:    the message
 * \li string:    type ("public", "private", "system")
 *
 * \note system messages are the motd (Send by the metaserver to the client, after login (but not
 * relogin)
 *       and after the motd got changed) and announcements by superusers.
 */
static const std::string IGPCMD_CHAT = "CHAT";

/**
 * Sent by the metaserver to inform the client, that the list of games was changed. No payload is
 * sent,
 * as e.g. clients in a game are not really interested about other games and we want to keep traffic
 * as low as possible.
 *
 * To get the new list of games, the client must send \ref IGPCMD_GAMES
 */
static const std::string IGPCMD_GAMES_UPDATE = "GAMES_UPDATE";

/**
 * Sent by the client without payload to ask for the current list of games.
 *
 * Sent by the metaserver with following payload:
 * \li string:    Number of game packages and for uint8_t i = 0; i < num; ++i {:
 * \li string:    Name of the game
 * \li string:    Widelands version
 * \li string:    Whether game is connectable ("true", "false")
 * }
 */
static const std::string IGPCMD_GAMES = "GAMES";

/**
 * Sent by the metaserver to inform the client, that the list of clients was changed. No payload is
 * sent,
 * as e.g. clients in a game are not really interested about other clients and we want to keep
 * traffic
 * as low as possible.
 *
 * To get the new list of clients, the client must send \ref IGPCMD_CLIENT
 */
static const std::string IGPCMD_CLIENTS_UPDATE = "CLIENTS_UPDATE";

/**
 * Sent by the client without payload to ask for the current list of clients.
 *
 * Sent by the metaserver with following payload:
 * \li string:    Number of client packages and for uint8_t i = 0; i < num; ++i {:
 * \li string:    Name of the client
 * \li string:    Widelands version
 * \li string:    Game the player is connected to, else empty.
 * \li string:    Clients rights (see client rights section above)
 * \li string:    Points of the client
 * }
 */
static const std::string IGPCMD_CLIENTS = "CLIENTS";

/**
 * Sent by the client to announce the startup of a game with following payload:
 * \li string:    name
 * \li string:    number of maximal clients
 * \note build_id is not necessary, as this is in every way the build_id of the hosting client.
 *
 * Sent by the metaserver to acknowledge the startup of a new game without payload. The metaserver
 * will
 * list the new game, but set it as not connectable and recheck the connectability for
 * INTERNET_GAMING_TIMEOUT ms.
 * If the game gets connectable in time, the metaserver lists the game as connectable, else it
 * removes the game from the list of games.
 */
static const std::string IGPCMD_GAME_OPEN = "GAME_OPEN";

/**
 * Sent by the client to initialize the connection to a game with following payload:
 * \li string:    name of the game the client wants to connect to
 * \note the client will wait for the metaserver answer, as it needs the ip adress. if the answer is
 *       not received at time it will retry until the maximum numbers of retries is reached and the
 *       client finally closes the connection.
 *
 * Sent by the metaserver to acknowledge the connection request and to submit the ip of the game
 * \li string:    ip of the game.
 * \note as soon as this message is sent, the metaserver will list the client as connected to the
 * game.
 */
static const std::string IGPCMD_GAME_CONNECT = "GAME_CONNECT";

/**
 * Sent by the client to close the connection to a game without payload, as the client can
 * only be on one game at time.
 * This is the case in *every* way a client leaves a game. No matter if a game was played or not or
 * whether
 * the client is the host or not.
 *
 * \note as soon as this message is sent, the metaserver will list the client as not connected to
 * any game.
 * \note if the client that sends this message is the host of the game, the game will be
 *       removed from list as well. However other clients connected to that game should send the
 *       \ref IGPCMD_GAME_DISCONNECT themselves.
 */
static const std::string IGPCMD_GAME_DISCONNECT = "GAME_DISCONNECT";

/**
 * Sent by the game hosting client to announce the start of the game. No payload.
 * \note the hosting client will wait for the metaserver answer,to ensure the game is listed. If
 * even
 *       retries are not answered, the connection to the metaserver will be closed and a message
 * shall be
 *       send in the newly started game.
 *
 * Sent by the metaserver to acknowledge the start without payload.
 */
static const std::string IGPCMD_GAME_START = "GAME_START";

/**
 * Sent by every participating player of a game to announce the end of the game and to send the
 * statistics.
 * Payload is:
 * \li string:     name of the map
 * \li string:     names of the winners seperated with spaces
 * \li string:     informative string about the win condition.
 * \li string:     in game time until end
 *
 * \note this does not end the physical game and thus the metaserver should not remove the game from
 *       the list. The clients might want to play on, so...
 *
 */
static const std::string IGPCMD_GAME_END = "GAME_END";

#endif  // end of include guard: WL_NETWORK_INTERNET_GAMING_PROTOCOL_H
