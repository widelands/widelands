/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#include "wui/buildingwindow.h"

#include <boost/format.hpp>

#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/image_transformations.h"
#include "graphic/rendertarget.h"
#include "logic/constructionsite.h"
#include "logic/dismantlesite.h"
#include "logic/maphollowregion.h"
#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/productionsite.h"
#include "logic/tribe.h"
#include "logic/warehouse.h"
#include "ui_basic/helpwindow.h"
#include "ui_basic/tabpanel.h"
#include "upcast.h"
#include "wui/actionconfirm.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_player.h"
#include "wui/waresqueuedisplay.h"

static const char * pic_bulldoze           = "pics/menu_bld_bulldoze.png";
static const char * pic_dismantle          = "pics/menu_bld_dismantle.png";
static const char * pic_debug              = "pics/menu_debug.png";


Building_Window::Building_Window
	(Interactive_GameBase & parent,
	 Widelands::Building  & b,
	 UI::Window *         & registry)
	:
	UI::Window
		(&parent, "building_window",
		 0, 0, Width, 0,
		 b.descname()),
	m_registry(registry),
	m_building       (b),
	m_workarea_job_id(Overlay_Manager::Job_Id::Null()),
	m_avoid_fastclick(false)
{
	delete m_registry;
	m_registry = this;

	m_capscache_player_number = 0;
	m_capsbuttons = nullptr;
	m_capscache = 0;
	m_caps_setup = false;
	m_toggle_workarea = nullptr;

	UI::Box * vbox = new UI::Box(this, 0, 0, UI::Box::Vertical);

	m_tabs = new UI::Tab_Panel(vbox, 0, 0, nullptr);
	vbox->add(m_tabs, UI::Box::AlignLeft, true);

	m_capsbuttons = new UI::Box(vbox, 0, 0, UI::Box::Horizontal);
	vbox->add(m_capsbuttons, UI::Box::AlignLeft, true);
	// actually create buttons on the first call to think(),
	// so that overriding create_capsbuttons() works

	set_center_panel(vbox);
	set_think(true);
	set_fastclick_panel(this);

	show_workarea();

	// Title for construction site
	if (upcast(Widelands::ConstructionSite, csite, &m_building)) {
		// Show name in parenthesis as it may take all width already
		const std::string title = (boost::format("(%s)") % csite->building().descname()).str();
		set_title(title);
	}
}


Building_Window::~Building_Window()
{
	if (m_workarea_job_id)
		igbase().egbase().map().overlay_manager().remove_overlay
			(m_workarea_job_id);
	if (m_helpwindow_registry.window)
		delete m_helpwindow_registry.window;
	m_registry = nullptr;
}

namespace Widelands {struct Building_Descr;}
using Widelands::Building;

