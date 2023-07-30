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

#include "wui/soldierlist.h"

#include <functional>

#include <SDL_mouse.h>
#include <SDL_timer.h>

#include "base/macros.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/textarea.h"
#include "wui/interactive_base.h"
#include "wui/soldiercapacitycontrol.h"

namespace {

constexpr uint32_t kMaxColumns = 6;
constexpr uint32_t kAnimateSpeed = 300;  ///< in pixels per second
constexpr int kIconBorder = 2;

}  // namespace

/**
 * Iconic representation of soldiers, including their levels and current health.
 */
struct SoldierPanel : UI::Panel {
	using SoldierFn = std::function<void(const Widelands::Soldier*)>;

	SoldierPanel(UI::Panel& parent,
	             Widelands::EditorGameBase& egbase,
	             Widelands::MapObject& building_or_ship);

	Widelands::EditorGameBase& egbase() const {
		return egbase_;
	}

	void think() override;
	void draw(RenderTarget& /*dst*/) override;

	void set_mouseover(const SoldierFn& fn);
	void set_click(const SoldierFn& fn);

	const Widelands::Ship* get_ship() const {
		assert(is_ship_);
		return dynamic_cast<const Widelands::Ship*>(building_or_ship_.get(egbase_));
	}
	Widelands::Ship* get_ship() {
		assert(is_ship_);
		return dynamic_cast<Widelands::Ship*>(building_or_ship_.get(egbase_));
	}
	const Widelands::Building* get_building() const {
		assert(!is_ship_);
		return dynamic_cast<const Widelands::Building*>(building_or_ship_.get(egbase_));
	}
	Widelands::Building* get_building() {
		assert(!is_ship_);
		return dynamic_cast<Widelands::Building*>(building_or_ship_.get(egbase_));
	}

	uint32_t get_max_capacity() const {
		if (is_ship_) {
			return get_ship()->get_capacity();
		}
		return get_building()->soldier_control()->max_soldier_capacity();
	}
	uint32_t get_cur_capacity() const {
		if (is_ship_) {
			return get_ship()->get_warship_soldier_capacity();
		}
		return get_building()->soldier_control()->soldier_capacity();
	}
	std::vector<Widelands::Soldier*> associated_soldiers() const {
		if (is_ship_) {
			return get_ship()->associated_soldiers();
		}
		return get_building()->soldier_control()->associated_soldiers();
	}

protected:
	void handle_mousein(bool inside) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;

private:
	Vector2i calc_pos(uint32_t row, uint32_t col) const;
	const Widelands::Soldier* find_soldier(int32_t x, int32_t y) const;

	struct Icon {
		Widelands::OPtr<Widelands::Soldier> soldier;
		uint32_t row;
		uint32_t col;
		Vector2i pos = Vector2i::zero();

		/**
		 * Keep track of how we last rendered this soldier,
		 * so that we can update when its status changes.
		 */
		/*@{*/
		uint32_t cache_level = 0;
		uint32_t cache_health = 0;
		bool cache_is_present = false;
		/*@}*/
	};

	Widelands::EditorGameBase& egbase_;
	Widelands::OPtr<Widelands::MapObject> building_or_ship_;
	bool is_ship_;

	SoldierFn mouseover_fn_;
	SoldierFn click_fn_;

	std::vector<Icon> icons_;

	uint32_t rows_;
	uint32_t cols_;

	int icon_width_;
	int icon_height_;

	int32_t last_animate_time_{0};
};

