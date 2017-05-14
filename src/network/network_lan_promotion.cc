/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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

#include "network/network_lan_promotion.h"

#ifndef _WIN32
#include <ifaddrs.h>
#endif

#include "base/i18n.h"
#include "base/log.h"
#include "base/warning.h"
#include "build_info.h"
#include "network/constants.h"

namespace {

	static const char *ip_versions[] = {"IPv4", "IPv6"};

	/**
	 * Returns the matching string for the given IP address.
	 * \param addr The address object to get the IP version for.
	 * \return A pointer to a constant string naming the IP version.
	 */
	const char* get_ip_version_string(const boost::asio::ip::address& addr) {
		assert(!addr.is_unspecified());
		if (addr.is_v4()) {
			return ip_versions[0];
		} else {
			assert(addr.is_v6());
			return ip_versions[1];
		}
	}

	/**
	 * Returns the matching string for the given IP address.
	 * \param version A whatever object to get the IP version for.
	 * \return A pointer to a constant string naming the IP version.
	 */
	const char* get_ip_version_string(const boost::asio::ip::udp& version) {
		if (version == boost::asio::ip::udp::v4()) {
			return ip_versions[0];
		} else {
			assert(version == boost::asio::ip::udp::v6());
			return ip_versions[1];
		}
	}
}

/*** class LanBase ***/
LanBase::LanBase(uint16_t port)
	: io_service(), socket_v4(io_service), socket_v6(io_service) {

#ifndef _WIN32
	// Iterate over all interfaces. If they support IPv4, store the broadcast-address
	// of the interface and try to start the socket. If they support IPv6, just start
	// the socket. There is one fixed broadcast-address for IPv6 (well, actually multicast)

	// Adapted example out of "man getifaddrs"
	// Admittedly: I don't like this part. But boost is not able to iterate over
	// the local IPs at this time. If they ever add it, replace this code
	struct ifaddrs *ifaddr, *ifa;
	int family, s, n;
	char host[NI_MAXHOST];
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
	for (ifa = ifaddr, n = 0; ifa != nullptr; ifa = ifa->ifa_next, n++) {
		if (ifa->ifa_addr == nullptr)
			continue;
		family = ifa->ifa_addr->sa_family;
		if (family == AF_INET && (ifa->ifa_flags & IFF_BROADCAST)) {
			s = getnameinfo(ifa->ifa_ifu.ifu_broadaddr, sizeof(struct sockaddr_in),
						host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);
			if (s == 0) {
				start_socket(&socket_v4, boost::asio::ip::udp::v4(), port);
				broadcast_addresses_v4.insert(host);
			}
		} else if (family == AF_INET6 && (ifa->ifa_flags & IFF_BROADCAST)) {
			start_socket(&socket_v6, boost::asio::ip::udp::v6(), port);
			// Nothing to insert here. There is only one "broadcast" address for IPv6 (I think)
		}
	}
	freeifaddrs(ifaddr);
#else
	//  As Microsoft does not seem to support if_nameindex, we just broadcast to
	//  INADDR_BROADCAST.
	broadcast_addresses_v4.insert("255.255.255.255");
#endif

	// Okay, needed this for development. But might be useful to debug network problems
	for (const std::string& ip : broadcast_addresses_v4)
		log("[LAN] Will broadcast to %s\n", ip.c_str());

	if (!is_open()) {
		// Hm, not good. Just try to open them and hope for the best
		start_socket(&socket_v4, boost::asio::ip::udp::v4(), port);
		start_socket(&socket_v6, boost::asio::ip::udp::v6(), port);
	}

	if (!is_open()) {
		// Still not open? Go back to main menu.
		report_network_error();
	}
}

LanBase::~LanBase() {
	close_socket(&socket_v4);
	close_socket(&socket_v6);
}

bool LanBase::avail() {
	boost::system::error_code ec;
	bool available_v4 = (socket_v4.is_open() && socket_v4.available(ec) > 0);
	if (ec) {
		close_socket(&socket_v4);
		available_v4 = false;
	}
	bool available_v6 = (socket_v6.is_open() && socket_v6.available(ec) > 0);
	if (ec) {
		close_socket(&socket_v6);
		available_v4 = false;
	}
	return available_v4 || available_v6;
}

bool LanBase::is_open() {
	return socket_v4.is_open() || socket_v6.is_open();
}

