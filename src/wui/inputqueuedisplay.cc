/*
 * Copyright (C) 2010-2025 by the Widelands Development Team
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

#include "wui/inputqueuedisplay.h"

#include "base/i18n.h"
#include "graphic/style_manager.h"
#include "graphic/text_layout.h"
#include "logic/player.h"
#include "wlapplication_mousewheel_options.h"
#include "wui/actionconfirm.h"
#include "wui/interactive_player.h"

constexpr int8_t kButtonSize = 25;

// The slider indexes the available priorities as 0..4, so here are some conversion functions
static size_t priority_to_index(const Widelands::WarePriority& priority) {
	if (priority == Widelands::WarePriority::kVeryLow) {
		return 0;
	}
	if (priority == Widelands::WarePriority::kLow) {
		return 1;
	}
	if (priority == Widelands::WarePriority::kNormal) {
		return 2;
	}
	if (priority == Widelands::WarePriority::kHigh) {
		return 3;
	}
	if (priority == Widelands::WarePriority::kVeryHigh) {
		return 4;
	}
	NEVER_HERE();
}
static const Widelands::WarePriority& index_to_priority(const size_t priority) {
	switch (priority) {
	case 0:
		return Widelands::WarePriority::kVeryLow;
	case 1:
		return Widelands::WarePriority::kLow;
	case 2:
		return Widelands::WarePriority::kNormal;
	case 3:
		return Widelands::WarePriority::kHigh;
	case 4:
		return Widelands::WarePriority::kVeryHigh;
	default:
		NEVER_HERE();
	}
}

static std::string priority_tooltip(const size_t priority) {
	switch (priority) {
	case 0:
		return _("Priority: Very Low");
	case 1:
		return _("Priority: Low");
	case 2:
		return _("Priority: Normal");
	case 3:
		return _("Priority: High");
	case 4:
		return _("Priority: Very High");
	default:
		NEVER_HERE();
	}
}

void ensure_box_can_hold_input_queues(UI::Box& b) {
	UI::Panel* p = &b;
	while (p->get_parent() != nullptr) {
		p = p->get_parent();
	}
	b.set_max_size(p->get_w() - 200, p->get_h() - 200);
	b.set_scrolling(true);
}

InputQueueDisplay::InputQueueDisplay(UI::Panel* parent,
                                     InteractiveBase& interactive_base,
                                     Widelands::Building& building,
                                     Widelands::InputQueue& queue,
                                     bool show_only,
                                     bool has_priority,
                                     BuildingWindow::CollapsedState* collapsed,
                                     uint32_t disambiguator_id)
   : InputQueueDisplay(parent,
                       interactive_base,
                       building,
                       queue.get_type(),
                       queue.get_index(),
                       &queue,
                       nullptr,
                       show_only,
                       has_priority,
                       collapsed,
                       disambiguator_id) {
}
InputQueueDisplay::InputQueueDisplay(UI::Panel* parent,
                                     InteractiveBase& interactive_base,
                                     Widelands::ConstructionSite& constructionsite,
                                     Widelands::WareWorker type,
                                     Widelands::DescriptionIndex ware_or_worker_index,
                                     BuildingWindow::CollapsedState* collapsed,
                                     uint32_t disambiguator_id)
   : InputQueueDisplay(
        parent,
        interactive_base,
        constructionsite,
        type,
        ware_or_worker_index,
        nullptr,
        dynamic_cast<Widelands::ProductionsiteSettings*>(constructionsite.get_settings()),
        false,
        true,
        collapsed,
        disambiguator_id) {
}

static inline std::string create_tooltip(const bool increase) {
	return format(
	   "<p>%s%s%s</p>",
	   g_style_manager->font_style(UI::FontStyle::kWuiTooltipHeader)
	      .as_font_tag(increase ?
	                      /** TRANSLATORS: Button tooltip in in a building's wares input queue */
	                      _("Increase the number of wares you want to be stored here") :
	                      /** TRANSLATORS: Button tooltip in in a building's wares input queue */
	                      _("Decrease the number of wares you want to be stored here")),
	   as_listitem(increase ?
	                  /** TRANSLATORS: Button tooltip in in a building's wares input queue -
	                     option explanation */
	                  _("Hold down Shift to increase all ware types at the same time") :
	                     /** TRANSLATORS: Button tooltip in in a building's wares input queue -
	                        option explanation */
	                     _("Hold down Shift to decrease all ware types at the same time"),
	               UI::FontStyle::kWuiTooltip),
	   as_listitem(increase ?
	                  /** TRANSLATORS: Button tooltip in in a building's wares input queue -
	                     option explanation */
	                  _("Hold down Ctrl to allow all of this ware") :
	                     /** TRANSLATORS: Button tooltip in in a building's wares input queue -
	                        option explanation */
	                     _("Hold down Ctrl to allow none of this ware"),
	               UI::FontStyle::kWuiTooltip));
}

