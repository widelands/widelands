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

#include "network/network_lan_promotion.h"

#ifndef _WIN32
#include <ifaddrs.h>
#endif

#include <memory>

#include "base/i18n.h"
#include "base/log.h"
#include "base/warning.h"
#include "build_info.h"
#include "network/constants.h"

namespace {

/**
 * Returns the IP version.
 * \param addr The address object to get the IP version for.
 * \return Either 4 or 6, depending on the version of the given address.
 */
int get_ip_version(const asio::ip::address& addr) {
	assert(!addr.is_unspecified());
	if (addr.is_v4()) {
		return 4;
	} else {
		assert(addr.is_v6());
		return 6;
	}
}

/**
 * Returns the IP version.
 * \param version A whatever object to get the IP version for.
 * \return Either 4 or 6, depending on the version of the given address.
 */
int get_ip_version(const asio::ip::udp& version) {
	if (version == asio::ip::udp::v4()) {
		return 4;
	} else {
		assert(version == asio::ip::udp::v6());
		return 6;
	}
}
}  // namespace

/*** class LanBase ***/
/**
 * [Internal] In an ideal world, we would use the same code with asio for all three operating
 * systems. Unfortunately, it isn't that easy and we need some platform specific code. For IPv4,
 * windows needs a special case: For Linux and Apple we have to iterate over all assigned IPv4
 * addresses (e.g. 192.168.1.68), transform them to broadcast addresses (e.g. 192.168.1.255) and
 * send our
 * packets to those addresses. For windows, we simply can send to 255.255.255.255.
 * For IPv6, Apple requires special handling. On the other two operating systems we can send to the
 * multicast
 * address ff02::1 (kind of a local broadcast) without specifying over which interface we want to
 * send.
 * On Apple we have to specify the interface, forcing us to send our message over all interfaces we
 * can find.
 */
LanBase::LanBase(uint16_t port) : io_service(), socket_v4(io_service), socket_v6(io_service) {

#ifndef _WIN32
	// Iterate over all interfaces. If they support IPv4, store the broadcast-address
	// of the interface and try to start the socket. If they support IPv6, just start
	// the socket. There is one fixed broadcast-address for IPv6 (well, actually multicast)

	// Adapted example out of "man getifaddrs"
	// TODO(Notabilis): I don't like this part. But asio is not able to iterate over
	// the local IPs and interfaces at this time. If they ever add it, replace this code
	struct ifaddrs *ifaddr, *ifa;
	int s, n;
	char host[NI_MAXHOST];
	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		exit(EXIT_FAILURE);
	}
	for (ifa = ifaddr, n = 0; ifa != nullptr; ifa = ifa->ifa_next, n++) {
		if (ifa->ifa_addr == nullptr) {
			continue;
		}
		if (!(ifa->ifa_flags & IFF_LOOPBACK) && !(ifa->ifa_flags & IFF_BROADCAST) &&
		    !(ifa->ifa_flags & IFF_MULTICAST)) {
			continue;
		}
		switch (ifa->ifa_addr->sa_family) {
		case AF_INET:
			s = getnameinfo(ifa->ifa_broadaddr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr,
			                0, NI_NUMERICHOST);
			if (s == 0) {
				start_socket(&socket_v4, asio::ip::udp::v4(), port);
				broadcast_addresses_v4.insert(host);
			}
			break;
		case AF_INET6:
#ifdef __APPLE__
			interface_indices_v6.insert(if_nametoindex(ifa->ifa_name));
#endif
			start_socket(&socket_v6, asio::ip::udp::v6(), port);
			// No address to store here. There is only one "broadcast" address for IPv6
			break;
		}
	}
	freeifaddrs(ifaddr);

#else
	//  As Microsoft does not seem to support if_nameindex, we just broadcast to
	//  INADDR_BROADCAST.
	broadcast_addresses_v4.insert("255.255.255.255");
