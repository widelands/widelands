/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_WAREHOUSESUPPLY_H
#define WL_ECONOMY_WAREHOUSESUPPLY_H

#include "economy/supply.h"
#include "logic/map_objects/tribes/warelist.h"
#include "logic/map_objects/tribes/wareworker.h"

namespace Widelands {

/*
WarehouseSupply is the implementation of Supply that is used by Warehouses.
It also manages the list of wares in the warehouse.
*/
struct WarehouseSupply : public Supply {
	explicit WarehouseSupply(Warehouse* const wh)
	   : ware_economy_(nullptr), worker_economy_(nullptr), warehouse_(wh) {
	}
	~WarehouseSupply() override;

	void set_economy(Economy*, WareWorker);

	void set_nrworkers(DescriptionIndex);
	void set_nrwares(DescriptionIndex);

	const WareList& get_wares() const {
		return wares_;
	}
	const WareList& get_workers() const {
		return workers_;
	}
	Quantity stock_wares(DescriptionIndex const i) const {
		return wares_.stock(i);
	}
	Quantity stock_workers(DescriptionIndex const i) const {
		return workers_.stock(i);
	}
	void add_wares(DescriptionIndex, Quantity count);
	void remove_wares(DescriptionIndex, Quantity count);
	void add_workers(DescriptionIndex, Quantity count);
	void remove_workers(DescriptionIndex, Quantity count);

	// Supply implementation
	PlayerImmovable* get_position(Game&) override;
	bool is_active() const override;
	SupplyProviders provider_type(Game*) const override;
	bool has_storage() const override;
	void get_ware_type(WareWorker& type, DescriptionIndex& ware) const override;

	void send_to_storage(Game&, Warehouse* wh) override;
	uint32_t nr_supplies(const Game&, const Request&) const override;
	WareInstance& launch_ware(Game&, const Request&) override;
	Worker& launch_worker(Game&, const Request&) override;

private:
	Economy* ware_economy_;
	Economy* worker_economy_;
	WareList wares_;
	WareList workers_;  //  we use this to keep the soldiers
	Warehouse* warehouse_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_WAREHOUSESUPPLY_H
