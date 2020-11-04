/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include "wui/fieldaction.h"

#include "base/i18n.h"
#include "base/macros.h"
#include "economy/economy.h"
#include "economy/flag.h"
#include "economy/road.h"
#include "economy/waterway.h"
#include "graphic/style_manager.h"
#include "logic/cmd_queue.h"
#include "logic/map_objects/checkstep.h"
#include "logic/map_objects/tribes/attack_target.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/maphollowregion.h"
#include "logic/mapregion.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/icongrid.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"
#include "wui/actionconfirm.h"
#include "wui/attack_box.h"
#include "wui/economy_options_window.h"
#include "wui/game_debug_ui.h"
#include "wui/interactive_player.h"
#include "wui/waresdisplay.h"
#include "wui/watchwindow.h"

namespace Widelands {
class BuildingDescr;
}  // namespace Widelands
using Widelands::Building;
using Widelands::EditorGameBase;
using Widelands::Game;

constexpr int kBuildGridCellSize = 50;

// The BuildGrid presents a selection of buildable buildings
struct BuildGrid : public UI::IconGrid {
	BuildGrid(UI::Panel* parent, Widelands::Player* plr, int32_t x, int32_t y, int32_t cols);

	boost::signals2::signal<void(Widelands::DescriptionIndex)> buildclicked;
	boost::signals2::signal<void(Widelands::DescriptionIndex)> buildmouseout;
	boost::signals2::signal<void(Widelands::DescriptionIndex)> buildmousein;

	void add(Widelands::DescriptionIndex);

private:
	void click_slot(int32_t idx);
	void mouseout_slot(int32_t idx);
	void mousein_slot(int32_t idx);

private:
	Widelands::Player* plr_;
};

BuildGrid::BuildGrid(UI::Panel* parent, Widelands::Player* plr, int32_t x, int32_t y, int32_t cols)
   : UI::IconGrid(parent, UI::PanelStyle::kWui, x, y, kBuildGridCellSize, kBuildGridCellSize, cols),
     plr_(plr) {
	icon_clicked.connect([this](Widelands::DescriptionIndex i) { click_slot(i); });
	mouseout.connect([this](Widelands::DescriptionIndex i) { mouseout_slot(i); });
	mousein.connect([this](Widelands::DescriptionIndex i) { mousein_slot(i); });
}

/*
===============
Add a new building to the list of buildable buildings
===============
*/
void BuildGrid::add(Widelands::DescriptionIndex id) {
	const Widelands::BuildingDescr& descr =
	   *plr_->tribe().get_building_descr(Widelands::DescriptionIndex(id));

	UI::IconGrid::add(descr.name(), descr.representative_image(&plr_->get_playercolor()),
	                  reinterpret_cast<void*>(id),
	                  descr.descname() + "<br>" +
	                     g_style_manager->ware_info_style(UI::WareInfoStyle::kNormal)
	                        .header_font()
	                        .as_font_tag(_("Construction costs:")) +
	                     "<br>" + waremap_to_richtext(plr_->tribe(), descr.buildcost()));
}

/*
===============
BuildGrid::click_slot [private]

The icon with the given index has been clicked. Figure out which building it
belongs to and trigger signal buildclicked.
===============
*/
void BuildGrid::click_slot(int32_t idx) {
	buildclicked(static_cast<int32_t>(reinterpret_cast<intptr_t>(get_data(idx))));
}

/*
===============
BuildGrid::mouseout_slot [private]

The mouse pointer has left the icon with the given index. Figure out which
building it belongs to and trigger signal buildmouseout.
===============
*/
void BuildGrid::mouseout_slot(int32_t idx) {
	buildmouseout(static_cast<int32_t>(reinterpret_cast<intptr_t>(get_data(idx))));
}

/*
===============
BuildGrid::mousein_slot [private]

The mouse pointer has entered the icon with the given index. Figure out which
building it belongs to and trigger signal buildmousein.
===============
*/
void BuildGrid::mousein_slot(int32_t idx) {
	buildmousein(static_cast<int32_t>(reinterpret_cast<intptr_t>(get_data(idx))));
}

/*
==============================================================================

FieldActionWindow IMPLEMENTATION

==============================================================================
*/
class FieldActionWindow : public UI::UniqueWindow {
public:
	FieldActionWindow(InteractiveBase* ibase,
	                  Widelands::Player* plr,
	                  UI::UniqueWindow::Registry* registry);
	~FieldActionWindow() override;

	InteractiveBase& ibase() {
		return dynamic_cast<InteractiveBase&>(*get_parent());
	}

	void think() override;

	void init();
	void add_buttons_auto();
	void add_buttons_build(int32_t buildcaps, int32_t max_nodecaps);
	void add_buttons_road(bool flag);
	void add_buttons_waterway(bool flag);
	void add_buttons_attack();

	void act_watch();
	void act_debug();
	void act_buildflag();
	void act_configure_economy();
	void act_ripflag();
	void act_buildroad();
	void act_abort_buildroad();
	void act_abort_buildroad_and_start_buildwaterway();
	void act_removeroad();
	void act_buildwaterway();
	void act_abort_buildwaterway();
	void act_abort_buildwaterway_and_start_buildroad();
	void act_removewaterway();
	void act_build(Widelands::DescriptionIndex);
	void building_icon_mouse_out(Widelands::DescriptionIndex);
	void building_icon_mouse_in(Widelands::DescriptionIndex);
	void act_geologist();
	void act_mark_removal();
	void act_unmark_removal();
	void act_attack();  /// Launch the attack

