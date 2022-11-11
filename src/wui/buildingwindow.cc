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

#include "wui/buildingwindow.h"

#include "base/macros.h"
#include "base/multithreading.h"
#include "graphic/image.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "logic/game_data_error.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/dismantlesite.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/player.h"
#include "ui_basic/tabpanel.h"
#include "wui/actionconfirm.h"
#include "wui/game_debug_ui.h"
#include "wui/helpwindow.h"
#include "wui/inputqueuedisplay.h"
#include "wui/interactive_player.h"
#include "wui/unique_window_handler.h"
#include "wui/waresdisplay.h"

constexpr const char* const kImgBulldoze = "images/wui/buildings/menu_bld_bulldoze.png";
constexpr const char* const kImgDismantle = "images/wui/buildings/menu_bld_dismantle.png";
constexpr const char* const kImgDebug = "images/wui/fieldaction/menu_debug.png";
constexpr const char* const kImgMuteThis = "images/wui/buildings/menu_mute_this.png";
constexpr const char* const kImgUnmuteThis = "images/wui/buildings/menu_unmute_this.png";
constexpr const char* const kImgMuteAll = "images/wui/buildings/menu_mute_all.png";
constexpr const char* const kImgUnmuteAll = "images/wui/buildings/menu_unmute_all.png";

BuildingWindow::BuildingWindow(InteractiveBase& parent,
                               UI::UniqueWindow::Registry& reg,
                               Widelands::Building& b,
                               const Widelands::BuildingDescr& descr,
                               bool avoid_fastclick)
   : UI::UniqueWindow(
        &parent, UI::WindowStyle::kWui, "building_window", &reg, Width, 0, b.descr().descname()),
     game_(parent.get_game()),
     is_dying_(false),
     parent_(&parent),
     building_(&b),
     building_descr_for_help_(&descr),
     building_position_(b.get_position()),
     showing_workarea_(false),
     avoid_fastclick_(avoid_fastclick),
     expeditionbtn_(nullptr),
     mute_this_(nullptr),
     mute_all_(nullptr) {
	buildingnotes_subscriber_ = Notifications::subscribe<Widelands::NoteBuilding>(
	   [this](const Widelands::NoteBuilding& note) { on_building_note(note); });
}

BuildingWindow::BuildingWindow(InteractiveBase& parent,
                               UI::UniqueWindow::Registry& reg,
                               Widelands::Building& b,
                               bool avoid_fastclick)
   : BuildingWindow(parent, reg, b, b.descr(), avoid_fastclick) {
}

BuildingWindow::~BuildingWindow() {
	// Accessing the toggle_workarea_ button can cause segfaults, so we leave it alone
	hide_workarea(false);
}

void BuildingWindow::on_building_note(const Widelands::NoteBuilding& note) {
	if (note.serial == building_.serial()) {
		switch (note.action) {
		// The building's state has changed
		case Widelands::NoteBuilding::Action::kChanged:
			if (!is_dying_) {
				const std::string active_tab = tabs_->tabs()[tabs_->active()]->get_name();
				NoteThreadSafeFunction::instantiate(
				   [this, active_tab]() {
					   init(true, showing_workarea_);
					   tabs_->activate(active_tab);
				   },
				   true);
			}
			break;
		// The building is no more. Next think() will call die().
		case Widelands::NoteBuilding::Action::kStartWarp:
			ibase()->add_wanted_building_window(
			   building_position_, get_pos(), is_minimal(), is_pinned());
			break;
		default:
			break;
		}
	}
}

void BuildingWindow::init(bool avoid_fastclick, bool workarea_preview_wanted) {
	capscache_player_number_ = 0;
	capsbuttons_ = nullptr;
	capscache_ = 0;
	caps_setup_ = false;
	toggle_workarea_ = nullptr;
	watch_button_ = nullptr;
	avoid_fastclick_ = avoid_fastclick;

	vbox_.reset(new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical));

	tabs_ = new UI::TabPanel(vbox_.get(), UI::TabPanelStyle::kWuiLight);
	vbox_->add(tabs_, UI::Box::Resizing::kFullSize);

	capsbuttons_ = new UI::Box(vbox_.get(), UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
	vbox_->add(capsbuttons_, UI::Box::Resizing::kFullSize);

	// actually create buttons on the first call to think(),
	// so that overriding create_capsbuttons() works

	set_center_panel(vbox_.get());
	set_thinks(true);
	set_fastclick_panel(this);
	if (workarea_preview_wanted) {
		show_workarea();
	} else {
		hide_workarea(true);
	}
}

