/*
 * Copyright (C) 2021 by the Widelands Development Team
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

#ifndef WL_LOGIC_MAP_OBJECTS_TRIBES_SHIPSTATES_H
#define WL_LOGIC_MAP_OBJECTS_TRIBES_SHIPSTATES_H

namespace Widelands {

// A ship with task expedition can be in four states: kExpeditionWaiting, kExpeditionScouting,
// kExpeditionPortspaceFound or kExpeditionColonizing in the first states, the owning player of
// this ship
// can give direction change commands to change the direction of the moving ship / send the ship
// in a
// direction. Once the ship is on its way, it is in kExpeditionScouting state. In the backend, a
// click
// on a direction begins to the movement into that direction until a coast is reached or the user
// cancels the direction through a direction change.
//
// The kExpeditionWaiting state means, that an event happend and thus the ship stopped and waits
// for a
// new command by the owner. An event leading to a kExpeditionWaiting state can be:
// * expedition is ready to start
// * new island appeared in vision range (only outer ring of vision range has to be checked due
// to the
//   always ongoing movement).
// * island was completely surrounded
//
// The kExpeditionPortspaceFound state means, that a port build space was found.
//
enum class ShipStates : uint8_t {
	kTransport = 0,
	kExpeditionWaiting = 1,
	kExpeditionScouting = 2,
	kExpeditionPortspaceFound = 3,
	kExpeditionColonizing = 4,
	kSinkRequest = 8,
	kSinkAnimation = 9,
};

enum class ShipType : uint8_t {
	kTransport = 1,
	kWarship = 2,
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MAP_OBJECTS_TRIBES_SHIPSTATES_H
