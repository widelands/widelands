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

#ifndef WL_ECONOMY_WORKERS_QUEUE_H
#define WL_ECONOMY_WORKERS_QUEUE_H

#include <vector>
#include "logic/map_objects/immovable.h"
#include "logic/widelands.h"

namespace Widelands {

class EditorGameBase;
class Game;
class MapObjectLoader;
struct MapObjectSaver;
class Player;
class Request;
class Worker;

/**
 * Similar to WaresQueue but for workers.
 */
// TODO(Notabilis): Lua support is missing.
// Add functions to scripting/lua_map.* to allow filling the queue per script
class WorkersQueue {
public:

    /**
     * Default constructor
     */
    WorkersQueue(PlayerImmovable &, DescriptionIndex, uint8_t size);

    /**
     * The type of workers required here.
     */
	DescriptionIndex get_worker() const {return worker_type_;}

	/**
	 * \return a list of workers that are currently in the building.
	 */
	std::vector<Worker *> workers() const {return workers_;};

	/**
	 * \return the maximum number of workers that this building can be
	 * configured to hold.
	 */
	Quantity max_capacity() const {return max_capacity_;};

	/**
	 * Is in [0, max_capacity()].
	 * \return the number of workers this building is configured to hold
	 * right now.
	 */
	Quantity capacity() const {return capacity_;};

	/**
	 * Sets the capacity for workers of this building.
	 * Has to be in [0, max_capacity()].
	 *
	 * New workers will be requested and old workers will be evicted
	 * as necessary.
	 */
	void set_capacity(Quantity capacity);

	void change_capacity(int32_t const difference) {
		Quantity const old_capacity = capacity();
		Quantity const new_capacity =
			std::min
				(static_cast<Quantity>
				 	(std::max
				 	 	(static_cast<int32_t>(old_capacity) + difference,
				 	 	 0)),
				 max_capacity_);
		if (old_capacity != new_capacity)
			set_capacity(new_capacity);
	}

	/**
	 * Evict the given worker from the building immediately,
	 * without changing the building's capacity.
	 */
	void drop(Worker &);

    /**
     * Removes the given amount of workers from the game.
     * There have to be at least the given amount of workers in the queue.
     */
    void remove_workers(Quantity amount);

	/**
	 * Add a new worker into this site.
	 * \return -1 if there is no space for him, 0 on success.
	 */
	int incorporate_worker(EditorGameBase &, Worker &);

	void remove_from_economy(Economy &);
	void add_to_economy(Economy &);

	Player & owner() const {return owner_.owner();}

	void read (FileRead  &, Game &, MapObjectLoader &);
	void write(FileWrite &, Game &, MapObjectSaver  &);

private:

    /**
     * Callback when a request is fulfilled and a worker enters the queue.
     */
	static void request_callback
		(Game &, Request &, DescriptionIndex, Worker *, PlayerImmovable &);

    /**
     * Updates the request for further workers.
     * Should be called when a worker is added or removed.
     */
	void update_request();

	PlayerImmovable & owner_;
	/// Type of the stored worker
	DescriptionIndex worker_type_;
	/// Number of workers that fit into the queue maximum.
	Quantity max_capacity_;
	/// Number of workers that fit currently into the queue.
	Quantity capacity_;

    /// The workers currently in the queue
    std::vector<Worker *> workers_;

    /// Currently pending request
	Request * request_;
};

}

#endif // WL_ECONOMY_WORKERS_QUEUE_H