// Stop everybody from thinking to avoid segfaults
void BuildingWindow::die() {
	is_dying_ = true;
	set_thinks(false);
	vbox_.reset(nullptr);
	UniqueWindow::die();
}

/*
===============
Draw a picture of the building in the background.
===============
*/
void BuildingWindow::draw(RenderTarget& dst) {
	Widelands::Building* building = building_.get(parent_->egbase());
	if (building == nullptr) {
		return;
	}

	UI::Window::draw(dst);

	// TODO(sirver): chang this to directly blit the animation. This needs support for or removal of
	// RenderTarget.
	const Image* image = building->representative_image();
	dst.blitrect_scale(
	   Rectf((get_inner_w() - image->width()) / 2.f, (get_inner_h() - image->height()) / 2.f,
	         image->width(), image->height()),
	   image, Recti(0, 0, image->width(), image->height()), 0.5, BlendMode::UseAlpha);
}

/*
===============
Check the capabilities and setup the capsbutton panel in case they've changed.
===============
*/
void BuildingWindow::think() {
	Widelands::Building* building = building_.get(parent_->egbase());
	if (building == nullptr || !ibase()->can_see(building->owner().player_number())) {
		die();
		return;
	}

	if (!caps_setup_ || capscache_player_number_ != ibase()->player_number() ||
	    building->get_playercaps() != capscache_) {
		capsbuttons_->free_children();
		create_capsbuttons(capsbuttons_, building);
		if (!avoid_fastclick_) {
			move_out_of_the_way();
			warp_mouse_to_fastclick_panel();
		}
		caps_setup_ = true;
	}

	if (mute_this_ != nullptr) {
		if (mute_all_ == nullptr) {
			NEVER_HERE();
		}
		mute_this_->set_pic(
		   g_image_cache->get(building->mute_messages() ? kImgUnmuteThis : kImgMuteThis));
		mute_this_->set_tooltip(building->mute_messages() ? _("Muted – click to unmute") :
                                                          _("Mute this building’s messages"));
		if (building->owner().is_muted(
		       building->owner().tribe().safe_building_index(building->descr().name()))) {
			mute_this_->set_enabled(false);
			mute_all_->set_pic(g_image_cache->get(kImgUnmuteAll));
			mute_all_->set_tooltip(_("All buildings of this type are muted – click to unmute"));
		} else {
			mute_this_->set_enabled(true);
			mute_all_->set_pic(g_image_cache->get(kImgMuteAll));
			mute_all_->set_tooltip(_("Mute all buildings of this type"));
		}
	}

	UI::Window::think();
}

static bool allow_muting(const Widelands::BuildingDescr& d) {
	if (d.type() == Widelands::MapObjectType::MILITARYSITE ||
	    d.type() == Widelands::MapObjectType::WAREHOUSE) {
		return true;
	}
	if (upcast(const Widelands::ProductionSiteDescr, p, &d)) {
		return !p->out_of_resource_message().empty() || !p->resource_not_needed_message().empty();
	}
	return false;
}

/**
 * Fill caps buttons into the given box.
 *
 * May be overridden to add additional buttons.
 *
 * \note Children of \p box must be allocated on the heap
 */
