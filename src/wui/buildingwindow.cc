/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#include "base/macros.h"
#include "graphic/graphic.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/maphollowregion.h"
#include "logic/player.h"
#include "ui_basic/tabpanel.h"
#include "wui/actionconfirm.h"
#include "wui/game_debug_ui.h"
#include "wui/helpwindow.h"
#include "wui/interactive_player.h"
#include "wui/unique_window_handler.h"
#include "wui/waresqueuedisplay.h"

static const char* pic_bulldoze = "images/wui/buildings/menu_bld_bulldoze.png";
static const char* pic_dismantle = "images/wui/buildings/menu_bld_dismantle.png";
static const char* pic_debug = "images/wui/fieldaction/menu_debug.png";

BuildingWindow::BuildingWindow(InteractiveGameBase& parent, Widelands::Building& b, bool avoid_fastclick)
   : UI::Window(&parent, "building_window", 0, 0, Width, 0, b.descr().descname()),
     building_(b),
     workarea_overlay_id_(0),
	  avoid_fastclick_(avoid_fastclick),
     expeditionbtn_(nullptr) {
		buildingnotes_subscriber_ = Notifications::subscribe<Widelands::NoteBuildingWindow>(
			[this](const Widelands::NoteBuildingWindow& note) {
				if (note.serial == building_.serial()) {
					switch (note.action) {
					// The building's state has changed
					case Widelands::NoteBuildingWindow::Action::kRefresh:
						init(true);
						break;
					// The building is no more
					case Widelands::NoteBuildingWindow::Action::kStartWarp:
						igbase().add_wanted_building_window(building().get_position(), get_pos(), is_minimal());
						// Fallthrough intended
					case Widelands::NoteBuildingWindow::Action::kClose:
						// Stop everybody from thinking to avoid segfaults
						set_thinks(false);
						vbox_.reset(nullptr);
						die();
						break;
					default:
						break;
					}
				}
		});
}

BuildingWindow::~BuildingWindow() {
	if (workarea_overlay_id_) {
		igbase().mutable_field_overlay_manager()->remove_overlay(workarea_overlay_id_);
	}
}

namespace Widelands {
class BuildingDescr;
}
using Widelands::Building;

void BuildingWindow::init(bool avoid_fastclick) {
	capscache_player_number_ = 0;
	capsbuttons_ = nullptr;
	capscache_ = 0;
	caps_setup_ = false;
	toggle_workarea_ = nullptr;
	avoid_fastclick_ = avoid_fastclick,

	vbox_.reset(new UI::Box(this, 0, 0, UI::Box::Vertical));

	tabs_ = new UI::TabPanel(vbox_.get(), 0, 0, nullptr);
	vbox_->add(tabs_, UI::Align::kLeft, true);

	capsbuttons_ = new UI::Box(vbox_.get(), 0, 0, UI::Box::Horizontal);
	vbox_->add(capsbuttons_, UI::Align::kLeft, true);
	// actually create buttons on the first call to think(),
	// so that overriding create_capsbuttons() works

	set_center_panel(vbox_.get());
	set_thinks(true);
	set_fastclick_panel(this);
	show_workarea();
}

/*
===============
Draw a picture of the building in the background.
===============
*/
void BuildingWindow::draw(RenderTarget& dst) {
	UI::Window::draw(dst);

	// TODO(sirver): chang this to directly blit the animation. This needs support for or removal of
	// RenderTarget.
	const Image* image = building().representative_image();
	dst.blitrect_scale(Rect((get_inner_w() - image->width()) / 2,
	                        (get_inner_h() - image->height()) / 2, image->width(), image->height()),
	                   image, Rect(0, 0, image->width(), image->height()), 0.5, BlendMode::UseAlpha);
}

