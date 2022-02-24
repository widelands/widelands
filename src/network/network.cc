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

#include "network/network.h"

#include <SDL_timer.h>

#include "base/log.h"

namespace {

bool do_resolve(const asio::ip::tcp& protocol,
                NetAddress* addr,
                const std::string& hostname,
                uint16_t port) {
	assert(addr != nullptr);
	try {
		asio::io_service io_service;
		asio::ip::tcp::resolver resolver(io_service);
		asio::ip::tcp::resolver::query query(protocol, hostname, as_string(port));
		asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
		if (iter == asio::ip::tcp::resolver::iterator()) {
			// Resolution failed
			log_err("Could not resolve network name '%s:%u' to %s-address\n", hostname.c_str(), port,
			        ((protocol == asio::ip::tcp::v4()) ? "IPv4" : "IPv6"));
			return false;
		}
		addr->ip = iter->endpoint().address();
		addr->port = port;
		verb_log_info("Resolved network name '%s:%u' to %s", hostname.c_str(), port,
		              addr->ip.to_string().c_str());
		return true;
	} catch (const std::system_error& ec) {
		// Resolution failed
		log_err("Could not resolve network name '%s:%u' to %s-address: %s\n", hostname.c_str(), port,
		        ((protocol == asio::ip::tcp::v4()) ? "IPv4" : "IPv6"), ec.what());
		return false;
	}
}
}  // namespace

bool NetAddress::resolve_to_v4(NetAddress* addr, const std::string& hostname, uint16_t port) {
	return do_resolve(asio::ip::tcp::v4(), addr, hostname, port);
}

bool NetAddress::resolve_to_v6(NetAddress* addr, const std::string& hostname, uint16_t port) {
	return do_resolve(asio::ip::tcp::v6(), addr, hostname, port);
}

bool NetAddress::parse_ip(NetAddress* addr, const std::string& ip, uint16_t port) {
	std::error_code ec;
	asio::ip::address new_addr = asio::ip::address::from_string(ip, ec);
	if (ec) {
		return false;
	}
	addr->ip = new_addr;
	addr->port = port;
	return true;
}

bool NetAddress::is_ipv6() const {
	return ip.is_v6();
}

bool NetAddress::is_valid() const {
	return port != 0 && !ip.is_unspecified();
}

CmdNetCheckSync::CmdNetCheckSync(const Time& dt, SyncReportCallback cb)
   : Command(dt), callback_(std::move(cb)) {
}

void CmdNetCheckSync::execute(Widelands::Game& /* game */) {
	callback_();
}

NetworkTime::NetworkTime() {
	reset(Time(0));
}

void NetworkTime::reset(const Time& ntime) {
	networktime_ = time_ = ntime;
	lastframe_ = SDL_GetTicks();
	latency_ = 0;
}

void NetworkTime::fastforward() {
	time_ = networktime_;
	lastframe_ = SDL_GetTicks();
}

void NetworkTime::think(uint32_t const speed) {
	uint32_t const curtime = SDL_GetTicks();
	int32_t delta = curtime - lastframe_;
	lastframe_ = curtime;

	// in case weird things are happening with the system time
	// (e.g. debugger, extremely slow simulation, ...)
	if (delta < 0) {
		delta = 0;
	} else if (delta > 1000) {
		delta = 1000;
	}

	delta = (delta * speed) / 1000;

	int32_t const behind = networktime_.get() - time_.get();

	// Play catch up
	uint32_t speedup = 0;
	if (latency_ > static_cast<uint32_t>(10 * delta)) {
		//  just try to kill as much of the latency as possible if we are that
		//  far behind
		speedup = latency_ / 3;
	} else if (latency_ > static_cast<uint32_t>(delta)) {
		speedup = delta / 8;  //  speed up by 12.5%
	}
	if (static_cast<int32_t>(delta + speedup) > behind) {
		speedup = behind - delta;
	}

	delta += speedup;
	latency_ -= speedup;

	if (delta > behind) {
		delta = behind;
	}

	time_.increment(Duration(delta));
}

const Time& NetworkTime::time() const {
	return time_;
}

const Time& NetworkTime::networktime() const {
	return networktime_;
}

void NetworkTime::receive(const Time& ntime) {
	if (ntime < networktime_) {
		throw wexception("NetworkTime: Time appears to be running backwards.");
	}

	uint32_t const behind = networktime_.get() - time_.get();

	latency_ = behind < latency_ ? behind : ((latency_ * 7) + behind) / 8;

#if 0
	log
		("NetworkTime: New networktime %i (local time %i), behind %i, latency "
		 "%u\n",
		 ntime, time_, networktime_ - time_, latency_);
#endif

	networktime_ = ntime;
}

/*** class SendPacket ***/

void SendPacket::data(const void* const packet_data, const size_t size) {
	if (buffer.empty()) {
		buffer.push_back(0);  //  this will finally be the length of the packet
		buffer.push_back(0);
		// Attention! These bytes are also used by the network relay protocol.
		// So if they are removed the protocol has to be updated
	}

	for (size_t idx = 0; idx < size; ++idx) {
		buffer.push_back(static_cast<const uint8_t*>(packet_data)[idx]);
	}
}

void SendPacket::reset() {
	buffer.clear();
}

size_t SendPacket::get_size() const {
	return buffer.size();
}

uint8_t* SendPacket::get_data() const {

	uint32_t const length = buffer.size();

	assert(length >= 2);
	assert(length < 0x10000);

	// update packet length
	buffer[0] = length >> 8;
	buffer[1] = length & 0xFF;

	return &(buffer[0]);
}

/*** class RecvPacket ***/
size_t RecvPacket::data(void* const packet_data, size_t const bufsize) {
	if (index_ + bufsize > buffer.size()) {
		throw wexception("Packet too short");
	}

	for (size_t read = 0; read < bufsize; ++read) {
		static_cast<uint8_t*>(packet_data)[read] = buffer[index_++];
	}

	return bufsize;
}

bool RecvPacket::end_of_file() const {
	return index_ < buffer.size();
}

DisconnectException::DisconnectException(const char* fmt, ...) {
	char buffer[kNetworkBufferSize];
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, va);
		va_end(va);
	}
	what_ = buffer;
}

char const* DisconnectException::what() const noexcept {
	return what_.c_str();
}