void BuildingWindow::create_capsbuttons(UI::Box* capsbuttons, Widelands::Building* building) {
	assert(building != nullptr);
	capscache_ = building->get_playercaps();
	capscache_player_number_ = ibase()->player_number();

	const Widelands::Player& owner = building->owner();
	const Widelands::PlayerNumber owner_number = owner.player_number();
	const bool can_see = ibase()->can_see(owner_number);
	const bool can_act = ibase()->can_act(owner_number);

	bool requires_destruction_separator = false;
	if (can_act) {
		// Check if this is a port building and if yes show expedition button
		if (upcast(Widelands::Warehouse const, warehouse, building)) {
			if (const Widelands::PortDock* pd = warehouse->get_portdock()) {
				expeditionbtn_ = new UI::Button(
				   capsbuttons, "start_or_cancel_expedition", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
				   g_image_cache->get("images/wui/buildings/start_expedition.png"));
				update_expedition_button(!pd->expedition_started());
				expeditionbtn_->sigclicked.connect([this]() { act_start_or_cancel_expedition(); });
				capsbuttons->add(expeditionbtn_);

				expedition_canceled_subscriber_ =
				   Notifications::subscribe<Widelands::NoteExpeditionCanceled>(
				      [this, pd](const Widelands::NoteExpeditionCanceled& canceled) {
					      // Check this was not just any but our Expedition
					      if (canceled.bootstrap == pd->expedition_bootstrap()) {
						      update_expedition_button(true);
					      }
				      });
			}
		} else if (upcast(const Widelands::ProductionSite, productionsite, building)) {
			const bool is_stopped = productionsite->is_stopped();
			UI::Button* stopbtn = new UI::Button(
			   capsbuttons, is_stopped ? "continue" : "stop", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			   g_image_cache->get(
			      (is_stopped ? "images/ui_basic/continue.png" : "images/ui_basic/stop.png")),
			   is_stopped ?
               /** TRANSLATORS: Stop/Continue toggle button for production sites. */
               _("Continue") :
               /** TRANSLATORS: Stop/Continue toggle button for production sites. */
               _("Stop"));
			stopbtn->sigclicked.connect([this]() { act_start_stop(); });
			capsbuttons->add(stopbtn);

			// Add a fixed width separator rather than infinite space so the
			// enhance/destroy/dismantle buttons are fixed in their position
			// and not subject to the number of buttons on the right of the
			// panel.
			UI::Panel* spacer = new UI::Panel(capsbuttons, UI::PanelStyle::kWui, 0, 0, 17, 34);
			capsbuttons->add(spacer);
		}  // upcast to productionsite

		upcast(Widelands::ConstructionSite, cs, building);
		if (((capscache_ & Widelands::Building::PCap_Enhancable) != 0u) ||
		    ((cs != nullptr) && cs->get_info().becomes->enhancement() != Widelands::INVALID_INDEX)) {
			const Widelands::DescriptionIndex& enhancement =
			   cs != nullptr ? cs->get_info().becomes->enhancement() : building->descr().enhancement();
			const Widelands::TribeDescr& tribe = owner.tribe();
			if (owner.is_building_type_allowed(enhancement) &&
			    owner.tribe().has_building(enhancement)) {
				const Widelands::BuildingDescr& building_descr = *tribe.get_building_descr(enhancement);
				std::string enhance_tooltip =
				   format(_("Enhance to %s"), building_descr.descname()) + "<br>" +
				   g_style_manager->ware_info_style(UI::WareInfoStyle::kNormal)
				      .header_font()
				      .as_font_tag(_("Construction costs:")) +
				   "<br>" + waremap_to_richtext(tribe, building_descr.enhancement_cost());

				UI::Button* enhancebtn =
				   new UI::Button(capsbuttons, "enhance", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
				                  building_descr.icon(), enhance_tooltip);

				//  button id = building id
				enhancebtn->sigclicked.connect(
				   [this, enhancement, cs] { act_enhance(enhancement, cs != nullptr); });
				capsbuttons->add(enhancebtn);
				requires_destruction_separator = true;
			}
		}

		if ((capscache_ & Widelands::Building::PCap_Bulldoze) != 0u) {
			UI::Button* destroybtn =
			   new UI::Button(capsbuttons, "destroy", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                  g_image_cache->get(kImgBulldoze), _("Destroy"));
			destroybtn->sigclicked.connect([this]() { act_bulldoze(); });
			capsbuttons->add(destroybtn);

			requires_destruction_separator = true;
		}

		if ((capscache_ & Widelands::Building::PCap_Dismantle) != 0u) {
			if (building->descr().can_be_dismantled()) {
				const Widelands::Buildcost wares =
				   Widelands::DismantleSite::count_returned_wares(building);
				if (!wares.empty()) {
					UI::Button* dismantlebtn =
					   new UI::Button(capsbuttons, "dismantle", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
					                  g_image_cache->get(kImgDismantle),
					                  std::string(_("Dismantle")) + "<br>" +
					                     g_style_manager->ware_info_style(UI::WareInfoStyle::kNormal)
					                        .header_font()
					                        .as_font_tag(_("Returns:")) +
					                     "<br>" + waremap_to_richtext(owner.tribe(), wares));
					dismantlebtn->sigclicked.connect([this]() { act_dismantle(); });
					capsbuttons->add(dismantlebtn);
					requires_destruction_separator = true;
				}
			}
		}

		if (requires_destruction_separator && can_see) {
			// Need this as well as the infinite space from the can_see section
			// to ensure there is a separation.
			UI::Panel* spacer = new UI::Panel(capsbuttons, UI::PanelStyle::kWui, 0, 0, 17, 34);
			capsbuttons->add(spacer);
			capsbuttons->add_inf_space();
		}

		if (allow_muting(building->descr())) {
			mute_this_ =
			   new UI::Button(capsbuttons, "mute_this", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                  g_image_cache->get(kImgMuteThis), "" /* set by next think() */);
			mute_all_ =
			   new UI::Button(capsbuttons, "mute_all", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                  g_image_cache->get(kImgMuteAll), "" /* set by next think() */);
			mute_all_->sigclicked.connect([this]() { act_mute(true); });
			mute_this_->sigclicked.connect([this]() { act_mute(false); });
			capsbuttons->add(mute_this_);
			capsbuttons->add(mute_all_);
		}
	}

	if (can_see) {
		const WorkareaInfo* wa_info;
		if (upcast(Widelands::ConstructionSite, csite, building)) {
			wa_info = &csite->building().workarea_info();
		} else {
			wa_info = &building->descr().workarea_info();
		}
		if (!wa_info->empty()) {
			toggle_workarea_ =
			   new UI::Button(capsbuttons, "workarea", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                  g_image_cache->get("images/wui/buildings/toggle_workarea.png"));
			toggle_workarea_->sigclicked.connect([this]() { toggle_workarea(); });

			capsbuttons->add(toggle_workarea_);
			configure_workarea_button();
			set_fastclick_panel(toggle_workarea_);
		}

		if (ibase()->get_display_flag(InteractiveBase::dfDebug)) {
			UI::Button* debugbtn =
			   new UI::Button(capsbuttons, "debug", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			                  g_image_cache->get(kImgDebug), _("Show Debug Window"));
			debugbtn->sigclicked.connect([this]() { act_debug(); });
			capsbuttons->add(debugbtn);
		}

		if (building->owner().tribe().safe_building_index(building->descr().name()) ==
		    building->owner().tribe().scouts_house()) {
			watch_button_ = new UI::Button(
			   capsbuttons, "watchscout", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
			   g_image_cache->get("images/wui/menus/watch_follow.png"), _("Watch the scout"));
			watch_button_->sigclicked.connect([this]() { clicked_watch(); });
			capsbuttons->add(watch_button_);
		}

		UI::Button* gotobtn =
		   new UI::Button(capsbuttons, "goto", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
		                  g_image_cache->get("images/wui/menus/goto.png"), _("Center view on this"));
		gotobtn->sigclicked.connect([this]() { clicked_goto(); });
		capsbuttons->add(gotobtn);

		if (!requires_destruction_separator) {
			// When there was no separation of destruction buttons put
			// the infinite space here (e.g. Warehouses)
			capsbuttons->add_inf_space();
		}

		UI::Button* helpbtn =
		   new UI::Button(capsbuttons, "help", 0, 0, 34, 34, UI::ButtonStyle::kWuiMenu,
		                  g_image_cache->get("images/ui_basic/menu_help.png"), _("Help"));

		UI::UniqueWindow::Registry& registry =
		   ibase()->unique_windows().get_registry(building_descr_for_help_->name() + "_help");
		registry.open_window = [this, &registry] {
			if (parent_ != nullptr) {
				Widelands::Building* building_in_lambda = building_.get(parent_->egbase());
				if (building_in_lambda == nullptr) {
					return;
				}
				new UI::BuildingHelpWindow(ibase(), registry, *building_descr_for_help_,
				                           building_in_lambda->owner().tribe(),
				                           &parent_->egbase().lua());
			}
		};

		helpbtn->sigclicked.connect([&registry]() { registry.toggle(); });
		capsbuttons->add(helpbtn);
	}
}

