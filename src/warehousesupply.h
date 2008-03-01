/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#include "transport.h" // unfortunately necessary for Supply

namespace Widelands {

/*
WarehouseSupply is the implementation of Supply that is used by Warehouses.
It also manages the list of wares in the warehouse.
*/
struct WarehouseSupply : public Supply {
	WarehouseSupply(Warehouse * const wh) : m_economy(0), m_warehouse(wh) {}
	virtual ~WarehouseSupply();

	void set_economy(Economy* e);

	void set_nrworkers(int32_t i);
	void set_nrwares(int32_t i);

	const WareList &get_wares() const {return m_wares;}
	const WareList &get_workers() const {return m_workers;}
	int32_t stock_wares  (Ware_Index const i) const {return m_wares  .stock(i);}
	int32_t stock_workers(Ware_Index const i) const {return m_workers.stock(i);}
	void add_wares     (Ware_Index, uint32_t count);
	void remove_wares  (Ware_Index, uint32_t count);
	void add_workers   (Ware_Index, uint32_t count);
	void remove_workers(Ware_Index, uint32_t count);

	// Supply implementation
	virtual PlayerImmovable* get_position(Game* g);
	virtual int32_t get_amount(int32_t ware) const;
	virtual bool is_active() const throw ();

	virtual WareInstance & launch_item(Game *, int32_t ware);
	virtual Worker* launch_worker(Game* g, int32_t ware);

	virtual Soldier* launch_soldier(Game* g, int32_t ware, Requeriments* req);
	virtual int32_t get_passing_requeriments(Game* g, int32_t ware, Requeriments* r);
	virtual void mark_as_used (Game* g, int32_t ware, Requeriments* r);
private:
	Economy   * m_economy;
	WareList    m_wares;
	WareList    m_workers; //  we use this to keep the soldiers
	Warehouse * m_warehouse;
};

}


#endif // WAREHOUSESUPPLY_H
