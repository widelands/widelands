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

#ifndef WL_NETWORK_NETWORK_H
#define WL_NETWORK_NETWORK_H

#include <exception>
#include <string>
#include <vector>

#include <SDL_net.h>
#include <boost/lexical_cast.hpp>

#include "base/wexception.h"
#include "io/streamread.h"
#include "io/streamwrite.h"
#include "logic/cmd_queue.h"
#include "network/network_protocol.h"

class Deserializer;
class FileRead;

struct SyncCallback {
	virtual ~SyncCallback() {
	}
	virtual void syncreport() = 0;
};

/**
 * This non-gamelogic command is used by \ref GameHost and \ref GameClient
 * to schedule taking a synchronization hash.
 */
struct CmdNetCheckSync : public Widelands::Command {
	CmdNetCheckSync(uint32_t dt, SyncCallback*);

	void execute(Widelands::Game&) override;

	Widelands::QueueCommandTypes id() const override {
		return Widelands::QueueCommandTypes::kNetCheckSync;
	}

private:
	SyncCallback* callback_;
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

	void reset(int32_t ntime);
	void fastforward();

	void think(uint32_t speed);
	int32_t time() const;
	int32_t networktime() const;
	void receive(int32_t ntime);

private:
	int32_t networktime_;
	int32_t time_;

	uint32_t lastframe_;

	/// This is an attempt to measure how far behind the network time we are.
	uint32_t latency_;
};

/**
 * Buffered StreamWrite object for assembling a packet that will be
 * sent over the network.
 */
struct SendPacket : public StreamWrite {
	SendPacket();

	void reset();

	void data(void const* data, size_t size) override;

	size_t get_size() const;

	uint8_t* get_data() const;

private:
	// First two bytes are overwritten on call to get_data()
	mutable std::vector<uint8_t> buffer;
};

/**
 * One packet, as received by the deserializer.
 */
struct RecvPacket : public StreamRead {
public:
	size_t data(void* data, size_t bufsize) override;
	bool end_of_file() const override;

private:
	friend struct Deserializer;
	std::vector<uint8_t> buffer;
	size_t index_;
};

struct FilePart {
	char part[NETFILEPARTSIZE];
};

struct NetTransferFile {
	uint32_t bytes;
	std::string filename;
	std::string md5sum;
	std::vector<FilePart> parts;
};

class Deserializer {
public:
	/**
	 * Adds the given data to the internal buffer.
	 */
	void read_data(const uint8_t *data, const int32_t len);

	/**
	 * \param packet The packet to fill with the received data.
	 * \return \c true if an entire packet has been received and written to the given packet.
	 */
	bool write_packet(RecvPacket *packet);

private:
	std::vector<uint8_t> queue_;
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
	explicit ProtocolException(uint8_t code)
	   : what_(boost::lexical_cast<std::string>(static_cast<unsigned int>(code))) {
	}

	/// \returns the command number of the received message
	const char* what() const noexcept override {
		return what_.c_str();
	}

private:
	const std::string what_;
};

#endif  // end of include guard: WL_NETWORK_NETWORK_H
