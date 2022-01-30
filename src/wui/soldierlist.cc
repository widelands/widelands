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

#include "wui/soldierlist.h"

#include <functional>

#include <SDL_mouse.h>
#include <SDL_timer.h>

#include "base/macros.h"
#include "graphic/font_handler.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "logic/map_objects/tribes/building.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/soldiercontrol.h"
#include "logic/player.h"
#include "ui_basic/box.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/textarea.h"
#include "wui/interactive_base.h"
#include "wui/soldiercapacitycontrol.h"

using Widelands::Soldier;
using Widelands::SoldierControl;

namespace {

constexpr uint32_t kMaxColumns = 6;
constexpr uint32_t kAnimateSpeed = 300;  ///< in pixels per second
constexpr int kIconBorder = 2;

}  // namespace

/**
 * Iconic representation of soldiers, including their levels and current health.
 */
struct SoldierPanel : UI::Panel {
	using SoldierFn = std::function<void(const Soldier*)>;

	SoldierPanel(UI::Panel& parent,
	             Widelands::EditorGameBase& egbase,
	             Widelands::Building& building);

	Widelands::EditorGameBase& egbase() const {
		return egbase_;
	}

	void think() override;
	void draw(RenderTarget&) override;

	void set_mouseover(const SoldierFn& fn);
	void set_click(const SoldierFn& fn);

protected:
	void handle_mousein(bool inside) override;
	bool
	handle_mousemove(uint8_t state, int32_t x, int32_t y, int32_t xdiff, int32_t ydiff) override;
	bool handle_mousepress(uint8_t btn, int32_t x, int32_t y) override;

private:
	Vector2i calc_pos(uint32_t row, uint32_t col) const;
	const Soldier* find_soldier(int32_t x, int32_t y) const;

	struct Icon {
		Widelands::OPtr<Soldier> soldier;
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
		/*@}*/
	};

	Widelands::EditorGameBase& egbase_;
	Widelands::OPtr<Widelands::Building> building_;

	SoldierFn mouseover_fn_;
	SoldierFn click_fn_;

	std::vector<Icon> icons_;

	uint32_t rows_;
	uint32_t cols_;

	int icon_width_;
	int icon_height_;

	int32_t last_animate_time_;
};

