/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#include <memory>
#include <thread>

#include "network/bufferedconnection.h"
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
	void send(ConnectionId id,
	          const SendPacket& packet,
	          NetPriority priority = NetPriority::kNormal) override;
	void send(const std::vector<ConnectionId>& ids,
	          const SendPacket& packet,
	          NetPriority priority = NetPriority::kNormal) override;

	/**
	 * Stops listening for connections.
	 */
	void stop_listening();

private:
	/**
	 * Returns whether the server is started and is listening.
	 * \return \c true if the server is listening, \c false otherwise.
	 */
	// Feel free to make this method public if you need it
	bool is_listening() const;

	/**
	 * Starts an asynchronous accept on the given acceptor.
	 * If someone wants to connect, establish a connection
	 * and add the connection to accept_queue_ and continue waiting.
	 * @param acceptor The acceptor we should be listening on.
	 * @param pair A pair of the BufferedConnection for the new client and its socket.
	 */
	void
	start_accepting(asio::ip::tcp::acceptor& acceptor,
	                std::pair<std::unique_ptr<BufferedConnection>, asio::ip::tcp::socket*>& pair);

	/**
	 * Tries to listen on the given port.
	 * If it fails, is_listening() will return \c false.
	 * \param port The port to listen on.
	 */
	explicit NetHost(uint16_t port);

	/**
	 * Prepare the given acceptor for accepting connections for the given
	 * network protocol and port.
	 * @param acceptor The acceptor to prepare.
	 * @param endpoint The IP version, transport protocol and port number we should listen on.
	 * @return \c True iff the acceptor is listening now.
	 */
	bool open_acceptor(asio::ip::tcp::acceptor* acceptor, const asio::ip::tcp::endpoint& endpoint);

	/// A map linking client ids to the respective network connections.
	/// Client ids not in this map should be considered invalid.
	std::map<NetHostInterface::ConnectionId, std::unique_ptr<BufferedConnection>> clients_;
	/// The next client id that will be used
	NetHostInterface::ConnectionId next_id_;
	/// An io_service needed by asio. Primary needed for async operations.
	asio::io_service io_service_;
	/// The acceptor we get IPv4 connection requests to.
	asio::ip::tcp::acceptor acceptor_v4_;
	/// The acceptor we get IPv6 connection requests to.
	asio::ip::tcp::acceptor acceptor_v6_;

	/// Socket and unconnected BuffereConnection that will be used for accepting IPv4 connections
	std::pair<std::unique_ptr<BufferedConnection>, asio::ip::tcp::socket*> accept_pair_v4_;
	/// Socket and unconnected BuffereConnection that will be used for accepting IPv6 connections
	std::pair<std::unique_ptr<BufferedConnection>, asio::ip::tcp::socket*> accept_pair_v6_;

	/// A thread used to wait for connections on the acceptor.
	std::thread asio_thread_;
	/// The new connections the acceptor accepted. Will be moved to clients_
	/// when try_accept() is called by the using class.
	std::queue<std::unique_ptr<BufferedConnection>> accept_queue_;
	/// A mutex avoiding concurrent access to accept_queue_.
	std::mutex mutex_accept_;
};

#endif  // end of include guard: WL_NETWORK_NETHOST_H
