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

#include "wui/soldierlist.h"

#include <functional>
#include <memory>

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
#include "ui_basic/button.h"
#include "ui_basic/textarea.h"
#include "wui/interactive_gamebase.h"
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
	using SoldierFn = std::function<void(Soldier*)>;

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
	Soldier* find_soldier(int32_t x, int32_t y) const;

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
	const SoldierControl* soldier_control_;

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
   : Panel(&parent, 0, 0, 0, 0),
     egbase_(gegbase),
     soldier_control_(building.soldier_control()),
     last_animate_time_(0) {
	assert(soldier_control_ != nullptr);
	Soldier::calc_info_icon_size(building.owner().tribe(), icon_width_, icon_height_);
	icon_width_ += 2 * kIconBorder;
	icon_height_ += 2 * kIconBorder;

	Widelands::Quantity maxcapacity = soldier_control_->max_soldier_capacity();
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
	for (Soldier* soldier : soldier_control_->present_soldiers()) {
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
	bool changes = false;
	uint32_t capacity = soldier_control_->soldier_capacity();

	// Update soldier list and target row/col:
	std::vector<Soldier*> soldierlist = soldier_control_->present_soldiers();
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
	// Fill a region matching the current site capacity with black
	uint32_t capacity = soldier_control_->soldier_capacity();
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
Soldier* SoldierPanel::find_soldier(int32_t x, int32_t y) const {
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
			if (Soldier* soldier = find_soldier(x, y)) {
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

private:
	void mouseover(const Soldier* soldier);
	void eject(Soldier* soldier);
	void show_soldier_options(Soldier* soldier);
	void set_soldier_preference(int32_t changed_to);
	void think() override;
	bool check_can_act() const;

	InteractiveBase& ibase_;
	Widelands::Building& building_;
	const UI::FontStyle font_style_;
	SoldierPanel soldierpanel_;
	UI::Radiogroup soldier_preference_;
	UI::Textarea infotext_;
	std::unique_ptr<UI::Button> create_new_soldier_;
};

SoldierList::SoldierList(UI::Panel& parent, InteractiveBase& ib, Widelands::Building& building)
   : UI::Box(&parent, 0, 0, UI::Box::Vertical),
     ibase_(ib),
     building_(building),
     font_style_(UI::FontStyle::kLabel),
     soldierpanel_(*this, ib.egbase(), building),
     infotext_(
        this,
        ib.omnipotent() ? _("Click soldier to configure levels") : _("Click soldier to send away")),
     create_new_soldier_(nullptr) {
	add(&soldierpanel_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	add_space(2);

	add(&infotext_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	soldierpanel_.set_mouseover([this](const Soldier* s) { mouseover(s); });
	soldierpanel_.set_click(
	   [this](Soldier* s) { ibase_.omnipotent() ? show_soldier_options(s) : eject(s); });

	// We don't want translators to translate this twice, so it's a bit involved.
	int w = UI::g_fh
	           ->render(as_richtext_paragraph(
	              (boost::format("%s ")  // We need some extra space to fix bug 724169
	               % (boost::format(
	                     /** TRANSLATORS: Health, Attack, Defense, Evade */
	                     _("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u")) %
	                  8 % 8 % 8 % 8 % 8 % 8 % 8 % 8))
	                 .str(),
	              font_style_))
	           ->width();
	uint32_t maxtextwidth = std::max(
	   w, UI::g_fh->render(as_richtext_paragraph(infotext_.get_text(), font_style_))->width());
	set_min_desired_breadth(maxtextwidth + 4);

	UI::Box* buttons = new UI::Box(this, 0, 0, UI::Box::Horizontal);

	const bool can_act = check_can_act();
	if (upcast(Widelands::MilitarySite, ms, &building)) {
		soldier_preference_.add_button(buttons, Vector2i::zero(),
		                               g_image_cache->get("images/wui/buildings/prefer_rookies.png"),
		                               _("Prefer rookies"));
		soldier_preference_.add_button(buttons, Vector2i(32, 0),
		                               g_image_cache->get("images/wui/buildings/prefer_heroes.png"),
		                               _("Prefer heroes"));
		UI::Radiobutton* button = soldier_preference_.get_first_button();
		while (button) {
			buttons->add(button);
			button = button->next_button();
		}

		soldier_preference_.set_state(0);
		if (ms->get_soldier_preference() == Widelands::SoldierPreference::kHeroes) {
			soldier_preference_.set_state(1);
		}
		if (can_act) {
			soldier_preference_.changedto.connect([this](int32_t a) { set_soldier_preference(a); });
		} else {
			soldier_preference_.set_enabled(false);
		}
	}
	if (ib.omnipotent()) {
		buttons->add_inf_space();
		create_new_soldier_.reset(new UI::Button(
		   buttons, "", 0, 0, 32, 32, UI::ButtonStyle::kWuiSecondary,
		   building.owner().tribe().get_worker_descr(building.owner().tribe().soldier())->icon(),
		   _("Create and add a new soldier")));
		buttons->add(create_new_soldier_.get());
		create_new_soldier_->sigclicked.connect([this]() {
			building_.mutable_soldier_control()->incorporate_soldier(
			   ibase_.egbase(), dynamic_cast<Widelands::Soldier&>(
			                       building_.owner()
			                          .tribe()
			                          .get_worker_descr(building_.owner().tribe().soldier())
			                          ->create(ibase_.egbase(), building_.get_owner(), &building_,
			                                   building_.get_position())));
		});
	}
	buttons->add_inf_space();
	buttons->add(create_soldier_capacity_control(*buttons, ib, building));
	add(buttons, UI::Box::Resizing::kFullSize);
}

bool SoldierList::check_can_act() const {
	return ibase_.omnipotent() || ibase_.can_act(building_.owner().player_number());
}

const SoldierControl* SoldierList::soldiers() const {
	return building_.soldier_control();
}

void SoldierList::think() {
	if (create_new_soldier_) {
		create_new_soldier_->set_enabled(building_.soldier_control()->soldier_capacity() >
		                                 building_.soldier_control()->stationed_soldiers().size());
	}
	// Only update the soldiers pref radio if player is spectator
	if (check_can_act()) {
		return;
	}
	if (upcast(Widelands::MilitarySite, ms, &building_)) {
		switch (ms->get_soldier_preference()) {
		case Widelands::SoldierPreference::kRookies:
			soldier_preference_.set_state(0);
			break;
		case Widelands::SoldierPreference::kHeroes:
			soldier_preference_.set_state(1);
			break;
		}
	}
}

void SoldierList::mouseover(const Soldier* soldier) {
	if (!soldier) {
		infotext_.set_text(ibase_.omnipotent() ? _("Click soldier to configure levels") :
		                                         _("Click soldier to send away"));
		return;
	}

	infotext_.set_text(
	   (boost::format(_("HP: %1$u/%2$u  AT: %3$u/%4$u  DE: %5$u/%6$u  EV: %7$u/%8$u")) %
	    soldier->get_health_level() % soldier->descr().get_max_health_level() %
	    soldier->get_attack_level() % soldier->descr().get_max_attack_level() %
	    soldier->get_defense_level() % soldier->descr().get_max_defense_level() %
	    soldier->get_evade_level() % soldier->descr().get_max_evade_level())
	      .str());
}

void SoldierList::eject(Soldier* soldier) {
	uint32_t const capacity_min = soldiers()->min_soldier_capacity();
	bool over_min = capacity_min < soldiers()->present_soldiers().size();

	if (check_can_act() && over_min) {
		assert(ibase_.get_game());
		ibase_.game().send_player_drop_soldier(building_, soldier->serial());
	}
}

void SoldierList::set_soldier_preference(int32_t changed_to) {
	assert(is_a(Widelands::MilitarySite, &building_));
	if (ibase_.get_game()) {
		ibase_.game().send_player_militarysite_set_soldier_preference(
		   building_, changed_to == 0 ? Widelands::SoldierPreference::kRookies :
		                                Widelands::SoldierPreference::kHeroes);
	} else {
		building_.get_owner()->military_site_set_soldier_preference(
		   building_, changed_to == 0 ? Widelands::SoldierPreference::kRookies :
		                                Widelands::SoldierPreference::kHeroes);
	}
}

void SoldierList::show_soldier_options(Soldier* soldier) {
	assert(ibase_.omnipotent());
	SoldierSettings s(ibase_, *soldier, true);
	s.run<UI::Panel::Returncodes>();
}

constexpr uint16_t kSliderWidth = 250;
constexpr uint16_t kSliderHeight = 24;
constexpr uint16_t kSpacing = 4;

SoldierSettings::SoldierSettings(InteractiveBase& ib, Widelands::Soldier& s, bool allow_delete)
   : UI::Window(&ib,
                "soldier_settings_" + std::to_string(s.serial()),
                0,
                0,
                200,
                200,
                (boost::format(_("Soldier %u")) % s.serial()).str()),
     egbase_(ib.egbase()),
     soldier_(s),
     main_box_(this, 0, 0, UI::Box::Vertical),
     upper_box_(&main_box_, 0, 0, UI::Box::Horizontal),
     label_box_(&upper_box_, 0, 0, UI::Box::Vertical),
     slider_box_(&upper_box_, 0, 0, UI::Box::Vertical),
     health_(&slider_box_,
             0,
             0,
             kSliderWidth,
             kSliderHeight,
             0,
             s.descr().get_max_health_level(),
             s.get_health_level(),
             UI::SliderStyle::kWuiDark,
             _("The soldier’s health level")),
     attack_(&slider_box_,
             0,
             0,
             kSliderWidth,
             kSliderHeight,
             0,
             s.descr().get_max_attack_level(),
             s.get_attack_level(),
             UI::SliderStyle::kWuiDark,
             _("The soldier’s attack level")),
     defense_(&slider_box_,
              0,
              0,
              kSliderWidth,
              kSliderHeight,
              0,
              s.descr().get_max_defense_level(),
              s.get_defense_level(),
              UI::SliderStyle::kWuiDark,
              _("The soldier’s defense level")),
     evade_(&slider_box_,
            0,
            0,
            kSliderWidth,
            kSliderHeight,
            0,
            s.descr().get_max_evade_level(),
            s.get_evade_level(),
            UI::SliderStyle::kWuiDark,
            _("The soldier’s evade level")),
     current_health_(&slider_box_,
                     0,
                     0,
                     kSliderWidth,
                     kSliderHeight,
                     1,
                     1,
                     1,
                     UI::SliderStyle::kWuiDark,
                     _("The soldier’s current health")),
     delete_(&main_box_,
             "delete",
             0,
             0,
             100,
             30,
             UI::ButtonStyle::kWuiSecondary,
             _("Delete"),
             _("Delete this soldier")),
     cancel_(&main_box_,
             "cancel",
             0,
             0,
             100,
             30,
             UI::ButtonStyle::kWuiSecondary,
             _("Cancel"),
             _("Discard all changes")),
     ok_(
        &main_box_, "ok", 0, 0, 100, 30, UI::ButtonStyle::kWuiPrimary, _("OK"), _("Apply changes")),
     hlabel_(&label_box_, "", UI::Align::kRight),
     alabel_(&label_box_, "", UI::Align::kRight),
     dlabel_(&label_box_, "", UI::Align::kRight),
     elabel_(&label_box_, "", UI::Align::kRight),
     clabel_(&label_box_, "", UI::Align::kRight) {
	assert(ib.omnipotent());

	label_box_.add(&hlabel_, UI::Box::Resizing::kFullSize);
	label_box_.add_inf_space();
	label_box_.add(&alabel_, UI::Box::Resizing::kFullSize);
	label_box_.add_inf_space();
	label_box_.add(&dlabel_, UI::Box::Resizing::kFullSize);
	label_box_.add_inf_space();
	label_box_.add(&elabel_, UI::Box::Resizing::kFullSize);
	label_box_.add_inf_space();
	label_box_.add(&clabel_, UI::Box::Resizing::kFullSize);
	slider_box_.add(&health_, UI::Box::Resizing::kFullSize);
	slider_box_.add_space(kSpacing);
	slider_box_.add(&attack_, UI::Box::Resizing::kFullSize);
	slider_box_.add_space(kSpacing);
	slider_box_.add(&defense_, UI::Box::Resizing::kFullSize);
	slider_box_.add_space(kSpacing);
	slider_box_.add(&evade_, UI::Box::Resizing::kFullSize);
	slider_box_.add_space(kSpacing);
	slider_box_.add(&current_health_, UI::Box::Resizing::kFullSize);

	upper_box_.add(&label_box_, UI::Box::Resizing::kFullSize);
	upper_box_.add_space(kSpacing);
	upper_box_.add(&slider_box_, UI::Box::Resizing::kFullSize);
	main_box_.add(&upper_box_, UI::Box::Resizing::kFullSize);
	main_box_.add(&delete_, UI::Box::Resizing::kFullSize);
	main_box_.add(&cancel_, UI::Box::Resizing::kFullSize);
	main_box_.add(&ok_, UI::Box::Resizing::kFullSize);

	health_.set_enabled(health_.get_max_value() > 0);
	attack_.set_enabled(attack_.get_max_value() > 0);
	defense_.set_enabled(defense_.get_max_value() > 0);
	evade_.set_enabled(evade_.get_max_value() > 0);

	health_.changed.connect([this]() { health_slider_changed(); });
	attack_.changed.connect([this]() { update_label_a(); });
	defense_.changed.connect([this]() { update_label_d(); });
	evade_.changed.connect([this]() { update_label_e(); });
	current_health_.changed.connect([this]() { update_label_c(); });
	delete_.sigclicked.connect([this]() { clicked_delete(); });
	cancel_.sigclicked.connect([this]() { die(); });
	ok_.sigclicked.connect([this]() { clicked_ok(); });

	health_slider_changed();
	current_health_.set_value(soldier_.get_current_health());
	update_label_a();
	update_label_d();
	update_label_e();

	hlabel_.set_fixed_width(kSliderWidth);
	alabel_.set_fixed_width(kSliderWidth);
	dlabel_.set_fixed_width(kSliderWidth);
	elabel_.set_fixed_width(kSliderWidth);
	clabel_.set_fixed_width(kSliderWidth);

	if (!allow_delete) {
		delete_.set_enabled(false);
		delete_.set_tooltip(_("Use the Delete Workers tool if you wish to delete this soldier."));
	}

	set_center_panel(&main_box_);
	center_to_parent();
}

inline static void update_label(UI::Textarea& l, const UI::Slider& s, const std::string& prefix) {
	l.set_text(
	   /** TRANSLATORS: "XYZ level: 1 / 5" */
	   (boost::format(_("%1%: %2% / %3%")) % prefix % s.get_value() % s.get_max_value()).str());
}
void SoldierSettings::update_label_h() {
	/** TRANSLATORS: Part of a string: "Health level: 1 / 5" */
	update_label(hlabel_, health_, _("Health level"));
}
void SoldierSettings::update_label_a() {
	/** TRANSLATORS: Part of a string: "Attack level: 1 / 5" */
	update_label(alabel_, attack_, _("Attack level"));
}
void SoldierSettings::update_label_d() {
	/** TRANSLATORS: Part of a string: "Defense level: 1 / 5" */
	update_label(dlabel_, defense_, _("Defense level"));
}
void SoldierSettings::update_label_e() {
	/** TRANSLATORS: Part of a string: "Evade level: 1 / 5" */
	update_label(elabel_, evade_, _("Evade level"));
}
void SoldierSettings::update_label_c() {
	/** TRANSLATORS: Part of a string: "Health: 12345 / 20000" */
	update_label(clabel_, current_health_, _("Health"));
}

void SoldierSettings::health_slider_changed() {
	current_health_.set_max_value(soldier_.descr().get_base_health() +
	                              health_.get_value() *
	                                 soldier_.descr().get_health_incr_per_level());
	update_label_h();
	update_label_c();
}

void SoldierSettings::clicked_ok() {
	soldier_.set_level(
	   health_.get_value(), attack_.get_value(), defense_.get_value(), evade_.get_value());
	soldier_.set_current_health(current_health_.get_value());
	die();
}
void SoldierSettings::clicked_delete() {
	dynamic_cast<Widelands::Building*>(soldier_.get_location(egbase_))
	   ->mutable_soldier_control()
	   ->outcorporate_soldier(soldier_);
	soldier_.remove(egbase_);
	die();
}

UI::Panel*
create_soldier_list(UI::Panel& parent, InteractiveBase& ib, Widelands::Building& building) {
	return new SoldierList(parent, ib, building);
}
