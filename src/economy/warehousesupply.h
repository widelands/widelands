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

#ifndef WL_ECONOMY_WAREHOUSESUPPLY_H
#define WL_ECONOMY_WAREHOUSESUPPLY_H

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

	void set_nrworkers(DescriptionIndex);
	void set_nrwares  (DescriptionIndex);

	const WareList & get_wares  () const {return m_wares;}
	const WareList & get_workers() const {return m_workers;}
	uint32_t stock_wares  (DescriptionIndex const i) const {
		return m_wares  .stock(i);
	}
	uint32_t stock_workers(DescriptionIndex const i) const {
		return m_workers.stock(i);
	}
	void add_wares     (DescriptionIndex, uint32_t count);
	void remove_wares  (DescriptionIndex, uint32_t count);
	void add_workers   (DescriptionIndex, uint32_t count);
	void remove_workers(DescriptionIndex, uint32_t count);

	// Supply implementation
	PlayerImmovable * get_position(Game &) override;
	bool is_active() const override;
	bool has_storage() const override;
	void get_ware_type(WareWorker & type, DescriptionIndex & ware) const override;

	void send_to_storage(Game &, Warehouse * wh) override;
	uint32_t nr_supplies(const Game &, const Request &) const override;
	WareInstance & launch_ware(Game &, const Request &) override;
	Worker & launch_worker(Game &, const Request &) override;

private:
	Economy   * m_economy;
	WareList    m_wares;
	WareList    m_workers; //  we use this to keep the soldiers
	Warehouse * m_warehouse;
};

}


#endif  // end of include guard: WL_ECONOMY_WAREHOUSESUPPLY_H
