/*
 * Copyright (C) 2004, 2006-2013 by the Widelands Development Team
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

#include "economy/ware_instance.h"

#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/fleet.h"
#include "economy/portdock.h"
#include "economy/request.h"
#include "economy/transfer.h"
#include "logic/game.h"
#include "logic/ship.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "logic/worker.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {


/**
 * Whenever a WareInstance is idle, it issues an IdleWareSupply.
 *
 * \todo This maybe shouldn't be here.
*/
struct IdleWareSupply : public Supply {
	IdleWareSupply(WareInstance &);
	virtual ~IdleWareSupply();

	void set_economy(Economy *);

	//  implementation of Supply
	virtual PlayerImmovable * get_position(Game &) override;
	virtual bool is_active() const override;
	virtual bool has_storage() const override;
	virtual void get_ware_type(WareWorker & type, Ware_Index & ware) const override;
	virtual void send_to_storage(Game &, Warehouse * wh) override;

	virtual uint32_t nr_supplies(const Game &, const Request &) const override;
	virtual WareInstance & launch_ware(Game &, const Request &) override;
	virtual Worker & launch_worker(Game &, const Request &) override;

private:
	WareInstance & m_ware;
	Economy      * m_economy;
};

/**
 * Initialize the Supply and update the economy.
*/
IdleWareSupply::IdleWareSupply(WareInstance & ware) :
	m_ware(ware), m_economy(nullptr)
{
	set_economy(ware.get_economy());
}

/**
 * Cleanup.
*/
IdleWareSupply::~IdleWareSupply()
{
	set_economy(nullptr);
}

/**
 * Add/remove self from economies as necessary.
*/
void IdleWareSupply::set_economy(Economy * const e)
{
	if (m_economy != e) {
		if (m_economy)
			m_economy->remove_supply(*this);
		if ((m_economy = e))
			m_economy->   add_supply(*this);
	}
}

/**
 * Figure out the player immovable that this ware belongs to.
 */
PlayerImmovable * IdleWareSupply::get_position(Game & game)
{
	Map_Object * const loc = m_ware.get_location(game);

	if (upcast(PlayerImmovable, playerimmovable, loc))
		return playerimmovable;

	if (upcast(Worker, worker, loc))
		return worker->get_location(game);

	if (upcast(Ship, ship, loc)) {
		if (PortDock * pd = ship->get_destination(game))
			return pd;

		return ship->get_fleet()->get_arbitrary_dock();
	}

	return nullptr;
}

bool IdleWareSupply::is_active() const
{
	return true;
}

bool IdleWareSupply::has_storage()  const
{
	return m_ware.is_moving();
}

void IdleWareSupply::get_ware_type(WareWorker & type, Ware_Index & ware) const
{
	type = wwWARE;
	ware = m_ware.descr_index();
}

uint32_t IdleWareSupply::nr_supplies(const Game &, const Request & req) const
{
	if
		(req.get_type() == wwWARE &&
		 req.get_index() == m_ware.descr_index())
		return 1;

	return 0;
}

/**
 * The ware is already "launched", so we only need to return it.
*/
WareInstance & IdleWareSupply::launch_ware(Game &, const Request & req) {
	if (req.get_type() != wwWARE)
		throw wexception
			("IdleWareSupply::launch_ware : called for non-ware request");
	if (req.get_index() != m_ware.descr_index())
		throw wexception
			("IdleWareSupply: ware(%u) (type = %i) requested for %i",
			 m_ware.serial(),
			 m_ware.descr_index().value(),
			 req.get_index().value());

	return m_ware;
}

Worker & IdleWareSupply::launch_worker(Game &, const Request &)
{
	throw wexception("IdleWareSupply::launch_worker makes no sense");
}

void IdleWareSupply::send_to_storage(Game & game, Warehouse * wh)
{
	assert(!has_storage());

	Transfer * t = new Transfer(game, m_ware);
	t->set_destination(*wh);
	m_ware.set_transfer(game, *t);
}


/*************************************************************************/
/*                     Ware Instance Implementation                      */
/*************************************************************************/
WareInstance::WareInstance
	(Ware_Index const i, const WareDescr * const ware_descr)