InputQueueDisplay::InputQueueDisplay(UI::Panel* parent,
                                     InteractiveBase& interactive_base,
                                     Widelands::Building& building,
                                     Widelands::WareWorker type,
                                     Widelands::DescriptionIndex ware_or_worker_index,
                                     Widelands::InputQueue* queue,
                                     Widelands::ProductionsiteSettings* settings,
                                     bool show_only,
                                     bool has_priority,
                                     BuildingWindow::CollapsedState* collapsed,
                                     uint32_t disambiguator_id)
   : UI::Box(parent,
             UI::PanelStyle::kWui,
             format("inputqueuedisplay_%u_%u", static_cast<unsigned>(type), ware_or_worker_index),
             0,
             0,
             UI::Box::Horizontal),
     ibase_(interactive_base),
     can_act_(!show_only && ibase_.can_act(building.owner().player_number())),
     show_only_(show_only),
     has_priority_(has_priority && type == Widelands::wwWARE),
     building_(&building),
     type_(type),
     index_(ware_or_worker_index),
     queue_(queue),
     settings_(settings),
     disambiguator_id_(disambiguator_id),
     max_fill_indicator_(*g_image_cache->get("images/wui/buildings/max_fill_indicator.png")),
     vbox_(this, UI::PanelStyle::kWui, "vbox", 0, 0, UI::Box::Vertical),
     hbox_(&vbox_, UI::PanelStyle::kWui, "hbox", 0, 0, UI::Box::Horizontal),
     b_decrease_desired_fill_(&hbox_,
                              "decrease_desired",
                              0,
                              0,
                              kButtonSize,
                              kButtonSize,
                              UI::ButtonStyle::kWuiMenu,
                              g_image_cache->get("images/ui_basic/scrollbar_left.png"),
                              create_tooltip(false)),
     b_increase_desired_fill_(&hbox_,
                              "increase_desired",
                              0,
                              0,
                              kButtonSize,
                              kButtonSize,
                              UI::ButtonStyle::kWuiMenu,
                              g_image_cache->get("images/ui_basic/scrollbar_right.png"),
                              create_tooltip(true)),
     b_decrease_real_fill_(&hbox_,
                           "decrease_real",
                           0,
                           0,
                           kButtonSize,
                           kButtonSize,
                           UI::ButtonStyle::kWuiMenu,
                           g_image_cache->get("images/ui_basic/scrollbar_down.png"),
                           _("Remove ware")),
     b_increase_real_fill_(&hbox_,
                           "increase_real",
                           0,
                           0,
                           kButtonSize,
                           kButtonSize,
                           UI::ButtonStyle::kWuiMenu,
                           g_image_cache->get("images/ui_basic/scrollbar_up.png"),
                           _("Add ware")),
     collapse_(this,
               "collapse",
               0,
               0,
               kButtonSize,
               kButtonSize * 3 / 2,
               UI::ButtonStyle::kWuiMenu,
               "",
               "",
               UI::Button::VisualState::kFlat),
     priority_(&hbox_,
               0,
               0,
               5 * kButtonSize,
               kButtonSize,
               0,
               4,
               has_priority_ ?
                  priority_to_index(settings_ != nullptr ?
                                       settings_->ware_queues.at(index_).priority :
                                       building.get_priority(type_, index_, disambiguator_id_)) :
                  2,
               UI::SliderStyle::kWuiLight,
               "",
               kButtonSize,
               can_act_ && has_priority_),
     spacer_(&hbox_, UI::PanelStyle::kWui, "spacer", 0, 0, priority_.get_w(), priority_.get_h()),
     priority_indicator_(
        &hbox_, UI::PanelStyle::kWui, "priority_indicator", 0, 0, kButtonSize / 5, kButtonSize),
     slider_was_moved_(nullptr),
     collapsed_(collapsed),
     nr_icons_(queue_ != nullptr            ? queue_->get_max_size() :
               type_ == Widelands::wwWORKER ? settings_->worker_queues.at(index_).max_fill :
                                              settings_->ware_queues.at(index_).max_fill),
     max_icons_(nr_icons_),
     icons_(nr_icons_, nullptr),
     total_fill_(&hbox_,
                 UI::PanelStyle::kWui,
                 "total_fill",
                 UI::FontStyle::kWuiLabel,
                 std::string(),
                 UI::Align::kRight),
     fill_index_under_mouse_(-1) {

	assert((queue_ == nullptr) ^ (settings_ == nullptr));

	b_decrease_real_fill_.set_visible((queue_ != nullptr) && ibase_.omnipotent());
	b_increase_real_fill_.set_visible((queue_ != nullptr) && ibase_.omnipotent());

	b_decrease_desired_fill_.set_visible(!show_only_);
	b_increase_desired_fill_.set_visible(!show_only_);
	b_decrease_desired_fill_.set_enabled(can_act_);
	b_increase_desired_fill_.set_enabled(can_act_);

	b_increase_desired_fill_.set_repeating(true);
	b_increase_real_fill_.set_repeating(true);
	b_decrease_desired_fill_.set_repeating(true);
	b_decrease_real_fill_.set_repeating(true);

	vbox_.add_space(kButtonSize / 4);

	hbox_.add(&b_decrease_real_fill_);
	hbox_.add(&b_decrease_desired_fill_);

	for (size_t i = 0; i < nr_icons_; ++i) {
		icons_[i] = new UI::Icon(
		   &hbox_, UI::PanelStyle::kWui, format("icon_%u", i), 0, 0, kButtonSize, kButtonSize,
		   type_ == Widelands::wwWARE ? building.owner().tribe().get_ware_descr(index_)->icon() :
		                                building.owner().tribe().get_worker_descr(index_)->icon());
		hbox_.add(icons_[i]);
	}
	hbox_.add_inf_space();
	hbox_.add(&b_increase_desired_fill_);
	hbox_.add(&b_increase_real_fill_);
	hbox_.add(&total_fill_, UI::Box::Resizing::kAlign, UI::Align::kCenter);

	priority_.set_cursor_fixed_height(kButtonSize * 2 / 3);

	// To make sure the fill buttons are aligned even when some queues
	// have priority buttons and some don't (e.g. in barracks)
	hbox_.add(&priority_indicator_);
	priority_.set_visible(has_priority_);
	spacer_.set_visible(!has_priority_);
	hbox_.add(&priority_, UI::Box::Resizing::kAlign, UI::Align::kCenter);
	hbox_.add(&spacer_);

	vbox_.add(&hbox_, UI::Box::Resizing::kFullSize);
	vbox_.add_space(kButtonSize / 4);
	add(&vbox_, UI::Box::Resizing::kExpandBoth);

	add_space(kButtonSize / 4);
	add(&collapse_);

	if (can_act_) {
		collapse_.sigclicked.connect([this]() {
			*collapsed_ = is_collapsed() ? BuildingWindow::CollapsedState::kExpanded :
			                               BuildingWindow::CollapsedState::kCollapsed;
		});

		b_decrease_desired_fill_.sigclicked.connect([this]() {
			if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
				recurse([](InputQueueDisplay& i) { i.clicked_desired_fill(-1); });
			} else {
				clicked_desired_fill(-1);
			}
		});
		b_increase_desired_fill_.sigclicked.connect([this]() {
			if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
				recurse([](InputQueueDisplay& i) { i.clicked_desired_fill(1); });
			} else {
				clicked_desired_fill(1);
			}
		});
		b_decrease_real_fill_.sigclicked.connect([this]() {
			if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
				recurse([](InputQueueDisplay& i) { i.clicked_real_fill(-1); });
			} else {
				clicked_real_fill(-1);
			}
		});
		b_increase_real_fill_.sigclicked.connect([this]() {
			if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
				recurse([](InputQueueDisplay& i) { i.clicked_real_fill(1); });
			} else {
				clicked_real_fill(1);
			}
		});
		priority_.changedto.connect([this](size_t i) {
			const Widelands::WarePriority& p = index_to_priority(i);
			slider_was_moved_ = &p;
			if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
				recurse([&p](InputQueueDisplay& iqd) { iqd.set_priority(p); });
			} else {
				set_priority(p);
			}
		});
	} else {
		collapse_.set_visible(false);
		*collapsed_ = BuildingWindow::CollapsedState::kCollapsed;
	}

	set_collapsed();

	set_tooltip(type_ == Widelands::wwWARE ?
	               building.owner().tribe().get_ware_descr(index_)->descname() :
	               building.owner().tribe().get_worker_descr(index_)->descname());

	if (nr_icons_ == 0) {
		// Can happen when this is a dropout queue that has already been emptied.
		assert(queue_ != nullptr);
		hide_from_view();
	}

	// Do not call think() yet, it might deadlock
}