/*
===============
Draw a picture of the building in the background.
===============
*/
void Building_Window::draw(RenderTarget & dst)
{
	UI::Window::draw(dst);

	const Animation& anim = g_gr->animations().get_animation(building().get_ui_anim());

	const Image* dark_frame = ImageTransformations::change_luminosity
		(&anim.representative_image(building().owner().get_playercolor()), 1.22, true);
	dst.blit(Point(get_inner_w() / 2, get_inner_h() / 2), dark_frame, CM_Normal, UI::Align_Center);
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
		if (!m_avoid_fastclick)
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

	const Widelands::Player & owner = building().owner();
	const Widelands::Player_Number owner_number = owner.player_number();
	const bool can_see = igbase().can_see(owner_number);
	const bool can_act = igbase().can_act(owner_number);

	bool requires_destruction_separator = false;
	if (can_act) {
		// Check if this is a port building and if yes show expedition button
		if (upcast(Widelands::Warehouse const, warehouse, &m_building)) {
			if (Widelands::PortDock * pd = warehouse->get_portdock()) {
				if (pd->expedition_started()) {
					UI::Button * expeditionbtn =
						new UI::Button
							(capsbuttons, "cancel_expedition", 0, 0, 34, 34,
							g_gr->images().get("pics/but4.png"),
							g_gr->images().get("pics/cancel_expedition.png"),
							_("Cancel the expedition"));
					expeditionbtn->sigclicked.connect
						(boost::bind(&Building_Window::act_start_or_cancel_expedition, boost::ref(*this)));
					capsbuttons->add(expeditionbtn, UI::Box::AlignCenter);
				} else {
					UI::Button * expeditionbtn =
						new UI::Button
							(capsbuttons, "start_expedition", 0, 0, 34, 34,
							g_gr->images().get("pics/but4.png"),
							g_gr->images().get("pics/start_expedition.png"),
							_("Start an expedition"));
					expeditionbtn->sigclicked.connect
						(boost::bind(&Building_Window::act_start_or_cancel_expedition, boost::ref(*this)));
					capsbuttons->add(expeditionbtn, UI::Box::AlignCenter);
				}
			}
		}
		else
		if (upcast(const Widelands::ProductionSite, productionsite, &m_building)) {
			if (!is_a(Widelands::MilitarySite, productionsite)) {
				const bool is_stopped = productionsite->is_stopped();
				UI::Button * stopbtn =
					new UI::Button
						(capsbuttons, is_stopped ? "continue" : "stop", 0, 0, 34, 34,
						 g_gr->images().get("pics/but4.png"),
						 g_gr->images().get((is_stopped ? "pics/continue.png" : "pics/stop.png")),
						 is_stopped ? _("Continue") : _("Stop"));
				stopbtn->sigclicked.connect(boost::bind(&Building_Window::act_start_stop, boost::ref(*this)));
				capsbuttons->add
					(stopbtn,
					 UI::Box::AlignCenter);


				// Add a fixed width separator rather than infinite space so the
				// enhance/destroy/dismantle buttons are fixed in their position
				// and not subject to the number of buttons on the right of the
				// panel.
				UI::Panel * spacer = new UI::Panel(capsbuttons, 0, 0, 17, 34);
				capsbuttons->add(spacer, UI::Box::AlignCenter);
			}
		} // upcast to productionsite

		if (m_capscache & Widelands::Building::PCap_Enhancable) {
			const std::set<Widelands::Building_Index> & enhancements =
				m_building.enhancements();
			const Widelands::Tribe_Descr & tribe  = owner.tribe();
			container_iterate_const(std::set<Widelands::Building_Index>, enhancements, i)
				if (owner.is_building_type_allowed(*i.current)) {
					const Widelands::Building_Descr & building_descr =
						*tribe.get_building_descr(*i.current);
					char buffer[128];
					snprintf
						(buffer, sizeof(buffer),
						 _("Enhance to %s"), building_descr.descname().c_str());
					UI::Button * enhancebtn =
						new UI::Button
							(capsbuttons, "enhance", 0, 0, 34, 34,
							 g_gr->images().get("pics/but4.png"),
							 building_descr.get_buildicon(),
							 std::string(buffer) + "<br><font size=11>" + _("Construction costs:") + "</font><br>" +
								 waremap_to_richtext(tribe, building_descr.enhancement_cost()));
					//  button id = building id
					enhancebtn->sigclicked.connect
						(boost::bind
							(&Building_Window::act_enhance,
							 boost::ref(*this),
							 boost::ref(*i.current)));
					capsbuttons->add
						(enhancebtn,
						 UI::Box::AlignCenter);
					requires_destruction_separator = true;
				}
		}

		if (m_capscache & Widelands::Building::PCap_Bulldoze) {
			UI::Button * destroybtn =
				new UI::Button
					(capsbuttons, "destroy", 0, 0, 34, 34,
					 g_gr->images().get("pics/but4.png"),
					 g_gr->images().get(pic_bulldoze),
					 _("Destroy"));
			destroybtn->sigclicked.connect
				(boost::bind(&Building_Window::act_bulldoze, boost::ref(*this)));
			capsbuttons->add
				(destroybtn,
				 UI::Box::AlignCenter);

			requires_destruction_separator = true;
		}

		if (m_capscache & Widelands::Building::PCap_Dismantle) {
			std::map<Widelands::Ware_Index, uint8_t> wares;
			Widelands::DismantleSite::count_returned_wares(&m_building, wares);
			UI::Button * dismantlebtn =
				new UI::Button
					(capsbuttons, "dismantle", 0, 0, 34, 34,
					 g_gr->images().get("pics/but4.png"),
					 g_gr->images().get(pic_dismantle),
					 std::string(_("Dismantle")) + "<br><font size=11>" + _("Returns:") + "</font><br>" +
						 waremap_to_richtext(owner.tribe(), wares));
			dismantlebtn->sigclicked.connect(boost::bind(&Building_Window::act_dismantle, boost::ref(*this)));
			capsbuttons->add
				(dismantlebtn,
				 UI::Box::AlignCenter);

			requires_destruction_separator = true;
		}

		if (requires_destruction_separator and can_see) {
			// Need this as well as the infinite space from the can_see section
			// to ensure there is a separation.
			UI::Panel * spacer = new UI::Panel(capsbuttons, 0, 0, 17, 34);
			capsbuttons->add(spacer, UI::Box::AlignCenter);
			capsbuttons->add_inf_space();
		}
	}

	if (can_see) {
		Workarea_Info wa_info;
		if (upcast(Widelands::ConstructionSite, csite, &m_building)) {
			wa_info = csite->building().m_workarea_info;
		} else {
			wa_info = m_building.descr().m_workarea_info;
		}
		if (!wa_info.empty()) {
			m_toggle_workarea = new UI::Button
				(capsbuttons, "workarea",
				 0, 0, 34, 34,
				 g_gr->images().get("pics/but4.png"),
				 g_gr->images().get("pics/workarea123.png"),
				 _("Hide work area"));
			m_toggle_workarea->sigclicked.connect
				(boost::bind(&Building_Window::toggle_workarea, boost::ref(*this)));

			capsbuttons->add(m_toggle_workarea, UI::Box::AlignCenter);
			configure_workarea_button();
			set_fastclick_panel(m_toggle_workarea);
		}

		if (igbase().get_display_flag(Interactive_Base::dfDebug)) {
			UI::Button * debugbtn =
				new UI::Button
					(capsbuttons, "debug", 0, 0, 34, 34,
					 g_gr->images().get("pics/but4.png"),
					 g_gr->images().get(pic_debug),
					 _("Debug"));
			debugbtn->sigclicked.connect(boost::bind(&Building_Window::act_debug, boost::ref(*this)));
			capsbuttons->add
				(debugbtn,
				 UI::Box::AlignCenter);
		}

		UI::Button * gotobtn =
			new UI::Button
				(capsbuttons, "goto", 0, 0, 34, 34,
				 g_gr->images().get("pics/but4.png"),
				 g_gr->images().get("pics/menu_goto.png"), _("Center view on this"));
		gotobtn->sigclicked.connect(boost::bind(&Building_Window::clicked_goto, boost::ref(*this)));
		capsbuttons->add
			(gotobtn,
			 UI::Box::AlignCenter);

		if (m_building.descr().has_help_text()) {
			if (not requires_destruction_separator) {
				// When there was no separation of destruction buttons put
				// the infinite space here (e.g. Warehouses)
				capsbuttons->add_inf_space();
			}

			UI::Button * helpbtn =
				new UI::Button
					(capsbuttons, "help", 0, 0, 34, 34,
					 g_gr->images().get("pics/but4.png"),
					 g_gr->images().get("pics/menu_help.png"),
					 _("Help"));
			helpbtn->sigclicked.connect
				(boost::bind(&Building_Window::help_clicked, boost::ref(*this)));
			capsbuttons->add
				(helpbtn,
				 UI::Box::AlignCenter);

		}
	}
}