/**
===============
Callback for bulldozing request
===============
*/
void BuildingWindow::act_bulldoze() {
	Widelands::Building* building = building_.get(parent_->egbase());
	if (building == nullptr) {
		return;
	}

	if ((SDL_GetModState() & KMOD_CTRL) != 0) {
		if ((building->get_playercaps() & Widelands::Building::PCap_Bulldoze) != 0u) {
			game_->send_player_bulldoze(*building);
		}
	} else if (upcast(InteractivePlayer, ipl, ibase())) {
		show_bulldoze_confirm(*ipl, *building);
	}
}

/**
===============
Callback for dismantling request
===============
*/
void BuildingWindow::act_dismantle() {
	Widelands::Building* building = building_.get(parent_->egbase());
	if (building == nullptr) {
		return;
	}

	if ((SDL_GetModState() & KMOD_CTRL) != 0) {
		if ((building->get_playercaps() & Widelands::Building::PCap_Dismantle) != 0u) {
			if (game_ != nullptr) {
				game_->send_player_dismantle(*building, (SDL_GetModState() & KMOD_SHIFT) == 0);
			} else {
				NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
			}
			hide_workarea(true);
		}
	} else if (upcast(InteractivePlayer, ipl, ibase())) {
		show_dismantle_confirm(*ipl, *building);
	}
}

