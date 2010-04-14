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

#include "logic/constructionsite.h"
#include "logic/player.h"
#include "logic/soldier.h"
#include "logic/trainingsite.h"
#include "productionsitewindow.h"
#include "ui_basic/table.h"
#include "ui_basic/tabpanel.h"

using Widelands::Soldier;
using Widelands::TrainingSite;
using Widelands::atrAttack;
using Widelands::atrDefense;
using Widelands::atrEvade;
using Widelands::atrHP;

static char const * pic_tab_military       = "pics/menu_tab_military.png";
static char const * pic_tab_training       = "pics/menu_tab_training.png";
static char const * pic_up_train           = "pics/menu_up_train.png";
static char const * pic_down_train         = "pics/menu_down_train.png";
static char const * pic_drop_soldier       = "pics/menu_drop_soldier.png";

/**
 * Status window for \ref TrainingSite
 */
struct TrainingSite_Window : public ProductionSite_Window {
	TrainingSite_Window
		(Interactive_GameBase & parent, TrainingSite &, UI::Window * & registry);

	TrainingSite & trainingsite() {
		return ref_cast<TrainingSite, Widelands::Building>(building());
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
		(not dynamic_cast<const Widelands::Building *>(base_immovable)
		 or
		 dynamic_cast<const Widelands::ConstructionSite *>(base_immovable))
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
