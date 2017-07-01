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

#include "network/network.h"

#include <SDL.h>

#include "base/log.h"

namespace {

bool do_resolve(const boost::asio::ip::tcp& protocol,
                NetAddress* addr,
                const std::string& hostname,
                uint16_t port) {
	assert(addr != nullptr);
	try {
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::resolver resolver(io_service);
		boost::asio::ip::tcp::resolver::query query(
		   protocol, hostname, boost::lexical_cast<std::string>(port));
		boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
		if (iter == boost::asio::ip::tcp::resolver::iterator()) {
			// Resolution failed
			return false;
		}
		addr->ip = iter->endpoint().address();
		addr->port = port;
		return true;
	} catch (const boost::system::system_error& ec) {
		// Resolution failed
		log("Could not resolve network name: %s\n", ec.what());
		return false;
	}
}
}

bool NetAddress::resolve_to_v4(NetAddress* addr, const std::string& hostname, uint16_t port) {
	return do_resolve(boost::asio::ip::tcp::v4(), addr, hostname, port);
}

bool NetAddress::resolve_to_v6(NetAddress* addr, const std::string& hostname, uint16_t port) {
	return do_resolve(boost::asio::ip::tcp::v6(), addr, hostname, port);
}

bool NetAddress::parse_ip(NetAddress* addr, const std::string& ip, uint16_t port) {
	boost::system::error_code ec;
	boost::asio::ip::address new_addr = boost::asio::ip::address::from_string(ip, ec);
	if (ec)
		return false;
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

CmdNetCheckSync::CmdNetCheckSync(uint32_t const dt, SyncCallback* const cb)
   : Command(dt), callback_(cb) {
}

void CmdNetCheckSync::execute(Widelands::Game&) {
	callback_->syncreport();
}

NetworkTime::NetworkTime() {
	reset(0);
}

void NetworkTime::reset(int32_t const ntime) {
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
	if (delta < 0)
		delta = 0;
	else if (delta > 1000)
		delta = 1000;

	delta = (delta * speed) / 1000;

	int32_t const behind = networktime_ - time_;

	// Play catch up
	uint32_t speedup = 0;
	if (latency_ > static_cast<uint32_t>(10 * delta))
		//  just try to kill as much of the latency as possible if we are that
		//  far behind
		speedup = latency_ / 3;
	else if (latency_ > static_cast<uint32_t>(delta))
		speedup = delta / 8;  //  speed up by 12.5%
	if (static_cast<int32_t>(delta + speedup) > behind)
		speedup = behind - delta;

	delta += speedup;
	latency_ -= speedup;

	if (delta > behind)
		delta = behind;

	time_ += delta;
}

int32_t NetworkTime::time() const {
	return time_;
}

int32_t NetworkTime::networktime() const {
	return networktime_;
}

void NetworkTime::receive(int32_t const ntime) {
	if (ntime < networktime_)
		throw wexception("NetworkTime: Time appears to be running backwards.");

	uint32_t const behind = networktime_ - time_;

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

SendPacket::SendPacket() {
}

void SendPacket::data(const void* const packet_data, const size_t size) {
	if (buffer.empty()) {
		buffer.push_back(0);  //  this will finally be the length of the packet
		buffer.push_back(0);
	}

	for (size_t idx = 0; idx < size; ++idx)
		buffer.push_back(static_cast<const uint8_t*>(packet_data)[idx]);
}

void SendPacket::reset() {
	buffer.clear();
}

size_t SendPacket::get_size() const {
	return buffer.size();
}

uint8_t* SendPacket::get_data() const {

	uint32_t const length = buffer.size();

	assert(length < 0x10000);

	// update packet length
	buffer[0] = length >> 8;
	buffer[1] = length & 0xFF;

	return &(buffer[0]);
}

/*** class RecvPacket ***/

size_t RecvPacket::data(void* const packet_data, size_t const bufsize) {
	if (index_ + bufsize > buffer.size())
		throw wexception("Packet too short");

	for (size_t read = 0; read < bufsize; ++read)
		static_cast<uint8_t*>(packet_data)[read] = buffer[index_++];

	return bufsize;
}

bool RecvPacket::end_of_file() const {
	return index_ < buffer.size();
}

void Deserializer::read_data(const uint8_t* data, const int32_t len) {

	queue_.insert(queue_.end(), &data[0], &data[len]);
}

bool Deserializer::write_packet(RecvPacket* packet) {
	// No data at all
	if (queue_.size() < 2)
		return false;

	uint16_t const size = queue_[0] << 8 | queue_[1];
	assert(size >= 2);

	// Not enough data for a complete packet
	if (queue_.size() < static_cast<size_t>(size))
		return false;

	packet->buffer.clear();
	packet->buffer.insert(packet->buffer.end(), queue_.begin() + 2, queue_.begin() + size);
	packet->index_ = 0;

	queue_.erase(queue_.begin(), queue_.begin() + size);
	return true;
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
