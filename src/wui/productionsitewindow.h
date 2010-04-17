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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _PRODUCTIONSITEWINDOW_H_
#define _PRODUCTIONSITEWINDOW_H_

#include "buildingwindow.h"
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

protected:
	virtual void think();

private:
	UI::Table<unsigned int> * m_worker_table;
};

#endif // _PRODUCTIONSITEWINDOW_H_
