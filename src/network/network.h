/*
 * Copyright (C) 2004-2008, 2012 by the Widelands Development Team
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

#ifndef NETWORK_H
#define NETWORK_H

#include <exception>
#include <string>
#include <vector>

#include <SDL_net.h>

#include "logic/cmd_queue.h"
#include "logic/widelands_streamread.h"
#include "logic/widelands_streamwrite.h"
#include "network/network_protocol.h"


class Deserializer;

struct SyncCallback {
	virtual ~SyncCallback() {}
	virtual void syncreport() = 0;
};


/**
 * This non-gamelogic command is used by \ref NetHost and \ref NetClient
 * to schedule taking a synchronization hash.
 */
struct Cmd_NetCheckSync : public Widelands::Command {
	Cmd_NetCheckSync (int32_t dt, SyncCallback *);

	virtual void execute (Widelands::Game &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_NETCHECKSYNC;}

private:
	SyncCallback * m_callback;
};


/**
 * Keeping track of network time: This class answers the question of how
 * far the local simulation time should proceed, given the history of network
 * time messages forwarded to the \ref recv() method.
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
	void recv(int32_t ntime);

private:
	int32_t m_networktime;
	int32_t m_time;

	int32_t m_lastframe;

	/// This is an attempt to measure how far behind the network time we are.
	uint32_t m_latency;
};


/**
 * Buffered StreamWrite object for assembling a packet that will be
 * sent via the \ref send() function.
 */
struct SendPacket : public Widelands::StreamWrite {
	SendPacket ();

	void send (TCPsocket);
	void reset ();

	void Data(void const * data, size_t size) override;

private:
	std::vector<uint8_t> buffer;
};


/**
 * One packet, as received by the deserializer.
 */
struct RecvPacket : public Widelands::StreamRead {
public:
	RecvPacket(Deserializer &);

	size_t Data(void * data, size_t bufsize) override;
	bool EndOfFile() const override;

private:
	std::vector<uint8_t> buffer;
	size_t m_index;
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
	 * Read data from the given socket.
	 * \return \c false if the socket was disconnected or another error
	 * occurred.
	 * \c true if some data could be read (this does not imply that \ref avail
	 * will return \c true !)
	 */
	bool read(TCPsocket sock);

	/**
	 * \return \c true if an entire packet has been received.
	 */
	bool avail() const;

private:
	friend struct RecvPacket;
	std::vector<uint8_t> queue;
};


/**
 * This exception is used internally during protocol handling to indicate
 * that the connection should be terminated with a reasonable error message.
 *
 * If the network handler catches a different exception from std::exception,
 * it assumes that it is due to malformed data sent by the server.
 */
struct DisconnectException : public std::exception {
	explicit DisconnectException
		(const char * fmt, ...)
	 PRINTF_FORMAT(2, 3);
	virtual ~DisconnectException() throw ();

	virtual const char * what() const throw () override;

private:
	std::string m_what;
};

/**
 * This exception is used internally during protocol handling to indicate that the connection
 * should be terminated because an unexpected message got received that is disallowed by the protocol.
 */
struct ProtocolException : public std::exception {
	explicit ProtocolException(uint8_t code) {m_what = code;}
	virtual ~ProtocolException() throw () {}

	/// do NOT use!!! This exception shall only return the command number of the received message
	/// via \ref ProtocolException:number()
	virtual const char * what() const throw () override {assert(false); return "dummy";}

	/// \returns the command number of the received message
	virtual int          number() const {return m_what;}
private:
	// no uint8_t, as lexical_cast does not support that format
	int m_what;
};

#endif
