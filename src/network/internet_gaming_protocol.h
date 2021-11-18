/*
 * Copyright (C) 2012-2021 by the Widelands Development Team
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
 * protocol versions must match. The metaserver supports the protocol version of latest stable build
 * and the newest version in trunk.
 * Used Versions:
 * 0: Build 19 and before [stable, supported]
 * 1: Between build 19 and build 20 - IPv6 support added
 * 2: Between build 19 and build 20 - Added UUID to allow reconnect with same username after
 *    crashes. When logging twice with a registered account, the second connection gets a free
 *     username assigned. Dropping RELOGIN command.
 * 3: Between build 19 and build 20 - Added network relay for internet games
 * 4: Between build 19 and build 20 - Using CHAP for password authentication
 * 5: Build 20 - Removed obsolete TELL_IP, modifications on user and game listing [supported]
 * 6: Between build 20 and build 21 - Added CHECK_PWD and PWD_OK commands
 */
constexpr unsigned int kInternetGamingProtocolVersion = 6;

/**
 * The default timeout time after which the client tries to resend a package or even finally closes
 * the
 * connection to the metaserver, if no answer to a previous package (which requires an answer) was
 * received. In case of a login or reconnect, this is the time to wait for the metaservers answer.
 *
 * value is in milliseconds
 */
// TODO(unknown): Should this be resettable by the user?
constexpr time_t kInternetGamingTimeout = 10;  // 10 seconds

/// Metaserver connection details
constexpr const char* const INTERNET_GAMING_METASERVER = "widelands.org";
// Default port for connecting to the metaserver
constexpr uint16_t kInternetGamingPort = 7395;
// Default port for connecting to the relay
constexpr uint16_t kInternetRelayPort = 7397;
// The following ones are only used between metaserver and relay
// Port used by the metaserver to contact the relay
// INTERNET_RELAY_RPC_PORT 7398
// Port used by the relay to contact the metaserver
// INTERNET_GAMING_RPC_PORT 7399

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * CLIENT RIGHTS                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/// Client rights
constexpr const char* const INTERNET_CLIENT_UNREGISTERED = "UNREGISTERED";
constexpr const char* const INTERNET_CLIENT_REGISTERED = "REGISTERED";
constexpr const char* const INTERNET_CLIENT_SUPERUSER = "SUPERUSER";
constexpr const char* const INTERNET_CLIENT_IRC = "IRC";

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * GAME STATUS                                                             *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/// States an online game can be in.
/// Other values might appear but should be considered as "CLOSED"
constexpr const char* const INTERNET_GAME_CLOSED =
   "CLOSED";                                                // Not yet connectable or not over relay
constexpr const char* const INTERNET_GAME_SETUP = "SETUP";  // Map selection and so
constexpr const char* const INTERNET_GAME_RUNNING = "RUNNING";  // Playing

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
 * packets (see \ref RecvPacket, \ref SendPacket).
 * Every packet starts with a single-byte command code.
 *
 * \note ALL PAYLOADS SHALL BE STRINGS - this is for easier handling and debugging of the
 *       communication between metaserver and client. If an unsigned or signed value has
 *       to be sent, convert it with as_string. Boolean values should
         be sent in form of "true" or "false".
 */

/**
 * The UUID:
 *
 * The UUID is a semi-permanent ID stored in the configuration file of Widelands.
 * It has to be stored in the file since it should survive crashes of the game or computer.
 * If the game is not started for 24 hours, a new one is created to increase privacy.
 * Basically it allows the metaserver to identify the user even when multiple users try to join with
 * the same username. Note that the sent UUID differs from the stored one: The sent UUID is
 * hash(username | stored-id).
 * In the case of registered players, the password can be used instead of a UUID. The username
 * alone can not be used for this, especially not for unregistered users: The metaserver can not
 * differentiate between a second connection by the user and an initial login of another user
 * (with the same name).
 *
 * Use-cases of the UUID:
 *
 * Reconnect after crash / network problems.
 * When Widelands breaks the connection without logging out, the server still assumes that the old
 * connection is active. So when the player reconnects, another name is chosen. Sending the UUID
 * allows to reclaim the old name, since the server recognizes that there isn't a second player
 * trying to use the same name.
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
 *       IGPCMD_DISCONNECT command first (in any case, this can happen when the program crashes
 *       or network connection is lost).
 *
 * \note If you want to change the payload of this command, change it only by appending new items.
 *       The reason is that this is the only command that can be sent by the metaserver even when
 *       the protocol versions differ.
 *
 */
constexpr const char* const IGPCMD_DISCONNECT = "DISCONNECT";