/**
===============
The help button has been pressed
===============
*/
void Building_Window::help_clicked()
{
	if (m_helpwindow_registry.window)
		delete m_helpwindow_registry.window;
	else
		new UI::LuaTextHelpWindow
			(&igbase(), m_helpwindow_registry,
			 m_building.descname(),
			 m_building.descr().helptext_script());
}

/**
===============
Callback for bulldozing request
===============
*/
void Building_Window::act_bulldoze()
{
	if (get_key_state(SDLK_LCTRL) or get_key_state(SDLK_RCTRL)) {
		if (m_building.get_playercaps() & Widelands::Building::PCap_Bulldoze)
			igbase().game().send_player_bulldoze(m_building);
	}
	else {
		show_bulldoze_confirm(ref_cast<Interactive_Player, Interactive_GameBase>(igbase()), m_building);
	}
}

/**
===============
Callback for dismantling request
===============
*/
void Building_Window::act_dismantle()
{
	if (get_key_state(SDLK_LCTRL) or get_key_state(SDLK_RCTRL)) {
		if (m_building.get_playercaps() & Widelands::Building::PCap_Dismantle)
			igbase().game().send_player_dismantle(m_building);
	}
	else {
		show_dismantle_confirm(ref_cast<Interactive_Player, Interactive_GameBase>(igbase()), m_building);
	}
}

