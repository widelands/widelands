/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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

#ifndef WL_ECONOMY_IDLEWORKERSUPPLY_H
#define WL_ECONOMY_IDLEWORKERSUPPLY_H

#include "economy/supply.h"

namespace Widelands {
class Economy;

struct IdleWorkerSupply : public Supply {
	explicit IdleWorkerSupply(Worker&);
	~IdleWorkerSupply() override;

	void set_economy(Economy*);
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
	Worker& worker_;
	Economy* economy_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_ECONOMY_IDLEWORKERSUPPLY_H
