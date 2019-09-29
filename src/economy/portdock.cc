/*
 * Copyright (C) 2011-2019 by the Widelands Development Team
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
#include "logic/path.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

PortdockDescr g_portdock_descr("portdock", "Port Dock");

const PortdockDescr& PortDock::descr() const {
	return g_portdock_descr;
}

PortdockDescr::PortdockDescr(char const* const init_name, char const* const init_descname)
   : MapObjectDescr(MapObjectType::PORTDOCK, init_name, init_descname, "") {
}

PortDock::PortDock(Warehouse* wh)
   : PlayerImmovable(g_portdock_descr), fleet_(nullptr), warehouse_(wh), expedition_ready_(false) {
}

PortDock::~PortDock() {
	assert(expedition_bootstrap_ == nullptr);
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

uint32_t PortDock::get_need_ship() const {
	return (waiting_.size() + (expedition_ready_ ? 20 : 0)) / (ships_coming_.size() + 1);
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

	if (upcast(Game, game, &get_owner()->egbase())) {
		for (ShippingItem& shipping_item : waiting_) {
			shipping_item.set_economy(*game, e);
		}
	}

	if (expedition_bootstrap_)
		expedition_bootstrap_->set_economy(e);
}

bool PortDock::init(EditorGameBase& egbase) {
	PlayerImmovable::init(egbase);

	for (const Coords& coords : dockpoints_) {
		set_position(egbase, coords);
	}

	init_fleet(egbase);
	return true;
}

/**
 * Create our initial singleton @ref Fleet. The fleet code ensures
 * that we merge with a larger fleet when possible.
 */