SoldierPanel::SoldierPanel(UI::Panel& parent,
                           Widelands::EditorGameBase& gegbase,
                           Widelands::MapObject& building_or_ship)
   : Panel(&parent, UI::PanelStyle::kWui, "soldier_panel", 0, 0, 0, 0),
     egbase_(gegbase),
     building_or_ship_(&building_or_ship),
     is_ship_(building_or_ship.descr().type() == Widelands::MapObjectType::SHIP) {
	Widelands::Soldier::calc_info_icon_size(
	   building_or_ship.owner().tribe(), icon_width_, icon_height_);
	icon_width_ += 2 * kIconBorder;
	icon_height_ += 2 * kIconBorder;

	if (building_or_ship.descr().type() == Widelands::MapObjectType::WAREHOUSE) {
		rows_ = 0;
		cols_ = 0;
		set_visible(false);
		set_thinks(false);
		return;
	}

	/* The +1 is because up to 1 additional soldier may be coming
	 * to the building for the hero/rookie exchange. */
	const Widelands::Quantity maxcapacity = get_max_capacity() + 1;
	if (maxcapacity <= kMaxColumns) {
		cols_ = maxcapacity;
		rows_ = 1;
	} else {
		cols_ = kMaxColumns;
		rows_ = (maxcapacity + cols_ - 1) / cols_;
	}

	set_size(cols_ * icon_width_, rows_ * icon_height_);
	set_desired_size(cols_ * icon_width_, rows_ * icon_height_);
	set_thinks(true);

	// Initialize the icons
	uint32_t row = 0;
	uint32_t col = 0;
	Widelands::Building* building = is_ship_ ? nullptr : get_building();
	std::vector<Widelands::Soldier*> onboard_soldiers;
	if (is_ship_) {
		onboard_soldiers = get_ship()->onboard_soldiers();
	}
	for (Widelands::Soldier* soldier : associated_soldiers()) {
		Icon icon;
		icon.soldier = soldier;
		icon.row = row;
		icon.col = col;
		icon.pos = calc_pos(row, col);
		icon.cache_health = 0;
		icon.cache_level = 0;
		icon.cache_is_present = is_ship_ ?
                                 (std::find(onboard_soldiers.begin(), onboard_soldiers.end(),
		                                      soldier) != onboard_soldiers.end()) :
                                 building->is_present(*soldier);
		icons_.push_back(icon);

		if (++col >= cols_) {
			col = 0;
			row++;
		}
	}
}

/**
 * Set the callback function that indicates which soldier the mouse is over.
 */
void SoldierPanel::set_mouseover(const SoldierPanel::SoldierFn& fn) {
	mouseover_fn_ = fn;
}

/**
 * Set the callback function that is called when a soldier is clicked.
 */
void SoldierPanel::set_click(const SoldierPanel::SoldierFn& fn) {
	click_fn_ = fn;
}

