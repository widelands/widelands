/*
 * Copyright (C) 2011-2013 by the Widelands Development Team
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

#include "economy/portdock.h"

#include <memory>

#include <boost/format.hpp>

#include "base/log.h"
#include "base/macros.h"
#include "economy/expedition_bootstrap.h"
#include "economy/fleet.h"
#include "economy/ware_instance.h"
#include "economy/wares_queue.h"
#include "io/filewrite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

PortdockDescr g_portdock_descr("portdock", "Port Dock");

const PortdockDescr& PortDock::descr() const {
	return g_portdock_descr;
}

PortdockDescr::PortdockDescr(char const* const init_name, char const* const init_descname)
   : MapObjectDescr(MapObjectType::PORTDOCK, init_name, init_descname) {
}

PortDock::PortDock(Warehouse* wh)
   : PlayerImmovable(g_portdock_descr),
     fleet_(nullptr),
     warehouse_(wh),
     need_ship_(false),
     expedition_ready_(false) {
}

PortDock::~PortDock() {
	assert(expedition_bootstrap_.get() == nullptr);
}

/**
 * Add a position where ships may dock.
 *
 * The caller is responsible for ensuring that the positions are connected
 * by water, i.e. ships can move freely between the positions.
 *
 * @param where must be a field that is entirely water
 *
 * @note This only works properly when called before @ref init
 */
void PortDock::add_position(Coords where) {
	dockpoints_.push_back(where);
}

Warehouse* PortDock::get_warehouse() const {
	return warehouse_;
}

/**
 * Update which @ref Fleet we belong to.
 *
 * @warning This should only be called via @ref Fleet itself.
 */
void PortDock::set_fleet(Fleet* fleet) {
	fleet_ = fleet;
}

int32_t PortDock::get_size() const {
	return SMALL;
}

bool PortDock::get_passable() const {
	return true;
}

PortDock::PositionList PortDock::get_positions(const EditorGameBase&) const {
	return dockpoints_;
}

Flag& PortDock::base_flag() {
	return warehouse_->base_flag();
}

/**
 * Return the dock that has the given flag as its base, or 0 if no dock of our fleet
 * has the given flag.
 */
PortDock* PortDock::get_dock(Flag& flag) const {
	if (fleet_)
		return fleet_->get_dock(flag);
	return nullptr;
}

/**
 * Signal to the dock that it now belongs to the given economy.
 *
 * Called by @ref Warehouse::set_economy, and responsible for forwarding the
 * change to @ref Fleet.
 */
void PortDock::set_economy(Economy* e) {
	if (e == get_economy())
		return;

	PlayerImmovable::set_economy(e);
	if (fleet_)
		fleet_->set_economy(e);

	if (upcast(Game, game, &owner().egbase())) {
		for (ShippingItem& shipping_item : waiting_) {
			shipping_item.set_economy(*game, e);
		}
	}

	if (expedition_bootstrap_)
		expedition_bootstrap_->set_economy(e);
}

void PortDock::draw(uint32_t, const TextToDraw, const Vector2f&, float, RenderTarget*) {
	// do nothing
}

void PortDock::init(EditorGameBase& egbase) {
	PlayerImmovable::init(egbase);

	for (const Coords& coords : dockpoints_) {
		set_position(egbase, coords);
	}

	init_fleet(egbase);
}

/**
 * Create our initial singleton @ref Fleet. The fleet code ensures
 * that we merge with a larger fleet when possible.
 */
void PortDock::init_fleet(EditorGameBase& egbase) {
	Fleet* fleet = new Fleet(owner());
	fleet->add_port(egbase, this);
	fleet->init(egbase);
	// Note: the Fleet calls our set_fleet automatically
}