/**
===============
Callback for starting / stoping the production site request
===============
*/
void Building_Window::act_start_stop() {
	if (dynamic_cast<const Widelands::ProductionSite *>(&m_building))
		igbase().game().send_player_start_stop_building (m_building);

	die();
}


/**
===============
Callback for starting an expedition request
===============
*/
void Building_Window::act_start_or_cancel_expedition() {
	if (upcast(Widelands::Warehouse const, warehouse, &m_building))
		if (warehouse->get_portdock())
			igbase().game().send_player_start_or_cancel_expedition(m_building);

	// No need to die here - as soon as the request is handled, the UI will get updated by the portdock
	//die();
}

/**
===============
Callback for enhancement request
===============
*/
void Building_Window::act_enhance(Widelands::Building_Index id)
{
	if (get_key_state(SDLK_LCTRL) or get_key_state(SDLK_RCTRL)) {
		if (m_building.get_playercaps() & Widelands::Building::PCap_Enhancable)
			igbase().game().send_player_enhance_building(m_building, id);
	}
	else {
		show_enhance_confirm
			(ref_cast<Interactive_Player, Interactive_GameBase>(igbase()),
			 m_building,
			 id);
	}
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
	if (m_workarea_job_id) {
		return; // already shown, nothing to be done
	}
	Workarea_Info workarea_info;
	if (upcast(Widelands::ConstructionSite, csite, &m_building)) {
		workarea_info = csite->building().m_workarea_info;
	} else {
		workarea_info = m_building.descr().m_workarea_info;
	}
	if (workarea_info.empty()) {
		return;
	}
	m_workarea_job_id = igbase().show_work_area(workarea_info, m_building.get_position());

	configure_workarea_button();
}

/**
 * Hide the workarea from view.
 */
void Building_Window::hide_workarea()
{
	if (m_workarea_job_id) {
		igbase().hide_work_area(m_workarea_job_id);
		m_workarea_job_id = Overlay_Manager::Job_Id::Null();

		configure_workarea_button();
	}
}

/**
 * Sets the perm_pressed state and the tooltip.
 */
void Building_Window::configure_workarea_button()
{
	if (m_toggle_workarea) {
		if (m_workarea_job_id) {
			m_toggle_workarea->set_tooltip(_("Hide work area"));
			m_toggle_workarea->set_perm_pressed(true);
		} else {
			m_toggle_workarea->set_tooltip(_("Show work area"));
			m_toggle_workarea->set_perm_pressed(false);
		}
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
	 Widelands::WaresQueue * const wq,
	 bool show_only)
{
	// The *max* width should be larger than the default width
	box->add(new WaresQueueDisplay(box, 0, 0, igbase(), b, wq, show_only), UI::Box::AlignLeft);
}

/**
 * Center the player's view on the building. Callback function
 * for the corresponding button.
 */
void Building_Window::clicked_goto()
{
	igbase().move_view_to(building().get_position());
}

