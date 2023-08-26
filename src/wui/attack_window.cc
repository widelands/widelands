/*
 * Copyright (C) 2002-2023 by the Widelands Development Team
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

#include "wui/attack_window.h"

#include <algorithm>
#include <memory>

#include <SDL_mouse.h>

#include "base/mutex.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/game_debug_ui.h"

constexpr Duration kUpdateTimeInGametimeMs(500);  //  half a second, gametime
constexpr int kSpacing = 8;

static unsigned next_serial_(0);
static std::map<unsigned, AttackWindow*> living_attack_windows_;

static const RGBAColor& get_soldier_color(const Widelands::Soldier* soldier) {
	constexpr uint8_t kAlpha = 64;
	Widelands::Serial serial = 0;

	if (soldier->is_shipping()) {
		serial = soldier->get_ship_serial();
	} else if (const Widelands::PlayerImmovable* loc =
	              soldier->get_location(soldier->owner().egbase());
	           loc != nullptr) {
		serial = loc->serial();
	}

	static std::map<Widelands::Serial, RGBAColor> soldier_colors;
	if (soldier_colors.count(serial) == 0) {
		soldier_colors.emplace(serial, RGBAColor(RNG::static_rand(256), RNG::static_rand(256),
		                                         RNG::static_rand(256), kAlpha));
	}
	return soldier_colors.at(serial);
}

AttackWindow::AttackWindow(InteractivePlayer& parent,
                           UI::UniqueWindow::Registry& reg,
                           Widelands::MapObject* target_building_or_ship,
                           const Widelands::Coords& target_coords,
                           bool fastclick)
   : UI::UniqueWindow(&parent, UI::WindowStyle::kWui, "attack", &reg, 0, 0, _("Attack")),
     serial_(next_serial_++),
     iplayer_(parent),
     map_(iplayer_.player().egbase().map()),
     target_building_or_ship_(target_building_or_ship),
     target_coordinates_(target_coords),
     attack_type_(target_building_or_ship == nullptr ? AttackPanel::AttackType::kNavalInvasion :
                  target_building_or_ship->descr().type() == Widelands::MapObjectType::SHIP ?
                                                       AttackPanel::AttackType::kShip :
                                                       AttackPanel::AttackType::kBuilding),

     mainbox_(this, UI::PanelStyle::kWui, "main_box", 0, 0, UI::Box::Vertical),
     attack_panel_(mainbox_,
                   iplayer_,
                   true,
                   &target_coordinates_,
                   attack_type_,
                   [this]() { return get_max_attackers(); }),
     bottombox_(&mainbox_, UI::PanelStyle::kWui, "bottom_box", 0, 0, UI::Box::Horizontal) {
	if (target_building_or_ship != nullptr) {
		const unsigned serial = serial_;
		living_attack_windows_[serial] = this;
		target_building_or_ship->removed.connect([serial](unsigned /* index */) {
			auto it = living_attack_windows_.find(serial);
			if (it != living_attack_windows_.end()) {
				it->second->die();
			}
		});
	}

	mainbox_.add(&attack_panel_, UI::Box::Resizing::kExpandBoth);
	init_bottombox();

	attack_panel_.act_attack.connect([this]() { act_attack(); });
	set_fastclick_panel(attack_panel_.attack_button_.get());
	set_center_panel(&mainbox_);

	// Update the list of soldiers now to avoid a flickering window in the next tick
	attack_panel_.update(false);

	center_to_parent();
	if (fastclick) {
		warp_mouse_to_fastclick_panel();
	}
	initialization_complete();
}

AttackPanel::AttackPanel(UI::Panel& parent,
                         InteractivePlayer& iplayer,
                         bool can_attack,
                         const Widelands::Coords* target_coordinates,
                         AttackType attack_type,
                         std::function<std::vector<Widelands::Bob*>()> get_max_attackers)
   : UI::Box(&parent, UI::PanelStyle::kWui, "attack_panel", 0, 0, UI::Box::Vertical),
     iplayer_(iplayer),
     target_coordinates_(target_coordinates),
     get_max_attackers_(get_max_attackers),
     attack_type_(attack_type),
     lastupdate_(0),

     linebox_(this, UI::PanelStyle::kWui, "line_box", 0, 0, UI::Box::Horizontal),
     columnbox_(&linebox_, UI::PanelStyle::kWui, "column_box", 0, 0, UI::Box::Vertical) {

	const std::vector<Widelands::Bob*> all_attackers = get_max_attackers_();

	init_slider(all_attackers, can_attack);
	init_soldier_lists(all_attackers);

	if (can_attack) {
		attack_button_->sigclicked.connect([this]() { act_attack(); });
	}
}