void PortDock::cleanup(EditorGameBase& egbase) {

	Warehouse* wh = nullptr;

	if (egbase.objects().object_still_available(warehouse_)) {

		// We need to remember this for possible recreation of portdock
		wh = warehouse_;

		// Transfer all our wares into the warehouse.
		if (upcast(Game, game, &egbase)) {
			for (ShippingItem& shipping_item : waiting_) {
				WareInstance* ware;
				shipping_item.get(*game, &ware, nullptr);
				if (ware) {
					ware->cancel_moving();
					warehouse_->incorporate_ware(*game, ware);
				} else {
					shipping_item.set_location(*game, warehouse_);
					shipping_item.end_shipping(*game);
				}
			}
		}
		waiting_.clear();
		warehouse_->portdock_ = nullptr;
	}

	if (upcast(Game, game, &egbase)) {
		for (ShippingItem& shipping_item : waiting_) {
			shipping_item.remove(*game);
		}
	}

	if (fleet_)
		fleet_->remove_port(egbase, this);

	for (const Coords& coords : dockpoints_) {
		unset_position(egbase, coords);
	}

	if (expedition_bootstrap_) {
		expedition_bootstrap_->cleanup(egbase);
		expedition_bootstrap_.reset(nullptr);
	}

	PlayerImmovable::cleanup(egbase);

	// Now let's attempt to recreate the portdock.
	if (wh) {
		if (!wh->cleanup_in_progress_) {
			if (upcast(Game, game, &egbase)) {
				if (game->is_loaded()) {  // Do not attempt when shutting down
					wh->restore_portdock_or_destroy(egbase);
				}
			}
		}
	}
}

/**
 * Add the flags of all ports that can be reached via this dock.
 */
void PortDock::add_neighbours(std::vector<RoutingNodeNeighbour>& neighbours) {
	if (fleet_ && fleet_->active())
		fleet_->add_neighbours(*this, neighbours);
}

/**
 * The given @p ware enters the dock, waiting to be transported away.
 */
void PortDock::add_shippingitem(Game& game, WareInstance& ware) {
	waiting_.push_back(ShippingItem(ware));
	ware.set_location(game, this);
	ware.update(game);
}

/**
 * The given @p ware, which is assumed to be inside the dock, has updated
 * its route.
 */
void PortDock::update_shippingitem(Game& game, WareInstance& ware) {
	for (std::vector<ShippingItem>::iterator item_iter = waiting_.begin();
	     item_iter != waiting_.end(); ++item_iter) {

		if (item_iter->object_.serial() == ware.serial()) {
			update_shippingitem(game, item_iter);
			return;
		}
	}
}

/**
 * The given @p worker enters the dock, waiting to be transported away.
 */
void PortDock::add_shippingitem(Game& game, Worker& worker) {
	waiting_.push_back(ShippingItem(worker));
	worker.set_location(this);
	update_shippingitem(game, worker);
}

/**
 * The given @p worker, which is assumed to be inside the dock, has
 * updated its route.
 */
void PortDock::update_shippingitem(Game& game, Worker& worker) {
	for (std::vector<ShippingItem>::iterator item_iter = waiting_.begin();
	     item_iter != waiting_.end(); ++item_iter) {

		if (item_iter->object_.serial() == worker.serial()) {
			update_shippingitem(game, item_iter);
			return;
		}
	}
}

void PortDock::update_shippingitem(Game& game, std::vector<ShippingItem>::iterator it) {
	it->update_destination(game, *this);

	PortDock* dst = it->get_destination(game);
	assert(dst != this);

	// Destination might have vanished or be in another economy altogether.
	if (dst && dst->get_economy() == get_economy()) {
		set_need_ship(game, true);
	} else {
		it->set_location(game, warehouse_);
		it->end_shipping(game);
		*it = waiting_.back();
		waiting_.pop_back();

		if (waiting_.empty())
			set_need_ship(game, false);
	}
}

/**
 * A ship has arrived at the dock. Clear all items designated for this dock,
 * and load the ship.
 */
