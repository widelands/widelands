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
/*
This file contains the options windows that are displayed when you click on
a building, plus the necessary hook function(s) in the class Building itself.

This is separated out because options windows should _never_ manipulate
buildings directly. Instead, they must send a player command through the Game
class.
*/

#include <boost/format.hpp>
using boost::format;

#include "buildingwindow.h"
#include "bulldozeconfirm.h"
#include "logic/constructionsite.h"
#include "game_debug_ui.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "i18n.h"
#include "interactive_player.h"
#include "logic/maphollowregion.h"
#include "logic/militarysite.h"
#include "economy/request.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "waresdisplay.h"
#include "economy/wares_queue.h"
#include "waresqueuedisplay.h"

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/listselect.h"
#include "ui_basic/progressbar.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

#include "upcast.h"

#include <SDL_types.h>
#include <sys/types.h>

#include <list>

using Widelands::Building;
using Widelands::Building_Index;
using Widelands::ConstructionSite;
using Widelands::MilitarySite;
using Widelands::ProductionSite;
using Widelands::Soldier;
using Widelands::TrainingSite;
using Widelands::Warehouse;
using Widelands::atrAttack;
using Widelands::atrDefense;
using Widelands::atrEvade;
using Widelands::atrHP;

static char const * pic_list_worker        = "pics/menu_list_workers.png";

static char const * pic_tab_military       = "pics/menu_tab_military.png";
static char const * pic_tab_training       = "pics/menu_tab_training.png";
static char const * pic_up_train           = "pics/menu_up_train.png";
static char const * pic_down_train         = "pics/menu_down_train.png";
static char const * pic_drop_soldier       = "pics/menu_drop_soldier.png";

/*
==============================================================================

Building UI IMPLEMENTATION

==============================================================================
*/

/*
===============
Create the building's options window if necessary.
===============
*/
void Building::show_options(Interactive_GameBase & igbase)
{
	if (m_optionswindow)
		m_optionswindow->move_to_top();
	else
		create_options_window(igbase, m_optionswindow);
}

/*
===============
Force the destruction of the options window.
===============
*/
void Building::hide_options() {delete m_optionswindow;}


struct ProductionSite_Window : public Building_Window {
	ProductionSite_Window
		(Interactive_GameBase & parent,
		 ProductionSite       &,
		 UI::Window *         & registry);

	ProductionSite & productionsite() {
		return ref_cast<ProductionSite, Building>(building());
	}

	virtual void think();

public:
	void list_worker_clicked();
protected:
	struct Production_Box : public UI::Box {
		Production_Box
			(UI::Panel & parent, ProductionSite_Window &, ProductionSite &);
	};
};



/*
==============================================================================

ProductionSite UI IMPLEMENTATION

==============================================================================
*/

/*
 * ProductionSite_Window_ListWorkerWindow
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
		(not dynamic_cast<const Building *>(base_immovable)
		 or
		 dynamic_cast<const ConstructionSite *>(base_immovable))
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
		(not dynamic_cast<TrainingSite const *>(&ps) and
		 not dynamic_cast<MilitarySite const *>(&ps))
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


/*
==============================================================================

MilitarySite UI IMPLEMENTATION

==============================================================================
*/

struct MilitarySite_Window : public Building_Window {
	MilitarySite_Window
		(Interactive_GameBase & parent,
		 MilitarySite       &,
		 UI::Window *       & registry);

	MilitarySite & militarysite() {
		return ref_cast<MilitarySite, Building>(building());
	}

	virtual void think();
private:
	void update();
	void drop_button_clicked ();
	void soldier_capacity_up () {act_change_soldier_capacity (1);}
	void soldier_capacity_down() {act_change_soldier_capacity(-1);}