/**
 * Initiate a connection.
 *
 * The first communication across the network stream is a IGPCMD_LOGIN command
 * sent by the client, with the following payload:
 * \li string:    protocol version (see kInternetGamingProtocolVersion)
 * \li string:    client name
 * \li string:    build_id of the client
 * \li string:    whether the client wants to login in to a registered account
 *                ("true" or "false" as string)
 * \li string:    for registered accounts: string of length 0
 *                for unregistered users the UUID to recognize the matching IPv4 and IPv6
 *                connections or to reclaim the username after a unintended disconnect.
 *                For an explanation of the UUID, see above.
 *
 * If the user tries to login to a registered account, a IGPCMD_PWD_CHALLENGE exchange follows
 * before
 * the server replies with a IGPCMD_LOGIN or IGPCMD_ERROR message.
 *
 * If the metaserver accepts, it replies with a IGPCMD_LOGIN command with the following payload:
 * \li string:    client name. Might be different to the previously chosen one, if the chosen
 *                name already used or is registered (and the connecting client is not registered).
 * \li string:    clients rights (see client rights section above)
 *
 * When the client is downgraded to an unregistered user on login, a special UUID value
 * of sha1(assignedName | passwordHash) has to be used on reconnects.
 *
 * If no answer is received in \ref kInternetGamingTimeout seconds the client will again try to
 * login
 * \ref INTERNET_GAMING_RETRIES times until it finally bails out something like "server does not
 * answer"
 *
 * For the case, that the metaserver does not accept the login, take a look at \ref IGPCMD_ERROR
 */
constexpr const char* const IGPCMD_LOGIN = "LOGIN";

/**
 * The client tries to check the password of the user without doing a full login.
 * Should be sent without logging in before.
 *
 * Payload:
 * \li string:    protocol version (see kInternetGamingProtocolVersion)
 * \li string:    client name
 * \li string:    build_id of the client
 *
 * A IGPCMD_PWD_CHALLENGE exchange follows before the server replies with a IGPCMD_PWD_OK
 * or IGPCMD_ERROR message.
 *
 * If the password is correct, the metaserver replies with a IGPCMD_PWD_OK command
 * with the following payload:
 * \li string:    client name. Will be the same as sent before.
 * \li string:    clients rights (see client rights section above)
 *
 * If the password is wrong or some other error occurred, \ref IGPCMD_ERROR is returned.
 */
constexpr const char* const IGPCMD_CHECK_PWD = "CHECK_PWD";
constexpr const char* const IGPCMD_PWD_OK = "PWD_OK";

/**
 * This is sent by the metaserver after a IGPCMD_LOGIN or IGPCMD_CHECK_PWD by a registered client.
 * This is the first message of the a protocol similar to the challenge handshake authentication
 * protocol (CHAP) for secure transmission of the users password.
 * The server sends the nonce for hashing:
 * \li string:    a nonce for hashing
 *
 * The client should answer the message by an own IGPCMD_PWD_CHALLENGE containing the hashed
 * password:
 * \li string:    HASH_SHA1(nonce | HASH_SHA1(password))
 *
 * If the transmitted value is correct, the normal IGPCMD_LOGIN sequence continues.
 * If the
 * value is wrong (e.g., wrong password) the connection is terminated by the servers
 * IGPCMD_DISCONNECT.
 */
constexpr const char* const IGPCMD_PWD_CHALLENGE = "PWD_CHALLENGE";

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
constexpr const char* const IGPCMD_ERROR = "ERROR";

/**
 * This is sent by the metaserver to inform the client, about the metaserver time = time(0). Payload
 * \li string:    the server time
 */
constexpr const char* const IGPCMD_TIME = "TIME";

/**
 * This is sent by a superuser client to change the motd. The server has to check the permissions
 * and if those
 * allow a motd change has to change the motd and afterwards to broadcast the new motd to all
 * clients.
 * If the client has no right to change the motd, the server disconnects the client with a
 * permission denied
 * message. It should further log that try to access superuser functionality.
 * \li string:    new motd
 */
constexpr const char* const IGPCMD_MOTD = "MOTD";

/**
 * This is sent by a superuser client as announcement. The server has to check the permissions and
 * if those
 * allow an announcement, the server broadcasts the announcement as system chat to all clients.
 * If the client has no right to change the motd, the server disconnects the client with a
 * permission denied
 * message. It should further log that try to access superuser functionality.
 * \li string:    announcement message
 */
constexpr const char* const IGPCMD_ANNOUNCEMENT = "ANNOUNCEMENT";

// in future here should the other superuser commands be

/**
 * Sent by the metaserver without payload. The client must reply with a \ref IGPCMD_PONG command.
 * If the client does not answer on a PING within \ref INTERNET_GAMING_CLIENT_TIMEOUT s it gets
 * disconnected.
 */
constexpr const char* const IGPCMD_PING = "PING";

/**
 * Reply to a \ref IGPCMD_PING command, without payload.
 */
constexpr const char* const IGPCMD_PONG = "PONG";

