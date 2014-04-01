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

#ifndef S__SUPPLY_H
#define S__SUPPLY_H

#include "logic/wareworker.h"
#include "trackptr.h"

namespace Widelands {

struct PlayerImmovable;
class Game;
class Request;
class Warehouse;
struct Ware_Index;
class WareInstance;
class Worker;

/**
 * A Supply is a virtual base class representing something that can offer
 * wares of any type for any purpose.
 *
 * Subsequent calls to get_position() can return different results.
 * If a Supply is "active", it should be transferred to a possible Request
 * quickly. Basically, supplies in warehouses (or unused supplies that are
 * being carried into a warehouse) are inactive, and supplies that are just
 * sitting on a flag are active.
 *
 * Important note: The implementation of Supply is responsible for adding
 * and removing itself from Economies. This rule holds true for Economy
 * changes.
 */
struct Supply : public Trackable {
	virtual PlayerImmovable * get_position(Game &) = 0;

	/**
	 * Indicates whether this supply is active as explained above (out
	 * on the road network).
	 */
	virtual bool is_active() const = 0;

	/**
	 * Indicates whether this supply is in storage or on its way to
	 * storage.
	 *
	 * If this is \c false, somebody needs to find this supply a warehouse.
	 */
	virtual bool has_storage() const = 0;

	/**
	 * Gets the ware type of this supply.
	 *
	 * \note This is only valid if \ref has_storage returns \c false.
	 */
	virtual void get_ware_type(WareWorker & type, Ware_Index & ware) const = 0;

	/**
	 * Send this to the given warehouse.
	 *
	 * Sets up all the required transfers; assumes that \ref has_storage
	 * returns \c false.
	 */
	virtual void send_to_storage(Game &, Warehouse * wh) = 0;

	/**
	 * \return the number of wares or workers that can be launched right
	 * now for the thing requested by the given request
	 */
	virtual uint32_t nr_supplies(const Game &, const Request &) const = 0;

	/**
	 * Prepare an ware to satisfy the given request. Note that the caller
	 * must assign a transfer to the launched ware.
	 *
	 * \throw wexception if the request is not an ware request or no such
	 * ware is available in the supply.
	 */
	virtual WareInstance & launch_ware(Game &, const Request &) = 0;

	/**
	 * Prepare a worker to satisfy the given request. Note that the caller
	 * must assign a transfer to the launched ware.
	 *
	 * \throw wexception if the request is not a worker request or no such
	 * worker is available in the supply.
	 */
	virtual Worker & launch_worker(Game &, const Request &) = 0;
};

}

#endif
