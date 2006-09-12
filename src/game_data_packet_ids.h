/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__GAME_DATA_PACKET_IDS_H
#define __S__GAME_DATA_PACKET_IDS_H

/*
 * This file contains the ids (the magic bytes) of all data packets
 * so that the packet creation fabric can create the right packet
 * reader, all IDs are ushorts
 */
#define PACKET_GAME_CLASS_DATA           1
#define PACKET_PLAYER_INFO_DATA          2
#define PACKET_MAP_DATA                  3
#define PACKET_PLAYER_ECONOMIES_DATA     4
#define PACKET_CMD_QUEUE_DATA            5
#define PACKET_INTERACTIVE_PLAYER_DATA   6
#define PACKET_COMPUTER_PLAYER_DATA      7

#define PACKET_END_OF_GAME_DATA        254

#endif
