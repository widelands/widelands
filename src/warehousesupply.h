/*
 * Copyright (C) 2008-2009 by the Widelands Development Team
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

#ifndef WAREHOUSESUPPLY_H
#define WAREHOUSESUPPLY_H

#include "economy/transport.h" // unfortunately necessary for Supply

namespace Widelands {

/*
WarehouseSupply is the implementation of Supply that is used by Warehouses.
It also manages the list of wares in the warehouse.
*/
struct WarehouseSupply : public Supply {
	WarehouseSupply(Warehouse * const wh) : m_economy(0), m_warehouse(wh) {}
	virtual ~WarehouseSupply();

	void set_economy(Economy *);

	void set_nrworkers(Ware_Index);
	void set_nrwares  (Ware_Index);

	const WareList &get_wares() const {return m_wares;}
	const WareList &get_workers() const {return m_workers;}
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
	virtual PlayerImmovable * get_position(Game &);
	virtual bool is_active() const throw ();

	virtual uint32_t nr_supplies(Game const &, Request const &) const;
	virtual WareInstance & launch_item(Game &, Request const &);
	virtual Worker & launch_worker(Game &, Request const &);

private:
	Economy   * m_economy;
	WareList    m_wares;
	WareList    m_workers; //  we use this to keep the soldiers
	Warehouse * m_warehouse;
};

}


#endif // WAREHOUSESUPPLY_H
