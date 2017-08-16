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

#ifndef WL_NETWORK_NETHOSTPROXY_H
#define WL_NETWORK_NETHOSTPROXY_H

#include <map>
#include <memory>

#include "network/nethost_interface.h"
#include "network/netrelayconnection.h"

/**
 * NetHostProxy manages is a NetHostInterface implementation communicating to
 * the game clients about a relay server.
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
	static std::unique_ptr<NetHostProxy> connect(const std::pair<NetAddress, NetAddress>& addresses, const std::string& name, const std::string& password);

	/**
	 * Closes the server.
	 */
	~NetHostProxy();

	// Inherited from NetHostInterface
	bool is_connected(ConnectionId id) const override;
	void close(ConnectionId id) override;
	bool try_accept(ConnectionId* new_id) override;
	bool try_receive(ConnectionId id, RecvPacket* packet) override;
	void send(ConnectionId id, const SendPacket& packet) override;
	void send(const std::vector<ConnectionId>& ids, const SendPacket& packet) override;

private:

	/**
	 * Tries to connect to the relay at the given address.
	 * If it fails, is_connected() will return \c false.
	 * \param addresses Two possible addresses to connect to.
	 * \param name The name of the game.
	 * \param password The password for connecting as host.
	 */
	explicit NetHostProxy(const std::pair<NetAddress, NetAddress>& addresses, const std::string& name, const std::string& password);

	void receive_commands();

	std::unique_ptr<NetRelayConnection> conn_;

	/// A list of clients which want to connect.
	std::queue<ConnectionId> accept_;

	/// For each connected client, the packages that have been received from him.
	std::map<ConnectionId, std::queue<RecvPacket>> received_;
};

#endif  // end of include guard: WL_NETWORK_NETHOSTPROXY_H
