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

#include "buildingwindow.h"

#include "bulldozeconfirm.h"
#include "game_debug_ui.h"
#include "graphic/rendertarget.h"
#include "interactive_player.h"
#include "logic/maphollowregion.h"
#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/productionsite.h"
#include "logic/tribe.h"
#include "ui_basic/tabpanel.h"
#include "upcast.h"
#include "waresqueuedisplay.h"

static char const * pic_bulldoze           = "pics/menu_bld_bulldoze.png";
static char const * pic_debug              = "pics/menu_debug.png";


Building_Window::Building_Window
	(Interactive_GameBase & parent,
	 Widelands::Building  & b,
	 UI::Window *         & registry)
	:
	UI::Window
		(&parent, "building_window",
		 0, 0, Width, 0,
		 b.info_string(parent.building_window_title_format()).c_str()),
	m_registry(registry),
	m_building       (b),
	m_workarea_job_id(Overlay_Manager::Job_Id::Null())
{
	delete m_registry;
	m_registry = this;

	m_capscache_player_number = 0;
	m_capsbuttons = 0;
	m_capscache = 0;
	m_caps_setup = false;
	m_toggle_workarea = 0;

	UI::Box * vbox = new UI::Box(this, 0, 0, UI::Box::Vertical);

	m_tabs = new UI::Tab_Panel(vbox, 0, 0, g_gr->get_no_picture());
	vbox->add(m_tabs, UI::Box::AlignLeft, true);

	m_capsbuttons = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
	vbox->add(m_capsbuttons, UI::Box::AlignLeft);
	// actually create buttons on the first call to think(),
	// so that overriding create_capsbuttons() works

	set_center_panel(vbox);
	set_think(true);

	char filename[] = "pics/workarea0cumulative.png";
	compile_assert(NUMBER_OF_WORKAREA_PICS <= 9);
	for (Workarea_Info::size_type i = 0; i < NUMBER_OF_WORKAREA_PICS; ++i) {
		++filename[13];
		workarea_cumulative_picid[i] = g_gr->get_picture(PicMod_Game, filename);
	}

	show_workarea();

	set_fastclick_panel(this);
}


Building_Window::~Building_Window()
{
	if (m_workarea_job_id)
		igbase().egbase().map().overlay_manager().remove_overlay
			(m_workarea_job_id);
	m_registry = 0;
}


/*
===============
Draw a picture of the building in the background.
===============
*/
void Building_Window::draw(RenderTarget & dst)
{
	dst.drawanim
		(Point(get_inner_w() / 2, get_inner_h() / 2),
		 building().get_ui_anim(),
		 0,
		 &building().owner());
}

/*
===============
Check the capabilities and setup the capsbutton panel in case they've changed.
===============
*/
void Building_Window::think()
{
	if (not igbase().can_see(building().owner().player_number()))
		die();

	if
		(! m_caps_setup
		 or
		 m_capscache_player_number != igbase().player_number()
		 or
		 building().get_playercaps() != m_capscache)
	{
		m_capsbuttons->free_children();
		create_capsbuttons(m_capsbuttons);
		move_out_of_the_way();
		warp_mouse_to_fastclick_panel();
		m_caps_setup = true;
	}


	UI::Window::think();
}

/**
 * Fill caps buttons into the given box.
 *
 * May be overridden to add additional buttons.
 *
 * \note Children of \p box must be allocated on the heap
 */
void Building_Window::create_capsbuttons(UI::Box * capsbuttons)
{
	m_capscache = building().get_playercaps();
	m_capscache_player_number = igbase().player_number();

	Widelands::Player const & owner = building().owner();
	Widelands::Player_Number const owner_number = owner.player_number();
	bool const can_see = igbase().can_see(owner_number);
	bool const can_act = igbase().can_act(owner_number);

	if (can_act) {
		if (upcast(Widelands::ProductionSite const, productionsite, &m_building))
			if (not dynamic_cast<Widelands::MilitarySite const *>(productionsite)) {
				bool const is_stopped = productionsite->is_stopped();
				capsbuttons->add
					(new UI::Callback_Button
						(capsbuttons, is_stopped ? "continue" : "stop",
						 0, 0, 34, 34,
						 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
						 g_gr->get_picture
						 	(PicMod_Game,
						 	 (is_stopped ? "pics/continue.png" : "pics/stop.png")),
						 boost::bind(&Building_Window::act_start_stop, boost::ref(*this)),
						 is_stopped ? _("Continue") : _("Stop")),
					 UI::Box::AlignCenter);
			}

		if (m_capscache & 1 << Widelands::Building::PCap_Enhancable) {
			std::set<Widelands::Building_Index> const & enhancements =
				m_building.enhancements();
			Widelands::Tribe_Descr const & tribe  = owner.tribe();
			container_iterate_const(std::set<Widelands::Building_Index>, enhancements, i)
				if (owner.is_building_type_allowed(*i.current)) {
					Widelands::Building_Descr const & building_descr =
						*tribe.get_building_descr(*i.current);
					char buffer[128];
					snprintf
						(buffer, sizeof(buffer),
						 _("Enhance to %s"), building_descr.descname().c_str());
					capsbuttons->add
						(new UI::Callback_Button
							(capsbuttons, "enhance",
							 0, 0, 34, 34,
							 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
							 building_descr.get_buildicon(),
							 boost::bind
								(&Building_Window::act_enhance,
								 boost::ref(*this),
								 boost::ref(*i.current)), //  button id = building id)
							 buffer),
						 UI::Box::AlignCenter);
				}
		}

		if (m_capscache & (1 << Widelands::Building::PCap_Bulldoze)) {
			capsbuttons->add
				(new UI::Callback_Button
					(capsbuttons, "destroy",
					 0, 0, 34, 34,
					 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
					 g_gr->get_picture(PicMod_Game, pic_bulldoze),
					 boost::bind(&Building_Window::act_bulldoze, boost::ref(*this)),
					 _("Destroy")),
				 UI::Box::AlignCenter);
		}
	}

	if (can_see) {
		if (m_building.descr().m_workarea_info.size()) {
			m_toggle_workarea = new UI::Callback_Button
				(capsbuttons, "workarea",
				 0, 0, 34, 34,
				 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
				 g_gr->get_picture(PicMod_Game,  "pics/workarea3cumulative.png"),
				 boost::bind(&Building_Window::toggle_workarea, boost::ref(*this)),
				 _("Hide workarea"));
			capsbuttons->add(m_toggle_workarea, UI::Box::AlignCenter);
			set_fastclick_panel(m_toggle_workarea);
		}

		if (igbase().get_display_flag(Interactive_Base::dfDebug)) {
			capsbuttons->add
				(new UI::Callback_Button
					(capsbuttons, "debug",
					 0, 0, 34, 34,
					 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
					 g_gr->get_picture(PicMod_Game,  pic_debug),
					 boost::bind(&Building_Window::act_debug, boost::ref(*this)),
					 _("Debug")),
				 UI::Box::AlignCenter);
		}

		capsbuttons->add
			(new UI::Callback_Button
				(capsbuttons, "goto",
				 0, 0, 34, 34,
				 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
				 g_gr->get_picture(PicMod_Game, "pics/menu_goto.png"),
				 boost::bind(&Building_Window::clicked_goto, boost::ref(*this))),
			 UI::Box::AlignCenter);
	}
}


