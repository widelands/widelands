/*
 * Copyright (C) 2006-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_EXPEDITION_BOOTSTRAP_H
#define WL_ECONOMY_EXPEDITION_BOOTSTRAP_H

#include <memory>

#include "base/macros.h"
#include "economy/input_queue.h"

namespace Widelands {

class EditorGameBase;
class PortDock;
class WareInstance;
class Warehouse;
class Worker;

/**
 * Handles the mustering of workers and wares in a port for an Expedition.
 *
 * This object is created in the port dock as soon as the start expedition button is
 * pressed. As soon as the ship is loaded, this object gets destroyed.
 * In case the Expedition is ::cancel()ed the wares will be returned to the port dock.
 * It is the responsibility of the Owner to finally dispose a canceled ExpeditionBootstrap.
 */

class ExpeditionBootstrap {
public:
	explicit ExpeditionBootstrap(PortDock* const portdock);
	virtual ~ExpeditionBootstrap();

	// Start bootstrapping an expedition. This will request all wares and workers.
	void start();

	// Cancel an ongoing bootstrap. This will incorporate wares and workers into
	// the corresponding warehouse.
	void cancel(Game& game);

	/**
	 * Returns a list of workers and wares that are ready to go to an expediton.
	 *
	 * Ownership is transferred and the object is in an undefined
	 * state after this and must be deleted.
	 */
	void get_waiting_workers_and_wares(Game&,
	                                   const TribeDescr&,
	                                   std::vector<Worker*>* return_workers,
	                                   std::vector<WareInstance*>* return_wares);

	// Changes the economy for the wares that are already in store.
	void set_economy(Economy* economy, WareWorker);

	// Returns the wares and workers currently waiting for the expedition.
	std::vector<InputQueue*> queues(bool all) const;

	// Returns the matching input queue for the given index and type.
	InputQueue& inputqueue(DescriptionIndex index, WareWorker type, bool) const;
	InputQueue* inputqueue(size_t additional_index) const;
	InputQueue& inputqueue(const Request&) const;

	void demand_additional_item(Game&, WareWorker, DescriptionIndex, bool);
	size_t count_additional_queues() const;

	// Tests if all wares for the expedition have arrived. If so, informs the portdock.
	void check_is_ready(Game& game);

	// Delete all wares we currently handle.
	void cleanup(EditorGameBase& egbase);

	/** Load this from a file.
	 *
	 * The actual data is stored in the buildingdata
	 * packet, and there in the warehouse data packet.
	 */
	void
	load(Warehouse& warehouse, FileRead& fr, Game& game, MapObjectLoader& mol, uint16_t version);

	/** Save this into a file.
	 *
	 * The actual data is stored in the buildingdata
	 * packet, and there in the warehouse data packet.
	 */
	void save(FileWrite& fw, Game& game, MapObjectSaver& mos);

private:
	// Handles arriving workers and wares.
	static void input_callback(Game&, InputQueue*, DescriptionIndex, Worker*, void*);

	/** The Expedition is bootstapped here. */
	PortDock* const portdock_;  // not owned
	Economy* ware_economy_;
	Economy* worker_economy_;

	std::vector<std::pair<std::unique_ptr<InputQueue>, bool>>
	   queues_;  // The bool indicates whether this queue can be removed

	DISALLOW_COPY_AND_ASSIGN(ExpeditionBootstrap);
};

struct NoteExpeditionCanceled {
	CAN_BE_SENT_AS_NOTE(NoteId::NoteExpeditionCanceled)

	ExpeditionBootstrap* bootstrap;

	explicit NoteExpeditionCanceled(ExpeditionBootstrap* const init_bootstrap)
	   : bootstrap(init_bootstrap) {
	}
};

}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_EXPEDITION_BOOTSTRAP_H
