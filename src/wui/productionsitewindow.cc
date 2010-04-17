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

#include "productionsitewindow.h"

#include "economy/request.h"
#include "logic/constructionsite.h"
#include "logic/militarysite.h"
#include "logic/trainingsite.h"
#include "logic/tribe.h"
#include "logic/worker.h"
#include "ui_basic/listselect.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"

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
	std::vector<Widelands::WaresQueue *> const & warequeues = ps.warequeues();

	if (warequeues.size()) {
		// Add the wares tab
		UI::Box * prod_box = new UI::Box
			(get_tabs(),
			0, 0, UI::Box::Vertical,
			g_gr->get_xres() - 80, g_gr->get_yres() - 80);

		for (uint32_t i = 0; i < warequeues.size(); ++i)
			create_ware_queue_panel(prod_box, ps, warequeues[i]);

		get_tabs()->add(g_gr->get_picture(PicMod_Game, pic_tab_wares), prod_box, _("Wares"));
	}

	// Add workers tab if applicable
	if (!productionsite().descr().nr_working_positions()) {
		m_worker_table = 0;
	} else {
		m_worker_table = new UI::Table<unsigned int>(get_tabs(), 0, 0, 340, 100);

		m_worker_table->add_column(150, _("Worker"));
		m_worker_table->add_column(40, _("Exp"));
		m_worker_table->add_column(150, _("Next Level"));

		for(unsigned int i = 0; i < productionsite().descr().nr_working_positions(); ++i)
			m_worker_table->add(i);

		get_tabs()->add
			(g_gr->get_picture(PicMod_UI, pic_tab_workers), m_worker_table,
			 productionsite().descr().nr_working_positions() > 1 ? _("Workers") : _("Worker"));
	}
}

void ProductionSite_Window::think()
{
	Building_Window::think();

	if (m_worker_table) {
		assert(productionsite().descr().nr_working_positions() == m_worker_table->size());

		for(unsigned int i = 0; i < productionsite().descr().nr_working_positions(); ++i) {
			const Widelands::Worker * worker = productionsite().working_positions()[i].worker;
			const Widelands::Request * request = productionsite().working_positions()[i].worker_request;
			UI::Table<unsigned int>::Entry_Record & er = m_worker_table->get_record(i);

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
						 worker->get_current_experience(), worker->get_needed_experience());

					er.set_string(1, buffer);
					er.set_string(2, worker->tribe().get_worker_descr(worker->becomes())->descname());
				} else {
					// Worker is not upgradeable
					er.set_string(1, "---");
					er.set_string(2, "---");
				}
			} else {
				const Widelands::Worker_Descr * desc =
					productionsite().tribe().get_worker_descr(request->get_index());
				er.set_picture(0, desc->icon(), request->is_open() ? _("(vacant)") : _("(coming)"));

				er.set_string(1, "");
				er.set_string(2, "");
			}
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
	new ProductionSite_Window(parent, *this, registry);
}