:
Map_Object   (ware_descr),
m_economy    (nullptr),
m_descr_index(i),
m_supply     (nullptr),
m_transfer   (nullptr)
{}

WareInstance::~WareInstance()
{
	if (m_supply) {
		molog("Ware %u still has supply %p\n", m_descr_index.value(), m_supply);
		delete m_supply;
	}
}

int32_t WareInstance::get_type() const
{
	return WARE;
}

void WareInstance::init(Editor_Game_Base & egbase)
{
	Map_Object::init(egbase);
}

void WareInstance::cleanup(Editor_Game_Base & egbase)
{
	// Unlink from our current location, if necessary
	if (upcast(Flag, flag, m_location.get(egbase)))
		flag->remove_ware(egbase, this);

	delete m_supply;
	m_supply = nullptr;

	cancel_moving();
	set_location(egbase, nullptr);

	Map_Object::cleanup(egbase);
}

/**
 * Ware accounting
*/
void WareInstance::set_economy(Economy * const e)
{
	if (m_descr_index == Ware_Index::Null() or m_economy == e)
		return;

	if (m_economy)
		m_economy->remove_wares(m_descr_index, 1);

	m_economy = e;
	if (m_supply)
		m_supply->set_economy(e);

	if (m_economy)
		m_economy->add_wares(m_descr_index, 1);
}

/**
 * Change the current location.
 * Once you've assigned a ware to its new location, you usually have to call
 * \ref update() as well.
*/
void WareInstance::set_location(Editor_Game_Base & egbase, Map_Object * const location)
{
	Map_Object * const oldlocation = m_location.get(egbase);

	if (oldlocation == location)
		return;

	m_location = location;

	if (location) {
		Economy * eco = nullptr;

		if (upcast(Flag const, flag, location))
			eco = flag->get_economy();
		else if (upcast(Worker const, worker, location))
			eco = worker->get_economy();
		else if (upcast(PortDock const, portdock, location))
			eco = portdock->get_economy();
		else if (upcast(Ship const, ship, location))
			eco = ship->get_economy();
		else
			throw wexception("WareInstance delivered to bad location %u", location->serial());

		if (oldlocation && get_economy()) {
			if (get_economy() != eco)
				throw wexception("WareInstance::set_location() implies change of economy");
		} else {
			set_economy(eco);
		}
	} else {
		set_economy(nullptr);
	}
}

/**
 * Handle delayed updates.
 */
void WareInstance::act(Game & game, uint32_t)
{
	update(game);
}

/**
 * Performs the state updates necessary for the current location:
 * - if it's a building, acknowledge the Request or incorporate into warehouse
 * - if it's a flag and we have no request, start the return to warehouse timer
 * and issue a Supply
 *
 * \note \ref update() may result in the deletion of this object.
 * \note It is important that this function is idempotent, i.e. calling
 *       \ref update() twice in a row should have the same effect as calling
 *       it only once, \em unless the instance is deleted as a side-effect of
 *       \ref update().
 */
void WareInstance::update(Game & game)
{
	if (!m_descr) // Upsy, we're not even initialized. Happens on load
		return;

	Map_Object * const loc = m_location.get(game);

	// Reset our state if we're not on location or outside an economy
	if (!get_economy()) {
		cancel_moving();
		return;
	}

	if (!loc) {
		// Before dying, output as much information as we can.
		log_general_info(game);

		// If our location gets lost, our owner is supposed to destroy us
		throw wexception("WARE(%u): WareInstance::update has no location\n", serial());
	}

	// Update whether we have a Supply or not
	if (!m_transfer || !m_transfer->get_request()) {
		if (!m_supply)
			m_supply = new IdleWareSupply(*this);
	} else {
		delete m_supply;
		m_supply = nullptr;
	}

	// Deal with transfers
	if (m_transfer) {
		upcast(PlayerImmovable, location, loc);

		if (!location) {
			return;  // wait
		}

		bool success;
		PlayerImmovable * const nextstep =
			m_transfer->get_next_step(location, success);
		m_transfer_nextstep = nextstep;

		if (!nextstep) {
			if (upcast(Flag, flag, location))
				flag->call_carrier(game, *this, nullptr);

			Transfer * const t = m_transfer;

			m_transfer = nullptr;
			m_transfer_nextstep = nullptr;

			if (success) {
				t->has_finished();
				return;
			} else {
				t->has_failed();

				cancel_moving();
				update(game);
				return;
			}
		}

		if (upcast(Flag, flag, location)) {
			flag->call_carrier
				(game,
				 *this,
				 dynamic_cast<Building const *>(nextstep)
				 &&
				 &nextstep->base_flag() != location
				 ?
				 &nextstep->base_flag() : nextstep);
		} else if (upcast(PortDock, pd, location)) {
			pd->update_shippingitem(game, *this);
		} else {
			throw wexception("Ware_Instance::update in bad type of PlayerImmovable %u", location->serial());
		}
	}
}