ssize_t LanBase::receive(void* const buf, size_t const len, NetAddress *addr) {
	assert(buf != nullptr);
	assert(addr != nullptr);
	boost::asio::ip::udp::endpoint sender_endpoint;
	size_t recv_len;
	if (socket_v4.is_open()) {
		try {
			if (socket_v4.available() > 0) {
				recv_len = socket_v4.receive_from(boost::asio::buffer(buf, len), sender_endpoint);
				*addr = NetAddress{sender_endpoint.address().to_string(), sender_endpoint.port()};
				assert(recv_len <= len);
				return recv_len;
			}
		} catch (const boost::system::system_error&) {
			// Some network error. Close the socket
			close_socket(&socket_v4);
		}
	}
	// We only reach this point if there was nothing to receive for IPv4
	if (socket_v6.is_open()) {
		try {
			if (socket_v6.available() > 0) {
				recv_len = socket_v6.receive_from(boost::asio::buffer(buf, len), sender_endpoint);
				*addr = NetAddress{sender_endpoint.address().to_string(), sender_endpoint.port()};
				assert(recv_len <= len);
				return recv_len;
			}
		} catch (const boost::system::system_error&) {
			close_socket(&socket_v6);
		}
	}
	// Nothing to receive at all. So lonely here...
	return 0;
}

bool LanBase::send(void const* const buf, size_t const len, const NetAddress& addr) {
	boost::system::error_code ec;
	const boost::asio::ip::address address = boost::asio::ip::address::from_string(addr.ip, ec);
	// If this assert failed, then there is some bug in the code. NetAddress should only be filled
	// with valid IP addresses (e.g. no hostnames)
	assert(!ec);
	boost::asio::ip::udp::endpoint destination(address, addr.port);
	boost::asio::ip::udp::socket *socket = nullptr;
	if (destination.address().is_v4()) {
		socket = &socket_v4;
	} else if (destination.address().is_v6()) {
		socket = &socket_v6;
	} else {
		NEVER_HERE();
	}
	assert(socket != nullptr);
	if (!socket->is_open()) {
		// I think this shouldn't happen normally. It might happen, though, if we receive
		// a broadcast and learn the IP, then our sockets goes down, then we try to send
		log("[LAN] Error: trying to send to an %s address but socket is not open",
			get_ip_version_string(address));
		return false;
	}
	socket->send_to(boost::asio::buffer(buf, len), destination, 0, ec);
	if (ec) {
		close_socket(socket);
		return false;
	}
	return true;
}

bool LanBase::broadcast(void const* const buf, size_t const len, uint16_t const port) {
	boost::system::error_code ec;
	bool error = false;
	if (socket_v4.is_open()) {
		for (const std::string& address : broadcast_addresses_v4) {
			boost::asio::ip::udp::endpoint destination(boost::asio::ip::address::from_string(address), port);
			socket_v4.send_to(boost::asio::buffer(buf, len), destination, 0, ec);
			if (ec) {
				close_socket(&socket_v4);
				error = true;
				break;
			}
		}
	}
	if (socket_v6.is_open()) {
		boost::asio::ip::udp::endpoint destination(boost::asio::ip::address::from_string("ff02::1"), port);
		socket_v6.send_to(boost::asio::buffer(buf, len), destination, 0, ec);
		if (ec) {
			close_socket(&socket_v6);
				error = true;
		}
	}
	return !error;
}

void LanBase::start_socket(boost::asio::ip::udp::socket *socket, boost::asio::ip::udp version, uint16_t port) {

    if (socket->is_open())
		return;

	boost::system::error_code ec;
	// Try to open the socket
	socket->open(version, ec);
	if (ec) {
		log("[LAN] Failed to start an %s socket: %s\n",
			get_ip_version_string(version), ec.message().c_str());
		return;
	}

	const boost::asio::socket_base::broadcast option_broadcast(true);
	socket->set_option(option_broadcast, ec);
	if (ec) {
		log("[LAN] Error setting options for %s socket, closing socket: %s\n",
			get_ip_version_string(version), ec.message().c_str());
		// Retrieve the error code to avoid throwing but ignore it
		close_socket(socket);
		return;
	}

	const boost::asio::socket_base::reuse_address option_reuse(true);
	socket->set_option(option_reuse, ec);
	// This one isn't really needed so ignore the error


	if (version == boost::asio::ip::udp::v6()) {
		const boost::asio::ip::v6_only option_v6only(true);
		socket->set_option(option_v6only, ec);
		// This one might not be needed, ignore the error and see whether we fail on bind()
	}

	socket->bind(boost::asio::ip::udp::endpoint(version, port), ec);
	if (ec) {
		log("[LAN] Error binding %s socket to UDP port %d, closing socket: %s\n",
			get_ip_version_string(version), port, ec.message().c_str());
		close_socket(socket);
		return;
	}

	log("[LAN] Started an %s socket on UDP port %d\n", get_ip_version_string(version), port);
}

