/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef _PRODUCTIONSITEWINDOW_H_
#define _PRODUCTIONSITEWINDOW_H_

#include "wui/buildingwindow.h"
#include "logic/productionsite.h"
#include "ui_basic/table.h"

struct ProductionSite_Window : public Building_Window {
	ProductionSite_Window
		(Interactive_GameBase & parent,
		 Widelands::ProductionSite &,
		 UI::Window *         & registry);

	Widelands::ProductionSite & productionsite() {
		return ref_cast<Widelands::ProductionSite, Widelands::Building>(building());
	}
	void update_worker_table();
protected:
	virtual void think() override;
	void evict_worker();

private:
	UI::Table<uintptr_t> * m_worker_table;
	UI::Box * m_worker_caps;
};

#endif // _PRODUCTIONSITEWINDOW_H_
