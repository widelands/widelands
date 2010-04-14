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
#include "ui_basic/textarea.h"

using Widelands::ProductionSite;

static char const * pic_list_worker = "pics/menu_list_workers.png";

/**
 * The window listing workers in a \ref ProductionSite
 */
struct ProductionSite_Window_ListWorkerWindow : public UI::Window {
	ProductionSite_Window_ListWorkerWindow
		(Interactive_GameBase & parent, ProductionSite & ps)
		:
		UI::Window              (&parent, 0, 0, 340, 100, _("Worker Listing")),
		m_ps_location           (ps.get_position()),
		m_ps                    (ps),
		m_list                  (*this),
		m_type_label            (*this),
		m_type_value_label      (*this),
		m_experience_label      (*this),
		m_experience_value_label(*this),
		m_becomes_label         (*this),
		m_becomes_value_label   (*this)
	{
		center_to_parent();
		move_to_top();
	}

	Interactive_GameBase & iaplayer() const {
		return ref_cast<Interactive_GameBase, UI::Panel>(*get_parent());
	}

	virtual void think();

private:
	void update();
	void fill_list();

	Widelands::Coords                     m_ps_location;
	ProductionSite                      & m_ps;
	struct List : public UI::Listselect<Widelands::Worker const *> {
		List                  (ProductionSite_Window_ListWorkerWindow & parent) :
			UI::Listselect<Widelands::Worker const *> (&parent, 5, 5, 155, 90)
		{}
	} m_list;

	struct Type_Label             : public UI::Textarea {
		Type_Label            (ProductionSite_Window_ListWorkerWindow & parent) :
			UI::Textarea
				(&parent, 165, 5, 150, 20, _("Type: "), UI::Align_CenterLeft)
		{}
	} m_type_label;

	struct Type_Value_Label       : public UI::Textarea {
		Type_Value_Label      (ProductionSite_Window_ListWorkerWindow & parent) :
			UI::Textarea
				(&parent, 245, 5, 200, 20, "---", UI::Align_CenterLeft)
		{}
	} m_type_value_label;

	struct Experience_Label       : public UI::Textarea {
		Experience_Label      (ProductionSite_Window_ListWorkerWindow & parent) :
			UI::Textarea
				(&parent,
				 165, 30, 150, 20,
				 _("Experience: "), UI::Align_CenterLeft)
		{}
	} m_experience_label;

	struct Experience_Value_Label : public UI::Textarea {
		Experience_Value_Label(ProductionSite_Window_ListWorkerWindow & parent) :
			UI::Textarea
				(&parent,
				 245, 30, 200, 20, "---", UI::Align_CenterLeft)
		{}
	} m_experience_value_label;

	struct Becomes_Label          : public UI::Textarea {
		Becomes_Label         (ProductionSite_Window_ListWorkerWindow & parent) :
			UI::Textarea
				(&parent,
				 165, 55, 70, 20,
				 _("Trying to become: "), UI::Align_CenterLeft)
		{}
	} m_becomes_label;

	struct Becomes_Value_Label    : public UI::Textarea {
		Becomes_Value_Label   (ProductionSite_Window_ListWorkerWindow & parent) :
			UI::Textarea
				(&parent, 180, 70, 200, 20, "---", UI::Align_CenterLeft)
		{}
	} m_becomes_value_label;
};


void ProductionSite_Window_ListWorkerWindow::think() {
	Widelands::BaseImmovable const * const base_immovable =
		iaplayer().egbase().map()[m_ps_location].get_immovable();
	if
		(not dynamic_cast<const Widelands::Building *>(base_immovable)
		 or
		 dynamic_cast<const Widelands::ConstructionSite *>(base_immovable))
	{
		// The Productionsite has been removed. Die quickly.
		die();
		return;
	}

	fill_list();
	UI::Window::think();
}

/*
 * fill list()
 */
