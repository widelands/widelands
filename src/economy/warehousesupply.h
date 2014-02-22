/*
 * Copyright (C) 2008-2010 by the Widelands Development Team
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

#ifndef WAREHOUSESUPPLY_H
#define WAREHOUSESUPPLY_H

#include "logic/warelist.h"
#include "logic/wareworker.h"
#include "economy/supply.h"

namespace Widelands {

/*
WarehouseSupply is the implementation of Supply that is used by Warehouses.
It also manages the list of wares in the warehouse.
*/
struct WarehouseSupply : public Supply {
	WarehouseSupply(Warehouse * const wh) : m_economy(nullptr), m_warehouse(wh) {}
	virtual ~WarehouseSupply();

	void set_economy(Economy *);

	void set_nrworkers(Ware_Index);
	void set_nrwares  (Ware_Index);

	const WareList & get_wares  () const {return m_wares;}
	const WareList & get_workers() const {return m_workers;}
	uint32_t stock_wares  (Ware_Index const i) const {
		return m_wares  .stock(i);
	}
	uint32_t stock_workers(Ware_Index const i) const {
		return m_workers.stock(i);
	}
	void add_wares     (Ware_Index, uint32_t count);
	void remove_wares  (Ware_Index, uint32_t count);
	void add_workers   (Ware_Index, uint32_t count);
	void remove_workers(Ware_Index, uint32_t count);

	// Supply implementation
	virtual PlayerImmovable * get_position(Game &) override;
	virtual bool is_active() const override;
	virtual bool has_storage() const override;
	virtual void get_ware_type(WareWorker & type, Ware_Index & ware) const override;

	virtual void send_to_storage(Game &, Warehouse * wh) override;
	virtual uint32_t nr_supplies(const Game &, const Request &) const override;
	virtual WareInstance & launch_ware(Game &, const Request &) override;
	virtual Worker & launch_worker(Game &, const Request &) override;

private:
	Economy   * m_economy;
	WareList    m_wares;
	WareList    m_workers; //  we use this to keep the soldiers
	Warehouse * m_warehouse;
};

}


#endif
