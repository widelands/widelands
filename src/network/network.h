/*
 * Copyright (C) 2004-2021 by the Widelands Development Team
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

#ifndef WL_NETWORK_NETWORK_H
#define WL_NETWORK_NETWORK_H

#include <functional>

#include <boost/asio.hpp>

#include "base/string.h"
#include "base/wexception.h"
#include "io/streamread.h"
#include "io/streamwrite.h"
#include "logic/cmd_queue.h"
#include "network/network_protocol.h"

constexpr size_t kNetworkBufferSize = 512;

/**
 * Simple structure to hold the IP address and port of a server.
 * This structure must not contain a hostname but only IP addresses.
 */
struct NetAddress {
	/**
	 * Tries to resolve the given hostname to an IPv4 address.
	 * \param[out] addr A NetAddress structure to write the result to,
	 *                  if resolution succeeds.
	 * \param hostname The name of the host.
	 * \param port The port on the host.
	 * \return \c True if the resolution succeeded, \c false otherwise.
	 */
	static bool resolve_to_v4(NetAddress* addr, const std::string& hostname, uint16_t port);

	/**
	 * Tries to resolve the given hostname to an IPv6 address.
	 * \param[out] addr A NetAddress structure to write the result to,
	 *                  if resolution succeeds.
	 * \param hostname The name of the host.
	 * \param port The port on the host.
	 * \return \c True if the resolution succeeded, \c false otherwise.
	 */
	static bool resolve_to_v6(NetAddress* addr, const std::string& hostname, uint16_t port);

	/**
	 * Parses the given string to an IP address.
	 * \param[out] addr A NetAddress structure to write the result to,
	 *                  if parsing succeeds.
	 * \param ip An IP address as string.
	 * \param port The port on the host.
	 * \return \c True if the parsing succeeded, \c false otherwise.
	 */
	static bool parse_ip(NetAddress* addr, const std::string& ip, uint16_t port);

	/**
	 * Returns whether the stored IP is in IPv6 format.
	 * @return \c true if the stored IP is in IPv6 format, \c false otherwise.
	 *   If it isn't an IPv6 address, it is an IPv4 address.
	 */
	bool is_ipv6() const;

	/**
	 * Returns whether valid IP address and port are stored.
	 * @return \c true if valid, \c false otherwise.
	 */
	bool is_valid() const;

	boost::asio::ip::address ip;
	uint16_t port;
};

using SyncReportCallback = std::function<void()>;

/**
 * This non-gamelogic command is used by \ref GameHost and \ref GameClient
 * to schedule taking a synchronization hash.
 */
struct CmdNetCheckSync : public Widelands::Command {
	CmdNetCheckSync(const Time& dt, SyncReportCallback);

	void execute(Widelands::Game&) override;

	Widelands::QueueCommandTypes id() const override {
		return Widelands::QueueCommandTypes::kNetCheckSync;
	}

private:
	SyncReportCallback callback_;
};

/**
 * Keeping track of network time: This class answers the question of how
 * far the local simulation time should proceed, given the history of network
 * time messages forwarded to the \ref receive() method.
 *
 * In general, the time progresses as fast as given by the speed, but we
 * introduce some elasticity to catch up with the network time if necessary,
 * and we never advance simulation past the received network time.
 */
class NetworkTime {
public:
	NetworkTime();

	void reset(const Time& ntime);
	void fastforward();

	void think(uint32_t speed);
	const Time& time() const;
	const Time& networktime() const;
	void receive(const Time& ntime);

private:
	Time networktime_;
	Time time_;

	uint32_t lastframe_;

	/// This is an attempt to measure how far behind the network time we are.
	uint32_t latency_;
};

/**
 * Buffered StreamWrite object for assembling a packet that will be
 * sent over the network.
 */
struct SendPacket : public StreamWrite {
	SendPacket() = default;

	void reset();

	void data(void const* data, size_t size) override;

	size_t get_size() const;

	uint8_t* get_data() const;

private:
	// First two bytes are overwritten on call to get_data()
	mutable std::vector<uint8_t> buffer;
};

/**
 * One packet, as received from the network.
 */
struct RecvPacket : public StreamRead {
public:
	size_t data(void* data, size_t bufsize) override;
	bool end_of_file() const override;

private:
	friend class BufferedConnection;
	std::vector<uint8_t> buffer;
	size_t index_ = 0U;
};

struct FilePart {
	char part[NETFILEPARTSIZE];
};

struct NetTransferFile {
	NetTransferFile() : bytes(0), filename(""), md5sum("") {
	}
	~NetTransferFile() = default;

	uint32_t bytes;
	std::string filename;
	std::string md5sum;
	std::vector<FilePart> parts;
};

/**
 * This exception is used internally during protocol handling to indicate
 * that the connection should be terminated with a reasonable error message.
 *
 * If the network handler catches a different exception from std::exception,
 * it assumes that it is due to malformed data sent by the server.
 */
struct DisconnectException : public std::exception {
	explicit DisconnectException(const char* fmt, ...) PRINTF_FORMAT(2, 3);

	const char* what() const noexcept override;

private:
	std::string what_;
};

/**
 * This exception is used internally during protocol handling to indicate that the connection
 * should be terminated because an unexpected message got received that is disallowed by the
 * protocol.
 */
struct ProtocolException : public std::exception {
	explicit ProtocolException(uint8_t code) : what_(as_string(static_cast<unsigned int>(code))) {
	}

	/// \returns the command number of the received message
	const char* what() const noexcept override {
		return what_.c_str();
	}

private:
	const std::string what_;
};

/**
 * The priorities for sending data over the network when using a BufferedConnection.
 * Data with low priority values is send first even when data with high priority values
 * have been passed to a BufferedConnection first
 */
// The values assigned to the entries are arbitrary, only their order is important
// No "enum class" on purpose since this has to be interpreted as ints (I need a known ordering)
enum NetPriority : uint8_t { kPing = 10, kNormal = 50, kFiletransfer = 100 };

#endif  // end of include guard: WL_NETWORK_NETWORK_H
