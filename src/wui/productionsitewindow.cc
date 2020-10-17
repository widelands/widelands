/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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
ProductionSiteWindow::ProductionSiteWindow(InteractiveBase& parent,
                                           UI::UniqueWindow::Registry& reg,
                                           Widelands::ProductionSite& ps,
                                           bool avoid_fastclick,
                                           bool workarea_preview_wanted)
   : BuildingWindow(parent, reg, ps, avoid_fastclick),
     production_site_(&ps),
     worker_table_(nullptr),
     worker_caps_(nullptr) {
	productionsitenotes_subscriber_ = Notifications::subscribe<Widelands::NoteBuilding>(
	   [this](const Widelands::NoteBuilding& note) {
		   if (is_dying_) {
			   return;
		   }
		   Widelands::ProductionSite* production_site = production_site_.get(ibase()->egbase());
		   if (production_site == nullptr) {
			   return;
		   }
		   if (note.serial == production_site->serial()) {
			   switch (note.action) {
			   case Widelands::NoteBuilding::Action::kWorkersChanged:
				   update_worker_table(production_site);
				   break;
			   default:
				   break;
			   }
		   }
	   });
	init(avoid_fastclick, workarea_preview_wanted);
}

void ProductionSiteWindow::init(bool avoid_fastclick, bool workarea_preview_wanted) {
	Widelands::ProductionSite* production_site = production_site_.get(ibase()->egbase());
	assert(production_site != nullptr);

	BuildingWindow::init(avoid_fastclick, workarea_preview_wanted);
	const std::vector<Widelands::InputQueue*>& inputqueues = production_site->inputqueues();

	if (inputqueues.size()) {
		// Add the wares tab
		UI::Box* prod_box = new UI::Box(
		   get_tabs(), 0, 0, UI::Box::Vertical, g_gr->get_xres() - 80, g_gr->get_yres() - 80);

		for (const Widelands::InputQueue* queue : inputqueues) {
			prod_box->add(new InputQueueDisplay(prod_box, 0, 0, *ibase(), *production_site, *queue));
		}

		get_tabs()->add("wares", g_image_cache->get(pic_tab_wares), prod_box, _("Wares"));
	}

	// Add workers tab if applicable
	if (!production_site->descr().nr_working_positions()) {
		worker_table_ = nullptr;
	} else {
		UI::Box* worker_box = new UI::Box(get_tabs(), 0, 0, UI::Box::Vertical);
		worker_table_ = new UI::Table<uintptr_t>(worker_box, 0, 0, 0, 100, UI::PanelStyle::kWui);
		worker_caps_ = new UI::Box(worker_box, 0, 0, UI::Box::Horizontal);

		const std::string workers_heading =
		   (production_site->descr().nr_working_positions() == 1) ?
		      /** TRANSLATORS: Header in production site window if there is 1 worker */
		      _("Worker") :
		      /** TRANSLATORS: Header in production site window if there is more than 1 worker. If you
		         need plural forms here, please let us know. */
		      _("Workers");

		worker_table_->add_column(210, workers_heading);
		worker_table_->add_column(60, _("Exp"));
		worker_table_->add_column(150, _("Next Level"));

		for (unsigned int i = 0; i < production_site->descr().nr_working_positions(); ++i) {
			worker_table_->add(i);
		}
		worker_table_->fit_height();

		if (ibase()->can_act(production_site->owner().player_number())) {
			worker_caps_->add_inf_space();
			UI::Button* evict_button =
			   new UI::Button(worker_caps_, "evict", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                  g_image_cache->get("images/wui/buildings/menu_drop_soldier.png"),
			                  _("Terminate the employment of the selected worker"));
			evict_button->sigclicked.connect([this]() { evict_worker(); });
			worker_caps_->add(evict_button);
		}

		worker_box->add(worker_table_, UI::Box::Resizing::kFullSize);
		worker_box->add_space(4);
		worker_box->add(worker_caps_, UI::Box::Resizing::kFullSize);
		get_tabs()->add("workers", g_image_cache->get(pic_tab_workers), worker_box, workers_heading);
		update_worker_table(production_site);
	}

	if (production_site->descr().type() == Widelands::MapObjectType::PRODUCTIONSITE) {
		// If this is not a productionsite, then we are a trainingsite window.
		// The derived class's init() will call initialization_complete().
		think();
		initialization_complete();
	}
}

void ProductionSiteWindow::think() {
	// BuildingWindow::think() will call die in case we are no longer in
	// existance.
	BuildingWindow::think();

	Widelands::ProductionSite* production_site = production_site_.get(ibase()->egbase());
	if (production_site == nullptr) {
		return;
	}

	// If we have pending requests, update table each tick.
	// This is required to update from 'vacant' to 'coming'
	for (unsigned int i = 0; i < production_site->descr().nr_working_positions(); ++i) {
		Widelands::Request* r = production_site->working_positions()[i].worker_request;
		if (r) {
			update_worker_table(production_site);
			break;
		}
	}
}

void ProductionSiteWindow::update_worker_table(Widelands::ProductionSite* production_site) {
	assert(production_site != nullptr);

	if (worker_table_ == nullptr) {
		return;
	}

	assert(production_site->descr().nr_working_positions() == worker_table_->size());

	for (unsigned int i = 0; i < production_site->descr().nr_working_positions(); ++i) {
		const Widelands::Worker* worker =
		   production_site->working_positions()[i].worker.get(ibase()->egbase());
		const Widelands::Request* request = production_site->working_positions()[i].worker_request;
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
			   production_site->owner().tribe().get_worker_descr(request->get_index());
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
	Widelands::ProductionSite* production_site = production_site_.get(ibase()->egbase());
	if (production_site == nullptr) {
		return;
	}

	if (worker_table_->has_selection()) {
		Widelands::Worker* worker =
		   production_site->working_positions()[worker_table_->get_selected()].worker.get(
		      ibase()->egbase());
		if (worker) {
			if (game_) {
				game_->send_player_evict_worker(*worker);
			} else {
				NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
			}
		}
	}
}