	/// Total number of attackers available for a specific enemy flag
	uint32_t get_max_attackers();

private:
	uint32_t add_tab(const std::string& name,
	                 const char* picname,
	                 UI::Panel* panel,
	                 const std::string& tooltip_text = "");
	UI::Button& add_button(UI::Box*,
	                       const char* name,
	                       const char* picname,
	                       void (FieldActionWindow::*fn)(),
	                       const std::string& tooltip_text,
	                       bool repeating = false);
	void reset_mouse_and_die();

	void clear_overlapping_workareas();

	Widelands::Player* player_;
	const Widelands::Map& map_;

	Widelands::FCoords node_;

	UI::TabPanel tabpanel_;
	bool fastclick_;  // if true, put the mouse over first button in first tab
	uint32_t best_tab_;
	bool showing_workarea_preview_;
	std::set<Widelands::Coords> overlapping_workareas_;
	bool is_showing_workarea_overlaps_;
	Widelands::DescriptionIndex building_under_mouse_;

	/// Variables to use with attack dialog.
	AttackBox* attack_box_;
};

constexpr const char* const kImgTabBuildroad = "images/wui/fieldaction/menu_tab_buildroad.png";
constexpr const char* const kImgTabBuildwaterway =
   "images/wui/fieldaction/menu_tab_buildwaterway.png";
constexpr const char* const kImgTabWatch = "images/wui/fieldaction/menu_tab_watch.png";
static const char* const pic_tab_buildhouse[] = {"images/wui/fieldaction/menu_tab_buildsmall.png",
                                                 "images/wui/fieldaction/menu_tab_buildmedium.png",
                                                 "images/wui/fieldaction/menu_tab_buildbig.png",
                                                 "images/wui/fieldaction/menu_tab_buildport.png"};
static const char* const tooltip_tab_build[] = {_("Build small building"),
                                                _("Build medium building"), _("Build big building"),
                                                _("Build port building")};
static const char* const name_tab_build[] = {"small", "medium", "big", "port"};

constexpr const char* const kImgTabBuildmine = "images/wui/fieldaction/menu_tab_buildmine.png";

constexpr const char* const kImgButtonBuildRoad = "images/wui/fieldaction/menu_build_way.png";
constexpr const char* const kImgButtonRemoveRoad = "images/wui/fieldaction/menu_rem_way.png";
constexpr const char* const kImgButtonBuildWaterway = "images/wui/fieldaction/menu_build_water.png";
constexpr const char* const kImgButtonRemoveWaterway = "images/wui/fieldaction/menu_rem_water.png";
constexpr const char* const kImgButtonBuildFlag = "images/wui/fieldaction/menu_build_flag.png";
constexpr const char* const kImgButtonRipFlag = "images/wui/fieldaction/menu_rip_flag.png";
constexpr const char* const kImgButtonWatchField = "images/wui/fieldaction/menu_watch_field.png";
constexpr const char* const kImgDebug = "images/wui/fieldaction/menu_debug.png";
constexpr const char* const kImgButtonAbort = "images/wui/menu_abort.png";
constexpr const char* const kImgButtonGeologist = "images/wui/fieldaction/menu_geologist.png";
constexpr const char* const kImgButtonMarkRemoval = "images/wui/fieldaction/menu_mark_removal.png";
constexpr const char* const kImgButtonUnmarkRemoval =
   "images/wui/fieldaction/menu_unmark_removal.png";

constexpr const char* const kImgTabTarget = "images/wui/fieldaction/menu_tab_target.png";
constexpr const char* const kImgTabAttack = "images/wui/fieldaction/menu_tab_attack.png";

/*
===============
Initialize a field action window, creating the appropriate buttons.
===============
*/
FieldActionWindow::FieldActionWindow(InteractiveBase* const ib,
                                     Widelands::Player* const plr,
                                     UI::UniqueWindow::Registry* const registry)
   : UI::UniqueWindow(ib, UI::WindowStyle::kWui, "field_action", registry, 68, 34, _("Action")),
     player_(plr),
     map_(ib->egbase().map()),
     node_(ib->get_sel_pos().node, &map_[ib->get_sel_pos().node]),
     tabpanel_(this, UI::TabPanelStyle::kWuiDark),
     fastclick_(true),
     best_tab_(0),
     showing_workarea_preview_(false),
     is_showing_workarea_overlaps_(ib->get_display_flag(InteractiveBase::dfShowWorkareaOverlap)),
     building_under_mouse_(Widelands::INVALID_INDEX),
     attack_box_(nullptr) {
	ib->set_sel_freeze(true);
	set_center_panel(&tabpanel_);
}

FieldActionWindow::~FieldActionWindow() {
	if (showing_workarea_preview_) {
		ibase().hide_workarea(node_, false);
	}
	showing_workarea_preview_ = false;
	clear_overlapping_workareas();
	ibase().set_sel_freeze(false);
	delete attack_box_;
}

