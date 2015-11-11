/*
 * Copyright (C) 2002-2004, 2006, 2009 by the Widelands Development Team
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

#ifndef WL_WUI_ENCYCLOPEDIA_WINDOW_H
#define WL_WUI_ENCYCLOPEDIA_WINDOW_H

#include "logic/building.h"
#include "logic/ware_descr.h"
#include "logic/worker_descr.h"
#include "ui_basic/box.h"
#include "ui_basic/listselect.h"
#include "ui_basic/multilinetextarea.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"
#include "ui_basic/window.h"

namespace Widelands {
class BuildingDescr;
class WareDescr;
class WorkerDescr;
class TribeDescr;
}

class InteractivePlayer;

struct EncyclopediaWindow : public UI::UniqueWindow {
	EncyclopediaWindow(InteractivePlayer &, UI::UniqueWindow::Registry &);
private:
	struct Building {
		Building(Widelands::BuildingIndex i, const Widelands::BuildingDescr * descr)
			:
			index_(i),
			descr_(descr)
			{}
		Widelands::BuildingIndex index_;
		const Widelands::BuildingDescr * descr_;

		bool operator<(const Building o) const {
			return descr_->descname() < o.descr_->descname();
		}
	};

	struct Ware {
		Ware(Widelands::DescriptionIndex i, const Widelands::WareDescr * descr)
			:
			index_(i),
			descr_(descr)
			{}
		Widelands::DescriptionIndex index_;
		const Widelands::WareDescr * descr_;

		bool operator<(const Ware o) const {
			return descr_->descname() < o.descr_->descname();
		}
	};

	struct Worker {
		Worker(Widelands::DescriptionIndex i, const Widelands::WorkerDescr * descr)
			:
			index_(i),
			descr_(descr)
			{}
		Widelands::DescriptionIndex index_;
		const Widelands::WorkerDescr * descr_;

		bool operator<(const Worker o) const {
			return descr_->descname() < o.descr_->descname();
		}
	};


	InteractivePlayer & iaplayer() const;
	UI::TabPanel tabs_;

	// Buildings
	UI::Box buildings_tab_box_;  // Wrapper box so we can add some padding
	UI::Box buildings_box_;      // Main contents box for Buildings tab
	UI::Listselect<Widelands::BuildingIndex> buildings_;
	UI::MultilineTextarea building_text_;
	void fill_buildings();
	void building_selected(uint32_t);

	// Wares
	UI::Box wares_tab_box_;      // Wrapper box so we can add some padding
	UI::Box wares_box_;          // Main contents box for Wares tab
	UI::Box wares_details_box_;  // Horizontal alignment for prod_sites_ and cond_table_
	UI::Listselect<Widelands::DescriptionIndex> wares_;
	UI::MultilineTextarea    ware_text_;
	UI::Listselect<Widelands::BuildingIndex> prod_sites_;
	UI::Table     <uintptr_t>                 cond_table_;
	Widelands::WareDescr const * selected_ware_;
	void fill_wares();
	void ware_selected(uint32_t);
	void prod_site_selected(uint32_t);

	// Workers
	UI::Box workers_tab_box_;  // Wrapper box so we can add some padding
	UI::Box workers_box_;      // Main contents box for Workers tab
	UI::Listselect<Widelands::DescriptionIndex> workers_;
	UI::MultilineTextarea worker_text_;
	void fill_workers();
	void worker_selected(uint32_t);
};

#endif  // end of include guard: WL_WUI_ENCYCLOPEDIA_WINDOW_H