void InputQueueDisplay::unlock_desired_fill(bool call_unlock_fn) {
	if (call_unlock_fn) {
		unlock_fn_();
	}

	lock_desired_fill_ = false;
	b_decrease_desired_fill_.set_tooltip(create_tooltip(false));
	b_increase_desired_fill_.set_tooltip(create_tooltip(true));
}

void InputQueueDisplay::lock_desired_fill(const std::string& reason,
                                          const std::string& unlock_title,
                                          const std::string& unlock_body,
                                          std::function<void()> unlock_fn) {
	lock_desired_fill_ = true;
	unlock_fn_ = unlock_fn;
	unlock_title_ = unlock_title;
	unlock_body_ = unlock_body;

	b_decrease_desired_fill_.set_tooltip(reason);
	b_increase_desired_fill_.set_tooltip(reason);
}

void InputQueueDisplay::recurse(const std::function<void(InputQueueDisplay&)>& functor) {
	for (UI::Panel* p = get_parent()->get_first_child(); p != nullptr; p = p->get_next_sibling()) {
		if (upcast(InputQueueDisplay, i, p)) {
			functor(*i);
		}
	}
}

int32_t InputQueueDisplay::fill_index_at(const int32_t x, const int32_t y) const {
	assert(nr_icons_ > 0);
	if (y < hbox_.get_y() || y > hbox_.get_y() + kButtonSize ||
	    x < hbox_.get_x() + icons_[0]->get_x() ||
	    x > hbox_.get_x() + icons_.back()->get_x() + kButtonSize) {
		return -1;
	}

	const uint32_t fill = (x + kButtonSize / 2 - hbox_.get_x() - icons_[0]->get_x()) / kButtonSize;
	assert(fill <= nr_icons_);
	return fill;
}

