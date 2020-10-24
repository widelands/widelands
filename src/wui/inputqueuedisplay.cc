/*
 * Copyright (C) 2010-2020 by the Widelands Development Team
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

#include "wui/inputqueuedisplay.h"

#include "base/i18n.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/player.h"
#include "wui/interactive_base.h"

constexpr int8_t kButtonSize = 24;

/*/ The slider indexes the available priorities as 0..4, so here are some conversion functions
static size_t priority_to_index(const int32_t priority) {
	switch (priority) {
	case Widelands::WarePriority::kVeryLow:
		return 0;
	case Widelands::WarePriority::kLow:
		return 1;
	case 2:  // TODO(Nordfriese): The value '2' is for savegame compatibility
	case Widelands::WarePriority::kNormal:
		return 2;
	case Widelands::WarePriority::kHigh:
		return 3;
	case Widelands::WarePriority::kVeryHigh:
		return 4;
	default: NEVER_HERE();
	}
}
static int32_t index_to_priority(const size_t priority) {
	switch (priority) {
	case 0: return Widelands::WarePriority::kVeryLow;
	case 1: return Widelands::WarePriority::kLow;
	case 2: return Widelands::WarePriority::kNormal;
	case 3: return Widelands::WarePriority::kHigh;
	case 4: return Widelands::WarePriority::kVeryHigh;
	default: NEVER_HERE();
	}
}

static std::string priority_tooltip(const size_t priority) {
	switch (priority) {
	case 0: return _("Priority: Very Low");
	case 1: return _("Priority: Low");
	case 2: return _("Priority: Normal");
	case 3: return _("Priority: High");
	case 4: return _("Priority: Very High");
	default: NEVER_HERE();
	}
} */

void ensure_box_can_hold_input_queues(UI::Box& b) {
	UI::Panel* p = &b;
	while (p->get_parent()) {
		 p = p->get_parent();
	}
	b.set_max_size(p->get_w() - 200, p->get_h() - 200);

	b.set_scrolling(true);
	b.set_scrollbar_style(UI::PanelStyle::kWui);
}

InputQueueDisplay::InputQueueDisplay(UI::Panel* parent, InteractiveBase& ib, Widelands::Building& bld, Widelands::InputQueue& q, bool show_only, bool has_priority)
: InputQueueDisplay(parent, ib, bld, q.get_type(), q.get_index(), &q, nullptr, show_only, has_priority) {
}
InputQueueDisplay::InputQueueDisplay(UI::Panel* parent, InteractiveBase& ib, Widelands::ConstructionSite& csite, Widelands::WareWorker ww, Widelands::DescriptionIndex di)
: InputQueueDisplay(parent, ib, csite, ww, di, nullptr, dynamic_cast<Widelands::ProductionsiteSettings*>(csite.get_settings()), false, true) {
}

static inline std::string create_tooltip(const bool increase) {
	return (boost::format("<p>%s%s%s</p>")
	    %
	    g_style_manager->font_style(UI::FontStyle::kTooltipHeader)
	       .as_font_tag(
	          increase ?
	          /** TRANSLATORS: Button tooltip in in a building's wares input queue */
	          _("Increase the number of wares you want to be stored here") :
	          /** TRANSLATORS: Button tooltip in in a building's wares input queue */
	          _("Decrease the number of wares you want to be stored here")
	          )
	    %
	    as_listitem(
	          increase ?
	       /** TRANSLATORS: Button tooltip in in a building's wares input queue - option explanation */
	       _("Hold down Shift to increase all ware types at the same time") :
	       /** TRANSLATORS: Button tooltip in in a building's wares input queue - option explanation */
	       _("Hold down Shift to decrease all ware types at the same time")
	       , UI::FontStyle::kTooltip)
	    % as_listitem(
	          increase ?
	         /** TRANSLATORS: Button tooltip in in a building's wares input queue - option explanation */
	         _("Hold down Ctrl to allow all of this ware") :
	         /** TRANSLATORS: Button tooltip in in a building's wares input queue - option explanation */
	         _("Hold down Ctrl to allow none of this ware")
	         , UI::FontStyle::kTooltip))
	      .str();
}

// constexpr unsigned kSliderNotMoved = std::numeric_limits<unsigned>::max();