AttackWindow::~AttackWindow() {
	living_attack_windows_.erase(serial_);
}

std::vector<Widelands::Bob*> AttackWindow::get_max_attackers() {
	MutexLock m(MutexLock::ID::kObjects);
	const Widelands::EditorGameBase& egbase = iplayer_.egbase();
	Widelands::Building* building = get_building();
	Widelands::Ship* ship = get_ship();
	if ((building == nullptr && ship == nullptr) && !is_naval_invasion()) {
		die();  // The target object no longer exists.
		return {};
	}
	std::vector<Widelands::Bob*> result_vector;

	// First try land-based attack
	if (building != nullptr) {
		bool sees = false;
		for (Widelands::Coords& coords : building->get_positions(egbase)) {
			if (iplayer_.player().is_seeing(map_.get_index(coords))) {
				// TODO(Nordfriese): This method decides by itself which soldier remains in the
				// building. This soldier will not show up in the result vector. Perhaps we should show
				// all available soldiers, grouped by building, so the player can choose between all
				// soldiers knowing that at least one of each group will have to stay at home. However,
				// this could clutter up the screen a lot. Especially if you have many small buildings.
				std::vector<Widelands::Soldier*> soldiers;
				iplayer_.get_player()->find_attack_soldiers(building->base_flag(), &soldiers);
				result_vector.insert(result_vector.end(), soldiers.begin(), soldiers.end());
				sees = true;
				break;
			}
		}

		// Player can't see any part of the building, so it can't be attacked
		// This is the same check as done later on in send_player_enemyflagaction()
		if (!sees) {
			return result_vector;
		}
	}

	// Look for nearby warships
	for (Widelands::Serial ship_serial : iplayer_.player().ships()) {
		Widelands::Ship* warship =
		   dynamic_cast<Widelands::Ship*>(egbase.objects().get_object(ship_serial));
		assert(warship != nullptr);
		if (warship->can_attack()) {
			if (ship != nullptr) {  // Ship-to-ship combat
				if (warship->has_attack_target(ship)) {
					result_vector.push_back(warship);
				}
			} else {  // Ship-to-land invasion
				bool found = false;

				if (building != nullptr) {
					if (warship->sees_portspace(building->get_positions(egbase)[0])) {
						found = true;
						break;
					}
				}

				if (found) {
					std::vector<Widelands::Soldier*> onboard = warship->onboard_soldiers();
					result_vector.insert(result_vector.end(), onboard.begin(), onboard.end());
				}
			}
		}
	}

	return result_vector;
}

std::unique_ptr<UI::HorizontalSlider> AttackPanel::add_slider(UI::Box& parent,
                                                              uint32_t width,
                                                              uint32_t height,
                                                              uint32_t min,
                                                              uint32_t max,
                                                              uint32_t initial,
                                                              char const* hint) {
	std::unique_ptr<UI::HorizontalSlider> result(new UI::HorizontalSlider(
	   &parent, "slider", 0, 0, width, height, min, max, initial, UI::SliderStyle::kWuiLight, hint));
	parent.add(result.get());
	return result;
}

UI::Textarea& AttackPanel::add_text(UI::Box& parent,
                                    const std::string& str,
                                    UI::Align alignment,
                                    const UI::FontStyle style) {
	UI::Textarea& result =
	   *new UI::Textarea(&parent, UI::PanelStyle::kWui, "label", style, str, UI::Align::kLeft);
	parent.add(&result, UI::Box::Resizing::kAlign, alignment);
	return result;
}