void PortDock::init_fleet(EditorGameBase& egbase) {
	Fleet* fleet = new Fleet(get_owner());
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
		while (!ships_coming_.empty()) {
			if (Ship* ship = ships_coming_.begin()->get(*game)) {
				ship->pop_destination(*game, *this);
			} else {
				ships_coming_.erase(ships_coming_.begin());
			}
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
	for (auto item_iter = waiting_.begin(); item_iter != waiting_.end(); ++item_iter) {

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
	for (auto item_iter = waiting_.begin(); item_iter != waiting_.end(); ++item_iter) {

		if (item_iter->object_.serial() == worker.serial()) {
			update_shippingitem(game, item_iter);
			return;
		}
	}
}

void PortDock::update_shippingitem(Game& game, std::list<ShippingItem>::iterator it) {
	it->update_destination(game, *this);

	const PortDock* dst = it->get_destination(game);
	assert(dst != this);

	// Destination might have vanished or be in another economy altogether.
	if (dst && dst->get_economy() == get_economy()) {
		if (ships_coming_.empty()) {
			set_need_ship(game, true);
		}
	} else {
		it->set_location(game, warehouse_);
		it->end_shipping(game);
		*it = waiting_.back();
		waiting_.pop_back();
	}
}

/**
 * Receive shipping item from unloading ship.
 * Called by ship code.
 */
void PortDock::shipping_item_arrived(Game& game, ShippingItem& si) {
	si.set_location(game, warehouse_);
	si.end_shipping(game);
}

/**
 * Receive shipping item from departing ship.
 * Called by ship code.
 */
void PortDock::shipping_item_returned(Game& game, ShippingItem& si) {
	si.set_location(game, this);
	waiting_.push_back(si);
}

/**
 * A ship changed destination and is now coming to the dock. Increase counter for need_ship.
 */
void PortDock::ship_coming(Ship& ship, bool affirmative) {
	OPtr<Ship> s(&ship);
	if (affirmative) {
		if (ships_coming_.find(s) == ships_coming_.end()) {
			ships_coming_.insert(s);
		}
	} else {
		auto it = ships_coming_.find(s);
		if (it != ships_coming_.end()) {
			ships_coming_.erase(it);
		}
	}
}

/**
 * A ship has arrived at the dock. Set its next destination and load it accordingly.
 */
void PortDock::ship_arrived(Game& game, Ship& ship) {
	if (expedition_ready_) {
		assert(expedition_bootstrap_ != nullptr);

		// Only use an empty ship.
		if (ship.get_nritems() == 0) {
			ship.clear_destinations(game);
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
			fleet_->update(game);
			return;
		}
	}

	ship.pop_destination(game, *this);
	fleet_->push_next_destinations(game, ship, *this);
	if (ship.get_current_destination(game)) {
		load_wares(game, ship);
	}
#ifndef NDEBUG
	else {
		assert(ship.get_nritems() == 0);
	}
#endif

	set_need_ship(game, !waiting_.empty());
}

void PortDock::load_wares(Game& game, Ship& ship) {
	uint32_t free_capacity = ship.descr().get_capacity() - ship.get_nritems();
	std::unordered_map<const PortDock*, bool> destination_check;
	for (auto it = waiting_.begin(); it != waiting_.end() && free_capacity;) {
		PortDock* dest = it->destination_dock_.get(game);
		assert(dest);
		assert(dest != this);
		// Decide whether to load this item
		bool load_item;
		auto dc = destination_check.find(dest);
		if (dc != destination_check.end()) {
			load_item = dc->second;
		} else {
			uint32_t time = ship.estimated_arrival_time(game, *dest);
			Path direct_route;
			fleet_->get_path(*this, *dest, direct_route);
			if (time == kInvalidDestination) {
				time = direct_route.get_nsteps();
			}
			for (const OPtr<Ship>& ship_ptr : ships_coming_) {
				Ship* s = ship_ptr.get(game);
				assert(s);
				uint32_t t = s->estimated_arrival_time(game, *dest, this);
				if (t == kInvalidDestination) {
					// The ship is not planning to go there yet, perhaps we can ask for a detour?
					t = s->estimated_arrival_time(game, *this);
					assert(s->count_destinations() >= 1);
					if (time >
					    t + static_cast<int32_t>(direct_route.get_nsteps() * s->count_destinations())) {
						time = kInvalidDestination;
						break;
					}
				} else if (t < time) {
					// A ship is coming that is planning to visit the ware's destination sooner than this
					// one
					time = kInvalidDestination;
					break;
				}
			}
			load_item = time < kInvalidDestination;
			destination_check.emplace(dest, load_item);
		}
		if (load_item) {
			if (!ship.has_destination(game, *dest)) {
				ship.push_destination(game, *dest);
			}
			ship.add_item(game, *it);
			it = waiting_.erase(it);
			--free_capacity;
		} else {
			++it;
		}
	}
}

void PortDock::set_need_ship(Game& game, bool need) {
	if (need && fleet_) {
		fleet_->update(game);
	}
}

/**
 * Return the number of wares or workers of the given type that are waiting at the dock.
 */
uint32_t PortDock::count_waiting(WareWorker waretype, DescriptionIndex wareindex) const {
	uint32_t count = 0;

	for (const ShippingItem& shipping_item : waiting_) {
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
 * If a destination dock is specified, count only items heading for this destination.
 */
uint32_t PortDock::count_waiting(const PortDock* dest) const {
	if (dest) {
		uint32_t w = 0;
		for (const ShippingItem& si : waiting_) {
			if (si.destination_dock_.serial() == dest->serial()) {
				++w;
			}
		}
		return w;
	}
	return waiting_.size();
}

/// \returns whether an expedition was started or is even ready
bool PortDock::expedition_started() const {
	return (expedition_bootstrap_ != nullptr) || expedition_ready_;
}

/// Start an expedition
void PortDock::start_expedition() {
	assert(!expedition_bootstrap_);
	expedition_bootstrap_.reset(new ExpeditionBootstrap(this));
	expedition_bootstrap_->start();
}

ExpeditionBootstrap* PortDock::expedition_bootstrap() const {
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

void PortDock::log_general_info(const EditorGameBase& egbase) const {
	PlayerImmovable::log_general_info(egbase);

	if (warehouse_) {
		Coords pos(warehouse_->get_position());
		molog(
		   "PortDock for warehouse %u (at %i,%i) in fleet %u, expedition_ready: %s, waiting: %" PRIuS
		   "\n",
		   warehouse_->serial(), pos.x, pos.y, fleet_ ? fleet_->serial() : 0,
		   expedition_ready_ ? "true" : "false", waiting_.size());
	} else {
		molog("PortDock without a warehouse in fleet %u, expedition_ready: %s, waiting: %" PRIuS "\n",
		      fleet_ ? fleet_->serial() : 0, expedition_ready_ ? "true" : "false", waiting_.size());
	}

	for (const ShippingItem& shipping_item : waiting_) {
		molog("  IT %u, destination %u\n", shipping_item.object_.serial(),
		      shipping_item.destination_dock_.serial());
	}
}

constexpr uint8_t kCurrentPacketVersion = 5;

PortDock::Loader::Loader() : warehouse_(0) {
}

void PortDock::Loader::load(FileRead& fr, uint8_t packet_version) {
	PlayerImmovable::Loader::load(fr);

	PortDock& pd = get<PortDock>();

	warehouse_ = fr.unsigned_32();
	uint16_t nrdockpoints = fr.unsigned_16();

	pd.dockpoints_.resize(nrdockpoints);
	for (uint16_t i = 0; i < nrdockpoints; ++i) {
		pd.dockpoints_[i] = read_coords_32(&fr, egbase().map().extent());
		pd.set_position(egbase(), pd.dockpoints_[i]);
	}

	if (packet_version >= 5) {
		const uint32_t ships = fr.unsigned_32();
		for (uint32_t i = 0; i < ships; ++i) {
			ships_coming_.insert(fr.unsigned_32());
		}
	} else {
		// TODO(GunChleoc): Savegame compatibility Build 20
		fr.unsigned_8();
		for (const Serial ship_serial : pd.owner().ships()) {
			Ship* ship = dynamic_cast<Ship*>(egbase().objects().get_object(ship_serial));
			if (ship->has_destination(egbase(), pd)) {
				ships_coming_.insert(ship_serial);
			}
		}
	}

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

	for (Serial s : ships_coming_) {
		pd.ships_coming_.insert(OPtr<Ship>(&mol().get<Ship>(s)));
	}
	for (uint32_t i = 0; i < waiting_.size(); ++i) {
		pd.waiting_.push_back(waiting_[i].get(mol()));
	}
	assert(pd.waiting_.size() == waiting_.size());
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

		// TODO(GunChleoc): Savegame compatibility Build 20
		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version >= 3 && packet_version <= kCurrentPacketVersion) {
			loader->init(egbase, mol, *new PortDock(nullptr));
			loader->load(fr, packet_version);
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

	fw.unsigned_32(ships_coming_.size());
	for (const OPtr<Ship>& s : ships_coming_) {
		fw.unsigned_32(mos.get_object_file_index(*s.get(egbase)));
	}

	fw.unsigned_32(waiting_.size());
	for (ShippingItem& shipping_item : waiting_) {
		shipping_item.save(egbase, mos, fw);
	}

	// Expedition specific stuff
	fw.unsigned_8(expedition_bootstrap_ != nullptr ? 1 : 0);
	fw.unsigned_8(expedition_ready_ ? 1 : 0);
}

}  // namespace Widelands