void FieldActionWindow::think() {
	if (is_showing_workarea_overlaps_ !=
	    ibase().get_display_flag(InteractiveBase::dfShowWorkareaOverlap)) {
		is_showing_workarea_overlaps_ = !is_showing_workarea_overlaps_;
		if (!is_showing_workarea_overlaps_) {
			clear_overlapping_workareas();
		} else {
			assert(overlapping_workareas_.empty());
			if (building_under_mouse_ != Widelands::INVALID_INDEX) {
				const Widelands::DescriptionIndex di = building_under_mouse_;
				building_icon_mouse_out(di);  // this unsets building_under_mouse_
				building_icon_mouse_in(di);
			}
		}
	}
	if (player_ && !player_->is_seeing(node_.field - &ibase().egbase().map()[0]) &&
	    !player_->see_all()) {
		die();
	}
}

void FieldActionWindow::clear_overlapping_workareas() {
	for (const Widelands::Coords& c : overlapping_workareas_) {
		ibase().hide_workarea(c, true);
	}
	overlapping_workareas_.clear();
}

/*
===============
Initialize after buttons have been registered.
This mainly deals with mouse placement
===============
*/
void FieldActionWindow::init() {
	center_to_parent();  // override UI::UniqueWindow position
	move_out_of_the_way();

	// Now force the mouse onto the first button
	set_mouse_pos(Vector2i(17 + 34 * best_tab_, fastclick_ ? 56 : 17));

	// Will only do something if we explicitly set another fast click panel
	// than the first button
	warp_mouse_to_fastclick_panel();
}

static bool suited_for_targeting(Widelands::PlayerNumber p,
                                 const Widelands::EditorGameBase& egbase,
                                 const Widelands::Immovable& i) {
	if (i.descr().collected_by().empty()) {
		return false;
	}
	const Widelands::Map& map = egbase.map();
	Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
	   map, Widelands::Area<Widelands::FCoords>(
	           map.get_fcoords(i.get_position()), egbase.descriptions().get_largest_workarea()));
	do {
		if (const Widelands::MapObject* mo = mr.location().field->get_immovable()) {
			if (mo->descr().type() < Widelands::MapObjectType::BUILDING) {
				continue;
			}

			const Widelands::BuildingDescr& descr =
			   mo->descr().type() == Widelands::MapObjectType::CONSTRUCTIONSITE ?
			      dynamic_cast<const Widelands::ConstructionSite&>(*mo).building() :
			      dynamic_cast<const Widelands::Building&>(*mo).descr();

			if (i.descr().collected_by().count(descr.name())) {
				upcast(const Widelands::Building, b, mo);
				assert(b);
				assert(!descr.workarea_info().empty());
				if (b->owner().player_number() == p &&
				    map.calc_distance(b->get_position(), i.get_position()) <=
				       descr.workarea_info().rbegin()->first) {
					return true;
				}
			}
		}
	} while (mr.advance(map));
	return false;
}