template <typename ParentWidget, typename StringOrImage, typename Functor>
UI::Button* add_button(ParentWidget* widget,
                       UI::Box& parent,
                       const std::string& name,
                       const StringOrImage& text_or_image,
                       Functor functor,
                       UI::ButtonStyle style,
                       const std::string& tooltip_text) {
	UI::Button* button =
	   new UI::Button(&parent, name, 8, 8, 34, 34, style, text_or_image, tooltip_text);
	button->sigclicked.connect([widget, functor]() { (widget->*functor)(); });
	parent.add(button);
	return button;
}

/*
 * Update available soldiers
 */
void AttackPanel::think() {
	if ((iplayer_.egbase().get_gametime() - lastupdate_) > kUpdateTimeInGametimeMs) {
		update(false);
	}

	UI::Box::think();
}

void AttackWindow::think() {
	if (Widelands::Ship* ship = get_ship(); ship != nullptr) {
		target_coordinates_ = ship->get_position();
	}

	if (!iplayer_.player().is_seeing(map_.get_index(target_coordinates_)) &&
	    !iplayer_.player().see_all()) {
		die();  // The target object can no longer be seen.
	}

	UI::UniqueWindow::think();
}

static inline std::string slider_heading(const uint32_t num_attackers, const bool ship) {
	return format(
	   ship ?
         /** TRANSLATORS: Number of ships that should attack. Used in the attack window. */
         ngettext("%u ship", "%u ships", num_attackers) :
         /** TRANSLATORS: Number of soldiers that should attack. Used in the attack window. */
         ngettext("%u soldier", "%u soldiers", num_attackers),
	   num_attackers);
}

void AttackPanel::update(bool action_on_panel) {
	MutexLock m(MutexLock::ID::kObjects);

	lastupdate_ = iplayer_.egbase().get_gametime();

	assert(soldiers_slider_.get());
	assert(soldiers_text_.get());
	assert(less_soldiers_.get());
	assert(more_soldiers_.get());
	assert(attacking_soldiers_.get());
	assert(remaining_soldiers_.get());

	std::vector<Widelands::Bob*> all_attackers = get_max_attackers_();
	const int max_attackers = all_attackers.size();

	// Update number of available soldiers
	if (soldiers_slider_->get_max_value() != max_attackers) {
		soldiers_slider_->set_max_value(max_attackers);
	}

	// Add new soldiers and remove missing soldiers to/from the list
	for (const auto& s : all_attackers) {
		if (!attacking_soldiers_->contains(s) && !remaining_soldiers_->contains(s)) {
			remaining_soldiers_->add(s);
		}
	}
	for (const auto& s : remaining_soldiers_->get_soldiers()) {
		if (std::find(all_attackers.begin(), all_attackers.end(), s) == all_attackers.end()) {
			remaining_soldiers_->remove(s);
		}
	}
	for (const auto& s : attacking_soldiers_->get_soldiers()) {
		if (std::find(all_attackers.begin(), all_attackers.end(), s) == all_attackers.end()) {
			attacking_soldiers_->remove(s);
		}
	}

	if (action_on_panel) {
		// The player clicked on soldiers in the list – update slider
		soldiers_slider_->set_value(attacking_soldiers_->count_soldiers());
	} else {
		// The slider was moved or we were called from think() – shift lacking/extra soldiers between
		// the lists
		const int32_t lacking = soldiers_slider_->get_value() - attacking_soldiers_->count_soldiers();
		if (lacking > 0) {
			for (int32_t i = 0; i < lacking; ++i) {
				const Widelands::Bob* s = remaining_soldiers_->get_soldier();
				remaining_soldiers_->remove(s);
				attacking_soldiers_->add(s);
			}
		} else if (lacking < 0) {
			for (int32_t i = 0; i > lacking; --i) {
				const Widelands::Bob* s = attacking_soldiers_->get_soldier();
				attacking_soldiers_->remove(s);
				remaining_soldiers_->add(s);
			}
		}
	}

	// Update slider, buttons and texts
	soldiers_slider_->set_enabled(max_attackers > 0);
	more_soldiers_->set_enabled(max_attackers > soldiers_slider_->get_value());
	less_soldiers_->set_enabled(soldiers_slider_->get_value() > 0);

	soldiers_text_->set_text(
	   slider_heading(soldiers_slider_->get_value(), attack_type_ == AttackType::kShip));

	more_soldiers_->set_title(std::to_string(max_attackers));
}