InputQueueDisplay::InputQueueDisplay(UI::Panel* parent, InteractiveBase& ib, Widelands::Building& bld, Widelands::WareWorker ww, Widelands::DescriptionIndex idx, Widelands::InputQueue* q, Widelands::ProductionsiteSettings* s, bool show_only, bool has_priority)
: UI::Box(parent, 0, 0, UI::Box::Vertical),
ibase_(ib),
can_act_(!show_only && ibase_.can_act(bld.owner().player_number())),
show_only_(show_only),
has_priority_(has_priority && ww == Widelands::wwWARE),
building_(bld),
type_(ww),
index_(idx),
queue_(q),
settings_(s),
max_fill_indicator_(*g_image_cache->get("images/wui/buildings/max_fill_indicator.png")),
// priority_image_(*g_image_cache->get("images/wui/buildings/priority_indicator.png")),
hbox_(this, 0, 0, UI::Box::Horizontal),
b_decrease_desired_fill_(&hbox_, "decrease_desired", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/ui_basic/scrollbar_left.png"), create_tooltip(false)),
b_increase_desired_fill_(&hbox_, "increase_desired", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/ui_basic/scrollbar_right.png"), create_tooltip(true)),
b_decrease_real_fill_(&hbox_, "decrease_real", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/ui_basic/scrollbar_down.png"), _("Remove ware")),
b_increase_real_fill_(&hbox_, "increase_real", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/ui_basic/scrollbar_up.png"), _("Add ware")),
/* b_p_very_low_(&hbox_, "p_very_low", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/stock_policy_remove.png"), _("Very low priority")),
b_p_low_(&hbox_, "p_low", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/low_priority_button.png"), _("Low priority")),
b_p_normal_(&hbox_, "p_low", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/normal_priority_button.png"), _("Normal priority")),
b_p_high_(&hbox_, "p_low", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/high_priority_button.png"), _("High priority")),
b_p_very_high_(&hbox_, "p_very_high", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/stock_policy_prefer.png"), _("Very high priority")), */
priority_buttons_(has_priority_ ? std::map<Widelands::WarePriority, UI::Button*> {
	{Widelands::WarePriority::kVeryLow, new UI::Button(&hbox_, "p_very_low", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/stock_policy_remove.png"), _("Very low priority"))},
	{Widelands::WarePriority::kLow, new UI::Button(&hbox_, "p_low", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/low_priority_button.png"), _("Low priority"))},
	{Widelands::WarePriority::kNormal, new UI::Button(&hbox_, "p_normal", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/normal_priority_button.png"), _("Normal priority"))},
	{Widelands::WarePriority::kHigh, new UI::Button(&hbox_, "p_high", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/high_priority_button.png"), _("High priority"))},
	{Widelands::WarePriority::kVeryHigh, new UI::Button(&hbox_, "p_very_high", 0, 0, kButtonSize, kButtonSize,
		UI::ButtonStyle::kWuiMenu, g_image_cache->get("images/wui/buildings/stock_policy_prefer.png"), _("Very high priority"))}
} : std::map<Widelands::WarePriority, UI::Button*> {}),
spacer_(&hbox_, 0, 0, 5 * kButtonSize, kButtonSize),
/* priority_(this, 0, 0, 3 * kButtonSize,
	                 kButtonSize * 2 / 3, 0, 4,
	                 has_priority_ ? priority_to_index(
					   	settings_ ? settings_->ware_queues.at(index_).priority : building_.get_priority(type_, index_)) : 2,
		UI::SliderStyle::kWuiLight, "", kButtonSize, can_act_ && has_priority_),
slider_was_moved_(kSliderNotMoved), */
nr_icons_(queue_ ? queue_->get_max_size() :
		type_ == Widelands::wwWORKER ?
				settings_->worker_queues.at(index_).max_fill :
				settings_->ware_queues.at(index_).max_fill),
icons_(nr_icons_, nullptr) {

	assert((queue_ == nullptr) ^ (settings_ == nullptr));

	// priority_.set_visible(has_priority_);
	b_decrease_real_fill_.set_visible(queue_ && ibase_.omnipotent());
	b_increase_real_fill_.set_visible(queue_ && ibase_.omnipotent());

	b_decrease_desired_fill_.set_visible(!show_only_);
	b_increase_desired_fill_.set_visible(!show_only_);
	b_decrease_desired_fill_.set_enabled(can_act_);
	b_increase_desired_fill_.set_enabled(can_act_);

	b_increase_desired_fill_.set_repeating(true);
	b_increase_real_fill_.set_repeating(true);
	b_decrease_desired_fill_.set_repeating(true);
	b_decrease_real_fill_.set_repeating(true);

	add_space(kButtonSize / 4);

	hbox_.add(&b_decrease_real_fill_);
	hbox_.add(&b_decrease_desired_fill_);

	for (size_t i = 0; i < nr_icons_; ++i) {
		icons_[i] = new UI::Icon(&hbox_, 0, 0, kButtonSize, kButtonSize,
			type_ == Widelands::wwWARE ? building_.owner().tribe().get_ware_descr(index_)->icon()
			: building_.owner().tribe().get_worker_descr(index_)->icon());
		hbox_.add(icons_[i]);
	}

	hbox_.add_inf_space();
	hbox_.add(&b_increase_desired_fill_);
	hbox_.add(&b_increase_real_fill_);

	hbox_.add_space(kButtonSize / 2);

	// for (UI::Button* b : {&b_p_very_low_, &b_p_low_, &b_p_normal_, &b_p_high_, &b_p_very_high_}) {
	for (auto& pair : priority_buttons_) {
		pair.second->set_visible(has_priority_);
		pair.second->set_enabled(can_act_);
		hbox_.add(pair.second);
	}
	// To make sure the fill buttons are aligned even when some queues
	// have priority buttons and some don't (e.g. in barracks)
	spacer_.set_visible(!has_priority_);
	hbox_.add(&spacer_);

	add(&hbox_, UI::Box::Resizing::kFullSize);
	add_space(kButtonSize / 4);

	if (can_act_) {
		b_decrease_desired_fill_.sigclicked.connect([this]() { clicked_desired_fill(-1); });
		b_increase_desired_fill_.sigclicked.connect([this]() { clicked_desired_fill(1); });
		b_decrease_real_fill_.sigclicked.connect([this]() { clicked_real_fill(-1); });
		b_increase_real_fill_.sigclicked.connect([this]() { clicked_real_fill(1); });
		for (auto& pair : priority_buttons_) {
			pair.second->sigclicked.connect([this, pair]() { set_priority(pair.first); });
		}
		/* priority_.changedto.connect([this](size_t i) {
			slider_was_moved_ = i;
			set_priority(i);
		}); */
	}

	set_tooltip(type_ == Widelands::wwWARE ? building_.owner().tribe().get_ware_descr(index_)->descname()
			: building_.owner().tribe().get_worker_descr(index_)->descname());

	// Initialize tooltips, icon colours and stuff
	think();
}

void InputQueueDisplay::set_priority(const Widelands::WarePriority& priority, const bool recursion_start) {
	if (recursion_start && SDL_GetModState() & KMOD_CTRL) {
		for (UI::Panel* p = get_parent()->get_first_child(); p; p = p->get_next_sibling()) {
			if (upcast(InputQueueDisplay, i, p)) {
				i->set_priority(priority, false);
			}
		}
		return;
	}

	if (!can_act_ || !has_priority_) {
		return;
	}

	// const int32_t priority = index_to_priority(index);

	if (priority == (queue_ ? building_.get_priority(type_, index_) : get_setting()->priority)) {
		return;
	}

	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_ware_priority(building_, type_, index_, priority, settings_ != nullptr);
	} else {
		if (queue_) {
			building_.set_priority(type_, index_, priority);
		} else {
			get_setting()->priority = priority;
		}
	}
}

void InputQueueDisplay::clicked_desired_fill(const int8_t delta, const bool recursion_start) {
	assert(delta == 1 || delta == -1);

	if (recursion_start && (SDL_GetModState() & KMOD_SHIFT)) {
		for (UI::Panel* p = get_parent()->get_first_child(); p; p = p->get_next_sibling()) {
			if (upcast(InputQueueDisplay, i, p)) {
				i->clicked_desired_fill(delta, false);
			}
		}
		return;
	}

	const unsigned desired_fill = queue_ ? queue_->get_max_fill() : get_setting()->desired_fill;
	const unsigned max_fill = queue_ ? queue_->get_max_size() : get_setting()->max_fill;
	assert(desired_fill <= max_fill);

	if (!can_act_ || desired_fill == (delta < 0 ? 0 : max_fill)) {
		return;
	}

	const bool ctrl_down = SDL_GetModState() & KMOD_CTRL;
	const unsigned new_fill = ctrl_down ? delta < 0 ? 0 : max_fill : desired_fill + delta;

	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_input_max_fill(building_, index_, type_, new_fill, settings_ != nullptr);
	} else {
		if (queue_) {
			queue_->set_max_fill(new_fill);
		} else {
			get_setting()->desired_fill = new_fill;
		}
	}
}

void InputQueueDisplay::clicked_real_fill(int8_t delta, bool recursion_start) {
	assert(delta == 1 || delta == -1);

	if (recursion_start && (SDL_GetModState() & KMOD_SHIFT)) {
		for (UI::Panel* p = get_parent()->get_first_child(); p; p = p->get_next_sibling()) {
			if (upcast(InputQueueDisplay, i, p)) {
				i->clicked_real_fill(delta, false);
			}
		}
		return;
	}

	if (!queue_ || !ibase_.omnipotent()) {
		return;
	}

	const unsigned real_fill = queue_->get_filled();
	const unsigned max_fill = queue_->get_max_size();
	assert(real_fill <= max_fill);

	if (real_fill == (delta < 0 ? 0 : max_fill)) {
		return;
	}

	const bool ctrl_down = SDL_GetModState() & KMOD_CTRL;
	const unsigned new_fill = ctrl_down ? delta < 0 ? 0 : max_fill : real_fill + delta;

	queue_->set_filled(new_fill);
}

inline Widelands::ProductionsiteSettings::InputQueueSetting* InputQueueDisplay::get_setting() const {
	return settings_ ? type_ == Widelands::wwWARE ? &settings_->ware_queues.at(index_) : &settings_->worker_queues.at(index_) : nullptr;
}

static const RGBAColor kColorComing(127, 127, 127, 191);
static const RGBAColor kColorMissing(191, 191, 191, 127);

void InputQueueDisplay::think() {
	if (queue_ && queue_->get_max_size() == 0) {
		set_visible(false);
		return;
	}
	set_visible(true);

	const Widelands::ProductionsiteSettings::InputQueueSetting* setting = get_setting();
	const unsigned max_fill = queue_ ? queue_->get_max_size() : setting->max_fill;
	const unsigned real_fill = queue_ ? queue_->get_filled() : setting->desired_fill;
	const unsigned desired_fill = queue_ ? queue_->get_max_fill() : setting->desired_fill;
	const unsigned nr_missing = queue_ ? queue_->get_missing() : 0;
	const unsigned nr_coming = queue_ && real_fill < desired_fill ? desired_fill - real_fill - nr_missing : 0;

	assert(desired_fill <= max_fill);
	assert(real_fill <= max_fill);
	assert(nr_missing <= max_fill - real_fill);
	assert(nr_coming <= max_fill - real_fill);
	assert(max_fill <= nr_icons_);

	for (unsigned i = 0; i < nr_icons_; ++i) {
		icons_[i]->set_visible(i < max_fill);
		icons_[i]->set_grey_out(i >= real_fill);
		icons_[i]->set_grey_out_color(i < real_fill + nr_coming ? kColorComing : kColorMissing);
	}

	if (has_priority_) {
		// const unsigned prio_idx = priority_to_index(queue_ ? building_.get_priority(type_, index_) : get_setting()->priority);
		// The purpose of this check is to prevent the slider from snapping back directly after
		// the user dragged it, because the playercommand is not executed immediately of course
		/* if (slider_was_moved_ == kSliderNotMoved || slider_was_moved_ == prio_idx) {
			priority_.set_value(prio_idx);
			slider_was_moved_ = kSliderNotMoved;
		} */
		// priority_.set_tooltip(priority_tooltip(priority_.get_value()));

		const Widelands::WarePriority& p = queue_ ? building_.get_priority(type_, index_) : get_setting()->priority;
		/* b_p_very_low_.set_enabled(p != Widelands::WarePriority::kVeryLow);
		b_p_low_.set_enabled(p != Widelands::WarePriority::kLow);
		b_p_normal_.set_enabled(p != Widelands::WarePriority::kNormal);
		b_p_high_.set_enabled(p != Widelands::WarePriority::kHigh);
		b_p_very_high_.set_enabled(p != Widelands::WarePriority::kVeryHigh); */
		for (auto& pair : priority_buttons_) {
			pair.second->set_perm_pressed(p == pair.first);
		}
	}
}

void InputQueueDisplay::draw_overlay(RenderTarget& r) {
	if (!show_only_) {
		assert(nr_icons_);
		const unsigned desired_fill = queue_ ? queue_->get_max_fill() : get_setting()->desired_fill;
		assert(desired_fill <= nr_icons_);

		const int xpos = desired_fill == 0 ? icons_[0]->get_x() : desired_fill == nr_icons_ ? icons_[nr_icons_ - 1]->get_x() + icons_[nr_icons_ - 1]->get_w()
				: (icons_[desired_fill - 1]->get_x() + icons_[desired_fill - 1]->get_w() + icons_[desired_fill]->get_x()) / 2;
		const int ypos = hbox_.get_y() + icons_[0]->get_y() + (icons_[0]->get_h() - max_fill_indicator_.height()) / 2;
		r.blit(Vector2i(xpos + hbox_.get_x() - max_fill_indicator_.width() / 2, ypos), &max_fill_indicator_);
	}

	UI::Box::draw_overlay(r);
}

void InputQueueDisplay::draw(RenderTarget& r) {
	UI::Box::draw(r);

	/* if (has_priority_) {
		r.blitrect_scale(Rectf(priority_.get_x(), priority_.get_y() - priority_.get_h() / 3, priority_.get_w(), priority_.get_h() * 5 / 3), &priority_image_,
		                 Recti(0, 0, priority_image_.width(), priority_image_.height()), 0.5f, BlendMode::Default);
	} */
}