/**
 * Called by a worker when it carries the ware into the given building.
 */
void WareInstance::enter_building(Game & game, Building & building)
{
	if (m_transfer) {
		if (m_transfer->get_destination(game) == &building) {
			Transfer * t = m_transfer;

			m_transfer = nullptr;
			m_transfer_nextstep = nullptr;

			t->has_finished();
			return;
		}

		bool success;
		PlayerImmovable * const nextstep =
			m_transfer->get_next_step(&building, success);
		m_transfer_nextstep = nextstep;

		if (success) {
			assert(nextstep);

			if (upcast(PortDock, pd, nextstep)) {
				pd->add_shippingitem(game, *this);
				return;
			}

			// There are some situations where we might end up in a warehouse
			// as part of a requested route, and we need to move out of it
			// again, e.g.:
			//  - we were requested just when we were being carried into the
			//    warehouse
			//  - we were carried into a harbour/warehouse to be
			//    shipped across the sea, but a better, land-based route has been
			//    found
			if (upcast(Warehouse, warehouse, &building)) {
				warehouse->do_launch_ware(game, *this);
				return;
			}

			throw wexception
				("MO(%u): ware(%s): do not know how to move from building %u (%s at (%u,%u)) "
				 "to %u (%s) -> not a warehouse!",
				 serial(), m_descr->name().c_str(), building.serial(),
				 building.name().c_str(), building.get_position().x,
				 building.get_position().y, nextstep->serial(),
				 nextstep->name().c_str());
		} else {
			Transfer * t = m_transfer;

			m_transfer = nullptr;
			m_transfer_nextstep = nullptr;

			t->has_failed();
			cancel_moving();

			if (is_a(Warehouse, &building)) {
				building.receive_ware(game, m_descr_index);
				remove(game);
			} else {
				update(game);
			}
			return;
		}
	} else {
		// We don't have a transfer, so just enter the building
		building.receive_ware(game, m_descr_index);
		remove(game);
	}
}

/**
 * Set ware state so that it follows the given transfer.
 *
 * \param t the new transfer (non-zero; use \ref cancel_transfer to stop a
 *          transfer).
 */
void WareInstance::set_transfer(Game & game, Transfer & t)
{
	m_transfer_nextstep = nullptr;

	// Reset current transfer
	if (m_transfer) {
		m_transfer->has_failed();
		m_transfer = nullptr;
	}

	// Set transfer state
	m_transfer = &t;

	delete m_supply;
	m_supply = nullptr;

	// Schedule an update.
	// Do not update immediately, because update() could try to reference
	// the Transfer object in a way that is not valid yet (note that this
	// function is called in the Transfer constructor before the Transfer
	// is linked to the corresponding Request).
	schedule_act(game, 1, 0);
}

/**
 * The transfer has been cancelled, just stop moving.
*/
void WareInstance::cancel_transfer(Game & game)
{
	m_transfer = nullptr;
	m_transfer_nextstep = nullptr;

	update(game);
}

/**
 * We are moving when there's a transfer, it's that simple.
*/
bool WareInstance::is_moving() const
{
	return m_transfer;
}

/**
 * Call this function if movement + potential request need to be cancelled for
 * whatever reason.
*/
void WareInstance::cancel_moving()
{
	molog("cancel_moving\n");

	if (m_transfer) {
		m_transfer->has_failed();
		m_transfer = nullptr;
		m_transfer_nextstep = nullptr;
	}
}

