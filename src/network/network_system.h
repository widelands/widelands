/*
 * Copyright (C) 2004, 2008 by the Widelands Development Team
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

#ifndef NETWORK_SYSTEM_H
#define NETWORK_SYSTEM_H

#include <stdint.h>
#ifndef _WIN32
// These includes work on Linux and should be fine on any other Unix-alike.
// If not, this is the right place to conditionally include what is needed.
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// be compatible to microsoft
#define closesocket close
#define DATATYPE    void

#else

// This is the header to include according to the documentation
// at msdn.microsoft.com
#include <winsock2.h>

#define DATATYPE    char
// microsoft doesn't have these
typedef uint16_t in_port_t;
typedef uint32_t in_addr_t;

#ifndef s_addr
#define s_addr S_addr
#endif

// This is no typedef on purpose
#define socklen_t int32_t

#endif

#endif