#endif

	if (!is_open()) {
		// Hm, not good. Just try to open them and hope for the best
		verb_log_info("[LAN] Trying to open both sockets.");
		start_socket(&socket_v4, asio::ip::udp::v4(), port);
		start_socket(&socket_v6, asio::ip::udp::v6(), port);
	}

	if (!is_open()) {
		// Still not open? Go back to main menu.
		log_err("[LAN] Error: No sockets could be opened.\n");
		report_network_error();
	}

	for (const std::string& ip : broadcast_addresses_v4) {
		verb_log_info("[LAN] Will broadcast to %s.", ip.c_str());
	}
	if (socket_v6.is_open()) {
		verb_log_info("[LAN] Will broadcast for IPv6.");
	}
}

LanBase::~LanBase() {
	close_socket(&socket_v4);
	close_socket(&socket_v6);
}

bool LanBase::is_available() {
	const auto do_is_available = [this](asio::ip::udp::socket& socket) -> bool {
		std::error_code ec;
		bool available = (socket.is_open() && socket.available(ec) > 0);
		if (ec) {
			log_err("[LAN] Error when checking whether data is available on IPv%d socket, closing it: "
			        "%s.\n",
			        get_ip_version(socket.local_endpoint().protocol()), ec.message().c_str());
			close_socket(&socket);
			return false;
		}
		return available;
	};

	return do_is_available(socket_v4) || do_is_available(socket_v6);
}

bool LanBase::is_open() {
	return socket_v4.is_open() || socket_v6.is_open();
}

size_t LanBase::receive(void* const buf, size_t const len, NetAddress* addr) {
	assert(buf != nullptr);
	assert(addr != nullptr);
	size_t recv_len = 0;

	const auto do_receive = [this, &buf, &len, &recv_len,
	                         addr](asio::ip::udp::socket& socket) -> bool {
		if (socket.is_open()) {
			try {
				if (socket.available() > 0) {
					asio::ip::udp::endpoint sender_endpoint;
					recv_len = socket.receive_from(asio::buffer(buf, len), sender_endpoint);
					*addr = NetAddress{sender_endpoint.address(), sender_endpoint.port()};
					assert(recv_len <= len);
					return true;
				}
			} catch (const std::system_error& ec) {
				// Some network error. Close the socket
				log_err("[LAN] Error when receiving data on IPv%d socket, closing it: %s.\n",
				        get_ip_version(socket.local_endpoint().protocol()), ec.what());
				close_socket(&socket);
			}
		}
		// Nothing received
		return false;
	};

	// Try to receive something somewhere
	if (!do_receive(socket_v4)) {
		do_receive(socket_v6);
	}

	// Return how much has been received, might be 0
	return recv_len;
}

bool LanBase::send(void const* const buf, size_t const len, const NetAddress& addr) {
	std::error_code ec;
	assert(addr.is_valid());
	// If this assert failed, then there is some bug in the code. NetAddress should only be filled
	// with valid IP addresses (e.g. no hostnames)
	assert(!ec);
	asio::ip::udp::endpoint destination(addr.ip, addr.port);
	asio::ip::udp::socket* socket = nullptr;
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
		log_err("[LAN] Error: trying to send to an IPv%d address but socket is not open.\n",
		        get_ip_version(addr.ip));
		return false;
	}
	socket->send_to(asio::buffer(buf, len), destination, 0, ec);
	if (ec) {
		log_err("[LAN] Error when trying to send something over IPv%d, closing socket: %s.\n",
		        get_ip_version(addr.ip), ec.message().c_str());
		close_socket(socket);
		return false;
	}
	return true;
}

