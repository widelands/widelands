/*
 * Copyright (C) 2011-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "economy/portdock.h"

#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "economy/expedition_bootstrap.h"
#include "economy/ship_fleet.h"
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
   : MapObjectDescr(MapObjectType::PORTDOCK, init_name, init_descname) {
}

PortDock::PortDock(Warehouse* wh)
   : PlayerImmovable(g_portdock_descr),
     fleet_(nullptr),
     warehouse_(wh),
     expedition_ready_(false),
     expedition_cancelling_(false) {
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
 * Update which @ref ShipFleet we belong to.
 *
 * @warning This should only be called via @ref ShipFleet itself.
 */
void PortDock::set_fleet(ShipFleet* fleet) {
	fleet_ = fleet;
}

int32_t PortDock::get_size() const {
	return SMALL;
}

bool PortDock::get_passable() const {
	return true;
}

PortDock::PositionList PortDock::get_positions(const EditorGameBase& /* egbase */) const {
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
	if (fleet_) {
		return fleet_->get_dock(flag);
	}
	return nullptr;
}

/**
 * Signal to the dock that it now belongs to the given economy.
 *
 * Called by @ref Warehouse::set_economy, and responsible for forwarding the
 * change to @ref ShipFleet.
 */
void PortDock::set_economy(Economy* e, WareWorker type) {
	if (e == get_economy(type)) {
		return;
	}

	PlayerImmovable::set_economy(e, type);
	if (fleet_) {
		fleet_->set_economy(e, type);
	}

	if (upcast(Game, game, &get_owner()->egbase())) {
		for (ShippingItem& shipping_item : waiting_) {
			shipping_item.set_economy(*game, e, type);
		}
	}

	if (expedition_bootstrap_) {
		expedition_bootstrap_->set_economy(e, type);
	}
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
 * Create our initial singleton @ref ShipFleet. The fleet code ensures
 * that we merge with a larger fleet when possible.
 */
void PortDock::init_fleet(EditorGameBase& egbase) {
	assert(!fleet_);
	ShipFleet* fleet = new ShipFleet(get_owner());
	fleet->add_port(egbase, this);
	fleet->init(egbase);
	// Note: the Fleet calls our set_fleet automatically, setting fleet_ to a valid ShipFleet*
	assert(fleet_);
	fleet_->update(egbase);
}

// Called directly before deletion to perform destructor duties that require an EditorGameBase
void PortDock::cleanup(EditorGameBase& egbase) {

	Warehouse* wh = nullptr;

	if (warehouse_) {

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

	if (fleet_) {
		fleet_->remove_port(egbase, this);
	}

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
	if (fleet_ && fleet_->active()) {
		fleet_->add_neighbours(*this, neighbours);
	}
}

/**
 * The given @p ware enters the dock, waiting to be transported away.
 */
void PortDock::add_shippingitem(Game& game, WareInstance& ware) {
	waiting_.emplace_back(ware);
	ware.set_location(game, this);
	ware.update(game);
	fleet_->update(game);
}

/**
 * The given @p ware, which is assumed to be inside the dock, has updated
 * its route.
 */
void PortDock::update_shippingitem(Game& game, const WareInstance& ware) {
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
	waiting_.emplace_back(worker);
	worker.set_location(this);
	update_shippingitem(game, worker);
	fleet_->update(game);
}

/**
 * The given @p worker, which is assumed to be inside the dock, has
 * updated its route.
 */
void PortDock::update_shippingitem(Game& game, const Worker& worker) {
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
	if (dst && dst->get_economy(wwWARE) == get_economy(wwWARE) &&
	    dst->get_economy(wwWORKER) == get_economy(wwWORKER)) {
	} else {
		it->set_location(game, warehouse_);
		it->end_shipping(game);
		waiting_.erase(it);
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

bool PortDock::load_one_item(Game& game, Ship& ship, const PortDock& dest) {
	// TODO(Nordfriese): Linear search, we should make waiting_ an
	// std::map<OPtr<PortDock>, std::vector<ShippingItem>>
	// for performance reasons (same for Ship::items_)
	for (auto it = waiting_.begin(); it != waiting_.end(); ++it) {
		if (it->get_destination(game) == &dest) {
			ship.add_item(game, *it);
			waiting_.erase(it);
			return true;
		}
	}
	return false;
}

/**
 * A ship has arrived at the dock. Set its next destination and load it accordingly.
 */
void PortDock::ship_arrived(Game& game, Ship& ship) {
	// The schedule handles everything
	fleet_->get_schedule().ship_arrived(game, ship, *this);
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

		switch (waretype) {
		case wwWORKER:
			if (worker && worker->descr().worker_index() == wareindex) {
				count++;
			}
			break;
		case wwWARE:
			if (ware && ware->descr_index() == wareindex) {
				count++;
			}
			break;
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

uint32_t PortDock::calc_max_priority(const EditorGameBase& egbase, const PortDock& dest) const {
	uint32_t priority = 0;
	for (const ShippingItem& si : waiting_) {
		if (si.destination_dock_.serial() == dest.serial()) {
			WareInstance* ware = nullptr;
			Worker* worker = nullptr;
			si.get(egbase, &ware, &worker);
			++priority;
			if (ware) {
				assert(!worker);
				if (ware->get_transfer() && ware->get_transfer()->get_request()) {
					// I don't know when this shouldn't be true,
					// but the regression tests assure me that it's possible…
					priority += ware->get_transfer()->get_request()->get_normalized_transfer_priority();
				}
			} else {
				assert(worker);
				if (worker->get_transfer() && worker->get_transfer()->get_request()) {
					priority +=
					   worker->get_transfer()->get_request()->get_normalized_transfer_priority();
				}
			}
		}
	}
	return priority;
}

/// \returns whether an expedition was started or is even ready
bool PortDock::expedition_started() const {
	return !expedition_cancelling_ && ((expedition_bootstrap_ != nullptr) || expedition_ready_);
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

void PortDock::set_expedition_bootstrap_complete(Game& game, bool complete) {
	if (expedition_ready_ != complete) {
		expedition_ready_ = complete;
		get_fleet()->update(game);
	}
}

void PortDock::cancel_expedition(Game& game) {
	// Reset
	assert(!expedition_cancelling_);
	expedition_cancelling_ = true;
	expedition_ready_ = false;

	expedition_bootstrap_->cancel(game);
	expedition_bootstrap_.reset(nullptr);

	expedition_cancelling_ = false;
}

void PortDock::log_general_info(const EditorGameBase& egbase) const {
	PlayerImmovable::log_general_info(egbase);

	if (warehouse_) {
		Coords pos(warehouse_->get_position());
		molog(
		   egbase.get_gametime(),
		   "PortDock for warehouse %u (at %i,%i) in fleet %u, expedition_ready: %s, waiting: %" PRIuS
		   "\n",
		   warehouse_->serial(), pos.x, pos.y, fleet_ ? fleet_->serial() : 0,
		   expedition_ready_ ? "true" : "false", waiting_.size());
	} else {
		molog(egbase.get_gametime(),
		      "PortDock without a warehouse in fleet %u, expedition_ready: %s, waiting: %" PRIuS "\n",
		      fleet_ ? fleet_->serial() : 0, expedition_ready_ ? "true" : "false", waiting_.size());
	}

	for (const ShippingItem& shipping_item : waiting_) {
		molog(egbase.get_gametime(), "  IT %u, destination %u\n", shipping_item.object_.serial(),
		      shipping_item.destination_dock_.serial());
	}
}

// Changelog of version 5 → 6: deleted the list with the serials of ships heading
// to this port as this information was moved to the ShippingSchedule
constexpr uint8_t kCurrentPacketVersion = 6;

PortDock::Loader::Loader() : warehouse_(0) {
}

void PortDock::Loader::load(FileRead& fr, uint8_t /* packet_version */) {
	PlayerImmovable::Loader::load(fr);

	PortDock& pd = get<PortDock>();

	warehouse_ = fr.unsigned_32();
	uint16_t nrdockpoints = fr.unsigned_16();

	pd.dockpoints_.resize(nrdockpoints);
	for (uint16_t i = 0; i < nrdockpoints; ++i) {
		pd.dockpoints_[i] = read_coords_32(&fr, egbase().map().extent());
		pd.set_position(egbase(), pd.dockpoints_[i]);
	}

	waiting_.resize(fr.unsigned_32());
	for (ShippingItem::Loader& shipping_loader : waiting_) {
		shipping_loader.load(fr);
	}

	// All the other expedition specific stuff is saved in the warehouse.
	if (fr.unsigned_8()) {  // Do we have an expedition?
		pd.expedition_bootstrap_.reset(new ExpeditionBootstrap(&pd));
	}
	pd.expedition_ready_ = fr.unsigned_8();
}

// During the first loading phase we only loaded the serials.
// Now all MapObjects have been created and we can load the actual pointers.
void PortDock::Loader::load_pointers() {
	PlayerImmovable::Loader::load_pointers();

	PortDock& pd = get<PortDock>();
	pd.warehouse_ = &mol().get<Warehouse>(warehouse_);

	for (ShippingItem::Loader& item : waiting_) {
		pd.waiting_.push_back(item.get(mol()));
	}
	assert(pd.waiting_.size() == waiting_.size());
}

void PortDock::Loader::load_finish() {
	PlayerImmovable::Loader::load_finish();

	PortDock& pd = get<PortDock>();

	if (pd.warehouse_->get_portdock() != &pd) {
		log_warn("Inconsistent PortDock <> Warehouse link\n");
		if (upcast(Game, game, &egbase())) {
			pd.schedule_destroy(*game);
		}
	}

	// This shouldn't be necessary, but let's check just in case
	if (!pd.fleet_) {
		pd.init_fleet(egbase());
	}
}

MapObject::Loader* PortDock::load(EditorGameBase& egbase, MapObjectLoader& mol, FileRead& fr) {
	std::unique_ptr<Loader> loader(new Loader);

	try {
		// The header has been peeled away by the caller

		uint8_t const packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersion) {
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

	fw.unsigned_32(waiting_.size());
	for (ShippingItem& shipping_item : waiting_) {
		shipping_item.save(egbase, mos, fw);
	}

	// Expedition specific stuff
	fw.unsigned_8(expedition_bootstrap_ != nullptr ? 1 : 0);
	fw.unsigned_8(expedition_ready_ ? 1 : 0);
}

}  // namespace Widelands