void SoldierPanel::think() {
	Widelands::Building* building = is_ship_ ? nullptr : get_building();
	Widelands::Ship* ship = is_ship_ ? get_ship() : nullptr;
	if (building == nullptr && ship == nullptr) {
		return;
	}

	bool changes = false;

	// Update soldier list and target row/col:
	std::vector<Widelands::Soldier*> soldierlist = associated_soldiers();
	uint32_t capacity = std::max<uint32_t>(get_cur_capacity(), soldierlist.size());
	std::vector<uint32_t> row_occupancy;
	row_occupancy.resize(rows_);

	// First pass: check whether existing icons are still valid, and compact them
	for (uint32_t idx = 0; idx < icons_.size(); ++idx) {
		Icon& icon = icons_[idx];
		Widelands::Soldier* soldier = icon.soldier.get(egbase());
		if (soldier != nullptr) {
			std::vector<Widelands::Soldier*>::iterator it =
			   std::find(soldierlist.begin(), soldierlist.end(), soldier);
			if (it != soldierlist.end()) {
				soldierlist.erase(it);
			} else {
				soldier = nullptr;
			}
		}

		if (soldier == nullptr) {
			icons_.erase(icons_.begin() + idx);
			idx--;
			changes = true;
			continue;
		}

		while ((icon.row != 0u) && (row_occupancy[icon.row] >= kMaxColumns ||
		                            icon.row * kMaxColumns + row_occupancy[icon.row] >= capacity)) {
			icon.row--;
		}

		icon.col = row_occupancy[icon.row]++;
	}

	// Second pass: add new soldiers
	while (!soldierlist.empty()) {
		Icon icon;
		icon.soldier = soldierlist.back();
		soldierlist.pop_back();
		icon.row = 0;
		while (row_occupancy[icon.row] >= kMaxColumns) {
			icon.row++;
		}
		icon.col = row_occupancy[icon.row]++;
		icon.pos = calc_pos(icon.row, icon.col);

		// Let soldiers slide in from the right border
		icon.pos.x = get_w();

		std::vector<Icon>::iterator insertpos = icons_.begin();

		for (std::vector<Icon>::iterator icon_iter = icons_.begin(); icon_iter != icons_.end();
		     ++icon_iter) {

			if (icon_iter->row <= icon.row) {
				insertpos = icon_iter + 1;
			}

			icon.pos.x = std::max<int32_t>(icon.pos.x, icon_iter->pos.x + icon_width_);
		}

		icons_.insert(insertpos, icon);
		changes = true;
	}

	// Third pass: animate icons
	int32_t curtime = SDL_GetTicks();
	int32_t dt = std::min(std::max(curtime - last_animate_time_, 0), 1000);
	int32_t maxdist = dt * kAnimateSpeed / 1000;
	last_animate_time_ = curtime;

	std::vector<Widelands::Soldier*> onboard_soldiers;
	if (is_ship_) {
		onboard_soldiers = get_ship()->onboard_soldiers();
	}
	for (Icon& icon : icons_) {
		Vector2i goal = calc_pos(icon.row, icon.col);
		Vector2i dp = goal - icon.pos;

		dp.x = std::min(std::max(dp.x, -maxdist), maxdist);
		dp.y = std::min(std::max(dp.y, -maxdist), maxdist);

		if (dp.x != 0 || dp.y != 0) {
			changes = true;
		}

		icon.pos += dp;

		// Check whether health and/or level of the soldier has changed
		Widelands::Soldier* soldier = icon.soldier.get(egbase());
		uint32_t level = soldier->get_attack_level();
		level = level * (soldier->descr().get_max_defense_level() + 1) + soldier->get_defense_level();
		level = level * (soldier->descr().get_max_evade_level() + 1) + soldier->get_evade_level();
		level = level * (soldier->descr().get_max_health_level() + 1) + soldier->get_health_level();

		uint32_t health = soldier->get_current_health();
		bool present = is_ship_ ? (std::find(onboard_soldiers.begin(), onboard_soldiers.end(),
		                                     soldier) != onboard_soldiers.end()) :
                                building->is_present(*soldier);

		if (health != icon.cache_health || level != icon.cache_level ||
		    present != icon.cache_is_present) {
			icon.cache_level = level;
			icon.cache_health = health;
			icon.cache_is_present = present;
			changes = true;
		}
	}

	if (changes) {
		Vector2i mousepos = get_mouse_position();
		mouseover_fn_(find_soldier(mousepos.x, mousepos.y));
	}
}

void SoldierPanel::draw(RenderTarget& dst) {
	Widelands::Building* building = is_ship_ ? nullptr : get_building();
	Widelands::Ship* ship = is_ship_ ? get_ship() : nullptr;
	if (building == nullptr && ship == nullptr) {
		return;
	}

	// Fill a region matching the current site capacity with black
	uint32_t capacity = std::max<uint32_t>(get_cur_capacity(), associated_soldiers().size());
	uint32_t fullrows = capacity / kMaxColumns;

	if (fullrows != 0u) {
		dst.fill_rect(Recti(0, 0, get_w(), icon_height_ * fullrows), RGBAColor(0, 0, 0, 0));
	}
	if ((capacity % kMaxColumns) != 0u) {
		dst.fill_rect(
		   Recti(0, icon_height_ * fullrows, icon_width_ * (capacity % kMaxColumns), icon_height_),
		   RGBAColor(0, 0, 0, 0));
	}

	// Draw icons
	for (const Icon& icon : icons_) {
		const Widelands::Soldier* soldier = icon.soldier.get(egbase());
		if (soldier == nullptr) {
			continue;
		}

		constexpr float kNoZoom = 1.f;
		soldier->draw_info_icon(icon.pos + Vector2i(kIconBorder, kIconBorder), kNoZoom,
		                        Widelands::Soldier::InfoMode::kInBuilding, InfoToDraw::kSoldierLevels,
		                        &dst);

		if (!icon.cache_is_present) {
			// Since the background is black, darkening the icon has the same effect
			// as drawing it with partial transparency but is easier to achieve.
			dst.brighten_rect(Recti(icon.pos, icon_width_, icon_height_), -128);
		}
	}
}