	Widelands::Coords               m_ms_location;
	UI::Box                         m_vbox;
	UI::Table<Soldier &>            m_table;
	UI::Callback_Button<MilitarySite_Window> m_drop_button;
	UI::Box                         m_bottom_box;
	UI::Panel                       m_capsbuttons;
	UI::Textarea                    m_capacity;
	UI::Callback_Button<MilitarySite_Window> m_capacity_down;
	UI::Callback_Button<MilitarySite_Window> m_capacity_up;
};


MilitarySite_Window::MilitarySite_Window
	(Interactive_GameBase & parent,
	 MilitarySite       & ms,
	 UI::Window *       & registry)
	:
	Building_Window(parent, ms, registry),
	m_ms_location  (ms.get_position()),
	m_vbox         (this, 5, 5, UI::Box::Vertical),
	m_table        (&m_vbox, 0, 0, 360, 200),
	m_drop_button
		(&m_vbox,
		 0, 0, 360, 32,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_Game, pic_drop_soldier),
		 &MilitarySite_Window::drop_button_clicked, *this),
	m_bottom_box   (&m_vbox, 0, 0, UI::Box::Horizontal),
	m_capsbuttons  (&m_bottom_box, 0, 34, 34 * 7, 34),
	m_capacity     (&m_bottom_box, 0, 0, _("Capacity"), UI::Align_Right),
	m_capacity_down
		(&m_bottom_box,
		 0, 0, 24, 24,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_Game, pic_down_train),
		 &MilitarySite_Window::soldier_capacity_down, *this),
	m_capacity_up
		(&m_bottom_box,
		 0, 0, 24, 24,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_Game, pic_up_train),
		 &MilitarySite_Window::soldier_capacity_up,   *this)
{
	//  soldiers view
	m_table.add_column(100, _("Name"));
	m_table.add_column (40, _("HP"));
	m_table.add_column (40, _("AT"));
	m_table.add_column (40, _("DE"));
	m_table.add_column (40, _("EV"));
	m_table.add_column(100, _("Level")); // enough space for scrollbar
	m_vbox.add(&m_table, UI::Box::AlignCenter);

	m_vbox.add_space(8);

	// Add drop soldier button
	m_vbox.add(&m_drop_button, UI::Box::AlignCenter);


	//  Add the bottom row of buttons.

	m_bottom_box.add_space(5);

	// Add the caps button
	create_capsbuttons(&m_capsbuttons);
	m_bottom_box.add(&m_capsbuttons, UI::Box::AlignLeft);

	// Capacity buttons
	m_bottom_box.add(&m_capacity,      UI::Box::AlignRight);
	m_bottom_box.add_space(8);
	m_bottom_box.add(&m_capacity_down, UI::Box::AlignRight);
	m_bottom_box.add(&m_capacity_up,   UI::Box::AlignRight);
	m_capacity_down.set_repeating(true);
	m_capacity_up  .set_repeating(true);

	m_bottom_box.add_space(5);

	fit_inner(m_bottom_box);

	m_vbox.add_space(8);

	m_vbox.add(&m_bottom_box, UI::Box::AlignCenter);

	fit_inner(m_vbox);

	move_inside_parent();
}


void MilitarySite_Window::think()
{
	Building_Window::think();

	Widelands::BaseImmovable const * const base_immovable =
		igbase().egbase().map()[m_ms_location].get_immovable();
	if
		(not dynamic_cast<const Building *>(base_immovable)
		 or
		 dynamic_cast<const ConstructionSite *>(base_immovable))
	{
		// The Site has been removed. Die quickly.
		die();
		return;
	}
	update();
}