bool InputQueueDisplay::handle_mousepress(const uint8_t btn, const int32_t x, const int32_t y) {
	const int32_t fill = fill_index_at(x, y);
	if (btn != SDL_BUTTON_LEFT || fill < 0 || show_only_ || !can_act_) {
		return UI::Box::handle_mousepress(btn, x, y);
	}

	if ((SDL_GetModState() & KMOD_SHIFT) != 0) {
		recurse([fill](InputQueueDisplay& i) { i.set_desired_fill(fill); });
	} else {
		set_desired_fill(fill);
	}

	return true;
}

bool InputQueueDisplay::handle_mousemove(
   uint8_t /*state*/, const int32_t x, const int32_t y, int32_t /*xdiff*/, int32_t /*ydiff*/) {
	fill_index_under_mouse_ = fill_index_at(x, y);
	return true;
}

void InputQueueDisplay::handle_mousein(bool inside) {
	if (!inside) {
		fill_index_under_mouse_ = -1;
	}
	UI::Box::handle_mousein(inside);
}

bool InputQueueDisplay::handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) {
	if (show_only_ || !can_act_) {
		return false;
	}
	bool big_step = false;
	int32_t change = get_mousewheel_change(MousewheelHandlerConfigID::kChangeValue, x, y,
	                                       // shift has special meaning, prevent it to work
	                                       // as part of modifier
	                                       modstate & ~KMOD_SHIFT);
	if (change == 0) {
		// Try big step
		change = get_mousewheel_change(
		   MousewheelHandlerConfigID::kChangeValueBig, x, y, modstate & ~KMOD_SHIFT);
		if (change == 0) {
			return false;
		}
		big_step = true;
	}

	if (get_mouse_position().x < priority_indicator_.get_x()) {
		// Mouse is over desired fill

		if (big_step) {
			change *= ChangeBigStep::kSmallRange;
		}

		if ((modstate & KMOD_SHIFT) != 0) {
			recurse([change](InputQueueDisplay& i) { i.change_desired_fill(change); });
		} else {
			change_desired_fill(change);
		}
		return true;
	}
	if (has_priority_) {
		// Mouse is over priority or collapse button
		// Can't just use method from Slider, because of the special
		// meaning of shift to change all input priorities together.

		if (big_step) {
			// This makes the steps: Very Low -> Normal -> Very High
			change *= 2;
		}

		// KMOD_SHIFT + changedto is already connected to recurse.
		priority_.change_value_by(change);
		return true;
	}

	return false;
}

