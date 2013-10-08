/*
 * Copyright (C) 2004-2009 by the Widelands Development Team
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

#include "log.h"
#include "wlapplication.h"



Cmd_NetCheckSync::Cmd_NetCheckSync(int32_t const dt, SyncCallback * const cb) :
Command (dt), m_callback(cb)
{}


void Cmd_NetCheckSync::execute (Widelands::Game &) {
	m_callback->syncreport();
}


NetworkTime::NetworkTime()
{
	reset(0);
}

void NetworkTime::reset(int32_t const ntime)
{
	m_networktime = m_time = ntime;
	m_lastframe = WLApplication::get()->get_time();
	m_latency = 0;
}

void NetworkTime::fastforward()
{
	m_time = m_networktime;
	m_lastframe = WLApplication::get()->get_time();
}

void NetworkTime::think(uint32_t const speed)
{
	int32_t const curtime = WLApplication::get()->get_time();
	int32_t delta = curtime - m_lastframe;
	m_lastframe = curtime;

	// in case weird things are happening with the system time
	// (e.g. debugger, extremely slow simulation, ...)
	if (delta < 0)
		delta = 0;
	else if (delta > 1000)
		delta = 1000;

	delta = (delta * speed) / 1000;

	int32_t const behind = m_networktime - m_time;

	// Play catch up
	uint32_t speedup = 0;
	if (m_latency > static_cast<uint32_t>(10 * delta))
		//  just try to kill as much of the latency as possible if we are that
		//  far behind
		speedup = m_latency / 3;
	else if (m_latency > static_cast<uint32_t>(delta))
		speedup = delta / 8; //  speed up by 12.5%
	if (static_cast<int32_t>(delta + speedup) > behind)
		speedup = behind - delta;

	delta += speedup;
	m_latency -= speedup;

	if (delta > behind)
		delta = behind;

	m_time += delta;
}

int32_t NetworkTime::time() const
{
	return m_time;
}

int32_t NetworkTime::networktime() const
{
	return m_networktime;
}

void NetworkTime::recv(int32_t const ntime)
{
	if (ntime < m_networktime)
		throw wexception("NetworkTime: Time appears to be running backwards.");

	uint32_t const behind = m_networktime - m_time;

	m_latency = behind < m_latency ? behind : ((m_latency * 7) + behind) / 8;

#if 0
	log
		("NetworkTime: New networktime %i (local time %i), behind %i, latency "
		 "%u\n",
		 ntime, m_time, m_networktime - m_time, m_latency);
#endif

	m_networktime = ntime;
}



/*** class SendPacket ***/

SendPacket::SendPacket () {}

void SendPacket::Data(const void * const data, const size_t size)
{
	if (buffer.empty()) {
		buffer.push_back (0); //  this will finally be the length of the packet
		buffer.push_back (0);
	}
	for (size_t idx = 0; idx < size; ++idx)
		buffer.push_back(static_cast<const uint8_t *>(data)[idx]);
}

void SendPacket::send (TCPsocket sock)
{
	uint32_t const length = buffer.size();

	assert (length < 0x10000);

	// update packet length
	buffer[0] = length >> 8;
	buffer[1] = length & 0xFF;

	if (sock)
		SDLNet_TCP_Send (sock, &(buffer[0]), buffer.size());
}

void SendPacket::reset ()
{
	buffer.clear();
}


/*** class RecvPacket ***/

RecvPacket::RecvPacket (Deserializer & des)
{
	uint16_t const size = des.queue[0] << 8 | des.queue[1];

	// The following should be caught by Deserializer::read and ::avail
	assert(des.queue.size() >= static_cast<size_t>(size));
	assert(size >= 2);

	buffer.insert(buffer.end(), des.queue.begin() + 2, des.queue.begin() + size);
	m_index = 0;

	des.queue.erase(des.queue.begin(), des.queue.begin() + size);
}

size_t RecvPacket::Data(void * const data, size_t const bufsize)
{
	if (m_index + bufsize > buffer.size())
		throw wexception("Packet too short");

	for (size_t read = 0; read < bufsize; ++read)
		static_cast<uint8_t *>(data)[read] = buffer[m_index++];

	return bufsize;
}

bool RecvPacket::EndOfFile() const
{
	return m_index < buffer.size();
}

bool Deserializer::read(TCPsocket sock)
{
	uint8_t buffer[512];
	const int32_t bytes = SDLNet_TCP_Recv(sock, buffer, sizeof(buffer));
	if (bytes <= 0)
		return false;

	queue.insert(queue.end(), &buffer[0], &buffer[bytes]);

	return queue.size() < 2 or 2 <= (queue[0] << 8 | queue[1]);
}

/**
 * Returns true if an entire packet is available
 */
bool Deserializer::avail() const
{
	if (queue.size() < 2)
		return false;

	const uint16_t size = queue[0] << 8 | queue[1];
	if (size < 2)
		return false;

	return queue.size() >= static_cast<size_t>(size);
}


DisconnectException::DisconnectException(const char * fmt, ...)
{
	char buffer[512];
	{
		va_list va;
		va_start(va, fmt);
		vsnprintf(buffer, sizeof(buffer), fmt, va);
		va_end(va);
	}
	m_what = buffer;
}

DisconnectException::~DisconnectException() throw ()
{
}

char const * DisconnectException::what() const throw ()
{
	return m_what.c_str();
}