/**
 * Sent by both metaserver and client to exchange chat messages, though with different payloads.
 *
 * The client sends this message to the metaserver with the following payload:
 * \li string:    the message
 * \li string:    name of client, if private message, else empty string.
 * The metaserver will echo the message if the client is allowed to send chat messages.
 *
 * The metaserver either broadcasts a chat message to all clients or sends it to the pm recipient
 * with the following payload:
 * \li string:    sender (may be empty if it is a system message)
 * \li string:    the message
 * \li string:    type ("public", "private", "system")
 *
 * \note system messages are the motd (Sent by the metaserver to the client, after login
 * (but not relogin) and after the motd got changed) and announcements by superusers.
 */
constexpr const char* const IGPCMD_CHAT = "CHAT";

/**
 * Sent by the client to issue a superuser command.
 *
 * The client sends this message to the metaserver with the following payload:
 * \li string:    the command
 * \li string:    arbitrary parameters.
 */
constexpr const char* const IGPCMD_CMD = "CMD";

/**
 * Sent by the metaserver to inform the client, that the list of games was changed. No payload is
 * sent,
 * as e.g. clients in a game are not really interested about other games and we want to keep traffic
 * as low as possible.
 *
 * To get the new list of games, the client must send \ref IGPCMD_GAMES
 */
constexpr const char* const IGPCMD_GAMES_UPDATE = "GAMES_UPDATE";

/**
 * Sent by the client without payload to ask for the current list of games.
 *
 * Sent by the metaserver with following payload:
 * \li string:    Number of game packages and for uint8_t i = 0; i < num; ++i {:
 * \li string:    Name of the game
 * \li string:    Widelands version
 * \li string:    Status of the game, see above. Note that only because a game is connectable
 *                this does not mean that gaming will work when the versions differ
 * }
 */
constexpr const char* const IGPCMD_GAMES = "GAMES";

/**
 * Sent by the metaserver to inform the client, that the list of clients was changed. No payload is
 * sent, as e.g. clients in a game are not really interested about other clients and we want to
 * keep traffic as low as possible.
 *
 * To get the new list of clients, the client must send \ref IGPCMD_CLIENT
 */
constexpr const char* const IGPCMD_CLIENTS_UPDATE = "CLIENTS_UPDATE";

/**
 * Sent by the client without payload to ask for the current list of clients.
 *
 * Sent by the metaserver with following payload:
 * \li string:    Number of client packages and for uint8_t i = 0; i < num; ++i {:
 * \li string:    Name of the client
 * \li string:    Widelands version
 * \li string:    Game the player is connected to, else empty
 * \li string:    Clients rights (see client rights section above)
 * }
 */
constexpr const char* const IGPCMD_CLIENTS = "CLIENTS";

/**
 * Sent by the client to announce the startup of a game with following payload:
 * \li string:    name of the game
 * \note build_id is not necessary, as this is the build_id of the hosting client anyway.
 *
 * Sent by the metaserver to acknowledge the startup of a new game with the following payload:
 * \li string:    a challenge that has to be "solved" to work as host of the new game.
 *                See IGPCMD_PWD_CHALLENGE. The response is send to the relay
 * \li string:    primary ip of relay server for the game.
 * \li string:    whether a secondary ip for the relay follows ("true" or "false" as string)
 * \li string:    secondary ip of the relay - only valid if previous was true
 * The metaserver will list the new game, but set it as not connectable.
 * When the client connects to the relay within kInternetGamingTimeout milliseconds,
 * the metaserver lists the game as connectable, else it removes the game from the list of games.
 */
constexpr const char* const IGPCMD_GAME_OPEN = "GAME_OPEN";

/**
 * Sent by the client to initialize the connection to a game with following payload:
 * \li string:    name of the game the client wants to connect to
 * \note the client will wait for the metaserver answer, as it needs the ip adress. if the answer is
 *       not received at time it will retry until the maximum numbers of retries is reached and the
 *       client finally closes the connection.
 *
 * Sent by the metaserver to acknowledge the connection request and to submit the ip of the game
 * \li string:    primary ip of the game.
 * \li string:    whether a secondary ip for the game follows ("true" or "false" as string)
 * \li string:    secondary ip of the game - only valid if previous was true
 * \note as soon as this message is sent, the metaserver will list the client as connected to the
 * game.
 */
constexpr const char* const IGPCMD_GAME_CONNECT = "GAME_CONNECT";

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
constexpr const char* const IGPCMD_GAME_DISCONNECT = "GAME_DISCONNECT";

/**
 * Sent by the game hosting client to announce the start of the game. No payload.
 * \note the hosting client will wait for the metaserver answer, to ensure the game is listed. If
 * even
 *       retries are not answered, the connection to the metaserver will be closed and a message
 * shall be
 *       sent in the newly started game.
 *
 * Sent by the metaserver to acknowledge the start without payload.
 */
constexpr const char* const IGPCMD_GAME_START = "GAME_START";

#endif  // end of include guard: WL_NETWORK_INTERNET_GAMING_PROTOCOL_H