/**
===============
Callback for starting / stoping the production site request
===============
*/
void BuildingWindow::act_start_stop() {
	Widelands::Building* building = building_.get(parent_->egbase());
	if (building == nullptr) {
		return;
	}

	if (building->descr().type() >= Widelands::MapObjectType::PRODUCTIONSITE) {
		if (game_ != nullptr) {
			game_->send_player_start_stop_building(*building);
		} else {
			NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
		}
	}
}

/**
===============
Callback for starting an expedition request
===============
*/
void BuildingWindow::act_start_or_cancel_expedition() {
	Widelands::Building* building = building_.get(parent_->egbase());
	if (building == nullptr) {
		return;
	}

	if (upcast(Widelands::Warehouse const, warehouse, building)) {
		if (warehouse->get_portdock() != nullptr) {
			expeditionbtn_->set_enabled(false);
			if (game_ != nullptr) {
				game_->send_player_start_or_cancel_expedition(*building);
			} else {
				NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
			}
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
void BuildingWindow::act_enhance(Widelands::DescriptionIndex id, bool csite) {
	Widelands::Building* building = building_.get(parent_->egbase());
	if (building == nullptr) {
		return;
	}
	if (csite) {
		upcast(Widelands::ConstructionSite, construction_site, building);
		assert(construction_site);
		if ((SDL_GetModState() & KMOD_CTRL) != 0) {
			if (game_ != nullptr) {
				game_->send_player_enhance_building(*construction_site,
				                                    construction_site->building().enhancement(),
				                                    (SDL_GetModState() & KMOD_SHIFT) == 0);
			} else {
				NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
			}
		} else if (upcast(InteractivePlayer, ipl, ibase())) {
			show_enhance_confirm(
			   *ipl, *construction_site, construction_site->get_info().becomes->enhancement(), true);
		}
		return;
	}

	if ((SDL_GetModState() & KMOD_CTRL) != 0) {
		if ((building->get_playercaps() & Widelands::Building::PCap_Enhancable) != 0u) {
			if (game_ != nullptr) {
				game_->send_player_enhance_building(
				   *building, id, (SDL_GetModState() & KMOD_SHIFT) == 0);
			} else {
				NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
			}
		}
	} else if (upcast(InteractivePlayer, ipl, ibase())) {
		show_enhance_confirm(*ipl, *building, id);
	}
}

void BuildingWindow::act_mute(bool all) {
	if (Widelands::Building* building = building_.get(parent_->egbase())) {
		if (game_ != nullptr) {
			game_->send_player_toggle_mute(*building, all);
		} else {
			NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
		}
	}
}

/*
===============
Callback for debug window
===============
*/
void BuildingWindow::act_debug() {
	show_field_debug(*ibase(), ibase()->egbase().map().get_fcoords(building_position_));
}

/**
 * Show the building's workarea (if it has one).
 */
void BuildingWindow::show_workarea() {
	if (showing_workarea_) {
		return;  // already shown, nothing to be done
	}
	Widelands::Building* building = building_.get(parent_->egbase());
	if (building == nullptr) {
		return;
	}

	const WorkareaInfo* workarea_info;
	if (upcast(Widelands::ConstructionSite, csite, building)) {
		workarea_info = &csite->building().workarea_info();
	} else {
		workarea_info = &building->descr().workarea_info();
	}
	if (workarea_info->empty()) {
		return;
	}
	ibase()->show_workarea(*workarea_info, building_position_);
	showing_workarea_ = true;

	configure_workarea_button();
}

/**
 * Hide the workarea from view. Also configures whe toggle_workarea_ button if 'configure_button'
 * is 'true'.
 */
void BuildingWindow::hide_workarea(bool configure_button) {
	if (!showing_workarea_) {
		return;  // already hidden, nothing to be done
	}

	ibase()->hide_workarea(building_position_, false);
	showing_workarea_ = false;
	if (configure_button) {
		configure_workarea_button();
	}
}

/**
 * Sets the perpressed_ state and the tooltip.
 */
void BuildingWindow::configure_workarea_button() {
	if (toggle_workarea_ != nullptr) {
		if (showing_workarea_) {
			toggle_workarea_->set_tooltip(_("Hide work area"));
			toggle_workarea_->set_perm_pressed(true);
		} else {
			toggle_workarea_->set_tooltip(_("Show work area"));
			toggle_workarea_->set_perm_pressed(false);
		}
	}
}

void BuildingWindow::toggle_workarea() {
	if (showing_workarea_) {
		hide_workarea(true);
	} else {
		show_workarea();
	}
}

/**
 * Center the player's view on the building. Callback function
 * for the corresponding button.
 */
void BuildingWindow::clicked_goto() {
	ibase()->map_view()->scroll_to_field(building_position_, MapView::Transition::Smooth);
}

void BuildingWindow::update_expedition_button(bool expedition_was_canceled) {
	assert(expeditionbtn_ != nullptr);
	if (expedition_was_canceled) {
		expeditionbtn_->set_tooltip(_("Start an expedition"));
		expeditionbtn_->set_pic(g_image_cache->get("images/wui/buildings/start_expedition.png"));
		tabs_->remove_last_tab("expedition_wares_queue");
	} else {
		expeditionbtn_->set_tooltip(_("Cancel the expedition"));
		expeditionbtn_->set_pic(g_image_cache->get("images/wui/buildings/cancel_expedition.png"));
	}
	expeditionbtn_->set_enabled(true);
}

constexpr uint16_t kCurrentPacketVersion = 1;
UI::Window& BuildingWindow::load(FileRead& fr, InteractiveBase& ib) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const int32_t x = fr.signed_32();
			const int32_t y = fr.signed_32();
			const bool workarea = fr.unsigned_8() != 0u;
			BuildingWindow& bw = dynamic_cast<BuildingWindow&>(
			   *ib.show_building_window(Widelands::Coords(x, y), true, workarea));
			bw.tabs_->activate(fr.unsigned_8());
			return bw;
		}
		throw Widelands::UnhandledVersionError(
		   "Building Window", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("building window: %s", e.what());
	}
}
void BuildingWindow::save(FileWrite& fw, Widelands::MapObjectSaver& /* mos */) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.signed_32(building_position_.x);
	fw.signed_32(building_position_.y);
	fw.unsigned_8(showing_workarea_ ? 1 : 0);
	fw.unsigned_8(tabs_->active());
}
