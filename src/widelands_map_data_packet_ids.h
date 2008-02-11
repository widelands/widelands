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

#ifndef WIDELANDS_MAP_DATA_PACKET_IDS_H
#define WIDELANDS_MAP_DATA_PACKET_IDS_H

/*
 * This file contains the ids (the magic bytes) of all data packets
 * so that the packet creation fabric can create the right packet
 * reader, all IDs are uint16_ts
 */
#define PACKET_HEIGHTS            1
#define PACKET_TERRAINS           2
#define PACKET_IMMOVABLE          3
#define PACKET_PLAYER_POSITION    4
#define PACKET_BOB                5
#define PACKET_RESOURCES          6
#define PACKET_PLAYER_NAM_TRIB    7        // Scenario packet
#define PACKET_TRIGGER            8        // Scenraio packet, non obligatory
#define PACKET_EVENT              9        // Scenraio packet, non obligatory
#define PACKET_ALLOWED_BUILDINGS 10        // Scenario packet, non obligatory

#define PACKET_FLAG              11        // Scenario packet, non obligatory (but always present)
#define PACKET_ROAD              12        // Scenario packet, non obligatory (but always present)
#define PACKET_BUILDING          13        // Scenario packet, non obligatory (but always present)
#define PACKET_WARE              14        // Scenario packet, non obligatory (but always present)

#define PACKET_FLAGDATA          15        // Scenario packet, non obligatory
#define PACKET_ROADDATA          16        // Scenario packet, non obligatory
#define PACKET_BUILDINGDATA      17        // Scenario packet, non obligatory
#define PACKET_WAREDATA          18        // Scenario packet, non obligatory
#define PACKET_BOBDATA           19        // Scenario packet, non obligatory
#define PACKET_IMMOVABLEDATA     20        // Scenario packet, non obligatory

#define PACKET_OWNED_FIELDS      21        // Scenario packet, non obligatory
#define PACKET_SEEN_FIELDS       22        // Scenario packet, non obligatory

#define PACKET_BATTLE            23        // Active battle, non obligatory
#define PACKET_ATTACK_CONTROLLER 24        // Active attack controller, non obligatory
#define PACKET_END_OF_MAP_DATA 254         // End of Map

#endif
