/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "wui/productionsitewindow.h"

#include <boost/format.hpp>

#include "economy/input_queue.h"
#include "economy/request.h"
#include "graphic/graphic.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "ui_basic/textarea.h"
#include "wui/inputqueuedisplay.h"

static char const* pic_tab_wares = "images/wui/buildings/menu_tab_wares.png";
static char const* pic_tab_workers = "images/wui/buildings/menu_list_workers.png";

/*
===============
Create the window and its panels, add it to the registry.
===============
*/
ProductionSiteWindow::ProductionSiteWindow(InteractiveGameBase& parent,
                                           UI::UniqueWindow::Registry& reg,
                                           Widelands::ProductionSite& ps,
                                           bool avoid_fastclick)
   : BuildingWindow(parent, reg, ps, avoid_fastclick) {
	productionsitenotes_subscriber_ = Notifications::subscribe<Widelands::NoteBuilding>(
	   [this](const Widelands::NoteBuilding& note) {
		   if (note.serial == building().serial() && !is_dying_) {
			   switch (note.action) {
			   case Widelands::NoteBuilding::Action::kWorkersChanged:
				   update_worker_table();
				   break;
			   default:
				   break;
			   }
		   }
		});
	init(avoid_fastclick);
}

void ProductionSiteWindow::init(bool avoid_fastclick) {
	BuildingWindow::init(avoid_fastclick);
	const std::vector<Widelands::InputQueue*>& inputqueues = productionsite().inputqueues();

	if (inputqueues.size()) {
		// Add the wares tab
		UI::Box* prod_box = new UI::Box(
		   get_tabs(), 0, 0, UI::Box::Vertical, g_gr->get_xres() - 80, g_gr->get_yres() - 80);

		for (uint32_t i = 0; i < inputqueues.size(); ++i) {
			prod_box->add(
			   new InputQueueDisplay(prod_box, 0, 0, *igbase(), productionsite(), inputqueues[i]));
		}

		get_tabs()->add("wares", g_gr->images().get(pic_tab_wares), prod_box, _("Wares"));
	}

	// Add workers tab if applicable
	if (!productionsite().descr().nr_working_positions()) {
		worker_table_ = nullptr;
	} else {
		UI::Box* worker_box = new UI::Box(get_tabs(), 0, 0, UI::Box::Vertical);
		worker_table_ = new UI::Table<uintptr_t>(worker_box, 0, 0, 0, 100, UI::PanelStyle::kWui);
		worker_caps_ = new UI::Box(worker_box, 0, 0, UI::Box::Horizontal);

		worker_table_->add_column(
		   210, (ngettext("Worker", "Workers", productionsite().descr().nr_working_positions())));
		worker_table_->add_column(60, _("Exp"));
		worker_table_->add_column(150, _("Next Level"));

		for (unsigned int i = 0; i < productionsite().descr().nr_working_positions(); ++i) {
			worker_table_->add(i);
		}
		worker_table_->fit_height();

		if (igbase()->can_act(building().owner().player_number())) {
			worker_caps_->add_inf_space();
			UI::Button* evict_button =
			   new UI::Button(worker_caps_, "evict", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                  g_gr->images().get("images/wui/buildings/menu_drop_soldier.png"),
			                  _("Terminate the employment of the selected worker"));
			evict_button->sigclicked.connect(
			   boost::bind(&ProductionSiteWindow::evict_worker, boost::ref(*this)));
			worker_caps_->add(evict_button);
		}

		worker_box->add(worker_table_, UI::Box::Resizing::kFullSize);
		worker_box->add_space(4);
		worker_box->add(worker_caps_, UI::Box::Resizing::kFullSize);
		get_tabs()->add(
		   "workers", g_gr->images().get(pic_tab_workers), worker_box,
		   (ngettext("Worker", "Workers", productionsite().descr().nr_working_positions())));
		update_worker_table();
	}
	think();
}

void ProductionSiteWindow::think() {
	BuildingWindow::think();
	// If we have pending requests, update table each tick.
	// This is required to update from 'vacant' to 'coming'
	for (unsigned int i = 0; i < productionsite().descr().nr_working_positions(); ++i) {
		Widelands::Request* r = productionsite().working_positions()[i].worker_request;
		if (r) {
			update_worker_table();
			break;
		}
	}
}

void ProductionSiteWindow::update_worker_table() {
	if (worker_table_ == nullptr) {
		return;
	}
	assert(productionsite().descr().nr_working_positions() == worker_table_->size());

	for (unsigned int i = 0; i < productionsite().descr().nr_working_positions(); ++i) {
		const Widelands::Worker* worker = productionsite().working_positions()[i].worker;
		const Widelands::Request* request = productionsite().working_positions()[i].worker_request;
		UI::Table<uintptr_t>::EntryRecord& er = worker_table_->get_record(i);

		if (worker) {
			er.set_picture(0, worker->descr().icon(), worker->descr().descname());

			if (worker->get_current_experience() != Widelands::INVALID_INDEX &&
			    worker->descr().get_needed_experience() != Widelands::INVALID_INDEX) {
				assert(worker->descr().becomes() != Widelands::INVALID_INDEX);
				assert(worker->owner().tribe().has_worker(worker->descr().becomes()));

				// Fill upgrade status
				/** TRANSLATORS: %1% = the experience a worker has */
				/** TRANSLATORS: %2% = the experience a worker needs to reach the next level */
				er.set_string(1, (boost::format(_("%1%/%2%")) % worker->get_current_experience() %
				                  worker->descr().get_needed_experience())
				                    .str());
				er.set_string(
				   2, worker->owner().tribe().get_worker_descr(worker->descr().becomes())->descname());
			} else {
				// Worker is not upgradeable
				er.set_string(1, "—");
				er.set_string(2, "—");
			}
		} else if (request) {
			const Widelands::WorkerDescr* desc =
			   productionsite().owner().tribe().get_worker_descr(request->get_index());
			er.set_picture(0, desc->icon(), request->is_open() ? _("(vacant)") : _("(coming)"));

			er.set_string(1, "");
			er.set_string(2, "");
		} else {
			// Should only occur during cleanup
			continue;
		}
	}
}

void ProductionSiteWindow::evict_worker() {
	if (worker_table_->has_selection()) {
		Widelands::Worker* worker =
		   productionsite().working_positions()[worker_table_->get_selected()].worker;
		if (worker) {
			igbase()->game().send_player_evict_worker(*worker);
		}
	}
}