Vector2i SoldierPanel::calc_pos(uint32_t row, uint32_t col) const {
	return Vector2i(col * icon_width_, row * icon_height_);
}

/**
 * Return the soldier (if any) at the given coordinates.
 */
const Widelands::Soldier* SoldierPanel::find_soldier(int32_t x, int32_t y) const {
	for (const Icon& icon : icons_) {
		Recti r(icon.pos, icon_width_, icon_height_);
		if (r.contains(Vector2i(x, y))) {
			return icon.soldier.get(egbase());
		}
	}

	return nullptr;
}

void SoldierPanel::handle_mousein(bool inside) {
	if (!inside && mouseover_fn_) {
		mouseover_fn_(nullptr);
	}
}

bool SoldierPanel::handle_mousemove(
   uint8_t /* state */, int32_t x, int32_t y, int32_t /* xdiff */, int32_t /* ydiff */) {
	if (mouseover_fn_) {
		mouseover_fn_(find_soldier(x, y));
	}
	return true;
}

bool SoldierPanel::handle_mousepress(uint8_t btn, int32_t x, int32_t y) {
	if (btn == SDL_BUTTON_LEFT) {
		if (click_fn_) {
			if (const Widelands::Soldier* soldier = find_soldier(x, y)) {
				click_fn_(soldier);
			}
		}
		return true;
	}

	return false;
}

/**
 * List of soldiers \ref MilitarySiteWindow and \ref TrainingSiteWindow
 */
struct SoldierList : UI::Box {
	SoldierList(UI::Panel& parent, InteractiveBase& ib, Widelands::MapObject& building_or_ship);

protected:
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

private:
	void mouseover(const Widelands::Soldier* soldier);
	void eject(const Widelands::Soldier* soldier);
	void set_soldier_preference(int32_t changed_to);
	void think() override;
	bool can_eject() const;
	void unset_infotext();

	InteractiveBase& ibase_;
	Widelands::MapObject& building_or_ship_;
	SoldierPanel soldierpanel_;
	UI::Radiogroup soldier_preference_;

	UI::Panel* soldier_capacity_control_;
};