/*
===============
Check the capabilities and setup the capsbutton panel in case they've changed.
===============
*/
void BuildingWindow::think() {
	if (!igbase().can_see(building().owner().player_number()))
		die();

	if (!caps_setup_ || capscache_player_number_ != igbase().player_number() ||
	    building().get_playercaps() != capscache_) {
		capsbuttons_->free_children();
		create_capsbuttons(capsbuttons_);
		if (!avoid_fastclick_) {
			move_out_of_the_way();
			warp_mouse_to_fastclick_panel();
		}
		caps_setup_ = true;
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
void BuildingWindow::create_capsbuttons(UI::Box* capsbuttons) {
	capscache_ = building().get_playercaps();
	capscache_player_number_ = igbase().player_number();

	const Widelands::Player& owner = building().owner();
	const Widelands::PlayerNumber owner_number = owner.player_number();
	const bool can_see = igbase().can_see(owner_number);
	const bool can_act = igbase().can_act(owner_number);

	bool requires_destruction_separator = false;
	if (can_act) {
		// Check if this is a port building and if yes show expedition button
		if (upcast(Widelands::Warehouse const, warehouse, &building_)) {
			if (Widelands::PortDock* pd = warehouse->get_portdock()) {
				expedition_canceled_subscriber_ =
				   Notifications::subscribe<Widelands::NoteExpeditionCanceled>([this](
				      const Widelands::NoteExpeditionCanceled&) { update_expedition_button(true); });
				expeditionbtn_ =
				   new UI::Button(capsbuttons, "start_or_cancel_expedition", 0, 0, 34, 34,
				                  g_gr->images().get("images/ui_basic/but4.png"),
				                  g_gr->images().get("images/wui/buildings/start_expedition.png"));
				update_expedition_button(!pd->expedition_started());
				expeditionbtn_->sigclicked.connect(
				   boost::bind(&BuildingWindow::act_start_or_cancel_expedition, boost::ref(*this)));
				capsbuttons->add(expeditionbtn_, UI::Align::kHCenter);
			}
		} else if (upcast(const Widelands::ProductionSite, productionsite, &building_)) {
			if (!is_a(Widelands::MilitarySite, productionsite)) {
				const bool is_stopped = productionsite->is_stopped();
				UI::Button* stopbtn = new UI::Button(
				   capsbuttons, is_stopped ? "continue" : "stop", 0, 0, 34, 34,
				   g_gr->images().get("images/ui_basic/but4.png"),
				   g_gr->images().get(
				      (is_stopped ? "images/ui_basic/continue.png" : "images/ui_basic/stop.png")),
				   is_stopped ?
				      /** TRANSLATORS: Stop/Continue toggle button for production sites. */
				      _("Continue") :
				      /** TRANSLATORS: Stop/Continue toggle button for production sites. */
				      _("Stop"));
				stopbtn->sigclicked.connect(
				   boost::bind(&BuildingWindow::act_start_stop, boost::ref(*this)));
				capsbuttons->add(stopbtn, UI::Align::kHCenter);

				// Add a fixed width separator rather than infinite space so the
				// enhance/destroy/dismantle buttons are fixed in their position
				// and not subject to the number of buttons on the right of the
				// panel.
				UI::Panel* spacer = new UI::Panel(capsbuttons, 0, 0, 17, 34);
				capsbuttons->add(spacer, UI::Align::kHCenter);
			}
		}  // upcast to productionsite

		if (capscache_ & Widelands::Building::PCap_Enhancable) {
			const Widelands::DescriptionIndex& enhancement = building_.descr().enhancement();
			const Widelands::TribeDescr& tribe = owner.tribe();
			if (owner.is_building_type_allowed(enhancement)) {
				const Widelands::BuildingDescr& building_descr = *tribe.get_building_descr(enhancement);

				std::string enhance_tooltip =
				   (boost::format(_("Enhance to %s")) % building_descr.descname().c_str()).str() +
				   "<br><font size=11>" + _("Construction costs:") + "</font><br>" +
				   waremap_to_richtext(tribe, building_descr.enhancement_cost());

				UI::Button* enhancebtn = new UI::Button(capsbuttons, "enhance", 0, 0, 34, 34,
				                                        g_gr->images().get("images/ui_basic/but4.png"),
				                                        building_descr.icon(), enhance_tooltip);

				//  button id = building id
				enhancebtn->sigclicked.connect([this, enhancement] { act_enhance(enhancement); });
				capsbuttons->add(enhancebtn, UI::Align::kHCenter);
				requires_destruction_separator = true;
			}
		}

		if (capscache_ & Widelands::Building::PCap_Bulldoze) {
			UI::Button* destroybtn = new UI::Button(capsbuttons, "destroy", 0, 0, 34, 34,
			                                        g_gr->images().get("images/ui_basic/but4.png"),
			                                        g_gr->images().get(pic_bulldoze), _("Destroy"));
			destroybtn->sigclicked.connect(
			   boost::bind(&BuildingWindow::act_bulldoze, boost::ref(*this)));
			capsbuttons->add(destroybtn, UI::Align::kHCenter);

			requires_destruction_separator = true;
		}

		if (capscache_ & Widelands::Building::PCap_Dismantle) {
			const Widelands::Buildcost wares =
			   Widelands::DismantleSite::count_returned_wares(&building_);
			if (!wares.empty()) {
				UI::Button* dismantlebtn = new UI::Button(
				   capsbuttons, "dismantle", 0, 0, 34, 34,
				   g_gr->images().get("images/ui_basic/but4.png"), g_gr->images().get(pic_dismantle),
				   std::string(_("Dismantle")) + "<br><font size=11>" + _("Returns:") + "</font><br>" +
				      waremap_to_richtext(owner.tribe(), wares));
				dismantlebtn->sigclicked.connect(
				   boost::bind(&BuildingWindow::act_dismantle, boost::ref(*this)));
				capsbuttons->add(dismantlebtn, UI::Align::kHCenter);
				requires_destruction_separator = true;
			}
		}

		if (requires_destruction_separator && can_see) {
			// Need this as well as the infinite space from the can_see section
			// to ensure there is a separation.
			UI::Panel* spacer = new UI::Panel(capsbuttons, 0, 0, 17, 34);
			capsbuttons->add(spacer, UI::Align::kHCenter);
			capsbuttons->add_inf_space();
		}
	}

	if (can_see) {
		WorkareaInfo wa_info;
		if (upcast(Widelands::ConstructionSite, csite, &building_)) {
			wa_info = csite->building().workarea_info_;
		} else {
			wa_info = building_.descr().workarea_info_;
		}
		if (!wa_info.empty()) {
			toggle_workarea_ = new UI::Button(
			   capsbuttons, "workarea", 0, 0, 34, 34, g_gr->images().get("images/ui_basic/but4.png"),
			   g_gr->images().get("images/wui/overlays/workarea123.png"), _("Hide work area"));
			toggle_workarea_->sigclicked.connect(
			   boost::bind(&BuildingWindow::toggle_workarea, boost::ref(*this)));

			capsbuttons->add(toggle_workarea_, UI::Align::kHCenter);
			configure_workarea_button();
			set_fastclick_panel(toggle_workarea_);
		}

		if (igbase().get_display_flag(InteractiveBase::dfDebug)) {
			UI::Button* debugbtn = new UI::Button(
			   capsbuttons, "debug", 0, 0, 34, 34, g_gr->images().get("images/ui_basic/but4.png"),
			   g_gr->images().get(pic_debug), _("Show Debug Window"));
			debugbtn->sigclicked.connect(boost::bind(&BuildingWindow::act_debug, boost::ref(*this)));
			capsbuttons->add(debugbtn, UI::Align::kHCenter);
		}

		UI::Button* gotobtn = new UI::Button(
		   capsbuttons, "goto", 0, 0, 34, 34, g_gr->images().get("images/ui_basic/but4.png"),
		   g_gr->images().get("images/wui/menus/menu_goto.png"), _("Center view on this"));
		gotobtn->sigclicked.connect(boost::bind(&BuildingWindow::clicked_goto, boost::ref(*this)));
		capsbuttons->add(gotobtn, UI::Align::kHCenter);

		if (!requires_destruction_separator) {
			// When there was no separation of destruction buttons put
			// the infinite space here (e.g. Warehouses)
			capsbuttons->add_inf_space();
		}

		UI::Button* helpbtn = new UI::Button(
		   capsbuttons, "help", 0, 0, 34, 34, g_gr->images().get("images/ui_basic/but4.png"),
		   g_gr->images().get("images/ui_basic/menu_help.png"), _("Help"));

		UI::UniqueWindow::Registry& registry =
		   igbase().unique_windows().get_registry(building_.descr().name() + "_help");
		registry.open_window = [this, &registry] {
			new UI::BuildingHelpWindow(&igbase(), registry, building_.descr(),
			                           building_.owner().tribe(), &igbase().egbase().lua());
		};

		helpbtn->sigclicked.connect(
		   boost::bind(&UI::UniqueWindow::Registry::toggle, boost::ref(registry)));
		capsbuttons->add(helpbtn, UI::Align::kHCenter);
	}
}

/**
===============
Callback for bulldozing request
===============
*/
void BuildingWindow::act_bulldoze() {
	if (get_key_state(SDL_SCANCODE_LCTRL) || get_key_state(SDL_SCANCODE_RCTRL)) {
		if (building_.get_playercaps() & Widelands::Building::PCap_Bulldoze)
			igbase().game().send_player_bulldoze(building_);
	} else {
		show_bulldoze_confirm(dynamic_cast<InteractivePlayer&>(igbase()), building_);
	}
}

/**
===============
Callback for dismantling request
===============
*/
void BuildingWindow::act_dismantle() {
	if (get_key_state(SDL_SCANCODE_LCTRL) || get_key_state(SDL_SCANCODE_RCTRL)) {
		if (building_.get_playercaps() & Widelands::Building::PCap_Dismantle)
			igbase().game().send_player_dismantle(building_);
	} else {
		show_dismantle_confirm(dynamic_cast<InteractivePlayer&>(igbase()), building_);
	}
}

/**
===============
Callback for starting / stoping the production site request
===============
*/
void BuildingWindow::act_start_stop() {
	if (dynamic_cast<const Widelands::ProductionSite*>(&building_)) {
		igbase().game().send_player_start_stop_building(building_);
	}
}

/**
===============
Callback for starting an expedition request
===============
*/
void BuildingWindow::act_start_or_cancel_expedition() {
	if (upcast(Widelands::Warehouse const, warehouse, &building_)) {
		if (warehouse->get_portdock()) {
			expeditionbtn_->set_enabled(false);
			igbase().game().send_player_start_or_cancel_expedition(building_);
		}
		get_tabs()->activate("expedition_wares_queue");
	}

	// No need to die here - as soon as the request is handled, the UI will get updated by the
	// portdock
}

/**
===============
Callback for enhancement request
===============
*/
void BuildingWindow::act_enhance(Widelands::DescriptionIndex id) {
	if (get_key_state(SDL_SCANCODE_LCTRL) || get_key_state(SDL_SCANCODE_RCTRL)) {
		if (building_.get_playercaps() & Widelands::Building::PCap_Enhancable)
			igbase().game().send_player_enhance_building(building_, id);
	} else {
		show_enhance_confirm(dynamic_cast<InteractivePlayer&>(igbase()), building_, id);
	}
}

/*
===============
Callback for debug window
===============
*/
void BuildingWindow::act_debug() {
	show_field_debug(igbase(), igbase().game().map().get_fcoords(building_.get_position()));
}

/**
 * Show the building's workarea (if it has one).
 */
void BuildingWindow::show_workarea() {
	if (workarea_overlay_id_) {
		return;  // already shown, nothing to be done
	}
	WorkareaInfo workarea_info;
	if (upcast(Widelands::ConstructionSite, csite, &building_)) {
		workarea_info = csite->building().workarea_info_;
	} else {
		workarea_info = building_.descr().workarea_info_;
	}
	if (workarea_info.empty()) {
		return;
	}
	workarea_overlay_id_ = igbase().show_work_area(workarea_info, building_.get_position());

	configure_workarea_button();
}

/**
 * Hide the workarea from view.
 */
void BuildingWindow::hide_workarea() {
	if (workarea_overlay_id_) {
		igbase().hide_work_area(workarea_overlay_id_);
		workarea_overlay_id_ = 0;

		configure_workarea_button();
	}
}

/**
 * Sets the perpressed_ state and the tooltip.
 */
void BuildingWindow::configure_workarea_button() {
	if (toggle_workarea_) {
		if (workarea_overlay_id_) {
			toggle_workarea_->set_tooltip(_("Hide work area"));
			toggle_workarea_->set_perm_pressed(true);
		} else {
			toggle_workarea_->set_tooltip(_("Show work area"));
			toggle_workarea_->set_perm_pressed(false);
		}
	}
}

void BuildingWindow::toggle_workarea() {
	if (workarea_overlay_id_) {
		hide_workarea();
	} else {
		show_workarea();
	}
}

void BuildingWindow::create_ware_queue_panel(UI::Box* const box,
                                             Widelands::Building& b,
                                             Widelands::WaresQueue* const wq,
                                             bool show_only) {
	// The *max* width should be larger than the default width
	box->add(new WaresQueueDisplay(box, 0, 0, igbase(), b, wq, show_only), UI::Align::kLeft);
}

/**
 * Center the player's view on the building. Callback function
 * for the corresponding button.
 */
void BuildingWindow::clicked_goto() {
	igbase().move_view_to(building().get_position());
}

void BuildingWindow::update_expedition_button(bool expedition_was_canceled) {
	assert(expeditionbtn_ != nullptr);
	if (expedition_was_canceled) {
		expeditionbtn_->set_tooltip(_("Start an expedition"));
		expeditionbtn_->set_pic(g_gr->images().get("images/wui/buildings/start_expedition.png"));
		tabs_->remove_last_tab("expedition_wares_queue");
	} else {
		expeditionbtn_->set_tooltip(_("Cancel the expedition"));
		expeditionbtn_->set_pic(g_gr->images().get("images/wui/buildings/cancel_expedition.png"));
	}
	expeditionbtn_->set_enabled(true);
}
