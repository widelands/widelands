/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_NETWORK_NETWORK_LAN_PROMOTION_H
#define WL_NETWORK_NETWORK_LAN_PROMOTION_H

#include <list>
#include <memory>
#include <set>

#include "network/network.h"

#define LAN_PROMOTION_PROTOCOL_VERSION 1

#define LAN_GAME_CLOSED 0
#define LAN_GAME_OPEN 1

struct NetGameInfo {
	char magic[6];
	uint8_t version;
	uint8_t state;

	char gameversion[32];
	char hostname[128];
	char map[32];
};

struct NetOpenGame {
	NetOpenGame() = default;
	explicit NetOpenGame(const NetAddress& init_address, const NetGameInfo& init_info)
	   : address(init_address), info(init_info) {
	}
	NetAddress address;
	NetGameInfo info;
};

/**
 * Base class for UDP networking.
 * This class is used by derived classes to find open games on the
 * local network and to announce a just opened game on the local network.
 * This class tries to create sockets for IPv4 and IPv6.
 */
struct LanBase {
protected:
	/**
	 * Tries to start a socket on the given port.
	 * Sockets for IPv4 and IPv6 are started.
	 * When both fail, report_network_error() is called.
	 * \param port The port to listen on.
	 */
	explicit LanBase(uint16_t port);

	~LanBase();

	/**
	 * Returns whether data is available to be read.
	 * \return \c True when receive() will return data, \c false otherwise.
	 */
	bool is_available();

	/**
	 * Returns whether at least one of the sockets is open.
	 * If this returns \c false, you probably have a problem.
	 * \return \c True when a socket is ready, \c false otherwise.
	 */
	bool is_open();

	/**
	 * Tries to receive some data.
	 * \param[out] buf The buffer to read data into.
	 * \param len The length of the buffer.
	 * \param[out] addr The address we received data from. Since UDP is a connection-less
	 *                  protocol, each receive() might receive data from another address.
	 * \return How many bytes have been written to \c buf. If 0 is returned there either was no data
	 *         available (check before with avail()) or there was some error (check with is_open())
	 */
	size_t receive(void* buf, size_t len, NetAddress* addr);

	/**
	 * Sends data to a specified address.
	 * \param buf The data to send.
	 * \param len The length of the buffer.
	 * \param addr The address to send to.
	 */
	bool send(void const* buf, size_t len, const NetAddress& addr);

	/**
	 * Broadcast some data in the local network.
	 * \param buf The data to send.
	 * \param len The length of the buffer.
	 * \param port The port to send to. No address is required.
	 */
	bool broadcast(void const* buf, size_t len, uint16_t port);

	/**
	 * Throws a WLWarning exception to jump back to the main menu.
	 * Calling this on network errors is in the responsibility of derived classes.
	 * (Most of the time, aborting makes sense when an error occurred. But e.g. in
	 * the destructor simply ignoring the error is okay.)
	 */
	void report_network_error();

private:
	/**
	 * Opens a listening UDP socket.
	 * \param[out] The socket to open. The object has to be created but the socket not opened before.
	 *             If it already has been opened before, nothing will be done.
	 * \param version Whether a IPv4 or IPv6 socket should be opened.
	 * \param port The port to listen on.
	 */
	void start_socket(asio::ip::udp::socket* socket, asio::ip::udp version, uint16_t port);

	/**
	 * Closes the given socket.
	 * Does nothing if the socket already has been closed.
	 * \param socket The socket to close.
	 */
	void close_socket(asio::ip::udp::socket* socket);

	/// No idea what this does. I think it is only really used when asynchronous operations are done.
	asio::io_service io_service;
	/// The socket for IPv4.
	asio::ip::udp::socket socket_v4;
	/// The socket for IPv6.
	asio::ip::udp::socket socket_v6;
	/// The found broadcast addresses for IPv4.
	/// No addresses for v6, there is only one fixed address.
	std::set<std::string> broadcast_addresses_v4;
#ifdef __APPLE__
	/// Apple forces us to define which interface to broadcast through.
	std::set<unsigned int> interface_indices_v6;
#endif  // __APPLE__
};

/**
 * Used to promote opened games locally.
 */
struct LanGamePromoter : public LanBase {
	LanGamePromoter();
	~LanGamePromoter();

	void run();

	void set_map(char const*);

private:
	NetGameInfo gameinfo;
	bool needupdate;
};

/**
 * Used to listen for open games while in the LAN-screen.
 */
struct LanGameFinder : LanBase {
	enum { GameOpened, GameClosed, GameUpdated };

	LanGameFinder();

	void reset();
	void run();

	void set_callback(void (*)(int32_t, const NetOpenGame* const, void*), void*);

private:
	std::list<std::unique_ptr<NetOpenGame>> opengames;

	void (*callback)(int32_t, const NetOpenGame* const, void*);
	void* userdata;
};

#endif  // end of include guard: WL_NETWORK_NETWORK_LAN_PROMOTION_H