SoldierList::SoldierList(UI::Panel& parent,
                         InteractiveBase& ib,
                         Widelands::MapObject& building_or_ship)
   : UI::Box(&parent, UI::PanelStyle::kWui, "soldier_list", 0, 0, UI::Box::Vertical),

     ibase_(ib),
     building_or_ship_(building_or_ship),

     soldierpanel_(*this, ib.egbase(), building_or_ship) {
	upcast(Widelands::MilitarySite, ms, &building_or_ship);
	upcast(Widelands::Warehouse, wh, &building_or_ship);
	upcast(Widelands::Ship, ship, &building_or_ship);

	unset_infotext();
	add(&soldierpanel_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	soldierpanel_.set_mouseover([this](const Widelands::Soldier* s) { mouseover(s); });
	soldierpanel_.set_click([this](const Widelands::Soldier* s) { eject(s); });

	UI::Box* buttons =
	   new UI::Box(this, UI::PanelStyle::kWui, "buttons_box", 0, 0, UI::Box::Horizontal);

	bool can_act = ibase_.can_act(building_or_ship_.owner().player_number());
	if (building_or_ship.descr().type() != Widelands::MapObjectType::TRAININGSITE) {
		// Make sure the creation order is consistent with enum SoldierPreference!
		soldier_preference_.add_button(
		   buttons, UI::PanelStyle::kWui, "prefer_rookies", Vector2i::zero(),
		   g_image_cache->get("images/wui/buildings/prefer_rookies.png"), _("Prefer rookies"));
		soldier_preference_.add_button(
		   buttons, UI::PanelStyle::kWui, "prefer_heroes", Vector2i::zero(),
		   g_image_cache->get("images/wui/buildings/prefer_heroes.png"), _("Prefer heroes"));
		soldier_preference_.add_button(buttons, UI::PanelStyle::kWui, "prefer_any", Vector2i::zero(),
		                               g_image_cache->get("images/wui/buildings/prefer_any.png"),
		                               _("No preference"));
		UI::Radiobutton* button = soldier_preference_.get_first_button();
		while (button != nullptr) {
			buttons->add(button);
			button = button->next_button();
		}

		soldier_preference_.set_state(
		   static_cast<uint8_t>(ms != nullptr ? ms->get_soldier_preference() :
		                        wh != nullptr ? wh->get_soldier_preference() :
                                              ship->get_soldier_preference()),
		   false);
		if (can_act) {
			soldier_preference_.changedto.connect([this](int32_t a) { set_soldier_preference(a); });
		} else {
			soldier_preference_.set_enabled(false);
		}
	}
	buttons->add_inf_space();
	soldier_capacity_control_ = create_soldier_capacity_control(*buttons, ib, building_or_ship);
	buttons->add(soldier_capacity_control_);
	add(buttons, UI::Box::Resizing::kFullSize);
}

void SoldierList::think() {
	switch (building_or_ship_.descr().type()) {
	case Widelands::MapObjectType::MILITARYSITE: {
		upcast(Widelands::MilitarySite, ms, &building_or_ship_);
		soldier_preference_.set_state(static_cast<uint8_t>(ms->get_soldier_preference()), false);
		break;
	}
	case Widelands::MapObjectType::WAREHOUSE: {
		upcast(Widelands::Warehouse, wh, &building_or_ship_);
		soldier_preference_.set_state(static_cast<uint8_t>(wh->get_soldier_preference()), false);
		break;
	}
	case Widelands::MapObjectType::SHIP: {
		upcast(Widelands::Ship, ship, &building_or_ship_);
		soldier_preference_.set_state(static_cast<uint8_t>(ship->get_soldier_preference()), false);
		break;
	}
	case Widelands::MapObjectType::TRAININGSITE:
		break;
	default:
		NEVER_HERE();
	}
}

void SoldierList::mouseover(const Widelands::Soldier* soldier) {
	if (soldier == nullptr) {
		unset_infotext();
		return;
	}

	set_tooltip(format(_("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u"),
	                   soldier->get_health_level(), soldier->descr().get_max_health_level(),
	                   soldier->get_attack_level(), soldier->descr().get_max_attack_level(),
	                   soldier->get_defense_level(), soldier->descr().get_max_defense_level(),
	                   soldier->get_evade_level(), soldier->descr().get_max_evade_level()));
}

void SoldierList::unset_infotext() {
	set_tooltip(can_eject() ? _("Click soldier to send away") : "");
}

bool SoldierList::can_eject() const {
	if (!ibase_.can_act(building_or_ship_.owner().player_number())) {
		return false;
	}
	if (building_or_ship_.descr().type() == Widelands::MapObjectType::WAREHOUSE) {
		return false;
	}

	const bool is_ship = building_or_ship_.descr().type() == Widelands::MapObjectType::SHIP;
	const Widelands::Building* building = is_ship ? nullptr : soldierpanel_.get_building();
	const Widelands::Ship* ship = is_ship ? soldierpanel_.get_ship() : nullptr;

	uint32_t const capacity_min = is_ship ? ship->min_warship_soldier_capacity() :
                                           building->soldier_control()->min_soldier_capacity();
	bool over_min =
	   capacity_min < (is_ship ? ship->onboard_soldiers().size() :
                                building->soldier_control()->present_soldiers().size());

	return over_min;
}

void SoldierList::eject(const Widelands::Soldier* soldier) {
	if (can_eject()) {
		if (Widelands::Game* game = ibase_.get_game(); game != nullptr) {
			game->send_player_drop_soldier(building_or_ship_, soldier->serial());
		} else {
			NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
		}
	}
}

void SoldierList::set_soldier_preference(int32_t changed_to) {
	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_soldier_preference(
		   building_or_ship_, static_cast<Widelands::SoldierPreference>(changed_to));
	} else {
		NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
	}
}

bool SoldierList::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	return soldier_capacity_control_->handle_mousewheel(x, y, modstate);
}

UI::Panel* create_soldier_list(UI::Panel& parent,
                               InteractiveBase& ib,
                               Widelands::MapObject& building_or_ship) {
	return new SoldierList(parent, ib, building_or_ship);
}