void PortDock::ship_arrived(Game& game, Ship& ship) {
	std::vector<ShippingItem> items_brought_by_ship;
	ship.withdraw_items(game, *this, items_brought_by_ship);

	for (ShippingItem& shipping_item : items_brought_by_ship) {
		shipping_item.set_location(game, warehouse_);
		shipping_item.end_shipping(game);
	}

	if (expedition_ready_) {
		assert(expedition_bootstrap_.get() != nullptr);

		// Only use an empty ship.
		if (ship.get_nritems() < 1) {
			// Load the ship
			std::vector<Worker*> workers;
			std::vector<WareInstance*> wares;
			expedition_bootstrap_->get_waiting_workers_and_wares(
			   game, owner().tribe(), &workers, &wares);

			for (Worker* worker : workers) {
				ship.add_item(game, ShippingItem(*worker));
			}
			for (WareInstance* ware : wares) {
				ship.add_item(game, ShippingItem(*ware));
			}

			ship.start_task_expedition(game);

			// The expedition goods are now on the ship, so from now on it is independent from the port
			// and thus we switch the port to normal, so we could even start a new expedition,
			cancel_expedition(game);
			Notifications::publish(NoteShipWindow(ship.serial(), NoteShipWindow::Action::kRefresh));
			return fleet_->update(game);
		}
	}

	if (ship.get_nritems() < ship.descr().get_capacity() && !waiting_.empty()) {
		uint32_t nrload =
		   std::min<uint32_t>(waiting_.size(), ship.descr().get_capacity() - ship.get_nritems());

		while (nrload--) {
			// Check if the item has still a valid destination
			if (waiting_.back().get_destination(game)) {
				// Destination is valid, so we load the item onto the ship
				ship.add_item(game, waiting_.back());
			} else {
				// The item has no valid destination anymore, so we just carry it
				// back in the warehouse
				waiting_.back().set_location(game, warehouse_);
				waiting_.back().end_shipping(game);
			}
			waiting_.pop_back();
		}

		if (waiting_.empty()) {
			set_need_ship(game, false);
		}
	}

	fleet_->update(game);
}

void PortDock::set_need_ship(Game& game, bool need) {
	molog("set_need_ship(%s)\n", need ? "true" : "false");

	if (need == need_ship_)
		return;

	need_ship_ = need;

	if (fleet_) {
		molog("... trigger fleet update\n");
		fleet_->update(game);
	}
}

/**
 * Return the number of wares or workers of the given type that are waiting at the dock.
 */
uint32_t PortDock::count_waiting(WareWorker waretype, DescriptionIndex wareindex) {
	uint32_t count = 0;

	for (ShippingItem& shipping_item : waiting_) {
		WareInstance* ware;
		Worker* worker;
		shipping_item.get(owner().egbase(), &ware, &worker);

		if (waretype == wwWORKER) {
			if (worker && worker->descr().worker_index() == wareindex)
				count++;
		} else {
			if (ware && ware->descr_index() == wareindex)
				count++;
		}
	}

	return count;
}

/**
 * Return the number of wares or workers waiting at the dock.
 */
uint32_t PortDock::count_waiting() {
	return waiting_.size();
}

/// \returns whether an expedition was started or is even ready
bool PortDock::expedition_started() {
	return (expedition_bootstrap_.get() != nullptr) || expedition_ready_;
}

/// Start an expedition
void PortDock::start_expedition() {
	assert(!expedition_bootstrap_);
	expedition_bootstrap_.reset(new ExpeditionBootstrap(this));
	expedition_bootstrap_->start();
}

ExpeditionBootstrap* PortDock::expedition_bootstrap() {
	return expedition_bootstrap_.get();
}

void PortDock::expedition_bootstrap_complete(Game& game) {
	expedition_ready_ = true;
	get_fleet()->update(game);
}

void PortDock::cancel_expedition(Game& game) {
	// Reset
	expedition_ready_ = false;

	expedition_bootstrap_->cancel(game);
	expedition_bootstrap_.reset(nullptr);
}