void AttackPanel::init_slider(const std::vector<Widelands::Bob*>& all_attackers, bool can_attack) {
	const size_t max_attackers = all_attackers.size();

	soldiers_text_.reset(&add_text(
	   columnbox_, slider_heading(max_attackers > 0 ? 1 : 0, attack_type_ == AttackType::kShip),
	   UI::Align::kCenter, UI::FontStyle::kWuiAttackBoxSliderLabel));
	soldiers_slider_ = add_slider(
	   columnbox_, 210, 17, 0, max_attackers, max_attackers > 0 ? 1 : 0,
	   attack_type_ == AttackType::kShip ? _("Number of ships") : _("Number of soldiers"));
	soldiers_slider_->changed.connect([this]() { update(false); });

	add(&linebox_, UI::Box::Resizing::kFullSize);
	less_soldiers_.reset(add_button(this, linebox_, "less", "0", &AttackPanel::send_less_soldiers,
	                                UI::ButtonStyle::kWuiSecondary,
	                                attack_type_ == AttackType::kShip ?
                                      _("Send less ships. Hold down Ctrl to send no ships") :
                                      _("Send less soldiers. Hold down Ctrl to send no soldiers")));
	linebox_.add(&columnbox_);
	more_soldiers_.reset(
	   add_button(this, linebox_, "more", std::to_string(max_attackers),
	              &AttackPanel::send_more_soldiers, UI::ButtonStyle::kWuiSecondary,
	              attack_type_ == AttackType::kShip ?
                    _("Send more ships. Hold down Ctrl to send as many ships as possible") :
                    _("Send more soldiers. Hold down Ctrl to send as many soldiers as possible")));
	linebox_.add_space(kSpacing);
	if (can_attack) {
		attack_button_.reset(add_button(
		   this, linebox_, "attack", g_image_cache->get("images/wui/buildings/menu_attack.png"),
		   &AttackPanel::act_attack, UI::ButtonStyle::kWuiPrimary, _("Start attack")));
		linebox_.add(attack_button_.get());
	}

	soldiers_slider_->set_enabled(max_attackers > 0);
	more_soldiers_->set_enabled(max_attackers > 0);
}

void AttackPanel::init_soldier_lists(const std::vector<Widelands::Bob*>& all_attackers) {
	attacking_soldiers_.reset(new ListOfSoldiers(this, this, 0, 0, 30, 30));
	remaining_soldiers_.reset(new ListOfSoldiers(this, this, 0, 0, 30, 30));
	attacking_soldiers_->set_complement(remaining_soldiers_.get());
	remaining_soldiers_->set_complement(attacking_soldiers_.get());
	for (const auto& s : all_attackers) {
		remaining_soldiers_->add(s);
	}

	const std::string tooltip_format("<p>%s%s%s</p>");
	{
		UI::Textarea& txt =
		   add_text(*this, _("Attackers:"), UI::Align::kLeft, UI::FontStyle::kWuiLabel);
		// Needed so we can get tooltips
		txt.set_handle_mouse(true);
		txt.set_tooltip(format(
		   tooltip_format,
		   g_style_manager->font_style(UI::FontStyle::kWuiTooltipHeader)
		      .as_font_tag(attack_type_ == AttackType::kShip ?
                            _("Click on a ship to remove her from the list of attackers") :
                            _("Click on a soldier to remove him from the list of attackers")),
		   as_listitem(attack_type_ == AttackType::kShip ?
                        _("Hold down Ctrl to remove all ships from the list") :
                        _("Hold down Ctrl to remove all soldiers from the list"),
		               UI::FontStyle::kWuiTooltip),
		   as_listitem(
		      attack_type_ == AttackType::kShip ?
               _("Hold down Shift to remove all ships up to the one you’re pointing at") :
               _("Hold down Shift to remove all soldiers up to the one you’re pointing at"),
		      UI::FontStyle::kWuiTooltip)));
		add(attacking_soldiers_.get(), UI::Box::Resizing::kFullSize);
	}

	{
		UI::Textarea& txt =
		   add_text(*this, _("Not attacking:"), UI::Align::kLeft, UI::FontStyle::kWuiLabel);
		txt.set_handle_mouse(true);
		txt.set_tooltip(format(
		   tooltip_format,
		   g_style_manager->font_style(UI::FontStyle::kWuiTooltipHeader)
		      .as_font_tag(attack_type_ == AttackType::kShip ?
                            _("Click on a ship to add her to the list of attackers") :
                            _("Click on a soldier to add him to the list of attackers")),
		   as_listitem(attack_type_ == AttackType::kShip ?
                        _("Hold down Ctrl to add all ships to the list") :
                        _("Hold down Ctrl to add all soldiers to the list"),
		               UI::FontStyle::kWuiTooltip),
		   as_listitem(attack_type_ == AttackType::kShip ?
                        _("Hold down Shift to add all ships up to the one you’re pointing at") :
                        _("Hold down Shift to add all soldiers up to the one you’re pointing at"),
		               UI::FontStyle::kWuiTooltip)));
		add(remaining_soldiers_.get(), UI::Box::Resizing::kFullSize);
	}
}