bool UI::PrioritySlider::handle_key(bool down, SDL_Keysym code) {
	// shift has special meaning, prevent it to work as part of modifier
	const uint16_t filtered_keymod = code.mod & ~KMOD_SHIFT;
	SDL_Keysym filtered_code = {code.scancode, code.sym, filtered_keymod, code.unused};

	// If this changes the slider's value while Shift is pressed, then KMOD_SHIFT + changedto
	// takes care of changing the other ones too.
	return Slider::handle_key(down, filtered_code);
}

void InputQueueDisplay::set_priority(const Widelands::WarePriority& priority) {
	MutexLock m(MutexLock::ID::kObjects);
	Widelands::Building* b = building_.get(ibase_.egbase());
	if (b == nullptr) {
		return;
	}

	if (!can_act_ || !has_priority_ ||
	    priority == (queue_ != nullptr ? b->get_priority(type_, index_, disambiguator_id_) :
	                                     get_setting()->priority)) {
		return;
	}

	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_ware_priority(
		   *b, type_, index_, priority, settings_ != nullptr, disambiguator_id_);
	} else {
		if (queue_ != nullptr) {
			b->set_priority(type_, index_, priority, disambiguator_id_);
		} else {
			get_setting()->priority = priority;
		}
	}
}

void InputQueueDisplay::clicked_desired_fill(const int8_t delta) {
	assert(delta == 1 || delta == -1);
	MutexLock m(MutexLock::ID::kObjects);
	Widelands::Building* b = building_.get(ibase_.egbase());
	if (b == nullptr) {
		return;
	}

	const bool ctrl_down = (SDL_GetModState() & KMOD_CTRL) != 0;

	if (lock_desired_fill_) {
		if (!ctrl_down) {
			upcast(InteractivePlayer, ipl, &ibase_);
			assert(ipl != nullptr);
			show_generic_callback_confirm(*ipl, b, unlock_title_, unlock_body_, [this, delta]() {
				unlock_desired_fill(true);
				clicked_desired_fill(delta);
			});
			return;
		}
		unlock_desired_fill(true);
	}

	const unsigned desired_fill =
	   queue_ != nullptr ? queue_->get_max_fill() : get_setting()->desired_fill;
	const unsigned max_fill = queue_ != nullptr ? queue_->get_max_size() : get_setting()->max_fill;
	assert(desired_fill <= max_fill);

	if (!can_act_ || desired_fill == (delta < 0 ? 0 : max_fill)) {
		return;
	}

	const unsigned new_fill = ctrl_down ? delta < 0 ? 0 : max_fill : desired_fill + delta;

	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_input_max_fill(
		   *b, index_, type_, new_fill, settings_ != nullptr, disambiguator_id_);
	} else {
		if (queue_ != nullptr) {
			queue_->set_max_fill(new_fill);
		} else {
			get_setting()->desired_fill = new_fill;
		}
	}
}