/*
==============
Update the listselect, maybe there are new soldiers
=============
*/
void MilitarySite_Window::update() {
	MilitarySite const & ms = militarysite();
	std::vector<Soldier *> soldiers = ms.presentSoldiers();

	if (soldiers.size() < m_table.size())
		m_table.clear();

	for (uint32_t i = 0; i < soldiers.size(); ++i) {
		Soldier & s = *soldiers[i];
		UI::Table<Soldier &>::Entry_Record * er =
			m_table.find(s);
		if (not er)
			er = &m_table.add(s);
		uint32_t const  hl = s.get_hp_level         ();
		uint32_t const mhl = s.get_max_hp_level     ();
		uint32_t const  al = s.get_attack_level     ();
		uint32_t const mal = s.get_max_attack_level ();
		uint32_t const  dl = s.get_defense_level    ();
		uint32_t const mdl = s.get_max_defense_level();
		uint32_t const  el = s.get_evade_level      ();
		uint32_t const mel = s.get_max_evade_level  ();
		er->set_string(0, s.descname().c_str());
		char buffer[sizeof("4294967295 / 4294967295")];
		sprintf(buffer,  "%u / %u", hl,                mhl);
		er->set_string(1, buffer);
		sprintf(buffer,  "%u / %u",      al,                 mal);
		er->set_string(2, buffer);
		sprintf(buffer,  "%u / %u",           dl,                  mdl);
		er->set_string(3, buffer);
		sprintf(buffer,  "%u / %u",                el,                   mel);
		er->set_string(4, buffer);
		sprintf(buffer, "%2u / %u", hl + al + dl + el, mhl + mal + mdl + mel);
		er->set_string(5, buffer);
	}
	m_table.sort();

	uint32_t const capacity     = ms.   soldierCapacity();
	char buffer[200];
	snprintf(buffer, sizeof(buffer), _("Capacity: %2u"), capacity);
	m_capacity.set_text (buffer);
	uint32_t const capacity_min = ms.minSoldierCapacity();
	bool const can_act = igbase().can_act(ms.owner().player_number());
	m_drop_button.set_enabled
		(can_act and m_table.has_selection() and capacity_min < m_table.size());
	m_capacity_down.set_enabled
		(can_act and capacity_min < capacity);
	m_capacity_up  .set_enabled
		(can_act and                capacity < ms.maxSoldierCapacity());
}

void MilitarySite_Window::drop_button_clicked()
{
	assert(m_table.has_selection());
	act_drop_soldier(m_table.get_selected().serial());
}

/*
===============
Create the production site information window.
===============
*/
void MilitarySite::create_options_window
	(Interactive_GameBase & plr, UI::Window * & registry)
{
	new MilitarySite_Window(plr, *this, registry);
}


/*
==============================================================================

TrainingSite UI IMPLEMENTATION

==============================================================================
*/

struct TrainingSite_Window : public ProductionSite_Window {
	TrainingSite_Window
		(Interactive_GameBase & parent, TrainingSite &, UI::Window * & registry);

	TrainingSite & trainingsite() {
		return ref_cast<TrainingSite, Building>(building());
	}

	virtual void think();

private:
	void update();

	Widelands::Coords      m_ms_location;

	struct Tab_Panel : public UI::Tab_Panel {
		Tab_Panel(TrainingSite_Window & parent, TrainingSite & ts) :
			UI::Tab_Panel
				(&parent, 0, 0, g_gr->get_picture(PicMod_UI, "pics/but1.png")),
			m_prod_box(*this, parent, ts),
			m_sold_box(*this)
		{
			set_snapparent(true);
			m_prod_box.resize();
			add(g_gr->get_picture(PicMod_Game, pic_tab_training), &m_prod_box);
			add(g_gr->get_picture(PicMod_Game, pic_tab_military), &m_sold_box);
			resize();
		}

		Production_Box         m_prod_box;

		struct Sold_Box : public UI::Box {
			Sold_Box(Tab_Panel & parent) :
				UI::Box(&parent, 0,  0, UI::Box::Vertical),
				m_table                (*this),
				m_drop_selected_soldier(*this),
				m_capacity_box         (*this)
			{
				resize();
			}

