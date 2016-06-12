/*
 * Copyright (C) 2004, 2006-2011 by the Widelands Development Team
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

#ifndef WL_ECONOMY_WARES_QUEUE_H
#define WL_ECONOMY_WARES_QUEUE_H

#include "logic/map_objects/immovable.h"
#include "logic/widelands.h"

namespace Widelands {

class Economy;
class EditorGameBase;
class Game;
class MapObjectLoader;
struct MapObjectSaver;
class Player;
class Request;
class Worker;

/**
 * This micro storage room can hold any number of items of a fixed ware.
 */
class WaresQueue {
public:
	using CallbackFn = void
		(Game &, WaresQueue *, DescriptionIndex ware, void * data);

	WaresQueue(PlayerImmovable &, DescriptionIndex, uint8_t size);

#ifndef NDEBUG
	~WaresQueue() {assert(ware_ == INVALID_INDEX);}
#endif

	DescriptionIndex get_ware()    const {return ware_;}
	Quantity get_max_fill() const {return max_fill_;}
	Quantity get_max_size() const {return max_size_;}
	Quantity get_filled()   const {return filled_;}

	void cleanup();

	void set_callback(CallbackFn *, void * data);

	void remove_from_economy(Economy &);
	void add_to_economy(Economy &);

	void set_max_size        (Quantity);
	void set_max_fill        (Quantity);
	void set_filled          (Quantity);
	void set_consume_interval(uint32_t);

	Player & owner() const {return owner_.owner();}

	void read (FileRead  &, Game &, MapObjectLoader &);
	void write(FileWrite &, Game &, MapObjectSaver  &);

private:
	static void request_callback
		(Game &, Request &, DescriptionIndex, Worker *, PlayerImmovable &);
	void update();

	PlayerImmovable & owner_;
	DescriptionIndex         ware_;    ///< ware ID
	Quantity max_size_;         ///< nr of items that fit into the queue maximum
	Quantity max_fill_;         ///< nr of wares that should be ideally in this queue
	Quantity filled_;           ///< nr of items that are currently in the queue

	///< time in ms between consumption at full speed
	uint32_t consume_interval_;

	Request         * request_; ///< currently pending request

	CallbackFn      * callback_fn_;
	void            * callback_data_;
};

}

#endif  // end of include guard: WL_ECONOMY_WARES_QUEUE_H