void InputQueueDisplay::change_desired_fill(const int8_t delta) {
	if (delta == 0) {
		return;
	}

	MutexLock m(MutexLock::ID::kObjects);
	Widelands::Building* b = building_.get(ibase_.egbase());
	if (b == nullptr) {
		return;
	}

	if (lock_desired_fill_) {
		const bool ctrl_down = (SDL_GetModState() & KMOD_CTRL) != 0;
		if (!ctrl_down) {
			upcast(InteractivePlayer, ipl, &ibase_);
			assert(ipl != nullptr);
			show_generic_callback_confirm(*ipl, b, unlock_title_, unlock_body_, [this, delta]() {
				unlock_desired_fill(true);
				change_desired_fill(delta);
			});
			return;
		}
		unlock_desired_fill(true);
	}

	unsigned desired_fill = queue_ != nullptr ? queue_->get_max_fill() : get_setting()->desired_fill;
	const unsigned max_fill = queue_ != nullptr ? queue_->get_max_size() : get_setting()->max_fill;
	assert(desired_fill <= max_fill);

	if (!can_act_ || desired_fill == (delta < 0 ? 0 : max_fill)) {
		return;
	}

	if (delta < 0 && static_cast<int>(desired_fill) <= -delta) {
		desired_fill = 0;
	} else {
		desired_fill += delta;
		if (desired_fill > max_fill) {
			desired_fill = max_fill;
		}
	}

	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_input_max_fill(
		   *b, index_, type_, desired_fill, settings_ != nullptr, disambiguator_id_);
	} else {
		if (queue_ != nullptr) {
			queue_->set_max_fill(desired_fill);
		} else {
			get_setting()->desired_fill = desired_fill;
		}
	}
}

void InputQueueDisplay::set_desired_fill(unsigned new_fill) {
	MutexLock m(MutexLock::ID::kObjects);
	Widelands::Building* b = building_.get(ibase_.egbase());
	if (b == nullptr) {
		return;
	}

	if (lock_desired_fill_) {
		const bool ctrl_down = (SDL_GetModState() & KMOD_CTRL) != 0;
		if (!ctrl_down) {
			upcast(InteractivePlayer, ipl, &ibase_);
			assert(ipl != nullptr);
			show_generic_callback_confirm(*ipl, b, unlock_title_, unlock_body_, [this, new_fill]() {
				unlock_desired_fill(true);
				set_desired_fill(new_fill);
			});
			return;
		}
		unlock_desired_fill(true);
	}

	const unsigned desired_fill =
	   queue_ != nullptr ? queue_->get_max_fill() : get_setting()->desired_fill;
	const unsigned max_fill = queue_ != nullptr ? queue_->get_max_size() : get_setting()->max_fill;
	assert(desired_fill <= max_fill);
	new_fill = std::min(new_fill, max_fill);

	if (!can_act_ || desired_fill == new_fill) {
		return;
	}

	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_input_max_fill(
		   *b, index_, type_, new_fill, settings_ != nullptr, disambiguator_id_);
	} else {
		if (queue_ != nullptr) {
			queue_->set_max_fill(new_fill);
		} else {
			get_setting()->desired_fill = new_fill;
		}
	}
}