void AttackWindow::init_bottombox() {
	MutexLock m(MutexLock::ID::kObjects);
	Widelands::Building* building = get_building();
	Widelands::Ship* ship = get_ship();
	if ((building == nullptr && ship == nullptr) && !is_naval_invasion()) {
		die();  // The target object no longer exists.
		return;
	}

	if (building != nullptr && building->descr().type() == Widelands::MapObjectType::MILITARYSITE) {
		do_not_conquer_.reset(new UI::Checkbox(
		   &bottombox_, UI::PanelStyle::kWui, "do_not_conquer", Vector2i(0, 0), _("Destroy target"),
		   _("Destroy the target building instead of conquering it")));
		do_not_conquer_->set_state(!dynamic_cast<const Widelands::MilitarySite*>(building)
		                               ->attack_target()
		                               ->get_allow_conquer(iplayer_.player_number()));
		bottombox_.add(do_not_conquer_.get(), UI::Box::Resizing::kAlign, UI::Align::kBottom);
	}
	bottombox_.add_inf_space();

	if (iplayer_.get_display_flag(InteractiveBase::dfDebug)) {
		add_button(this, bottombox_, "debug",
		           g_image_cache->get("images/wui/fieldaction/menu_debug.png"),
		           &AttackWindow::act_debug, UI::ButtonStyle::kWuiMenu, _("Show Debug Window"));
		bottombox_.add_space(kSpacing);
	}
	add_button(this, bottombox_, "goto", g_image_cache->get("images/wui/menus/goto.png"),
	           &AttackWindow::act_goto, UI::ButtonStyle::kWuiMenu, _("Center view on this"));

	mainbox_.add(&bottombox_, UI::Box::Resizing::kFullSize);
}

/** The attack button was pressed. */
void AttackWindow::act_attack() {
	MutexLock m(MutexLock::ID::kObjects);

	if (is_naval_invasion()) {
		std::map<Widelands::OPtr<Widelands::Ship>, std::vector<uint32_t>> soldiers_on_warships;

		for (Widelands::Serial serial : attack_panel_.soldiers()) {
			if (Widelands::Soldier* temp_attacker =
			       dynamic_cast<Widelands::Soldier*>(iplayer_.egbase().objects().get_object(serial));
			    temp_attacker != nullptr) {
				if (Widelands::Ship* ship = dynamic_cast<Widelands::Ship*>(
				       iplayer_.egbase().objects().get_object(temp_attacker->get_ship_serial()));
				    ship != nullptr) {
					std::vector<uint32_t>& parameters_vector = soldiers_on_warships[ship];
					if (parameters_vector.empty()) {
						parameters_vector.push_back(target_coordinates_.x);
						parameters_vector.push_back(target_coordinates_.y);
					}
					parameters_vector.push_back(serial);
				}
			}
		}

		for (auto& pair : soldiers_on_warships) {
			iplayer_.game().send_player_warship_command(
			   *pair.first.get(iplayer_.egbase()), Widelands::WarshipCommand::kAttack, pair.second);
		}

		iplayer_.map_view()->mouse_to_field(target_coordinates_, MapView::Transition::Jump);
	} else if (Widelands::Building* building = get_building(); building != nullptr) {
		iplayer_.game().send_player_enemyflagaction(building->base_flag(), iplayer_.player_number(),
		                                            attack_panel_.soldiers(), get_allow_conquer());
		iplayer_.map_view()->mouse_to_field(building->get_position(), MapView::Transition::Jump);
	} else if (Widelands::Ship* ship = get_ship(); ship != nullptr) {
		for (Widelands::Serial serial : attack_panel_.soldiers()) {
			if (Widelands::Ship* warship =
			       dynamic_cast<Widelands::Ship*>(iplayer_.egbase().objects().get_object(serial));
			    warship != nullptr) {
				iplayer_.game().send_player_warship_command(
				   *warship, Widelands::WarshipCommand::kAttack, {ship->serial()});
			}
		}
		iplayer_.map_view()->mouse_to_field(ship->get_position(), MapView::Transition::Jump);
	}
	die();
}

