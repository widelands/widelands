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

#ifndef WL_NETWORK_NETHOSTPROXY_H
#define WL_NETWORK_NETHOSTPROXY_H

#include <memory>

#include "network/bufferedconnection.h"
#include "network/nethost_interface.h"

/**
 * Represents a host in-game, but talks through the 'wlnr' relay binary.
 */
class NetHostProxy : public NetHostInterface {
public:
	/**
	 * Tries to connect to the relay at the given address.
	 * \param address The address to connect to.
	 * \param name The name of the game.
	 * \param password The password for connecting as host.
	 * \return A pointer to a ready \c NetHostProxy object or a nullptr if the connection failed.
	 */
	static std::unique_ptr<NetHostProxy> connect(const std::pair<NetAddress, NetAddress>& addresses,
	                                             const std::string& name,
	                                             const std::string& password);

	/**
	 * Closes the server.
	 */
	~NetHostProxy() override;

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

private:
	/**
	 * Tries to connect to the relay at the given address.
	 * If it fails, is_connected() will return \c false.
	 * \param addresses Two possible addresses to connect to.
	 * \param name The name of the game.
	 * \param password The password for connecting as host.
	 */
	NetHostProxy(const std::pair<NetAddress, NetAddress>& addresses,
	             const std::string& name,
	             const std::string& password);

	void receive_commands();

	std::unique_ptr<BufferedConnection> conn_;

	/// A list of clients which want to connect.
	std::queue<ConnectionId> accept_;

	/// The clients connected through the relay
	struct Client {
		/// The state of the client
		enum class State {
			/// The relay introduced the client but try_accept() hasn't been called for it yet
			kConnecting,
			/// A normally connected client
			kConnected,
			/// The relay told us that the client disconnected but there are still packages in the
			/// buffer
			kDisconnected
		};

		Client() : state_(State::kConnecting), received_() {
		}

		// deleted since RecvPacket does not offer a copy constructor
		Client(const Client& other) = delete;

		/// The current connection state
		State state_;
		/// The packages that have been received
		std::queue<std::unique_ptr<RecvPacket>> received_;
	};
	/// The connected clients
	std::map<ConnectionId, Client> clients_;
};

#endif  // end of include guard: WL_NETWORK_NETHOSTPROXY_H