void InputQueueDisplay::clicked_real_fill(const int8_t delta) {
	assert(delta == 1 || delta == -1);
	MutexLock m(MutexLock::ID::kObjects);

	if ((queue_ == nullptr) || !ibase_.omnipotent() || (building_.get(ibase_.egbase()) == nullptr)) {
		return;
	}

	const unsigned real_fill = queue_->get_filled();
	const unsigned max_fill = queue_->get_max_size();
	assert(real_fill <= max_fill);

	if (real_fill == (delta < 0 ? 0 : max_fill)) {
		return;
	}

	const bool ctrl_down = (SDL_GetModState() & KMOD_CTRL) != 0;
	const unsigned new_fill = ctrl_down ? delta < 0 ? 0 : max_fill : real_fill + delta;

	queue_->set_filled(new_fill);
}

void InputQueueDisplay::set_collapsed() {
	priority_.set_visible(has_priority_ && !is_collapsed());
	spacer_.set_visible(!has_priority_ && !is_collapsed());
	b_decrease_desired_fill_.set_visible(!is_collapsed() && !show_only_);
	b_increase_desired_fill_.set_visible(!is_collapsed() && !show_only_);
	b_decrease_real_fill_.set_visible(!is_collapsed() && ibase_.omnipotent());
	b_increase_real_fill_.set_visible(!is_collapsed() && ibase_.omnipotent());
	collapse_.set_tooltip(is_collapsed() ? _("Show controls") : _("Hide controls"));
	collapse_.set_pic(g_image_cache->get(is_collapsed() ? "images/ui_basic/scrollbar_right.png" :
	                                                      "images/ui_basic/scrollbar_left.png"));
}

inline Widelands::ProductionsiteSettings::InputQueueSetting*
InputQueueDisplay::get_setting() const {
	return settings_ != nullptr ? type_ == Widelands::wwWARE ? &settings_->ware_queues.at(index_) :
	                                                           &settings_->worker_queues.at(index_) :
	                              nullptr;
}

void InputQueueDisplay::set_max_icons(size_t m) {
	max_icons_ = m;
}

static const RGBAColor kColorComing(127, 127, 127, 191);
static const RGBAColor kColorMissing(191, 191, 191, 127);

/** Prevent the queue from being drawn and receiving UI events. */
void InputQueueDisplay::hide_from_view() {
	set_visible(false);
	set_thinks(false);
}

void InputQueueDisplay::think() {
	MutexLock m(MutexLock::ID::kObjects);
	Widelands::Building* b = building_.get(ibase_.egbase());
	if (b == nullptr) {
		return;
	}

	if ((queue_ != nullptr) && queue_->get_max_size() == 0) {
		hide_from_view();
		return;
	}

	const Widelands::ProductionsiteSettings::InputQueueSetting* setting = get_setting();
	const unsigned max_fill = queue_ != nullptr ? queue_->get_max_size() : setting->max_fill;
	const unsigned real_fill = queue_ != nullptr ? queue_->get_filled() : setting->desired_fill;
	const unsigned desired_fill = queue_ != nullptr ? queue_->get_max_fill() : setting->desired_fill;
	const unsigned nr_missing = queue_ != nullptr ? queue_->get_missing() : 0;
	const unsigned nr_coming =
	   (queue_ != nullptr) && real_fill < desired_fill ? desired_fill - real_fill - nr_missing : 0;

	assert(desired_fill <= max_fill);
	assert(real_fill <= max_fill);
	assert(nr_missing <= max_fill - real_fill);
	assert(nr_coming <= max_fill - real_fill);
	assert(max_fill <= nr_icons_);

	for (unsigned i = 0; i < nr_icons_; ++i) {
		icons_[i]->set_visible(i < max_fill && i < max_icons_);
		icons_[i]->set_grey_out(i >= real_fill);
		icons_[i]->set_grey_out_color(i < real_fill + nr_coming ? kColorComing : kColorMissing);
	}
	total_fill_.set_text(max_fill > max_icons_ ? format_l(_("+%u"), max_fill - max_icons_) :
	                                             std::string());

	if (has_priority_) {
		const Widelands::WarePriority& p = queue_ != nullptr ?  // NOLINT
		                                      b->get_priority(type_, index_, disambiguator_id_) :
		                                      get_setting()->priority;
		// The purpose of this check is to prevent the slider from snapping back directly after
		// the user dragged it, because the playercommand is not executed immediately of course
		if ((slider_was_moved_ == nullptr) || *slider_was_moved_ == p) {
			priority_.set_value(priority_to_index(p));
			slider_was_moved_ = nullptr;
		}
		priority_.set_tooltip(priority_tooltip(priority_.get_value()));
	}

	set_collapsed();
}