/*
===============
Callback for bulldozing request
===============
*/
void Building_Window::act_bulldoze()
{
	show_bulldoze_confirm(ref_cast<Interactive_Player, Interactive_GameBase>(igbase()), m_building);
}

void Building_Window::act_start_stop() {
	if (dynamic_cast<Widelands::ProductionSite const *>(&m_building))
		igbase().game().send_player_start_stop_building (m_building);

	die();
}

/*
===============
Callback for bulldozing request
===============
*/
void Building_Window::act_enhance(Widelands::Building_Index const id)
{
	if (m_building.get_playercaps() & (1 << Widelands::Building::PCap_Enhancable))
		igbase().game().send_player_enhance_building (m_building, id);

	die();
}

/*
===============
Callback for debug window
===============
*/
void Building_Window::act_debug()
{
	show_field_debug
		(igbase(),
		 igbase().game().map().get_fcoords(m_building.get_position()));
}

/**
 * Show the building's workarea (if it has one).
 */
void Building_Window::show_workarea()
{
	if (m_workarea_job_id)
		return; // already shown, nothing to be done

	Workarea_Info const & workarea_info = m_building.descr().m_workarea_info;
	if (workarea_info.size() == 0)
		return; // building has no workarea

	Widelands::Map & map =
		ref_cast<Interactive_GameBase const, UI::Panel>(*get_parent()).egbase()
		.map();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	m_workarea_job_id = overlay_manager.get_a_job_id();

	Widelands::HollowArea<> hollow_area
		(Widelands::Area<>(m_building.get_position(), 0), 0);
	Workarea_Info::const_iterator it = workarea_info.begin();
	for
		(Workarea_Info::size_type i =
		 	std::min(workarea_info.size(), NUMBER_OF_WORKAREA_PICS);
		 i;
		 ++it)
	{
		--i;
		hollow_area.radius = it->first;
		Widelands::MapHollowRegion<> mr(map, hollow_area);
		do
			overlay_manager.register_overlay
				(mr.location(),
				 workarea_cumulative_picid[i],
				 0,
				 Point::invalid(),
				 m_workarea_job_id);
		while (mr.advance(map));
		hollow_area.hole_radius = hollow_area.radius;
	}

	if (m_toggle_workarea)
		m_toggle_workarea->set_tooltip(_("Hide workarea"));
}

/**
 * Hide the workarea from view.
 */
void Building_Window::hide_workarea()
{
	if (m_workarea_job_id) {
		Widelands::Map & map =
			ref_cast<Interactive_GameBase const, UI::Panel>(*get_parent()).egbase()
			.map();
		Overlay_Manager & overlay_manager = map.overlay_manager();
		overlay_manager.remove_overlay(m_workarea_job_id);
		m_workarea_job_id = Overlay_Manager::Job_Id::Null();

		if (m_toggle_workarea)
			m_toggle_workarea->set_tooltip(_("Show workarea"));
	}
}


void Building_Window::toggle_workarea() {
	if (m_workarea_job_id) {
		hide_workarea();
	} else {
		show_workarea();
	}
}

void Building_Window::create_ware_queue_panel
	(UI::Box               * const box,
	 Widelands::Building   &       b,
	 Widelands::WaresQueue * const wq)
{
	// The *max* width should be larger than the default width
	box->add
		(create_wares_queue_display(box, igbase(), b, wq, 3 * Width),
		 UI::Box::AlignLeft);
}

/**
 * Center the player's view on the building. Callback function
 * for the corresponding button.
 */
void Building_Window::clicked_goto()
{
	igbase().move_view_to(building().get_position());
}
