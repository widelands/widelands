/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
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

#ifndef WL_NETWORK_NETHOST_H
#define WL_NETWORK_NETHOST_H

#include <map>
#include <memory>

#include "network/network.h"

/**
 * NetHost manages the client connections of a network game in which this computer
 * participates as a server.
 * This class tries to create sockets for IPv4 and IPv6.
 */
class NetHost {
public:
	/// IDs used to enumerate the clients.
	using ConnectionId = uint32_t;

	/**
	 * Tries to listen on the given port.
	 * \param port The port to listen on.
	 * \return A pointer to a listening \c NetHost object or a nullptr if the connection failed.
	 */
	static std::unique_ptr<NetHost> listen(const uint16_t port);

	/**
	 * Closes the server.
	 */
	~NetHost();

	/**
	 * Returns whether the server is started and is listening.
	 * \return \c true if the server is listening, \c false otherwise.
	 */
	bool is_listening() const;

	/**
	 * Returns whether the given client is connected.
	 * \param The id of the client to check.
	 * \return \c true if the connection is open, \c false otherwise.
	 */
	bool is_connected(ConnectionId id) const;

	/**
	 * Stops listening for connections.
	 */
	void stop_listening();

	/**
	 * Closes the connection to the given client.
	 * \param id The id of the client to close the connection to.
	 */
	void close(ConnectionId id);

	/**
	 * Tries to accept a new client.
	 * \param new_id The connection id of the new client will be stored here.
	 * \return \c true if a client has connected, \c false otherwise.
	 *   The given id is only modified when \c true is returned.
	 *   Calling this on a closed server will return false.
	 *   The returned id is always greater than 0.
	 */
	bool try_accept(ConnectionId* new_id);

	/**
	 * Tries to receive a packet.
	 * \param id The connection id of the client that should be received.
	 * \param packet A packet that should be overwritten with the received data.
	 * \return \c true if a packet is available, \c false otherwise.
	 *   The given packet is only modified when \c true is returned.
	 *   Calling this on a closed connection will return false.
	 */
	bool try_receive(ConnectionId id, RecvPacket* packet);

	/**
	 * Sends a packet.
	 * Calling this on a closed connection will silently fail.
	 * \param id The connection id of the client that should be sent to.
	 * \param packet The packet to send.
	 */
	void send(ConnectionId id, const SendPacket& packet);

private:
	/**
	 * Tries to listen on the given port.
	 * If it fails, is_listening() will return \c false.
	 * \param port The port to listen on.
	 */
	NetHost(const uint16_t port);

	bool open_acceptor(boost::asio::ip::tcp::acceptor* acceptor,
	                   const boost::asio::ip::tcp::endpoint& endpoint);

	/**
	 * Helper structure to store variables about a connected client.
	 */
	struct Client {
		/**
		 * Initializes the structure with the given socket.
		 * \param sock The socket to listen on. The socket is moved by this
		 *             constructor so the given socket is no longer valid.
		 */
		Client(boost::asio::ip::tcp::socket&& sock);

		/// The socket to send/receive with.
		boost::asio::ip::tcp::socket socket;
		/// The deserializer to feed the received data to. It will transform it into data packets.
		Deserializer deserializer;
	};

	/// A map linking client ids to the respective data about the clients.
	/// Client ids not in this map should be considered invalid.
	std::map<NetHost::ConnectionId, Client> clients_;
	/// The next client id that will be used
	NetHost::ConnectionId next_id_;
	/// An io_service needed by boost.asio. Primary needed for async operations.
	boost::asio::io_service io_service_;
	/// The acceptor we get IPv4 connection requests to.
	boost::asio::ip::tcp::acceptor acceptor_v4_;
	/// The acceptor we get IPv6 connection requests to.
	boost::asio::ip::tcp::acceptor acceptor_v6_;
};

#endif  // end of include guard: WL_NETWORK_NETHOST_H