static const RGBAColor kPriorityColors[] = {RGBAColor(0, 0, 255, 127), RGBAColor(63, 127, 255, 127),
                                            RGBAColor(255, 255, 0, 127),
                                            RGBAColor(255, 127, 0, 127), RGBAColor(255, 0, 0, 127)};

void InputQueueDisplay::draw(RenderTarget& r) {
	// Draw priority indicator
	if (has_priority_ && !is_collapsed()) {
		const int x = hbox_.get_x() + priority_.get_x();
		for (size_t i = 0; i < 5; ++i) {
			r.fill_rect(Recti(x + i * kButtonSize, hbox_.get_y() + kButtonSize * 2 / 5, kButtonSize,
			                  kButtonSize / 5),
			            kPriorityColors[i], BlendMode::Default);
		}
	}

	UI::Box::draw(r);
}

void InputQueueDisplay::draw_overlay(RenderTarget& r) {
	MutexLock m(MutexLock::ID::kObjects);
	Widelands::Building* b = building_.get(ibase_.egbase());
	if (b == nullptr) {
		return;
	}

	// Draw max fill indicator
	if (!show_only_) {
		assert(nr_icons_ > 0);
		const unsigned desired_fill =
		   queue_ != nullptr ? queue_->get_max_fill() : get_setting()->desired_fill;
		assert(desired_fill <= nr_icons_);

		auto calc_xpos = [this](const size_t fill) {
			return (fill == 0 ? icons_[0]->get_x() :
			        fill == nr_icons_ ?
			                    icons_[nr_icons_ - 1]->get_x() + icons_[nr_icons_ - 1]->get_w() :
			                    (icons_[fill - 1]->get_x() + icons_[fill - 1]->get_w() +
			                     icons_[fill]->get_x()) /
			                       2) +
			       hbox_.get_x() - max_fill_indicator_.width() / 2;
		};

		const int ypos = hbox_.get_y() + icons_[0]->get_y() +
		                 (icons_[0]->get_h() - max_fill_indicator_.height()) / 2;
		r.blit(Vector2i(calc_xpos(desired_fill), ypos), &max_fill_indicator_);

		if (can_act_ && fill_index_under_mouse_ >= 0 &&
		    (!lock_desired_fill_ || (SDL_GetModState() & KMOD_CTRL) != 0)) {
			r.blitrect_scale(Rectf(calc_xpos(fill_index_under_mouse_), ypos,
			                       max_fill_indicator_.width(), max_fill_indicator_.height()),
			                 &max_fill_indicator_, max_fill_indicator_.rect(), 0.4f,
			                 BlendMode::Default);
		}
	}

	// Draw priority indicator
	if (has_priority_ && is_collapsed()) {
		const size_t p =
		   priority_to_index(queue_ != nullptr ? b->get_priority(type_, index_, disambiguator_id_) :
		                                         get_setting()->priority);
		const int w = priority_indicator_.get_w();
		// Add kButtonSize / 4 to the position to align it against the collapse button
		const int x = hbox_.get_x() + priority_indicator_.get_x() + kButtonSize / 4;
		r.brighten_rect(Recti(x, hbox_.get_y(), w, kButtonSize), -32);
		r.fill_rect(Recti(x, hbox_.get_y() + (4 - p) * w, w, w), kPriorityColors[p], BlendMode::Copy);
	}

	UI::Box::draw_overlay(r);
}
