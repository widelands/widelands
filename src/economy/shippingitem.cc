/*
 * Copyright (C) 2011 by the Widelands Development Team
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

#include "shippingitem.h"

#include "logic/game_data_error.h"
#include "logic/worker.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "portdock.h"
#include "ware_instance.h"

namespace Widelands {

ShippingItem::ShippingItem(WareInstance & ware) :
	m_object(&ware)
{
}

ShippingItem::ShippingItem(Worker & worker) :
	m_object(&worker)
{
}

void ShippingItem::get(Editor_Game_Base & game, WareInstance * & ware, Worker * & worker)
{
	Map_Object * obj = m_object.get(game);
	if (obj) {
		if (obj->get_type() == Map_Object::WARE) {
			ware = dynamic_cast<WareInstance *>(obj);
			worker = 0;
		} else {
			worker = dynamic_cast<Worker *>(obj);
			ware = 0;
		}
	} else {
		ware = 0;
		worker = 0;
	}
}

void ShippingItem::set_economy(Game & game, Economy * e)
{
	WareInstance * ware;
	Worker * worker;
	get(game, ware, worker);

	if (ware)
		ware->set_economy(e);
	if (worker)
		worker->set_economy(e);
}

void ShippingItem::set_location(Game & game, Map_Object * obj)
{
	WareInstance * ware;
	Worker * worker;
	get(game, ware, worker);

	if (ware)
		ware->set_location(game, obj);
	if (worker) {
		worker->set_location(dynamic_cast<PlayerImmovable *>(obj));
		if (upcast(Building, building, obj)) {
			worker->set_position(game, building->get_position());
		}
	}
}

void ShippingItem::end_shipping(Game & game)
{
	WareInstance * ware;
	Worker * worker;
	get(game, ware, worker);

	if (ware)
		ware->schedule_act(game, 10);
	if (worker)
		worker->end_shipping(game);
}

PortDock * ShippingItem::get_destination(Game & game)
{
	return m_destination_dock.get(game);
}

void ShippingItem::fetch_destination(Game & game, PortDock & pd)
{
	WareInstance * ware;
	Worker * worker;
	get(game, ware, worker);

	PlayerImmovable * next = 0;

	if (ware)
		next = ware->get_next_move_step(game);
	if (worker) {
		Transfer * transfer = worker->get_transfer();
		if (transfer) {
			bool success;
			next = transfer->get_next_step(&pd, success);
		}
	}

	m_destination_dock = dynamic_cast<PortDock *>(next);
}

void ShippingItem::schedule_update(Game & game, int32_t delay)
{
	WareInstance * ware;
	Worker * worker;
	get(game, ware, worker);

	if (ware) {
		ware->schedule_act(game, delay);
	}
	if (worker) {
		worker->send_signal(game, "wakeup");
	}
}

/**
 * Remove the underlying item directly. This is used when ships are removed.
 */
void ShippingItem::remove(Editor_Game_Base & egbase)
{
	if (Map_Object * obj = m_object.get(egbase)) {
		obj->remove(egbase);
		m_object = 0;
	}
}


#define SHIPPINGITEM_SAVEGAME_VERSION 1

void ShippingItem::Loader::load(FileRead & fr)
{
	uint8_t version = fr.Unsigned8();
	if (1 <= version && version <= SHIPPINGITEM_SAVEGAME_VERSION) {
		m_serial = fr.Unsigned32();
	} else
		throw game_data_error("unknown ShippingItem version %u", version);
}

ShippingItem ShippingItem::Loader::get(Map_Map_Object_Loader & mol)
{
	ShippingItem it;
	if (m_serial != 0)
		it.m_object = &mol.get<Map_Object>(m_serial);
	return it;
}

void ShippingItem::save(Editor_Game_Base & egbase, Map_Map_Object_Saver & mos, FileWrite & fw)
{
	fw.Unsigned8(SHIPPINGITEM_SAVEGAME_VERSION);
	fw.Unsigned32(mos.get_object_file_index_or_zero(m_object.get(egbase)));
}

} // namespace Widelands