void LanBase::report_network_error() {
	// No socket open? Sorry, but we can't continue this way
	throw WLWarning(_("Failed to use the local network!"),
		_("Widelands was unable to use the local network. "
		  "Maybe some other process is already running a server on port %d, %d or %d "
		  "or your network setup is broken."),
		WIDELANDS_LAN_DISCOVERY_PORT, WIDELANDS_LAN_PROMOTION_PORT, WIDELANDS_PORT);
}

void LanBase::close_socket(boost::asio::ip::udp::socket *socket) {
	boost::system::error_code ec;
	if (socket->is_open()) {
		const boost::asio::ip::udp::endpoint& endpoint = socket->local_endpoint(ec);
		if (!ec)
			log("[LAN] Closing an %s socket.\n", get_ip_version_string(endpoint.protocol()));
		socket->shutdown(boost::asio::ip::udp::socket::shutdown_both, ec);
		socket->close(ec);
	}
}

/*** class LanGamePromoter ***/

LanGamePromoter::LanGamePromoter()
	: LanBase(WIDELANDS_LAN_PROMOTION_PORT) {

	needupdate = true;

	memset(&gameinfo, 0, sizeof(gameinfo));
	strcpy(gameinfo.magic, "GAME");

	gameinfo.version = LAN_PROMOTION_PROTOCOL_VERSION;
	gameinfo.state = LAN_GAME_OPEN;

	strncpy(gameinfo.gameversion, build_id().c_str(), sizeof(gameinfo.gameversion));

	strncpy(gameinfo.hostname, boost::asio::ip::host_name().c_str(), sizeof(gameinfo.hostname));
}

LanGamePromoter::~LanGamePromoter() {
	gameinfo.state = LAN_GAME_CLOSED;

	// Don't care about errors at this point
	broadcast(&gameinfo, sizeof(gameinfo), WIDELANDS_LAN_DISCOVERY_PORT);
}

void LanGamePromoter::run() {
	if (needupdate) {
		needupdate = false;

		if (!broadcast(&gameinfo, sizeof(gameinfo), WIDELANDS_LAN_DISCOVERY_PORT))
			report_network_error();
	}

	while (avail()) {
		char magic[8];
		NetAddress addr;

		if (receive(magic, 8, &addr) < 8)
			continue;

		log("Received %s packet from %s\n", magic, addr.ip.c_str());

		if (!strncmp(magic, "QUERY", 6) && magic[6] == LAN_PROMOTION_PROTOCOL_VERSION) {
			if (!send(&gameinfo, sizeof(gameinfo), addr))
				report_network_error();
		}
	}
}

void LanGamePromoter::set_map(char const* map) {
	strncpy(gameinfo.map, map, sizeof(gameinfo.map));

	needupdate = true;
}

/*** class LanGameFinder ***/

LanGameFinder::LanGameFinder()
	: LanBase(WIDELANDS_LAN_DISCOVERY_PORT), callback(nullptr) {

	reset();
}

void LanGameFinder::reset() {
	char magic[8];

	opengames.clear();

	strncpy(magic, "QUERY", 8);
	magic[6] = LAN_PROMOTION_PROTOCOL_VERSION;

	if (!broadcast(magic, 8, WIDELANDS_LAN_PROMOTION_PORT))
		report_network_error();
}

void LanGameFinder::run() {
	while (avail()) {
		NetGameInfo info;
		NetAddress addr;

		if (receive(&info, sizeof(info), &addr) < static_cast<int32_t>(sizeof(info)))
			continue;

		log("Received %s packet from %s\n", info.magic, addr.ip.c_str());

		if (strncmp(info.magic, "GAME", 6))
			continue;

		if (info.version != LAN_PROMOTION_PROTOCOL_VERSION)
			continue;

		//  if the game already is in the list, update the information
		//  otherwise just append it to the list
		bool was_in_list = false;
		for (NetOpenGame* opengame : opengames) {
			if (0 == strncmp(opengame->info.hostname, info.hostname, 128)) {
				opengame->info = info;
				callback(GameUpdated, opengame, userdata);
				was_in_list = true;
				break;
			}
		}

		if (!was_in_list) {
			opengames.push_back(new NetOpenGame);
			DIAG_OFF("-Wold-style-cast")
			addr.port = WIDELANDS_PORT;
			opengames.back()->address = addr;
			DIAG_ON("-Wold-style-cast")
			opengames.back()->info = info;
			callback(GameOpened, opengames.back(), userdata);
			break;
		}
	}
}

void LanGameFinder::set_callback(void (*const cb)(int32_t, NetOpenGame const*, void*),
                                 void* const ud) {
	callback = cb;
	userdata = ud;
}
