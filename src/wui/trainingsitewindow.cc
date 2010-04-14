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
#include "soldierlist.h"
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
				m_capacity_box         (*this)
			{
				add
					(create_soldier_list
						(*this,
						 *static_cast<Interactive_GameBase*>(parent.get_parent()->get_parent()),
						 static_cast<TrainingSite_Window*>(parent.get_parent())->trainingsite()),
					 AlignCenter);

				add(&m_capacity_box, UI::Align_Left);

				resize();
			}

			struct Capacity_Box : public UI::Box {
				Capacity_Box(Sold_Box & parent) :
					UI::Box               (&parent, 0,  0, UI::Box::Horizontal),
					m_capacity_label      (*this),
					m_capacity_decrement  (*this),
					m_capacity_value_label(*this),
					m_capacity_increment  (*this)
				{
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
	fit_inner(m_tabpanel);
}

/*
===============
Make sure the window is redrawn when necessary.
===============
*/
void TrainingSite_Window::think()
{
	ProductionSite_Window::think();

	TrainingSite const & ts = trainingsite();
	uint32_t const capacity     = ts.   soldierCapacity();
	char buffer[sizeof("4294967295")];
	sprintf (buffer, "%2u", ts.soldierCapacity());
	m_tabpanel.m_sold_box.m_capacity_box.m_capacity_value_label.set_text
		(buffer);
	uint32_t const capacity_min = ts.minSoldierCapacity();
	bool const can_act = igbase().can_act(ts.owner().player_number());
	m_tabpanel.m_sold_box.m_capacity_box.m_capacity_decrement.set_enabled
		(can_act and capacity_min < capacity);
	m_tabpanel.m_sold_box.m_capacity_box.m_capacity_increment.set_enabled
		(can_act and                capacity < ts.maxSoldierCapacity());
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
