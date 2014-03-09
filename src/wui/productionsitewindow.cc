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

#include "wui/productionsitewindow.h"

#include <libintl.h>

#include "economy/request.h"
#include "graphic/graphic.h"
#include "logic/constructionsite.h"
#include "logic/militarysite.h"
#include "logic/trainingsite.h"
#include "logic/tribe.h"
#include "logic/worker.h"
#include "ui_basic/listselect.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "wui/waresqueuedisplay.h"

using Widelands::ProductionSite;

static char const * pic_tab_wares = "pics/menu_tab_wares.png";
static char const * pic_tab_workers = "pics/menu_list_workers.png";

/*
===============
Create the window and its panels, add it to the registry.
===============
*/
ProductionSite_Window::ProductionSite_Window
	(Interactive_GameBase & parent,
	 ProductionSite       & ps,
	 UI::Window *         & registry)
	: Building_Window(parent, ps, registry)
{
	const std::vector<Widelands::WaresQueue*>& warequeues = ps.warequeues();

	if (warequeues.size()) {
		// Add the wares tab
		UI::Box * prod_box = new UI::Box
			(get_tabs(),
			0, 0, UI::Box::Vertical,
			g_gr->get_xres() - 80, g_gr->get_yres() - 80);

		for (uint32_t i = 0; i < warequeues.size(); ++i)
			prod_box->add
				(new WaresQueueDisplay(prod_box, 0, 0, igbase(), ps, warequeues[i]),
				 UI::Box::AlignLeft);

		get_tabs()->add
			("wares", g_gr->images().get(pic_tab_wares),
			 prod_box, _("Wares"));
	}

	// Add workers tab if applicable
	if (!productionsite().descr().nr_working_positions()) {
		m_worker_table = nullptr;
	} else {
		UI::Box * worker_box = new UI::Box
			(get_tabs(),
			 0, 0, UI::Box::Vertical);
		m_worker_table = new UI::Table<uintptr_t>(worker_box, 0, 0, 0, 100);
		m_worker_caps = new UI::Box(worker_box, 0, 0, UI::Box::Horizontal);

		m_worker_table->add_column(150, (ngettext
			("Worker", "Workers", productionsite().descr().nr_working_positions())
		));
		m_worker_table->add_column(60, _("Exp"));
		m_worker_table->add_column(150, _("Next Level"));

		for
			(unsigned int i = 0;
			 i < productionsite().descr().nr_working_positions(); ++i)
			m_worker_table->add(i);

		if (igbase().can_act(building().owner().player_number())) {
			m_worker_caps->add_inf_space();
			UI::Button * evict_button = new UI::Button
							(m_worker_caps, "evict", 0, 0, 34, 34,
							 g_gr->images().get("pics/but4.png"),
							 g_gr->images().get("pics/menu_drop_soldier.png"),
							 _("Terminate the employment of the selected worker"));
			evict_button->sigclicked.connect
					(boost::bind(&ProductionSite_Window::evict_worker, boost::ref(*this)));
			m_worker_caps->add(evict_button, UI::Box::AlignCenter);
		}

		worker_box->add(m_worker_table, UI::Box::AlignLeft, true);
		worker_box->add(m_worker_caps, UI::Box::AlignLeft, true);
		get_tabs()->add
			("workers", g_gr->images().get(pic_tab_workers),
			 worker_box,
			 (ngettext("Worker", "Workers", productionsite().descr().nr_working_positions()))
			);
		update_worker_table();
	}
}

void ProductionSite_Window::think()
{
	Building_Window::think();
	// If we have pending requests, update table each tick.
	// This is required to update from 'vacant' to 'coming'
	for
		(unsigned int i = 0;
			i < productionsite().descr().nr_working_positions(); ++i)
	{
		Widelands::Request* r = productionsite().working_positions()[i].worker_request;
		if (r) {
			update_worker_table();
			break;
		}
	}
}

/*
===============
Create the production site information window.
===============
*/
void ProductionSite::create_options_window
	(Interactive_GameBase & parent, UI::Window * & registry)
{
	ProductionSite_Window* win = new ProductionSite_Window(parent, *this, registry);
	Building::options_window_connections.push_back
		(Building::workers_changed.connect(boost::bind
			(&ProductionSite_Window::update_worker_table, boost::ref(*win))));
}


void ProductionSite_Window::update_worker_table()
{
	if (m_worker_table == nullptr) {
		return;
	}
	assert
		(productionsite().descr().nr_working_positions() ==
			m_worker_table->size());

	for
		(unsigned int i = 0;
			i < productionsite().descr().nr_working_positions(); ++i)
	{
		const Widelands::Worker * worker =
			productionsite().working_positions()[i].worker;
		const Widelands::Request * request =
			productionsite().working_positions()[i].worker_request;
		UI::Table<uintptr_t>::Entry_Record & er =
			m_worker_table->get_record(i);

		if (worker) {
			er.set_picture(0, worker->icon(), worker->descname());

			if
				(worker->get_current_experience() != -1
					and
					worker->get_needed_experience () != -1)
			{
				assert(worker->becomes());

				// Fill upgrade status
				char buffer[7];
				snprintf
					(buffer, sizeof(buffer),
						"%i/%i",
						worker->get_current_experience(),
						worker->get_needed_experience());

				er.set_string(1, buffer);
				er.set_string
					(2, worker->tribe().get_worker_descr
						(worker->becomes())->descname());
			} else {
				// Worker is not upgradeable
				er.set_string(1, "---");
				er.set_string(2, "---");
			}
		} else if (request) {
			const Widelands::Worker_Descr * desc =
				productionsite().tribe().get_worker_descr(request->get_index());
			er.set_picture
				(0, desc->icon(),
					request->is_open() ? _("(vacant)") : _("(coming)"));

			er.set_string(1, "");
			er.set_string(2, "");
		} else {
			// Should only occur during cleanup
			continue;
		}
	}
	m_worker_table->update();
}

void ProductionSite_Window::evict_worker() {
	if (m_worker_table->has_selection()) {
		Widelands::Worker * worker =
			productionsite().working_positions()[m_worker_table->get_selected()].worker;
		if (worker) {
			igbase().game().send_player_evict_worker(*worker);
		}
	}
}
