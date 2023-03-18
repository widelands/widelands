/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#ifndef WL_WUI_WAREHOUSEWINDOW_H
#define WL_WUI_WAREHOUSEWINDOW_H

#include "logic/map_objects/tribes/warehouse.h"
#include "wui/buildingwindow.h"

/**
 * Status window for warehouses
 */
struct WarehouseWindow : public BuildingWindow {
	WarehouseWindow(InteractiveBase& parent,
	                BuildingWindow::Registry& reg,
	                Widelands::Warehouse&,
	                bool avoid_fastclick,
	                bool workarea_preview_wanted);

private:
	void init(bool avoid_fastclick, bool workarea_preview_wanted) override;
	void setup_name_field_editbox(UI::Box& vbox) override;

	Widelands::OPtr<Widelands::Warehouse> warehouse_;

	DISALLOW_COPY_AND_ASSIGN(WarehouseWindow);
};

#endif  // end of include guard: WL_WUI_WAREHOUSEWINDOW_H
