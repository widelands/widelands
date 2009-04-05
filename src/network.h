/*
 * Copyright (C) 2004-2008 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <exception>

#include "cmd_queue.h"
#include "widelands_streamread.h"
#include "widelands_streamwrite.h"

#include <SDL_net.h>

struct Deserializer;

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

	virtual void execute (Widelands::Game &);

	virtual uint8_t id() const {return QUEUE_CMD_NETCHECKSYNC;}

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

	void Data(void const * data, size_t size);

private:
	std::vector<uint8_t> buffer;
};


/**
 * One packet, as received by the deserializer.
 */
struct RecvPacket : public Widelands::StreamRead {
public:
	RecvPacket(Deserializer &);

	size_t Data(void * data, size_t bufsize);
	bool EndOfFile() const;

private:
	std::vector<uint8_t> buffer;
	size_t m_index;
};

struct Deserializer {
	/**
	 * Read data from the given socket.
	 * \return \c false if the socket was disconnected or another error
	 * occurred.
	 * \c true if some data could be read (this does not imply that \ref avail
	 * will return \c true !)
	 */
	bool read (TCPsocket);

	/**
	 * \return \c true if an entire packet has been received.
	 */
	bool avail () const;

private:
	friend class RecvPacket;
	std::vector<uint8_t> queue;
	size_t index;
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
		throw () PRINTF_FORMAT(2, 3);
	virtual ~DisconnectException() throw ();

	virtual const char * what() const throw ();
private:
	std::string m_what;
};

#endif