			struct Table : public UI::Table<Soldier &> {
				Table(UI::Box & parent) :
					UI::Table<Soldier &>(&parent, 0,  0, 360, 200)
				{
					add_column(100, _("Name"));
					add_column (40, _("HP"));
					add_column (40, _("AT"));
					add_column (40, _("DE"));
					add_column (40, _("EV"));
					add_column(100, _("Level")); //  enough space for scrollbar
					parent.add (this, UI::Align_Left);
				}
			}                      m_table;

			struct Drop_Selected_Soldier : public UI::Button {
				Drop_Selected_Soldier(UI::Box & sold_box) :
					UI::Button
						(&sold_box,
						 0, 0, 360, 32,
						 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
						 g_gr->get_picture(PicMod_Game, pic_drop_soldier))
				{
					sold_box.add(this, UI::Align_Left);
				}
				void clicked();
			} m_drop_selected_soldier;

			struct Capacity_Box : public UI::Box {
				Capacity_Box(Sold_Box & parent) :
					UI::Box               (&parent, 0,  0, UI::Box::Horizontal),
					m_capacity_label      (*this),
					m_capacity_decrement  (*this),
					m_capacity_value_label(*this),
					m_capacity_increment  (*this)
				{
					parent.add(this, UI::Align_Left);
				}

				struct Capacity_Label : public UI::Textarea {
					Capacity_Label(Capacity_Box & parent) :
						UI::Textarea(&parent, 0, 11, _("Capacity"), UI::Align_Left)
					{
						parent.add(this, UI::Align_Left);
					}
				} m_capacity_label;

				struct Capacity_Decrement : public UI::Button {
					Capacity_Decrement(UI::Box & parent) :
						UI::Button
							(&parent,
							 70, 4, 24, 24,
							 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
							 g_gr->get_picture(PicMod_Game, pic_down_train))
					{
						set_repeating(true);
						parent.add(this, UI::Align_Top);
					}
					void clicked() {
						ref_cast<TrainingSite_Window, UI::Panel>
							(*get_parent()->get_parent()->get_parent()->get_parent())
							.act_change_soldier_capacity(-1);
					}
				} m_capacity_decrement;

				struct Capacity_Value_Label : public UI::Textarea {
					Capacity_Value_Label(Capacity_Box & parent) :
						UI::Textarea(&parent, 0, 11, _("xx"), UI::Align_Center)
					{
						parent.add(this, UI::Align_Top);
					}
				} m_capacity_value_label;

				struct Capacity_Increment : public UI::Button {
					Capacity_Increment(UI::Box & parent) :
						UI::Button
							(&parent,
							 118, 4, 24, 24,
							 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
							 g_gr->get_picture(PicMod_Game, pic_up_train))
					{
						set_repeating(true);
						parent.add(this, UI::Align_Top);
					}
					void clicked() {
						ref_cast<TrainingSite_Window, UI::Panel>
							(*get_parent()->get_parent()->get_parent()->get_parent())
							.act_change_soldier_capacity(1);
					}
				} m_capacity_increment;

			}  m_capacity_box;

		} m_sold_box;

	} m_tabpanel;
};


/*
===============
Create the window and its panels, add it to the registry.
===============
*/
TrainingSite_Window::TrainingSite_Window
	(Interactive_GameBase & parent, TrainingSite & ts, UI::Window * & registry)
	:
	ProductionSite_Window  (parent, ts, registry),
	m_ms_location          (ts.get_position()),
	m_tabpanel             (*this, ts)
{
	fit_inner (m_tabpanel);
}


/*
===============
Make sure the window is redrawn when necessary.
===============
*/
void TrainingSite_Window::think()
{
	ProductionSite_Window::think();

	Widelands::BaseImmovable const * const base_immovable =
		igbase().egbase().map()[m_ms_location].get_immovable();
	if
		(not dynamic_cast<const Building *>(base_immovable)
		 or
		 dynamic_cast<const ConstructionSite *>(base_immovable))
	{
		// The Site has been removed. Die quickly.
		die();
		return;
	}
	update();
}