/** Center the player's view on the building. Callback function for the corresponding button. */
void AttackWindow::act_goto() {
	iplayer_.map_view()->scroll_to_field(target_coordinates_, MapView::Transition::Smooth);
}

/** Callback for debug window. */
void AttackWindow::act_debug() {
	show_field_debug(iplayer_, map_.get_fcoords(target_coordinates_));
}

void AttackPanel::send_less_soldiers() {
	assert(soldiers_slider_.get());
	soldiers_slider_->set_value(
	   (SDL_GetModState() & KMOD_CTRL) != 0 ? 0 : soldiers_slider_->get_value() - 1);
}

void AttackPanel::send_more_soldiers() {
	soldiers_slider_->set_value((SDL_GetModState() & KMOD_CTRL) != 0 ?
                                  soldiers_slider_->get_max_value() :
                                  soldiers_slider_->get_value() + 1);
}

size_t AttackPanel::count_soldiers() const {
	return attacking_soldiers_->count_soldiers();
}

std::vector<Widelands::Serial> AttackPanel::soldiers() const {
	std::vector<Widelands::Serial> result;
	for (const auto& s : attacking_soldiers_->get_soldiers()) {
		result.push_back(s->serial());
	}
	return result;
}

constexpr int kSoldierIconWidth = 32;
constexpr int kSoldierIconHeight = 30;

AttackPanel::ListOfSoldiers::ListOfSoldiers(UI::Panel* const parent,
                                            AttackPanel* parent_box,
                                            int32_t const x,
                                            int32_t const y,
                                            int const w,
                                            int const h,
                                            bool restrict_rows)
   : UI::Panel(parent, UI::PanelStyle::kWui, "list_of_soldiers", x, y, w, h),
     restricted_row_number_(restrict_rows),
     attack_box_(parent_box) {
	update_desired_size();
}

bool AttackPanel::ListOfSoldiers::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn != SDL_BUTTON_LEFT || (other_ == nullptr)) {
		return UI::Panel::handle_mousepress(btn, x, y);
	}
	if ((SDL_GetModState() & KMOD_CTRL) != 0) {
		for (const auto& s : get_soldiers()) {
			remove(s);
			other_->add(s);
		}
	} else {
		const Widelands::Bob* soldier = soldier_at(x, y);
		if (soldier == nullptr) {
			return UI::Panel::handle_mousepress(btn, x, y);
		}
		if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
			for (const auto& s : get_soldiers()) {
				remove(s);
				other_->add(s);
				if (s == soldier) {
					break;
				}
			}
		} else {
			remove(soldier);
			other_->add(soldier);
		}
	}
	attack_box_->update(true);
	return true;
}

void AttackPanel::ListOfSoldiers::handle_mousein(bool /*inside*/) {
	set_tooltip(std::string());
}

