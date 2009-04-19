/*
 * Copyright (C) 2004, 2006-2009 by the Widelands Development Team
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

#ifndef S__SUPPLY_H
#define S__SUPPLY_H

#include "trackptr.h"

namespace Widelands {

struct PlayerImmovable;
struct Game;
struct Request;
struct WareInstance;
struct Worker;

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
	virtual bool is_active() const throw () = 0;

	/**
	 * \return the number of items or workers that can be launched right
	 * now for the thing requested by the given request
	 */
	virtual uint32_t nr_supplies(Game const &, Request const &) const = 0;

	/**
	 * Prepare an item to satisfy the given request. Note that the caller
	 * must assign a transfer to the launched item.
	 *
	 * \throw wexception if the request is not an item request or no such
	 * item is available in the supply.
	 */
	virtual WareInstance & launch_item(Game &, Request const &) = 0;

	/**
	 * Prepare a worker to satisfy the given request. Note that the caller
	 * must assign a transfer to the launched item.
	 *
	 * \throw wexception if the request is not a worker request or no such
	 * worker is available in the supply.
	 */
	virtual Worker & launch_worker(Game &, Request const &) = 0;
};

}
#endif 


