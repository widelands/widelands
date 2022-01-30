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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
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
#include "wui/interactive_gamebase.h"

static char const* pic_tab_wares = "images/wui/buildings/menu_tab_wares.png";
static char const* pic_tab_workers = "images/wui/buildings/menu_list_workers.png";

constexpr int8_t kButtonSize = 34;

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
     worker_caps_(nullptr),
     worker_type_(nullptr),
     worker_xp_decrease_(nullptr),
     worker_xp_increase_(nullptr) {
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
				   worker_table_selection_changed();
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

	if (!inputqueues.empty()) {
		// Add the wares tab
		UI::Box* prod_box = new UI::Box(get_tabs(), UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical,
		                                g_gr->get_xres() - 80, g_gr->get_yres() - 80);
		ensure_box_can_hold_input_queues(*prod_box);

		for (Widelands::InputQueue* queue : inputqueues) {
			prod_box->add(
			   new InputQueueDisplay(prod_box, *ibase(), *production_site, *queue, false, true),
			   UI::Box::Resizing::kFullSize);
		}

		get_tabs()->add("wares", g_image_cache->get(pic_tab_wares), prod_box, _("Wares"));
	}

	// Add workers tab if applicable
	if (!production_site->descr().nr_working_positions()) {
		worker_table_ = nullptr;
	} else {
		UI::Box* worker_box = new UI::Box(get_tabs(), UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical);
		worker_table_ = new UI::Table<uintptr_t>(worker_box, 0, 0, 0, 100, UI::PanelStyle::kWui);
		worker_caps_ = new UI::Box(worker_box, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

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

		if (ibase()->omnipotent()) {
			worker_caps_->set_desired_size(100, 50);  // Prevent dropdown asserts
			worker_type_ = new UI::Dropdown<Widelands::DescriptionIndex>(
			   worker_caps_, "worker_type", 0, 0, 100, 8, kButtonSize, _("Worker"),
			   UI::DropdownType::kTextual, UI::PanelStyle::kWui, UI::ButtonStyle::kWuiMenu);
			worker_xp_decrease_ = new UI::Button(
			   worker_caps_, "xp_decrease", 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiMenu,
			   g_image_cache->get("images/ui_basic/scrollbar_down.png"),
			   _("Decrease experience by 1"));
			worker_xp_increase_ = new UI::Button(
			   worker_caps_, "xp_increase", 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiMenu,
			   g_image_cache->get("images/ui_basic/scrollbar_up.png"), _("Increase experience by 1"));
			worker_caps_->add(worker_type_, UI::Box::Resizing::kExpandBoth);
			worker_caps_->add_space(kButtonSize);
			worker_caps_->add(worker_xp_decrease_);
			worker_caps_->add(worker_xp_increase_);
			worker_caps_->add_space(kButtonSize);
			worker_type_->set_enabled(false);
			worker_table_->selected.connect([this](uint32_t) { worker_table_selection_changed(); });
			worker_type_->selected.connect([this]() { worker_table_dropdown_clicked(); });
			worker_xp_decrease_->sigclicked.connect([this]() { worker_table_xp_clicked(-1); });
			worker_xp_increase_->sigclicked.connect([this]() { worker_table_xp_clicked(1); });
		} else {
			worker_caps_->add_inf_space();
		}
		if (ibase()->can_act(production_site->owner().player_number())) {
			UI::Button* evict_button = new UI::Button(
			   worker_caps_, "evict", 0, 0, kButtonSize, kButtonSize, UI::ButtonStyle::kWuiMenu,
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
	worker_table_selection_changed();

	if (production_site->descr().type() == Widelands::MapObjectType::PRODUCTIONSITE) {
		// If this is not a productionsite, then we are a trainingsite window.
		// The derived class's init() will call initialization_complete().
		think();
		initialization_complete();
	}
}

void ProductionSiteWindow::clicked_watch() {
	if (Widelands::ProductionSite* p = production_site_.get(ibase()->egbase())) {
		if (upcast(InteractiveGameBase, igb, ibase())) {
			igb->show_watch_window(*p->working_positions()->begin()->worker.get(ibase()->egbase()));
		}
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

	if (watch_button_) {
		watch_button_->set_enabled(
		   production_site->working_positions()->begin()->worker.get(ibase()->egbase()));
	}

	// If we have pending requests, update table each tick.
	// This is required to update from 'vacant' to 'coming'
	for (unsigned int i = 0; i < production_site->descr().nr_working_positions(); ++i) {
		Widelands::Request* r = production_site->working_positions()->at(i).worker_request;
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
		   production_site->working_positions()->at(i).worker.get(ibase()->egbase());
		const Widelands::Request* request =
		   production_site->working_positions()->at(i).worker_request;
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
				er.set_string(1, format(_("%1%/%2%"), worker->get_current_experience(),
				                        worker->descr().get_needed_experience()));
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
		Widelands::Worker* worker = production_site->working_positions()
		                               ->at(worker_table_->get_selected())
		                               .worker.get(ibase()->egbase());
		if (worker) {
			if (game_) {
				game_->send_player_evict_worker(*worker);
			} else {
				NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
			}
		}
	}
}

void ProductionSiteWindow::worker_table_selection_changed() {
	Widelands::ProductionSite* ps = production_site_.get(ibase()->egbase());
	if (!ps || !ibase()->omnipotent()) {
		return;
	}

	assert(worker_table_);
	assert(worker_type_);
	assert(worker_xp_decrease_);
	assert(worker_xp_increase_);

	worker_type_->clear();
	if (worker_table_->has_selection()) {
		const std::vector<std::pair<Widelands::DescriptionIndex, Widelands::Quantity>>
		   working_positions = ps->descr().working_positions();
		const size_t selected_index = worker_table_->get_selected();
		const Widelands::Worker* worker =
		   ps->working_positions()->at(selected_index).worker.get(ibase()->egbase());

		Widelands::DescriptionIndex di = Widelands::INVALID_INDEX;
		size_t i = 0;
		for (const auto& pair : working_positions) {
			if (i + pair.second > selected_index) {
				di = pair.first;
				break;
			}
			i += pair.second;
		}
		assert(di != Widelands::INVALID_INDEX);

		worker_type_->set_enabled(true);
		worker_type_->add(_("(vacant)"), Widelands::INVALID_INDEX, nullptr, worker == nullptr);
		const Widelands::WorkerDescr* descr = ibase()->egbase().descriptions().get_worker_descr(di);
		while (descr) {
			worker_type_->add(
			   descr->descname(), di, descr->icon(), worker && &worker->descr() == descr);
			di = descr->becomes();
			descr = ibase()->egbase().descriptions().get_worker_descr(di);
		}
		update_worker_xp_buttons(worker);
	} else {
		worker_type_->set_enabled(false);
		update_worker_xp_buttons(nullptr);
	}
}

void ProductionSiteWindow::update_worker_xp_buttons(const Widelands::Worker* w) {
	if (w && !w->needs_experience()) {
		w = nullptr;
	}
	const int32_t cur = w ? w->get_current_experience() : 0;
	const int32_t max = w ? w->descr().get_needed_experience() - 1 : 0;
	assert(cur >= 0);
	assert(max >= 0);
	worker_xp_decrease_->set_enabled(cur > 0);
	worker_xp_increase_->set_enabled(cur < max);
}

void ProductionSiteWindow::worker_table_dropdown_clicked() {
	Widelands::ProductionSite* ps = production_site_.get(ibase()->egbase());
	if (ps == nullptr) {
		return;
	}

	assert(ibase()->omnipotent());
	assert(worker_table_);
	assert(worker_table_->has_selection());
	assert(worker_type_);
	assert(worker_type_->has_selection());
	const Widelands::DescriptionIndex selected = worker_type_->get_selected();

	const size_t selected_index = worker_table_->get_selected();
	Widelands::Worker* worker =
	   ps->working_positions()->at(selected_index).worker.get(ibase()->egbase());

	const Widelands::DescriptionIndex current =
	   worker ? ibase()->egbase().descriptions().safe_worker_index(worker->descr().name()) :
               Widelands::INVALID_INDEX;
	if (current == selected) {
		return;
	}
	if (worker) {
		worker->remove(ibase()->egbase());
	}
	if (selected != Widelands::INVALID_INDEX) {
#ifndef NDEBUG
		const bool success =
#endif
		   ps->warp_worker(ibase()->egbase(),
		                   *ibase()->egbase().descriptions().get_worker_descr(selected),
		                   selected_index);
#ifndef NDEBUG
		assert(success);
#endif
		Notifications::publish(
		   Widelands::NoteBuilding(ps->serial(), Widelands::NoteBuilding::Action::kWorkersChanged));
	}
	worker_table_selection_changed();
}

void ProductionSiteWindow::worker_table_xp_clicked(int8_t delta) {
	Widelands::ProductionSite* ps = production_site_.get(ibase()->egbase());
	if (ps == nullptr || delta == 0) {
		return;
	}

	assert(ibase()->omnipotent());
	assert(worker_table_);
	assert(worker_table_->has_selection());

	const size_t selected_index = worker_table_->get_selected();
	Widelands::Worker* worker =
	   ps->working_positions()->at(selected_index).worker.get(ibase()->egbase());
	if (!worker) {
		return;
	}

	assert(worker->needs_experience());
	const int32_t max_xp = worker->descr().get_needed_experience() - 1;
	worker->set_current_experience(
	   std::max(0, std::min(max_xp, worker->get_current_experience() + delta)));
	Notifications::publish(
	   Widelands::NoteBuilding(ps->serial(), Widelands::NoteBuilding::Action::kWorkersChanged));
	update_worker_xp_buttons(worker);
}