bool AttackPanel::ListOfSoldiers::handle_mousemove(
   uint8_t /*state*/, int32_t x, int32_t y, int32_t /*xdiff*/, int32_t /*ydiff*/) {
	if (const Widelands::Bob* mo = soldier_at(x, y); mo != nullptr) {
		if (mo->descr().type() == Widelands::MapObjectType::SHIP) {
			upcast(const Widelands::Ship, ship, mo);
			set_tooltip(format(_("%1$s  HP: %2$u/%3$u  AT: +%4$u%%"), ship->get_shipname(),
			                   ship->get_hitpoints(), ship->descr().max_hitpoints_,
			                   ship->get_sea_attack_soldier_bonus(attack_box_->iplayer_.egbase())));
		} else {
			upcast(const Widelands::Soldier, soldier, mo);
			set_tooltip(format(_("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u"),
			                   soldier->get_health_level(), soldier->descr().get_max_health_level(),
			                   soldier->get_attack_level(), soldier->descr().get_max_attack_level(),
			                   soldier->get_defense_level(), soldier->descr().get_max_defense_level(),
			                   soldier->get_evade_level(), soldier->descr().get_max_evade_level()));
		}
	} else {
		set_tooltip(std::string());
	}
	return true;
}

// whole window
bool AttackPanel::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	return soldiers_slider_->handle_mousewheel(x, y, modstate);
}

Widelands::Extent AttackPanel::ListOfSoldiers::size() const {
	const size_t nr_soldiers = count_soldiers();
	size_t rows = nr_soldiers / current_size_;
	if (nr_soldiers == 0 || rows * current_size_ < nr_soldiers) {
		++rows;
	}
	if (restricted_row_number_) {
		return Widelands::Extent(rows, current_size_);
	}
	return Widelands::Extent(current_size_, rows);
}

void AttackPanel::ListOfSoldiers::update_desired_size() {
	current_size_ = std::max(
	   1, restricted_row_number_ ? get_h() / kSoldierIconHeight : get_w() / kSoldierIconWidth);
	const Widelands::Extent e = size();
	set_desired_size(e.w * kSoldierIconWidth, e.h * kSoldierIconHeight);
}

const Widelands::Bob* AttackPanel::ListOfSoldiers::soldier_at(int32_t x, int32_t y) const {
	if (x < 0 || y < 0 || soldiers_.empty()) {
		return nullptr;
	}
	const int32_t col = x / kSoldierIconWidth;
	const int32_t row = y / kSoldierIconHeight;
	assert(col >= 0);
	assert(row >= 0);
	if ((restricted_row_number_ ? row : col) >= current_size_) {
		return nullptr;
	}
	const int index = restricted_row_number_ ? current_size_ * col + row : current_size_ * row + col;
	assert(index >= 0);
	return static_cast<unsigned int>(index) < soldiers_.size() ? soldiers_[index] : nullptr;
}

void AttackPanel::ListOfSoldiers::add(const Widelands::Bob* s) {
	soldiers_.push_back(s);
	sort();
	update_desired_size();
}

void AttackPanel::ListOfSoldiers::remove(const Widelands::Bob* s) {
	const auto it = std::find(soldiers_.begin(), soldiers_.end(), s);
	assert(it != soldiers_.end());
	soldiers_.erase(it);
	sort();
	update_desired_size();
}

void AttackPanel::ListOfSoldiers::sort() {
	std::sort(
	   soldiers_.begin(), soldiers_.end(), [this](const Widelands::Bob* a, const Widelands::Bob* b) {
		   const Widelands::Map& map = attack_box_->iplayer_.egbase().map();
		   return map.calc_distance(a->get_position(), *attack_box_->target_coordinates_) <
		          map.calc_distance(b->get_position(), *attack_box_->target_coordinates_);
	   });
}