void PortDock::log_general_info(const EditorGameBase& egbase) {
	PlayerImmovable::log_general_info(egbase);

	if (warehouse_) {
		Coords pos(warehouse_->get_position());
		molog("PortDock for warehouse %u (at %i,%i) in fleet %u, need_ship: %s, waiting: %" PRIuS
		      "\n",
		      warehouse_->serial(), pos.x, pos.y, fleet_ ? fleet_->serial() : 0,
		      need_ship_ ? "true" : "false", waiting_.size());
	} else {
		molog("PortDock without a warehouse in fleet %u, need_ship: %s, waiting: %" PRIuS "\n",
		      fleet_ ? fleet_->serial() : 0, need_ship_ ? "true" : "false", waiting_.size());
	}

	for (ShippingItem& shipping_item : waiting_) {
		molog("  IT %u, destination %u\n", shipping_item.object_.serial(),
		      shipping_item.destination_dock_.serial());
	}
}

constexpr uint8_t kCurrentPacketVersion = 3;

PortDock::Loader::Loader() : warehouse_(0) {
}

void PortDock::Loader::load(FileRead& fr) {
	PlayerImmovable::Loader::load(fr);

	PortDock& pd = get<PortDock>();

	warehouse_ = fr.unsigned_32();
	uint16_t nrdockpoints = fr.unsigned_16();

	pd.dockpoints_.resize(nrdockpoints);
	for (uint16_t i = 0; i < nrdockpoints; ++i) {
		pd.dockpoints_[i] = read_coords_32(&fr, egbase().map().extent());
		pd.set_position(egbase(), pd.dockpoints_[i]);
	}

	pd.need_ship_ = fr.unsigned_8();

	waiting_.resize(fr.unsigned_32());
	for (ShippingItem::Loader& shipping_loader : waiting_) {
		shipping_loader.load(fr);
	}

	// All the other expedition specific stuff is saved in the warehouse.
	if (fr.unsigned_8()) {  // Do we have an expedition?
		pd.expedition_bootstrap_.reset(new ExpeditionBootstrap(&pd));
	}
	pd.expedition_ready_ = (fr.unsigned_8() == 1) ? true : false;
}

void PortDock::Loader::load_pointers() {
	PlayerImmovable::Loader::load_pointers();

	PortDock& pd = get<PortDock>();
	pd.warehouse_ = &mol().get<Warehouse>(warehouse_);

	pd.waiting_.resize(waiting_.size());
	for (uint32_t i = 0; i < waiting_.size(); ++i) {
		pd.waiting_[i] = waiting_[i].get(mol());
	}
}

void PortDock::Loader::load_finish() {
	PlayerImmovable::Loader::load_finish();

	PortDock& pd = get<PortDock>();

	if (pd.warehouse_->get_portdock() != &pd) {
		log("Inconsistent PortDock <> Warehouse link\n");
		if (upcast(Game, game, &egbase()))
			pd.schedule_destroy(*game);
	}

	// This shouldn't be necessary, but let's check just in case
	if (!pd.fleet_)
		pd.init_fleet(egbase());
}

MapObject::Loader* PortDock::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
			loader->init(egbase, mol, *new PortDock(nullptr));
			loader->load(fr);
		} else {
			throw UnhandledVersionError("PortDock", packet_version, kCurrentPacketVersion);
		}
	} catch (const std::exception& e) {
		throw wexception("loading portdock: %s", e.what());
	}

	return loader.release();
}

void PortDock::save(EditorGameBase& egbase, MapObjectSaver& mos, FileWrite& fw) {
	fw.unsigned_8(HeaderPortDock);
	fw.unsigned_8(kCurrentPacketVersion);

	PlayerImmovable::save(egbase, mos, fw);

	fw.unsigned_32(mos.get_object_file_index(*warehouse_));
	fw.unsigned_16(dockpoints_.size());
	for (const Coords& coords : dockpoints_) {
		write_coords_32(&fw, coords);
	}

	fw.unsigned_8(need_ship_);

	fw.unsigned_32(waiting_.size());
	for (ShippingItem& shipping_item : waiting_) {
		shipping_item.save(egbase, mos, fw);
	}

	// Expedition specific stuff
	fw.unsigned_8(expedition_bootstrap_.get() != nullptr ? 1 : 0);
	fw.unsigned_8(expedition_ready_ ? 1 : 0);
}

}  // namespace Widelands