/*
===============
Add the buttons you normally get when clicking on a field.
===============
*/
void FieldActionWindow::add_buttons_auto() {
	UI::Box* buildbox = nullptr;
	UI::Box& watchbox = *new UI::Box(&tabpanel_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

	upcast(InteractiveGameBase, igbase, &ibase());

	if (upcast(InteractivePlayer, ipl, igbase)) {
		// Target immovables for removal by workers
		if (upcast(const Widelands::Immovable, mo, map_.get_immovable(node_))) {
			if (mo->is_marked_for_removal(ipl->player_number())) {
				UI::Box& box =
				   *new UI::Box(&tabpanel_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
				add_button(&box, "unmark_for_removal", kImgButtonUnmarkRemoval,
				           &FieldActionWindow::act_unmark_removal,
				           _("Marked for removal by a worker – click to unmark"));
				add_tab("target", kImgTabTarget, &box, _("Immovable Actions"));
			} else if (suited_for_targeting(ipl->player_number(), ipl->egbase(), *mo)) {
				UI::Box& box =
				   *new UI::Box(&tabpanel_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);
				add_button(&box, "mark_for_removal", kImgButtonMarkRemoval,
				           &FieldActionWindow::act_mark_removal,
				           _("Mark this immovable for timely removal by a suited worker"));
				add_tab("target", kImgTabTarget, &box, _("Immovable Actions"));
			}
		}
	}

	// Add road-building actions

	const Widelands::PlayerNumber owner = node_.field->get_owned_by();

	if (!igbase || igbase->can_see(owner)) {
		Widelands::BaseImmovable* const imm = map_.get_immovable(node_);
		const bool can_act = igbase ? igbase->can_act(owner) : true;

		// The box with road-building buttons
		buildbox = new UI::Box(&tabpanel_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

		if (upcast(Widelands::Flag, flag, imm)) {
			// Add flag actions
			if (can_act) {
				add_button(buildbox, "build_road", kImgButtonBuildRoad,
				           &FieldActionWindow::act_buildroad, _("Build road"));
				if (map_.get_waterway_max_length() >= 2 &&
				    Widelands::CheckStepFerry(igbase->egbase()).reachable_dest(map_, node_)) {
					add_button(buildbox, "build_waterway", kImgButtonBuildWaterway,
					           &FieldActionWindow::act_buildwaterway, _("Build waterway"));
				}

				Building* const building = flag->get_building();

				if (!building || (building->get_playercaps() & Building::PCap_Bulldoze)) {
					add_button(buildbox, "rip_flag", kImgButtonRipFlag, &FieldActionWindow::act_ripflag,
					           _("Destroy this flag"));
				}
			}

			if (ibase().egbase().is_game()) {
				add_button(buildbox, "configure_economy", "images/wui/stats/genstats_nrwares.png",
				           &FieldActionWindow::act_configure_economy,
				           _("Configure this flag’s economy"));
				if (can_act) {
					add_button(buildbox, "geologist", kImgButtonGeologist,
					           &FieldActionWindow::act_geologist, _("Send geologist to explore site"));
				}
			}
		} else {
			const int32_t buildcaps = player_ ? player_->get_buildcaps(node_) : 0;
			const int32_t nodecaps = map_.get_max_nodecaps(ibase().egbase(), node_);

			// Add house building
			if (player_ && ((nodecaps & Widelands::BUILDCAPS_SIZEMASK) ||
			                (nodecaps & Widelands::BUILDCAPS_MINE))) {
				add_buttons_build(buildcaps, nodecaps);
			}

			// Add build actions
			if (buildcaps & Widelands::BUILDCAPS_FLAG) {
				add_button(buildbox, "build_flag", kImgButtonBuildFlag,
				           &FieldActionWindow::act_buildflag, _("Place a flag"));
			}

			if (can_act && dynamic_cast<const Widelands::Road*>(imm)) {
				add_button(buildbox, "destroy_road", kImgButtonRemoveRoad,
				           &FieldActionWindow::act_removeroad, _("Destroy a road"));
			}

			if (can_act && dynamic_cast<const Widelands::Waterway*>(imm)) {
				add_button(buildbox, "destroy_waterway", kImgButtonRemoveWaterway,
				           &FieldActionWindow::act_removewaterway, _("Destroy a waterway"));
			}
		}
	} else if (player_) {
		if (upcast(Building, building, map_.get_immovable(node_))) {
			for (Widelands::Coords& coords : building->get_positions(igbase->egbase())) {
				if (player_->is_seeing(
				       Widelands::Map::get_index(coords, ibase().egbase().map().get_width()))) {
					add_buttons_attack();
					break;
				}
			}
		}
	}

	//  Watch actions, only when in game (no use in editor).
	if (ibase().egbase().is_game()) {
		add_button(&watchbox, "watch", kImgButtonWatchField, &FieldActionWindow::act_watch,
		           _("Watch field in a separate window"));
	}

	if (ibase().get_display_flag(InteractiveBase::dfDebug)) {
		add_button(
		   &watchbox, "debug", kImgDebug, &FieldActionWindow::act_debug, _("Show Debug Window"));
	}

	// Add tabs
	if (buildbox && buildbox->get_nritems()) {
		add_tab("roads", kImgTabBuildroad, buildbox, _("Build road"));
	}

	add_tab("watch", kImgTabWatch, &watchbox, _("Watch"));
}

void FieldActionWindow::add_buttons_attack() {
	UI::Box& a_box = *new UI::Box(&tabpanel_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

	if (upcast(Widelands::Building, building, map_.get_immovable(node_))) {
		if (const Widelands::AttackTarget* attack_target = building->attack_target()) {
			if (player_ && player_->is_hostile(building->owner()) &&
			    attack_target->can_be_attacked()) {
				attack_box_ = new AttackBox(&a_box, player_, &node_, 0, 0);
				a_box.add(attack_box_);

				UI::Button* attack_button = attack_box_->get_attack_button();
				attack_button->sigclicked.connect([this]() { act_attack(); });
				set_fastclick_panel(attack_button);
			}
		}
	}

	if (a_box.get_nritems()) {  //  add tab
		add_tab("attack", kImgTabAttack, &a_box, _("Attack"));
	}
}

/*
===============
Add buttons for house building.
===============
*/
void FieldActionWindow::add_buttons_build(int32_t buildcaps, int32_t max_nodecaps) {
	if (!player_) {
		return;
	}
	const Widelands::FCoords brn = map_.br_n(node_);
	if (!node_.field->is_interior(player_->player_number()) ||
	    !brn.field->is_interior(player_->player_number())) {
		return;
	}
	if (!(brn.field->get_immovable() &&
	      brn.field->get_immovable()->descr().type() == Widelands::MapObjectType::FLAG) &&
	    !(player_->get_buildcaps(brn) & Widelands::BUILDCAPS_FLAG)) {
		return;
	}
	BuildGrid* bbg_house[4] = {nullptr, nullptr, nullptr, nullptr};
	BuildGrid* bbg_mine = nullptr;

	const Widelands::TribeDescr& tribe = player_->tribe();

	fastclick_ = false;

	for (const Widelands::DescriptionIndex& building_index : tribe.buildings()) {
		const Widelands::BuildingDescr* building_descr = tribe.get_building_descr(building_index);
		BuildGrid** ppgrid;

		//  Some building types cannot be built (i.e. construction site) and not
		//  allowed buildings.
		if (ibase().egbase().is_game()) {
			if (!building_descr->is_buildable() ||
			    !player_->is_building_type_allowed(building_index)) {
				continue;
			}
			if (!building_descr->is_useful_on_map(
			       ibase().egbase().map().allows_seafaring(),
			       ibase().egbase().map().get_waterway_max_length() >= 2)) {
				continue;
			}
		} else if (!building_descr->is_buildable() && !building_descr->is_enhanced()) {
			continue;
		}

		if (building_descr->get_built_over_immovable() != Widelands::INVALID_INDEX &&
		    !(node_.field->get_immovable() && node_.field->get_immovable()->has_attribute(
		                                         building_descr->get_built_over_immovable()))) {
			continue;
		}
		// Figure out if we can build it here, and in which tab it belongs
		if (building_descr->get_ismine()) {
			if (!((building_descr->get_built_over_immovable() == Widelands::INVALID_INDEX ?
			          buildcaps :
			          max_nodecaps) &
			      Widelands::BUILDCAPS_MINE)) {
				continue;
			}

			ppgrid = &bbg_mine;
		} else {
			int32_t size = building_descr->get_size() - Widelands::BaseImmovable::SMALL;

			if (((building_descr->get_built_over_immovable() == Widelands::INVALID_INDEX ?
			         buildcaps :
			         max_nodecaps) &
			     Widelands::BUILDCAPS_SIZEMASK) < size + 1) {
				continue;
			}
			if (building_descr->get_isport() && !(buildcaps & Widelands::BUILDCAPS_PORT)) {
				continue;
			}
			if (building_descr->get_size() >= Widelands::BaseImmovable::BIG &&
			    !((map_.l_n(node_).field->is_interior(player_->player_number())) &&
			      (map_.tr_n(node_).field->is_interior(player_->player_number())) &&
			      (map_.tl_n(node_).field->is_interior(player_->player_number())))) {
				continue;
			}

			if (building_descr->get_isport()) {
				ppgrid = &bbg_house[3];
			} else {
				ppgrid = &bbg_house[size];
			}
		}

		// Allocate the tab's grid if necessary
		if (!*ppgrid) {
			*ppgrid = new BuildGrid(&tabpanel_, player_, 0, 0, 5);
			(*ppgrid)->buildclicked.connect([this](Widelands::DescriptionIndex i) { act_build(i); });
			(*ppgrid)->buildmouseout.connect(
			   [this](Widelands::DescriptionIndex i) { building_icon_mouse_out(i); });
			(*ppgrid)->buildmousein.connect(
			   [this](Widelands::DescriptionIndex i) { building_icon_mouse_in(i); });
		}

		// Add it to the grid
		(*ppgrid)->add(building_index);
	}

	// Add all necessary tabs
	for (int32_t i = 0; i < 4; ++i) {
		if (bbg_house[i]) {
			tabpanel_.activate(best_tab_ =
			                      add_tab(name_tab_build[i], pic_tab_buildhouse[i], bbg_house[i],
			                              i18n::translate(tooltip_tab_build[i])));
		}
	}

	if (bbg_mine) {
		tabpanel_.activate(best_tab_ =
		                      add_tab("mines", kImgTabBuildmine, bbg_mine, _("Build mines")));
	}
}

/*
===============
Buttons used during road building: Set flag here and Abort
===============
*/
void FieldActionWindow::add_buttons_road(bool flag) {
	UI::Box& buildbox = *new UI::Box(&tabpanel_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

	if (flag) {
		add_button(&buildbox, "build_flag", kImgButtonBuildFlag, &FieldActionWindow::act_buildflag,
		           _("Build flag"));
	}

	add_button(&buildbox, "cancel_road", kImgButtonAbort, &FieldActionWindow::act_abort_buildroad,
	           _("Cancel road"));
	const Widelands::Map& map = ibase().egbase().map();
	if (map.get_waterway_max_length() >= 2 &&
	    Widelands::CheckStepFerry(ibase().egbase())
	       .reachable_dest(map, map.get_fcoords(ibase().get_build_road_start()))) {
		add_button(&buildbox, "cancel_road_build_waterway", kImgButtonBuildWaterway,
		           &FieldActionWindow::act_abort_buildroad_and_start_buildwaterway,
		           _("Cancel road and start building waterway"));
	}

	// Add the box as tab
	add_tab("roads", kImgTabBuildroad, &buildbox, _("Build roads"));
}

void FieldActionWindow::add_buttons_waterway(bool flag) {
	UI::Box& buildbox = *new UI::Box(&tabpanel_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

	if (flag) {
		add_button(&buildbox, "build_flag", kImgButtonBuildFlag, &FieldActionWindow::act_buildflag,
		           _("Build flag"));
	}

	add_button(&buildbox, "cancel_waterway", kImgButtonAbort,
	           &FieldActionWindow::act_abort_buildwaterway, _("Cancel waterway"));
	add_button(&buildbox, "cancel_waterway_build_road", kImgButtonBuildRoad,
	           &FieldActionWindow::act_abort_buildwaterway_and_start_buildroad,
	           _("Cancel waterway and start building road"));

	// Add the box as tab
	add_tab("waterways", kImgTabBuildwaterway, &buildbox, _("Build waterways"));
}

/*
===============
Convenience function: Adds a new tab to the main tab panel
===============
*/
uint32_t FieldActionWindow::add_tab(const std::string& name,
                                    const char* picname,
                                    UI::Panel* panel,
                                    const std::string& tooltip_text) {
	return tabpanel_.add(name, g_image_cache->get(picname), panel, tooltip_text);
}

UI::Button& FieldActionWindow::add_button(UI::Box* const box,
                                          const char* const name,
                                          const char* const picname,
                                          void (FieldActionWindow::*fn)(),
                                          const std::string& tooltip_text,
                                          bool repeating) {
	UI::Button& button = *new UI::Button(box, name, 0, 0, 34, 34, UI::ButtonStyle::kWuiPrimary,
	                                     g_image_cache->get(picname), tooltip_text);
	button.sigclicked.connect([this, fn]() { (this->*fn)(); });
	button.set_repeating(repeating);
	box->add(&button);

	return button;
}

/*
===============
Call this from the button handlers.
It resets the mouse to its original position and closes the window
===============
*/
void FieldActionWindow::reset_mouse_and_die() {
	ibase().map_view()->mouse_to_field(node_, MapView::Transition::Jump);
	die();
}

/*
===============
Open a watch window for the given field and delete self.
===============
*/
void FieldActionWindow::act_watch() {
	upcast(InteractiveGameBase, igbase, &ibase());
	show_watch_window(*igbase, node_);
	reset_mouse_and_die();
}

/*
===============
Show a debug widow for this field.
===============
*/
void FieldActionWindow::act_debug() {
	show_field_debug(ibase(), node_);
}

/*
===============
Build a flag at this field
===============
*/
void FieldActionWindow::act_buildflag() {
	upcast(Game, game, &ibase().egbase());
	if (game) {
		game->send_player_build_flag(player_->player_number(), node_);
	} else {
		player_->build_flag(node_);
	}

	if (ibase().in_road_building_mode()) {
		ibase().finish_build_road();
	} else if (game) {
		upcast(InteractivePlayer, iaplayer, &ibase());
		iaplayer->set_flag_to_connect(node_);
	}

	reset_mouse_and_die();
}

void FieldActionWindow::act_configure_economy() {
	if (upcast(const Widelands::Flag, flag, node_.field->get_immovable())) {
		if (upcast(InteractiveGameBase, igbase, &ibase())) {
			Widelands::Economy* ware_economy = flag->get_economy(Widelands::wwWARE);
			const bool can_act = igbase->can_act(ware_economy->owner().player_number());
			EconomyOptionsWindow::create(&ibase(), *flag, Widelands::WareWorker::wwWARE, can_act);
		}
	}
	die();
}

/*
===============
Remove the flag at this field
===============
*/
void FieldActionWindow::act_ripflag() {
	reset_mouse_and_die();
	Widelands::EditorGameBase& egbase = ibase().egbase();
	upcast(Game, game, &egbase);
	upcast(InteractivePlayer, iaplayer, &ibase());

	if (upcast(Widelands::Flag, flag, node_.field->get_immovable())) {
		const bool ctrl_pressed = SDL_GetModState() & KMOD_CTRL;
		if (Building* const building = flag->get_building()) {
			if (building->get_playercaps() & Building::PCap_Bulldoze) {
				if (ctrl_pressed) {
					game->send_player_bulldoze(*flag, ctrl_pressed);
				} else {
					show_bulldoze_confirm(*iaplayer, *building, flag);
				}
			}
		} else {
			game->send_player_bulldoze(*flag, ctrl_pressed);
		}
	}
}

/*
===============
Start road building.
===============
*/
void FieldActionWindow::act_buildroad() {
	// If we are already building a road just ignore this
	if (!ibase().in_road_building_mode()) {
		ibase().start_build_road(node_, player_->player_number(), RoadBuildingType::kRoad);
		reset_mouse_and_die();
	}
}

void FieldActionWindow::act_buildwaterway() {
	// If we are already building a waterway just ignore this
	if (!ibase().in_road_building_mode(RoadBuildingType::kWaterway)) {
		ibase().start_build_road(node_, player_->player_number(), RoadBuildingType::kWaterway);
		reset_mouse_and_die();
	}
}

/*
===============
Abort building a road.
===============
*/
void FieldActionWindow::act_abort_buildroad() {
	if (!ibase().in_road_building_mode(RoadBuildingType::kRoad)) {
		return;
	}

	ibase().abort_build_road();
	reset_mouse_and_die();
}

void FieldActionWindow::act_abort_buildwaterway() {
	if (!ibase().in_road_building_mode(RoadBuildingType::kWaterway)) {
		return;
	}

	ibase().abort_build_road();
	reset_mouse_and_die();
}

void FieldActionWindow::act_abort_buildroad_and_start_buildwaterway() {
	if (!ibase().in_road_building_mode(RoadBuildingType::kRoad) ||
	    ibase().in_road_building_mode(RoadBuildingType::kWaterway)) {
		return;
	}
	const Widelands::Coords c = ibase().get_build_road_start();
	ibase().abort_build_road();
	ibase().start_build_road(c, player_->player_number(), RoadBuildingType::kWaterway);
	reset_mouse_and_die();
}

void FieldActionWindow::act_abort_buildwaterway_and_start_buildroad() {
	if (ibase().in_road_building_mode(RoadBuildingType::kRoad) ||
	    !ibase().in_road_building_mode(RoadBuildingType::kWaterway)) {
		return;
	}
	const Widelands::Coords c = ibase().get_build_road_start();
	ibase().abort_build_road();
	ibase().start_build_road(c, player_->player_number(), RoadBuildingType::kRoad);
	reset_mouse_and_die();
}

/*
===============
Remove the road at the given field
===============
*/
void FieldActionWindow::act_removeroad() {
	Widelands::EditorGameBase& egbase = ibase().egbase();
	if (upcast(Widelands::Road, road, egbase.map().get_immovable(node_))) {
		upcast(Game, game, &ibase().egbase());
		game->send_player_bulldoze(*road, SDL_GetModState() & KMOD_CTRL);
	}
	reset_mouse_and_die();
}

void FieldActionWindow::act_removewaterway() {
	Widelands::EditorGameBase& egbase = ibase().egbase();
	if (upcast(Widelands::Waterway, waterway, egbase.map().get_immovable(node_))) {
		upcast(Game, game, &ibase().egbase());
		game->send_player_bulldoze(*waterway, SDL_GetModState() & KMOD_CTRL);
	}
	reset_mouse_and_die();
}

/*
===============
Start construction of the building with the give description index
===============
*/
void FieldActionWindow::act_build(Widelands::DescriptionIndex idx) {
	upcast(Game, game, &ibase().egbase());
	upcast(InteractivePlayer, iaplayer, &ibase());

	game->send_player_build(iaplayer->player_number(), node_, Widelands::DescriptionIndex(idx));
	iaplayer->set_flag_to_connect(game->map().br_n(node_));
	reset_mouse_and_die();
}

void FieldActionWindow::building_icon_mouse_out(Widelands::DescriptionIndex) {
	if (showing_workarea_preview_) {
		ibase().hide_workarea(node_, false);
		showing_workarea_preview_ = false;
		building_under_mouse_ = Widelands::INVALID_INDEX;
		clear_overlapping_workareas();
	}
}

constexpr uint32_t kOverlapColorDefault = 0xff3f3fbf;
constexpr uint32_t kOverlapColorPositive = 0xff3fbf3f;
constexpr uint32_t kOverlapColorNegative = 0xffbf3f3f;
constexpr uint32_t kOverlapColorPale = 0x7fffffff;

void FieldActionWindow::building_icon_mouse_in(const Widelands::DescriptionIndex idx) {
	if (!showing_workarea_preview_) {
		assert(overlapping_workareas_.empty());
		building_under_mouse_ = idx;
		const Widelands::BuildingDescr& descr = *player_->tribe().get_building_descr(idx);
		const WorkareaInfo& workarea_info = descr.workarea_info();
		ibase().show_workarea(workarea_info, node_);
		showing_workarea_preview_ = true;
		if (!is_showing_workarea_overlaps_) {
			return;
		}

		const Widelands::Map& map = ibase().egbase().map();
		uint32_t workarea_radius = 0;
		for (const auto& pair : workarea_info) {
			workarea_radius = std::max(workarea_radius, pair.first);
		}
		if (workarea_radius == 0) {
			return;
		}
		std::set<Widelands::TCoords<>> main_region =
		   map.triangles_in_region(map.to_set(Widelands::Area<>(node_, workarea_radius)));

		Widelands::MapRegion<Widelands::Area<Widelands::FCoords>> mr(
		   map, Widelands::Area<Widelands::FCoords>(
		           node_, workarea_radius + ibase().egbase().descriptions().get_largest_workarea()));
		do {
			if (player_->is_seeing(map.get_index(mr.location()))) {
				if (Widelands::BaseImmovable* imm = mr.location().field->get_immovable()) {
					const Widelands::MapObjectType imm_type = imm->descr().type();
					if (imm_type < Widelands::MapObjectType::BUILDING) {
						// We are not interested in trees and pebbles
						continue;
					}
					const Widelands::BuildingDescr* d = nullptr;
					bool positive = false;  // unused default value to make g++ happy
					if (imm_type == Widelands::MapObjectType::CONSTRUCTIONSITE) {
						upcast(Widelands::ConstructionSite, cs, imm);
						d = cs->get_info().becomes;
						assert(d);
						if ((descr.type() == Widelands::MapObjectType::PRODUCTIONSITE &&
						     (d->type() != Widelands::MapObjectType::PRODUCTIONSITE ||
						      !dynamic_cast<const Widelands::ProductionSiteDescr&>(descr)
						          .highlight_overlapping_workarea_for(d->name(), &positive))) ||
						    ((descr.type() == Widelands::MapObjectType::MILITARYSITE ||
						      descr.type() == Widelands::MapObjectType::WAREHOUSE) &&
						     d->type() != Widelands::MapObjectType::MILITARYSITE &&
						     d->type() != Widelands::MapObjectType::WAREHOUSE)) {
							continue;
						}
					} else if (descr.type() == Widelands::MapObjectType::PRODUCTIONSITE) {
						if (imm_type != Widelands::MapObjectType::PRODUCTIONSITE ||
						    imm->get_owner() != player_ ||
						    !dynamic_cast<const Widelands::ProductionSiteDescr&>(descr)
						        .highlight_overlapping_workarea_for(imm->descr().name(), &positive)) {
							continue;
						}
					} else if (descr.type() == Widelands::MapObjectType::WAREHOUSE ||
					           descr.type() == Widelands::MapObjectType::MILITARYSITE) {
						if (imm_type != Widelands::MapObjectType::MILITARYSITE &&
						    imm_type != Widelands::MapObjectType::WAREHOUSE) {
							continue;
						}
					}
					upcast(Widelands::Building, bld, imm);
					if (bld->get_position() != mr.location()) {
						// Don't count big buildings more than once
						continue;
					}
					if (!d) {
						d = &bld->descr();
					}
					const WorkareaInfo& wa = d->workarea_info();
					uint32_t wa_radius = 0;
					for (const auto& pair : wa) {
						wa_radius = std::max(wa_radius, pair.first);
					}
					if (wa_radius == 0) {
						continue;
					}
					if (map.calc_distance(node_, mr.location()) <= workarea_radius + wa_radius) {
						std::map<Widelands::TCoords<>, uint32_t> colors;
						for (const Widelands::TCoords<>& t : map.triangles_in_region(
						        map.to_set(Widelands::Area<>(mr.location(), wa_radius)))) {
							colors[t] = mr.location() == t.node || mr.location() == map.br_n(t.node) ||
							                  mr.location() == (t.t == Widelands::TriangleIndex::D ?
							                                       map.bl_n(t.node) :
							                                       map.r_n(t.node)) ||
							                  main_region.count(t) ?
							               descr.type() == Widelands::MapObjectType::PRODUCTIONSITE ?
							               positive ? kOverlapColorPositive : kOverlapColorNegative :
							               kOverlapColorDefault :
							               kOverlapColorPale;
						}
						ibase().show_workarea(wa, mr.location(), colors);
						overlapping_workareas_.insert(mr.location());
					}
				}
			}
		} while (mr.advance(map));
	}
}

/*
===============
Call a geologist on this flag.
===============
*/
void FieldActionWindow::act_geologist() {
	upcast(Game, game, &ibase().egbase());
	if (upcast(Widelands::Flag, flag, game->map().get_immovable(node_))) {
		game->send_player_flagaction(*flag);
	}
	reset_mouse_and_die();
}

void FieldActionWindow::act_mark_removal() {
	upcast(Game, game, &ibase().egbase());
	if (upcast(Widelands::Immovable, i, game->map().get_immovable(node_))) {
		game->send_player_mark_object_for_removal(
		   dynamic_cast<InteractiveGameBase&>(ibase()).player_number(), *i, true);
	}
	reset_mouse_and_die();
}
void FieldActionWindow::act_unmark_removal() {
	upcast(Game, game, &ibase().egbase());
	if (upcast(Widelands::Immovable, i, game->map().get_immovable(node_))) {
		game->send_player_mark_object_for_removal(
		   dynamic_cast<InteractiveGameBase&>(ibase()).player_number(), *i, false);
	}
	reset_mouse_and_die();
}

/**
 * Here there are a problem: the sender of an event is always the owner of
 * were is done this even. But for attacks, the owner of an event is the
 * player who start an attack, so is needed to get an extra parameter to
 * the send_player_enemyflagaction, the player number
 */
void FieldActionWindow::act_attack() {
	assert(attack_box_);
	upcast(Game, game, &ibase().egbase());
	if (upcast(Building, building, game->map().get_immovable(node_))) {
		if (attack_box_->count_soldiers() > 0) {
			upcast(InteractivePlayer const, iaplayer, &ibase());
			game->send_player_enemyflagaction(
			   building->base_flag(), iaplayer->player_number(), attack_box_->soldiers());
		}
	}
	reset_mouse_and_die();
}

/*
===============
show_field_action

Perform a field action (other than building options).
Bring up a field action window or continue road building.
===============
*/
void show_field_action(InteractiveBase* const ibase,
                       Widelands::Player* const player,
                       UI::UniqueWindow::Registry* const registry) {
	if (ibase->in_road_building_mode()) {
		// we're building a road or waterway right now
		const Widelands::Map& map = player->egbase().map();
		const Widelands::FCoords target = map.get_fcoords(ibase->get_sel_pos().node);

		// if user clicked on the same field again, build a flag
		if (target == ibase->get_build_road_end()) {
			FieldActionWindow& w = *new FieldActionWindow(ibase, player, registry);
			if (ibase->in_road_building_mode(RoadBuildingType::kRoad)) {
				w.add_buttons_road(target != ibase->get_build_road_start() &&
				                   (player->get_buildcaps(target) & Widelands::BUILDCAPS_FLAG));
			} else {
				w.add_buttons_waterway(target != ibase->get_build_road_start() &&
				                       (player->get_buildcaps(target) & Widelands::BUILDCAPS_FLAG));
			}
			w.init();
			return;
		}

		// append or take away from the road
		if (!ibase->append_build_road(target) ||
		    (ibase->in_road_building_mode(RoadBuildingType::kWaterway) &&
		     target != ibase->get_build_road_end())) {
			FieldActionWindow& w = *new FieldActionWindow(ibase, player, registry);
			if (ibase->in_road_building_mode(RoadBuildingType::kRoad)) {
				w.add_buttons_road(false);
			} else {
				w.add_buttons_waterway(false);
			}
			w.init();
			return;
		}

		// did he click on a flag or a road where a flag can be built?
		if (upcast(const Widelands::PlayerImmovable, i, map.get_immovable(target))) {
			bool finish = false;
			if (i->descr().type() == Widelands::MapObjectType::FLAG) {
				finish = true;
			} else if (i->descr().type() == Widelands::MapObjectType::ROAD ||
			           i->descr().type() == Widelands::MapObjectType::WATERWAY) {
				if (player->get_buildcaps(target) & Widelands::BUILDCAPS_FLAG) {
					upcast(Game, game, &player->egbase());
					game->send_player_build_flag(player->player_number(), target);
					finish = true;
				}
			}
			if (finish) {
				ibase->finish_build_road();
				// We are done, so we close the window.
				registry->destroy();
				return;
			} else {
				FieldActionWindow& w = *new FieldActionWindow(ibase, player, registry);
				if (ibase->in_road_building_mode(RoadBuildingType::kRoad)) {
					w.add_buttons_road(false);
				} else {
					w.add_buttons_waterway(false);
				}
				w.init();
				return;
			}
		}
	} else {
		FieldActionWindow& w = *new FieldActionWindow(ibase, player, registry);
		w.add_buttons_auto();
		return w.init();
	}
}
