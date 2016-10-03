/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#ifndef WL_WUI_WAREHOUSEWINDOW_H
#define WL_WUI_WAREHOUSEWINDOW_H

#include "logic/map_objects/tribes/warehouse.h"
#include "wui/buildingwindow.h"

/**
 * Status window for warehouses
 */
struct WarehouseWindow : public BuildingWindow {
	WarehouseWindow(InteractiveGameBase& parent, Widelands::Warehouse&);

	Widelands::Warehouse& warehouse() {
		return dynamic_cast<Widelands::Warehouse&>(building());
	}

protected:
	void init() override;
private:
	DISALLOW_COPY_AND_ASSIGN(WarehouseWindow);
};

#endif  // end of include guard: WL_WUI_WAREHOUSEWINDOW_H