void ProductionSite_Window_ListWorkerWindow::fill_list() {
	uint32_t const m_last_select = m_list.selection_index();
	m_list.clear();

	uint32_t const nr_working_positions = m_ps.descr().nr_working_positions();
	for (uint32_t i = 0; i < nr_working_positions; ++i)
		if
			(Widelands::Worker const * const worker =
			 	m_ps.working_positions()[i].worker)
			m_list.add(worker->descname().c_str(), worker, worker->icon());
		else
			m_list.add
				(m_ps.working_positions()[i].worker_request->is_open() ?
				 _("(vacant)") : _("(coming)"),
				 0);
	if      (m_last_select < m_list.size())
		m_list.select(m_last_select);
	else if (m_list.size())
		m_list.select(m_list.size() - 1);

	update();
}

/**
 * \brief Update worker info subwindow, following user selection
 */
void ProductionSite_Window_ListWorkerWindow::update()
{
	if (m_list.has_selection() and m_list.get_selected()) {
		Widelands::Worker      const & worker = *m_list.get_selected();
		Widelands::Tribe_Descr const & tribe  = worker.tribe();

		m_type_value_label.set_text(worker.descname());

		if
			(worker.get_current_experience() != -1
			 and
			 worker.get_needed_experience () != -1)
		{
			assert(worker.becomes());

			// Fill upgrade status
			char buffer[7];
			snprintf
				(buffer, sizeof(buffer),
				 "%i/%i",
				 worker.get_current_experience(), worker.get_needed_experience());
			m_experience_value_label.set_text(buffer);

			// Get the descriptive name of the ongoing upgrade
			m_becomes_value_label.set_text
				(tribe.get_worker_descr(worker.becomes())->descname());

		} else {
			// Worker is not upgradeable
			m_experience_value_label.set_text("---");
			m_becomes_value_label   .set_text("---");
		}
	} else {
		m_type_value_label      .set_text("---");
		m_becomes_value_label   .set_text("---");
		m_experience_value_label.set_text("---");
		m_becomes_value_label   .set_text("---");
	}
}

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
	UI::Box * prod_box = 0;
	if
		(not dynamic_cast<Widelands::TrainingSite const *>(&ps) and
		 not dynamic_cast<Widelands::MilitarySite const *>(&ps))
	{
		prod_box = new Production_Box (*this, *this, ps);
		fit_inner(*prod_box);
		move_inside_parent();
	}
}

ProductionSite_Window::Production_Box::Production_Box
	(UI::Panel & parent, ProductionSite_Window & window, ProductionSite & ps)
	:
	UI::Box
		(&parent,
		 0, 0,
		 UI::Box::Vertical,
		 g_gr->get_xres() - 80, g_gr->get_yres() - 80)
{

	// Add the wares queue
	std::vector<Widelands::WaresQueue *> const & warequeues = ps.warequeues();
	for (uint32_t i = 0; i < warequeues.size(); ++i)
		window.create_ware_queue_panel(this, ps, warequeues[i]);

	add_space(8);

	// Add caps buttons
	add(window.create_capsbuttons(this), UI::Box::AlignLeft);

	// Add list worker button
	add
		(new UI::Callback_Button<ProductionSite_Window>
		 	(this,
		 	 0, 0, 32, 32,
		 	 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 	 g_gr->get_picture(PicMod_Game,  pic_list_worker),
		 	 &ProductionSite_Window::list_worker_clicked, window,
		 	 _("Show worker listing")),
		 UI::Box::AlignLeft);
}


/*
 * List worker button has been clicked
 */
void ProductionSite_Window::list_worker_clicked() {
	assert(m_registry == this);

	m_registry =
		new ProductionSite_Window_ListWorkerWindow(igbase(), productionsite());
	die();
}

/*
===============
Make sure the window is redrawn when necessary.
===============
*/
void ProductionSite_Window::think()
{
	Building_Window::think();
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