bool LanBase::broadcast(void const* const buf, size_t const len, uint16_t const port) {

	const auto do_broadcast = [this, buf, len, port](
	                             asio::ip::udp::socket& socket, const std::string& address) -> bool {
		if (socket.is_open()) {
			std::error_code ec;
			asio::ip::udp::endpoint destination(asio::ip::address::from_string(address), port);
			socket.send_to(asio::buffer(buf, len), destination, 0, ec);
			if (!ec) {
				return true;
			}
#ifdef __APPLE__
			if (get_ip_version(destination.address()) == 4) {
#endif  // __APPLE__
				log_err("[LAN] Error when broadcasting on IPv%d socket to %s, closing it: %s.\n",
				        get_ip_version(destination.address()), address.c_str(), ec.message().c_str());
				close_socket(&socket);
#ifdef __APPLE__
			} else {
				log_err("[LAN] Error when broadcasting on IPv6 socket to %s: %s.\n", address.c_str(),
				        ec.message().c_str());
			}
#endif  // __APPLE__
		}
		return false;
	};

	bool one_success = false;

	// IPv4 broadcasting is the same for all
	for (const std::string& address : broadcast_addresses_v4) {
		one_success |= do_broadcast(socket_v4, address);
	}
#ifndef __APPLE__
	// For IPv6 on Linux and Windows just send on an undefined network interface
	one_success |= do_broadcast(socket_v6, "ff02::1");
#else   // __APPLE__

	// Apple forces us to define which interface we want to send through
	for (auto it = interface_indices_v6.begin(); it != interface_indices_v6.end();) {
		socket_v6.set_option(asio::ip::multicast::outbound_interface(*it));
		bool success = do_broadcast(socket_v6, "ff02::1");
		one_success |= success;
		if (!success) {
			// Remove this interface id from the set
			it = interface_indices_v6.erase(it);
			if (interface_indices_v6.empty()) {
				log_warn("[LAN] Warning: No more multicast capable IPv6 interfaces. "
				         "Other LAN players won't find your game.\n");
			}
		} else {
			++it;
		}
	}
#endif  // __APPLE__
	return one_success;
}

void LanBase::start_socket(asio::ip::udp::socket* socket, asio::ip::udp version, uint16_t port) {

	if (socket->is_open()) {
		return;
	}

	std::error_code ec;
	// Try to open the socket
	socket->open(version, ec);
	if (ec) {
		log_err("[LAN] Failed to start an IPv%d socket: %s.\n", get_ip_version(version),
		        ec.message().c_str());
		return;
	}

	const asio::socket_base::broadcast option_broadcast(true);
	socket->set_option(option_broadcast, ec);
	if (ec) {
		log_err("[LAN] Error setting options for IPv%d socket, closing socket: %s.\n",
		        get_ip_version(version), ec.message().c_str());
		// Retrieve the error code to avoid throwing but ignore it
		close_socket(socket);
		return;
	}

	const asio::socket_base::reuse_address option_reuse(true);
	socket->set_option(option_reuse, ec);
	// This one isn't really needed so ignore the error

	if (version == asio::ip::udp::v6()) {
		const asio::ip::v6_only option_v6only(true);
		socket->set_option(option_v6only, ec);
		// This one might not be needed, ignore the error and see whether we fail on bind()
	}

	socket->bind(asio::ip::udp::endpoint(version, port), ec);
	if (ec) {
		log_err("[LAN] Error binding IPv%d socket to UDP port %d, closing socket: %s.\n",
		        get_ip_version(version), port, ec.message().c_str());
		close_socket(socket);
		return;
	}

	verb_log_info("[LAN] Started an IPv%d socket on UDP port %d.", get_ip_version(version), port);
}

void LanBase::report_network_error() {
	// No socket open? Sorry, but we can't continue this way
	const std::vector<std::string> ports_list({as_string(kWidelandsLanDiscoveryPort),
	                                           as_string(kWidelandsLanPromotionPort),
	                                           as_string(kWidelandsLanPort)});

	throw WLWarning(_("Failed to use the local network!"),
	                /** TRANSLATORS: %s is a list of alternative ports with "or" */
	                _("Widelands was unable to use the local network. "
	                  "Maybe some other process is already running a server on port %s "
	                  "or your network setup is broken."),
	                i18n::localize_list(ports_list, i18n::ConcatenateWith::OR).c_str());
}

void LanBase::close_socket(asio::ip::udp::socket* socket) {
	std::error_code ec;
	if (socket->is_open()) {
		const asio::ip::udp::endpoint& endpoint = socket->local_endpoint(ec);
		if (!ec) {
			verb_log_info("[LAN] Closing an IPv%d socket.", get_ip_version(endpoint.protocol()));
		}
		socket->shutdown(asio::ip::udp::socket::shutdown_both, ec);
		socket->close(ec);
	}
}

/*** class LanGamePromoter ***/