/*
==============
Update the listselect, maybe there are new soldiers
FIXME What if a soldier have been removed and another added? This needs review.
=============
*/
void TrainingSite_Window::update() {
	std::vector<Soldier *> soldiers = trainingsite().presentSoldiers();

	if (soldiers.size() != m_tabpanel.m_sold_box.m_table.size())
		m_tabpanel.m_sold_box.m_table.clear();

	for (uint32_t i = 0; i < soldiers.size(); ++i) {
		Soldier & s = *soldiers[i];
		UI::Table<Soldier &>::Entry_Record * er =
			m_tabpanel.m_sold_box.m_table.find(s);
		if (not er)
			er = &m_tabpanel.m_sold_box.m_table.add(s);
		uint32_t const  hl = s.get_hp_level         ();
		uint32_t const mhl = s.get_max_hp_level     ();
		uint32_t const  al = s.get_attack_level     ();
		uint32_t const mal = s.get_max_attack_level ();
		uint32_t const  dl = s.get_defense_level    ();
		uint32_t const mdl = s.get_max_defense_level();
		uint32_t const  el = s.get_evade_level      ();
		uint32_t const mel = s.get_max_evade_level  ();
		er->set_string(0, s.descname().c_str());
		char buffer[sizeof("4294967295 / 4294967295")];
		sprintf(buffer,  "%u / %u", hl,                mhl);
		er->set_string(1, buffer);
		sprintf(buffer,  "%u / %u",      al,                 mal);
		er->set_string(2, buffer);
		sprintf(buffer,  "%u / %u",           dl,                  mdl);
		er->set_string(3, buffer);
		sprintf(buffer,  "%u / %u",                el,                   mel);
		er->set_string(4, buffer);
		sprintf(buffer, "%2u / %u", hl + al + dl + el, mhl + mal + mdl + mel);
		er->set_string(5, buffer);
	}
	m_tabpanel.m_sold_box.m_table.sort();

	TrainingSite const & ts = trainingsite();
	uint32_t const capacity     = ts.   soldierCapacity();
	char buffer[sizeof("4294967295")];
	sprintf (buffer, "%2u", ts.soldierCapacity());
	m_tabpanel.m_sold_box.m_capacity_box.m_capacity_value_label.set_text
		(buffer);
	uint32_t const capacity_min = ts.minSoldierCapacity();
	bool const can_act = igbase().can_act(ts.owner().player_number());
	m_tabpanel.m_sold_box.m_drop_selected_soldier.set_enabled
		(can_act and m_tabpanel.m_sold_box.m_table.has_selection() and
		 capacity_min < m_tabpanel.m_sold_box.m_table.size());
	m_tabpanel.m_sold_box.m_capacity_box.m_capacity_decrement.set_enabled
		(can_act and capacity_min < capacity);
	m_tabpanel.m_sold_box.m_capacity_box.m_capacity_increment.set_enabled
		(can_act and                capacity < ts.maxSoldierCapacity());
}


/*
==============
Handle the click at drop soldier. Enqueue a command at command queue to
get out selected soldier from this training site.
=============
*/
void TrainingSite_Window::Tab_Panel::Sold_Box::Drop_Selected_Soldier::clicked()
{
	Sold_Box & sold_box = ref_cast<Sold_Box, UI::Panel>(*get_parent());
	if (sold_box.m_table.selection_index() != Table::no_selection_index())
		ref_cast<TrainingSite_Window, UI::Panel>
			(*sold_box.get_parent()->get_parent())
			.act_drop_soldier(sold_box.m_table.get_selected().serial());
}

/*
===============
Create the training site information window.
===============
*/
void TrainingSite::create_options_window
	(Interactive_GameBase & plr, UI::Window * & registry)
{
	new TrainingSite_Window(plr, *this, registry);
}
