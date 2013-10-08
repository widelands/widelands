/*
 * Copyright (C) 2006-2013 by the Widelands Development Team
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

#ifndef EXPEDITION_BOOTSTRAP_H
#define EXPEDITION_BOOTSTRAP_H

#include <vector>
#include <memory>

#include <boost/noncopyable.hpp>

#include "economy/wares_queue.h"

namespace Widelands {

class Economy;
class Editor_Game_Base;
class Game;
class PortDock;
class Request;
class WareInstance;
class Warehouse;
class Waresqueue;
class Worker;

// Handles the mustering of workers and wares in a port for one Expedition. This
// object is created in the port dock as soon as the start expedition button is
// pressed. As soon as the ship is loaded, this object gets destroyed.
class ExpeditionBootstrap : boost::noncopyable {
public:
	ExpeditionBootstrap(PortDock* const portdock);
	virtual ~ExpeditionBootstrap();

	// Start an expedition. This will request all wares and workers.
	void start_expedition();

	// Cancel an ongoing expedition. This will incorporate wares and workers
	// into the corresponding warehouse.
	// NOCOM(#sirver): What happens if the ship is already away?
	void cancel_expedition(Game& game);

	// Constructs a list of workers and wares that are ready to go to an
	// expedition. Ownership is transferred and the object is in an undefined
	// state after this and must be deleted.
	// // NOCOM(#sirver): bad design
	void get_waiting_workers_and_wares
		(Game&, const Tribe_Descr&, std::vector<Worker*>* return_workers,
		 std::vector<WareInstance*>* return_wares);

	// Returns the wares currently in stock.
	std::vector<WaresQueue*> wares() const;

	// Changes the economy for the items that are already in store.
	void set_economy(Economy* economy);

	// Returns the waresqueue for this ware.
	WaresQueue& waresqueue(Ware_Index index) const;

	// Delete all items we currently handle.
	void cleanup(Editor_Game_Base& egbase);

private:
	struct ExpeditionWorker;

	// Handles arriving workers and wares.
	static void worker_callback(Game&, Request& r, Ware_Index, Worker*, PlayerImmovable&);
	static void ware_callback(Game& game, WaresQueue*, Ware_Index, void* const data);
	void handle_worker_callback(Game &, Request &, Worker *);

	// Tests if all wares for the expedition have arrived. If so, informs the portdock.
	void is_expedition_ready(Game& game);

	PortDock* const portdock_;  // not owned
	Economy* economy_;

	std::vector<std::unique_ptr<WaresQueue>> wares_;
	std::vector<std::unique_ptr<ExpeditionWorker>> workers_;
};

}  // namespace Widelands

#endif /* end of include guard: EXPEDITION_BOOTSTRAP_H */