void AttackPanel::ListOfSoldiers::draw(RenderTarget& dst) {
	const size_t nr_soldiers = soldiers_.size();
	int32_t column = 0;
	int32_t row = 0;
	for (uint32_t i = 0; i < nr_soldiers; ++i) {
		Vector2i location(column * kSoldierIconWidth, row * kSoldierIconHeight);

		if (soldiers_[i]->descr().type() == Widelands::MapObjectType::SHIP) {
			upcast(const Widelands::Ship, ship, soldiers_[i]);

			ship->draw_healthbar(attack_box_->iplayer_.egbase(), &dst, location.cast<float>(), 1.f);

		} else {
			upcast(const Widelands::Soldier, soldier, soldiers_[i]);

			dst.fill_rect(Recti(location, kSoldierIconWidth, kSoldierIconHeight),
			              get_soldier_color(soldier), BlendMode::Default);

			soldier->draw_info_icon(location, 1.0f, Widelands::Soldier::InfoMode::kInBuilding,
			                        InfoToDraw::kSoldierLevels, &dst);

			if (soldier->is_shipping()) {
				constexpr float kOffset = 0.35f;
				constexpr float kSize = 0.5f;
				constexpr float kAlpha = 0.9f;
				const Image* anchor = g_image_cache->get("images/wui/overlays/port_hint.png");
				dst.blitrect_scale(
				   Rectf((column + kOffset) * kSoldierIconWidth, (row + kOffset) * kSoldierIconHeight,
				         kSoldierIconWidth * kSize, kSoldierIconHeight * kSize),
				   anchor, Recti(0, 0, anchor->width(), anchor->height()), kAlpha, BlendMode::Default);
			}
		}

		if (restricted_row_number_) {
			++row;
			if (row >= current_size_) {
				row = 0;
				++column;
			}
		} else {
			++column;
			if (column >= current_size_) {
				column = 0;
				++row;
			}
		}
	}
}

/* Changelog:
 * 1: v1.1
 * 2: Use map object serial as well as coords
 */
constexpr uint16_t kCurrentPacketVersion = 2;
UI::Window& AttackWindow::load(FileRead& fr, InteractiveBase& ib, Widelands::MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersion) {
			const int32_t x = fr.signed_32();
			const int32_t y = fr.signed_32();
			const Widelands::Serial mo_serial = packet_version >= 2 ? fr.unsigned_32() : 0;
			AttackWindow* a =
			   dynamic_cast<AttackWindow*>(dynamic_cast<InteractivePlayer&>(ib).show_attack_window(
			      Widelands::Coords(x, y),
			      mo_serial == 0 ? nullptr : &mol.get<Widelands::MapObject>(mo_serial), false));
			assert(a != nullptr);

			const uint8_t destroy = fr.unsigned_8();
			if (a->do_not_conquer_ != nullptr) {
				a->do_not_conquer_->set_state(destroy == 0);
			}

			for (const Widelands::Bob* s : a->attack_panel_.attacking_soldiers_->get_soldiers()) {
				a->attack_panel_.attacking_soldiers_->remove(s);
				a->attack_panel_.remaining_soldiers_->add(s);
			}
			for (size_t i = fr.unsigned_32(); i != 0u; --i) {
				const Widelands::Bob* s = &mol.get<Widelands::Bob>(fr.unsigned_32());
				if (a->attack_panel_.remaining_soldiers_->contains(s)) {
					a->attack_panel_.remaining_soldiers_->remove(s);
					a->attack_panel_.attacking_soldiers_->add(s);
				}
				/* Since the attack window only updates a soldier list every 500 ms, it is
				 * possible for the saved list of soldiers to send to contain a soldier
				 * who is no longer available. Skip this situation silently.
				 */
			}

			return *a;
		}
		throw Widelands::UnhandledVersionError(
		   "Attack Window", packet_version, kCurrentPacketVersion);

	} catch (const WException& e) {
		throw Widelands::GameDataError("attack window: %s", e.what());
	}
}
void AttackWindow::save(FileWrite& fw, Widelands::MapObjectSaver& mos) const {
	fw.unsigned_16(kCurrentPacketVersion);
	fw.signed_32(target_coordinates_.x);
	fw.signed_32(target_coordinates_.y);
	fw.unsigned_32(
	   mos.get_object_file_index_or_zero(target_building_or_ship_.get(iplayer_.egbase())));
	// is_naval_invasion_ will be set automatically by the constructor on loading

	fw.unsigned_8(do_not_conquer_ && !do_not_conquer_->get_state() ? 1 : 0);

	fw.unsigned_32(attack_panel_.attacking_soldiers_->get_soldiers().size());
	for (const Widelands::Bob* s : attack_panel_.attacking_soldiers_->get_soldiers()) {
		fw.unsigned_32(mos.get_object_file_index(*s));
	}
}
