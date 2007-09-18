/*
 * Copyright (C) 2002-2004, 2007 by the Widelands Development Team
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

#include "network_buffer.h"

#include "wexception.h"

#include <cassert>

Network_Buffer::Network_Buffer() {
   m_buffer_real_len = 1000;
   m_buffer_len = 0; // To correct for size entry
   m_buffer = (uchar*) malloc(1000);
   m_buffer_pointer = 0;

   put_16(0); // This will become our size
}


Network_Buffer::~Network_Buffer() {free(m_buffer);}

/*
 * finish this buffer, this basically corrects
 * the size entry
 */
void Network_Buffer::finish() {
   SDLNet_Write16(m_buffer_len-2, m_buffer);
}

/*
 * Get functions
 */
uchar Network_Buffer::get_8(bool remove) {
   uchar retval = *((uchar*)(m_buffer+m_buffer_pointer));

   if (remove)
      m_buffer_pointer += 1;
   return retval;
}

ushort Network_Buffer::get_16(bool remove) {
   ushort retval = SDLNet_Read16(m_buffer+m_buffer_pointer);

   if (remove)
      m_buffer_pointer += 2;
   return retval;
}

uint Network_Buffer::get_32(bool remove) {
   uint retval = SDLNet_Read32(m_buffer+m_buffer_pointer);

   if (remove)
      m_buffer_pointer += 4;
   return retval;
}

std::string Network_Buffer::get_string(bool remove) {
   std::string retval;

   assert(remove); // TODO: allow peeking also for string

   uint s = get_16(1);
   for (uint i=0; i<s; i++)
      retval.append(1, get_16(1));

   return retval;
}

/*
 * Put functions
 */
void Network_Buffer::put_8(uchar val) {
   const uint s = 1;

   while ((m_buffer_pointer + s) >= m_buffer_real_len)
      grow_buffer();

   *((uchar*)(m_buffer+m_buffer_pointer)) = val;

   m_buffer_pointer += s;
   m_buffer_len += s;
}

void Network_Buffer::put_16(ushort val) {
   const uint s = 2;

   while ((m_buffer_pointer + s) >= m_buffer_real_len)
      grow_buffer();

   SDLNet_Write16(val, m_buffer+m_buffer_pointer);

   m_buffer_pointer += s;
   m_buffer_len += s;
}

void Network_Buffer::put_32(uint val) {
   const uint s = 4;

   while ((m_buffer_pointer + s) >= m_buffer_real_len)
      grow_buffer();

   SDLNet_Write32(val, m_buffer+m_buffer_pointer);

   m_buffer_pointer += s;
   m_buffer_len += s;
}

void Network_Buffer::put_string(std::string string) {
   put_16(string.size());

   for (uint i=0; i<string.size(); i++) {
      put_16(string[i]);
	}
}

/*
 * Fill this m_buffer
 *
 * return -1 on error
 */
int Network_Buffer::fill(TCPsocket sock) {
   // Ok, there is data on this socket. This is assumed
	if (SDLNet_TCP_Recv(sock, m_buffer, 2) < 2) return -1; // Some error.

   // we reset the data pointer
   m_buffer_pointer = 0;

   uint s = get_16();

   // Check the s of the packet
   while ((m_buffer_pointer + s) >= m_buffer_real_len)
      grow_buffer();

   // Get the rest of the packet
   // TODO: This should have some timeout variable, otherwise
   // the game can lock here (when some, but not all data arrives)
   uint received = 0;
	while (received != s) {
      uint retval = SDLNet_TCP_Recv(sock, m_buffer+m_buffer_pointer+received, s - received);

      if (retval <= 0)
         throw wexception("Network_Buffer::fill: SDLNet_TCP_Recv brought up an error!: %s\n", SDLNet_GetError());
      received += retval;
	}

   m_buffer_len += s;
   return 0;
}

/*
 * Make the m_buffer bigger
 */
void Network_Buffer::grow_buffer() {
   m_buffer_real_len += 1000;
   m_buffer = (uchar*) realloc(m_buffer, m_buffer_real_len);
}