LanGamePromoter::LanGamePromoter() : LanBase(kWidelandsLanPromotionPort) {

	needupdate = true;

	memset(&gameinfo, 0, sizeof(gameinfo));
	strncpy(gameinfo.magic, "GAME", sizeof(gameinfo.magic));

	gameinfo.version = LAN_PROMOTION_PROTOCOL_VERSION;
	gameinfo.state = LAN_GAME_OPEN;

	strncpy(gameinfo.gameversion, build_id().c_str(), sizeof(gameinfo.gameversion) - 1);
	gameinfo.gameversion[sizeof(gameinfo.gameversion) - 1] = '\0';

	strncpy(gameinfo.hostname, asio::ip::host_name().c_str(), sizeof(gameinfo.hostname) - 1);
	gameinfo.hostname[sizeof(gameinfo.hostname) - 1] = '\0';
}

LanGamePromoter::~LanGamePromoter() {
	gameinfo.state = LAN_GAME_CLOSED;

	// Don't care about errors at this point
	broadcast(&gameinfo, sizeof(gameinfo), kWidelandsLanDiscoveryPort);
}

void LanGamePromoter::run() {
	if (needupdate) {
		needupdate = false;

		if (!broadcast(&gameinfo, sizeof(gameinfo), kWidelandsLanDiscoveryPort)) {
			report_network_error();
		}
	}

	while (is_available()) {
		char magic[8];
		NetAddress addr;

		if (receive(magic, 8, &addr) < 8) {
			continue;
		}

		verb_log_info("Received %s packet from %s", magic, addr.ip.to_string().c_str());

		if (!strncmp(magic, "QUERY", 6) && magic[6] == LAN_PROMOTION_PROTOCOL_VERSION) {
			if (!send(&gameinfo, sizeof(gameinfo), addr)) {
				report_network_error();
			}
		}
	}
}

void LanGamePromoter::set_map(char const* map) {
	strncpy(gameinfo.map, map, sizeof(gameinfo.map) - 1);
	gameinfo.map[sizeof(gameinfo.map) - 1] = '\0';

	needupdate = true;
}

/*** class LanGameFinder ***/

LanGameFinder::LanGameFinder() : LanBase(kWidelandsLanDiscoveryPort), callback(nullptr) {

	reset();
}

void LanGameFinder::reset() {
	char magic[8];

	opengames.clear();

	strncpy(magic, "QUERY", 8);
	magic[6] = LAN_PROMOTION_PROTOCOL_VERSION;

	if (!broadcast(magic, 8, kWidelandsLanPromotionPort)) {
		report_network_error();
	}
}

void LanGameFinder::run() {
	while (is_available()) {
		NetGameInfo info;
		NetAddress addr;

		if (receive(&info, sizeof(info), &addr) < sizeof(info)) {
			continue;
		}

		verb_log_info("Received %s packet from %s", info.magic, addr.ip.to_string().c_str());

		if (strncmp(info.magic, "GAME", 6) != 0 || info.version != LAN_PROMOTION_PROTOCOL_VERSION) {
			continue;
		}

		// Make sure that the callback function has been set before we do any callbacks
		if (!callback) {
			continue;
		}

		//  if the game already is in the list, update the information
		//  otherwise just append it to the list
		bool was_in_list = false;
		for (const auto& opengame : opengames) {
			if (0 == strncmp(opengame->info.hostname, info.hostname, 128)) {
				opengame->info = info;
				if (!opengame->address.is_ipv6() && addr.is_ipv6()) {
					opengame->address.ip = addr.ip;
				}
				callback(GameUpdated, opengame.get(), userdata);
				was_in_list = true;
				break;
			}
		}

		if (!was_in_list) {
			addr.port = kWidelandsLanPort;
			opengames.push_back(std::unique_ptr<NetOpenGame>(new NetOpenGame(addr, info)));
			callback(GameOpened, opengames.back().get(), userdata);
			break;
		}
	}
}

void LanGameFinder::set_callback(void (*const cb)(int32_t, const NetOpenGame* const, void*),
                                 void* const ud) {
	callback = cb;
	userdata = ud;
}