/**
 * Return the next flag we should be moving to, or the final target if the route
 * has been completed successfully.
*/
PlayerImmovable * WareInstance::get_next_move_step(Game & game)
{
	return
		m_transfer ?
		dynamic_cast<PlayerImmovable *>(m_transfer_nextstep.get(game)) : nullptr;
}

void WareInstance::log_general_info(const Editor_Game_Base & egbase)
{
	Map_Object::log_general_info(egbase);

	molog("Ware: %s\n", descr().name().c_str());
	molog("Location: %u\n", m_location.serial());
}


/*
==============================

Load/save support

==============================
*/

#define WAREINSTANCE_SAVEGAME_VERSION 1

WareInstance::Loader::Loader() :
	m_location(0),
	m_transfer_nextstep(0)
{
}

void WareInstance::Loader::load(FileRead & fr)
{
	Map_Object::Loader::load(fr);

	WareInstance & ware = get<WareInstance>();
	m_location = fr.Unsigned32();
	m_transfer_nextstep = fr.Unsigned32();
	if (fr.Unsigned8()) {
		ware.m_transfer =
			new Transfer(ref_cast<Game, Editor_Game_Base>(egbase()), ware);
		ware.m_transfer->read(fr, m_transfer);
	}
}

void WareInstance::Loader::load_pointers()
{
	Map_Object::Loader::load_pointers();

	WareInstance & ware = get<WareInstance>();

	// There is a race condition where a ware may lose its location and be scheduled
	// for removal via the update callback, but the game is saved just before the
	// removal. This is why we allow a null location on load.
	if (m_location)
		ware.set_location(egbase(), &mol().get<Map_Object>(m_location));
	if (m_transfer_nextstep)
		ware.m_transfer_nextstep = &mol().get<Map_Object>(m_transfer_nextstep);
	if (ware.m_transfer)
		ware.m_transfer->read_pointers(mol(), m_transfer);
}

void WareInstance::Loader::load_finish()
{
	Map_Object::Loader::load_finish();

	WareInstance & ware = get<WareInstance>();
	if (!ware.m_transfer || !ware.m_transfer->get_request()) {
		if (!ware.m_supply)
			ware.m_supply = new IdleWareSupply(ware);
	}
}


void WareInstance::save
	(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(header_WareInstance);
	fw.Unsigned8(WAREINSTANCE_SAVEGAME_VERSION);
	fw.CString(descr().tribe().name());
	fw.CString(descr().name());

	Map_Object::save(egbase, mos, fw);

	fw.Unsigned32(mos.get_object_file_index_or_zero(m_location.get(egbase)));
	fw.Unsigned32
		(mos.get_object_file_index_or_zero(m_transfer_nextstep.get(egbase)));
	if (m_transfer) {
		fw.Unsigned8(1);
		m_transfer->write(mos, fw);
	} else {
		fw.Unsigned8(0);
	}
}

Map_Object::Loader * WareInstance::load
	(Editor_Game_Base & egbase, Map_Map_Object_Loader & mol, FileRead & fr)
{
	try {
		uint8_t version = fr.Unsigned8();

		if (version != WAREINSTANCE_SAVEGAME_VERSION)
			throw wexception("unknown/unhandled version %i", version);

		const std::string tribename = fr.CString();
		const std::string warename = fr.CString();

		egbase.manually_load_tribe(tribename);

		const Tribe_Descr * tribe = egbase.get_tribe(tribename);
		if (!tribe)
			throw wexception("unknown tribe '%s'", tribename.c_str());

		Ware_Index wareindex = tribe->ware_index(warename);
		const WareDescr * descr = tribe->get_ware_descr(wareindex);

		std::unique_ptr<Loader> loader(new Loader);
		loader->init(egbase, mol, *new WareInstance(wareindex, descr));
		loader->load(fr);

		return loader.release();
	} catch (const std::exception & e) {
		throw wexception("WareInstance: %s", e.what());
	}

	return nullptr; // Should never be reached
}

}