SoldierPanel::SoldierPanel(UI::Panel& parent,
                           Widelands::EditorGameBase& gegbase,
                           Widelands::Building& building)
   : Panel(&parent, UI::PanelStyle::kWui, 0, 0, 0, 0),
     egbase_(gegbase),
     building_(&building),
     last_animate_time_(0) {
	assert(building.soldier_control() != nullptr);
	Soldier::calc_info_icon_size(building.owner().tribe(), icon_width_, icon_height_);
	icon_width_ += 2 * kIconBorder;
	icon_height_ += 2 * kIconBorder;

	Widelands::Quantity maxcapacity = building.soldier_control()->max_soldier_capacity();
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
	for (Soldier* soldier : building.soldier_control()->present_soldiers()) {
		Icon icon;
		icon.soldier = soldier;
		icon.row = row;
		icon.col = col;
		icon.pos = calc_pos(row, col);
		icon.cache_health = 0;
		icon.cache_level = 0;
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
	Widelands::Building* b = building_.get(egbase_);
	if (!b) {
		return;
	}

	bool changes = false;
	uint32_t capacity = b->soldier_control()->soldier_capacity();

	// Update soldier list and target row/col:
	std::vector<Soldier*> soldierlist = b->soldier_control()->present_soldiers();
	std::vector<uint32_t> row_occupancy;
	row_occupancy.resize(rows_);

	// First pass: check whether existing icons are still valid, and compact them
	for (uint32_t idx = 0; idx < icons_.size(); ++idx) {
		Icon& icon = icons_[idx];
		Soldier* soldier = icon.soldier.get(egbase());
		if (soldier) {
			std::vector<Soldier*>::iterator it =
			   std::find(soldierlist.begin(), soldierlist.end(), soldier);
			if (it != soldierlist.end()) {
				soldierlist.erase(it);
			} else {
				soldier = nullptr;
			}
		}

		if (!soldier) {
			icons_.erase(icons_.begin() + idx);
			idx--;
			changes = true;
			continue;
		}

		while (icon.row && (row_occupancy[icon.row] >= kMaxColumns ||
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
		Soldier* soldier = icon.soldier.get(egbase());
		uint32_t level = soldier->get_attack_level();
		level = level * (soldier->descr().get_max_defense_level() + 1) + soldier->get_defense_level();
		level = level * (soldier->descr().get_max_evade_level() + 1) + soldier->get_evade_level();
		level = level * (soldier->descr().get_max_health_level() + 1) + soldier->get_health_level();

		uint32_t health = soldier->get_current_health();

		if (health != icon.cache_health || level != icon.cache_level) {
			icon.cache_level = level;
			icon.cache_health = health;
			changes = true;
		}
	}

	if (changes) {
		Vector2i mousepos = get_mouse_position();
		mouseover_fn_(find_soldier(mousepos.x, mousepos.y));
	}
}

void SoldierPanel::draw(RenderTarget& dst) {
	Widelands::Building* b = building_.get(egbase_);
	if (!b) {
		return;
	}

	// Fill a region matching the current site capacity with black
	uint32_t capacity = b->soldier_control()->soldier_capacity();
	uint32_t fullrows = capacity / kMaxColumns;

	if (fullrows) {
		dst.fill_rect(Recti(0, 0, get_w(), icon_height_ * fullrows), RGBAColor(0, 0, 0, 0));
	}
	if (capacity % kMaxColumns) {
		dst.fill_rect(
		   Recti(0, icon_height_ * fullrows, icon_width_ * (capacity % kMaxColumns), icon_height_),
		   RGBAColor(0, 0, 0, 0));
	}

	// Draw icons
	for (const Icon& icon : icons_) {
		const Soldier* soldier = icon.soldier.get(egbase());
		if (!soldier) {
			continue;
		}

		constexpr float kNoZoom = 1.f;
		soldier->draw_info_icon(icon.pos + Vector2i(kIconBorder, kIconBorder), kNoZoom,
		                        Soldier::InfoMode::kInBuilding, InfoToDraw::kSoldierLevels, &dst);
	}
}

Vector2i SoldierPanel::calc_pos(uint32_t row, uint32_t col) const {
	return Vector2i(col * icon_width_, row * icon_height_);
}

/**
 * Return the soldier (if any) at the given coordinates.
 */
const Soldier* SoldierPanel::find_soldier(int32_t x, int32_t y) const {
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
			if (const Soldier* soldier = find_soldier(x, y)) {
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
	SoldierList(UI::Panel& parent, InteractiveBase& ib, Widelands::Building& building);

	const SoldierControl* soldiers() const;

protected:
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

private:
	void mouseover(const Soldier* soldier);
	void eject(const Soldier* soldier);
	void set_soldier_preference(int32_t changed_to);
	void think() override;

	InteractiveBase& ibase_;
	Widelands::Building& building_;
	const UI::FontStyle font_style_;
	SoldierPanel soldierpanel_;
	UI::Radiogroup soldier_preference_;
	UI::Textarea infotext_;

	UI::Panel* soldier_capacity_control_;
};

SoldierList::SoldierList(UI::Panel& parent, InteractiveBase& ib, Widelands::Building& building)
   : UI::Box(&parent, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),

     ibase_(ib),
     building_(building),
     font_style_(UI::FontStyle::kWuiLabel),
     soldierpanel_(*this, ib.egbase(), building),
     infotext_(
        this, UI::PanelStyle::kWui, UI::FontStyle::kWuiLabel, _("Click soldier to send away")) {
	add(&soldierpanel_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	add_space(2);

	add(&infotext_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	soldierpanel_.set_mouseover([this](const Soldier* s) { mouseover(s); });
	soldierpanel_.set_click([this](const Soldier* s) { eject(s); });

	// We don't want translators to translate this twice, so it's a bit involved.
	int w = UI::g_fh
	           ->render(as_richtext_paragraph(
	              format("%s "  // We need some extra space to fix bug 724169
	                     ,
	                     format(
	                        /** TRANSLATORS: Health, Attack, Defense, Evade */
	                        _("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u"), 8, 8, 8,
	                        8, 8, 8, 8, 8)),
	              font_style_))
	           ->width();
	uint32_t maxtextwidth = std::max(
	   w, UI::g_fh->render(as_richtext_paragraph(_("Click soldier to send away"), font_style_))
	         ->width());
	set_min_desired_breadth(maxtextwidth + 4);

	UI::Box* buttons = new UI::Box(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal);

	bool can_act = ibase_.can_act(building_.owner().player_number());
	if (upcast(Widelands::MilitarySite, ms, &building)) {
		soldier_preference_.add_button(buttons, UI::PanelStyle::kWui, Vector2i::zero(),
		                               g_image_cache->get("images/wui/buildings/prefer_rookies.png"),
		                               _("Prefer rookies"));
		soldier_preference_.add_button(buttons, UI::PanelStyle::kWui, Vector2i(32, 0),
		                               g_image_cache->get("images/wui/buildings/prefer_heroes.png"),
		                               _("Prefer heroes"));
		UI::Radiobutton* button = soldier_preference_.get_first_button();
		while (button) {
			buttons->add(button);
			button = button->next_button();
		}

		soldier_preference_.set_state(
		   ms->get_soldier_preference() == Widelands::SoldierPreference::kHeroes ? 1 : 0, false);
		if (can_act) {
			soldier_preference_.changedto.connect([this](int32_t a) { set_soldier_preference(a); });
		} else {
			soldier_preference_.set_enabled(false);
		}
	}
	buttons->add_inf_space();
	soldier_capacity_control_ = create_soldier_capacity_control(*buttons, ib, building);
	buttons->add(soldier_capacity_control_);
	add(buttons, UI::Box::Resizing::kFullSize);
}

const SoldierControl* SoldierList::soldiers() const {
	return building_.soldier_control();
}

void SoldierList::think() {
	if (upcast(Widelands::MilitarySite, ms, &building_)) {
		switch (ms->get_soldier_preference()) {
		case Widelands::SoldierPreference::kRookies:
			soldier_preference_.set_state(0, false);
			break;
		case Widelands::SoldierPreference::kHeroes:
			soldier_preference_.set_state(1, false);
			break;
		}
	}
}

void SoldierList::mouseover(const Soldier* soldier) {
	if (!soldier) {
		infotext_.set_text(_("Click soldier to send away"));
		return;
	}

	infotext_.set_text(format(_("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u"),
	                          soldier->get_health_level(), soldier->descr().get_max_health_level(),
	                          soldier->get_attack_level(), soldier->descr().get_max_attack_level(),
	                          soldier->get_defense_level(), soldier->descr().get_max_defense_level(),
	                          soldier->get_evade_level(), soldier->descr().get_max_evade_level()));
}

void SoldierList::eject(const Soldier* soldier) {
	uint32_t const capacity_min = soldiers()->min_soldier_capacity();
	bool can_act = ibase_.can_act(building_.owner().player_number());
	bool over_min = capacity_min < soldiers()->present_soldiers().size();

	if (can_act && over_min) {
		if (Widelands::Game* game = ibase_.get_game()) {
			game->send_player_drop_soldier(building_, soldier->serial());
		} else {
			NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
		}
	}
}

void SoldierList::set_soldier_preference(int32_t changed_to) {
	assert(building_.descr().type() == Widelands::MapObjectType::MILITARYSITE);
	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_militarysite_set_soldier_preference(
		   building_, changed_to == 0 ? Widelands::SoldierPreference::kRookies :
                                      Widelands::SoldierPreference::kHeroes);
	} else {
		NEVER_HERE();  // TODO(Nordfriese / Scenario Editor): implement
	}
}

bool SoldierList::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	return soldier_capacity_control_->handle_mousewheel(x, y, modstate);
}

UI::Panel*
create_soldier_list(UI::Panel& parent, InteractiveBase& ib, Widelands::Building& building) {
	return new SoldierList(parent, ib, building);
}
