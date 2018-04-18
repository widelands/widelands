/*
 * Copyright (C) 2008-2018 by the Widelands Development Team
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

#include "network/nethost_interface.h"

/**
 * NetHost manages the client connections of a network game in which this computer
 * participates as a server.
 * This class tries to create sockets for IPv4 and IPv6 for gaming in the local network.
 */
class NetHost : public NetHostInterface {
public:
	/**
	 * Tries to listen on the given port.
	 * \param port The port to listen on.
	 * \return A pointer to a listening \c NetHost object or a nullptr if the connection failed.
	 */
	static std::unique_ptr<NetHost> listen(const uint16_t port);

	/**
	 * Closes the server.
	 */
	~NetHost() override;

	// Inherited from NetHostInterface
	bool is_connected(ConnectionId id) const override;
	void close(ConnectionId id) override;
	bool try_accept(ConnectionId* new_id) override;
	std::unique_ptr<RecvPacket> try_receive(ConnectionId id) override;
	void send(ConnectionId id, const SendPacket& packet) override;
	void send(const std::vector<ConnectionId>& ids, const SendPacket& packet) override;

private:
	/**
	 * Returns whether the server is started and is listening.
	 * \return \c true if the server is listening, \c false otherwise.
	 */
	// Feel free to make this method public if you need it
	bool is_listening() const;

	/**
	 * Stops listening for connections.
	 */
	// Feel free to make this method public if you need it
	void stop_listening();

	/**
	 * Tries to listen on the given port.
	 * If it fails, is_listening() will return \c false.
	 * \param port The port to listen on.
	 */
	explicit NetHost(uint16_t port);

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
		explicit Client(boost::asio::ip::tcp::socket&& sock);

		/// The socket to send/receive with.
		boost::asio::ip::tcp::socket socket;
		/// The deserializer to feed the received data to. It will transform it into data packets.
		Deserializer deserializer;
	};

	/// A map linking client ids to the respective data about the clients.
	/// Client ids not in this map should be considered invalid.
	std::map<NetHostInterface::ConnectionId, Client> clients_;
	/// The next client id that will be used
	NetHostInterface::ConnectionId next_id_;
	/// An io_service needed by boost.asio. Primary needed for async operations.
	boost::asio::io_service io_service_;
	/// The acceptor we get IPv4 connection requests to.
	boost::asio::ip::tcp::acceptor acceptor_v4_;
	/// The acceptor we get IPv6 connection requests to.
	boost::asio::ip::tcp::acceptor acceptor_v6_;
};

#endif  // end of include guard: WL_NETWORK_NETHOST_H
