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

constexpr int8_t kButtonSize = 25;

// The slider indexes the available priorities as 0..4, so here are some conversion functions
static size_t priority_to_index(const Widelands::WarePriority& priority) {
	if (priority == Widelands::WarePriority::kVeryLow) {
		return 0;
	} else if (priority == Widelands::WarePriority::kLow) {
		return 1;
	} else if (priority == Widelands::WarePriority::kNormal) {
		return 2;
	} else if (priority == Widelands::WarePriority::kHigh) {
		return 3;
	} else if (priority == Widelands::WarePriority::kVeryHigh) {
		return 4;
	}
	// TODO(Nordfriese): For savegame compatibility. Replace with NEVER_HERE() after v1.0
	return 1;
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
	while (p->get_parent()) {
		p = p->get_parent();
	}
	b.set_max_size(p->get_w() - 200, p->get_h() - 200);
	b.set_scrolling(true);
}

InputQueueDisplay::InputQueueDisplay(UI::Panel* parent,
                                     InteractiveBase& ib,
                                     Widelands::Building& bld,
                                     Widelands::InputQueue& q,
                                     bool show_only,
                                     bool has_priority)
   : InputQueueDisplay(
        parent, ib, bld, q.get_type(), q.get_index(), &q, nullptr, show_only, has_priority) {
}
InputQueueDisplay::InputQueueDisplay(UI::Panel* parent,
                                     InteractiveBase& ib,
                                     Widelands::ConstructionSite& csite,
                                     Widelands::WareWorker ww,
                                     Widelands::DescriptionIndex di)
   : InputQueueDisplay(parent,
                       ib,
                       csite,
                       ww,
                       di,
                       nullptr,
                       dynamic_cast<Widelands::ProductionsiteSettings*>(csite.get_settings()),
                       false,
                       true) {
}

static inline std::string create_tooltip(const bool increase) {
	return (boost::format("<p>%s%s%s</p>") %
	        g_style_manager->font_style(UI::FontStyle::kWuiTooltipHeader)
	           .as_font_tag(
	              increase ?
	                 /** TRANSLATORS: Button tooltip in in a building's wares input queue */
	                 _("Increase the number of wares you want to be stored here") :
	                 /** TRANSLATORS: Button tooltip in in a building's wares input queue */
	                 _("Decrease the number of wares you want to be stored here")) %
	        as_listitem(increase ?
	                       /** TRANSLATORS: Button tooltip in in a building's wares input queue -
	                          option explanation */
	                       _("Hold down Shift to increase all ware types at the same time") :
	                       /** TRANSLATORS: Button tooltip in in a building's wares input queue -
	                          option explanation */
	                       _("Hold down Shift to decrease all ware types at the same time"),
	                    UI::FontStyle::kWuiTooltip) %
	        as_listitem(increase ?
	                       /** TRANSLATORS: Button tooltip in in a building's wares input queue -
	                          option explanation */
	                       _("Hold down Ctrl to allow all of this ware") :
	                       /** TRANSLATORS: Button tooltip in in a building's wares input queue -
	                          option explanation */
	                       _("Hold down Ctrl to allow none of this ware"),
	                    UI::FontStyle::kWuiTooltip))
	   .str();
}

InputQueueDisplay::InputQueueDisplay(UI::Panel* parent,
                                     InteractiveBase& ib,
                                     Widelands::Building& bld,
                                     Widelands::WareWorker ww,
                                     Widelands::DescriptionIndex idx,
                                     Widelands::InputQueue* q,
                                     Widelands::ProductionsiteSettings* s,
                                     bool show_only,
                                     bool has_priority)
   : UI::Box(parent, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
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
     vbox_(this, UI::PanelStyle::kWui, 0, 0, UI::Box::Vertical),
     hbox_(&vbox_, UI::PanelStyle::kWui, 0, 0, UI::Box::Horizontal),
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
                  priority_to_index(settings_ ? settings_->ware_queues.at(index_).priority :
                                                building_.get_priority(type_, index_)) :
                  2,
               UI::SliderStyle::kWuiLight,
               "",
               kButtonSize,
               can_act_ && has_priority_),
     spacer_(&hbox_, UI::PanelStyle::kWui, 0, 0, priority_.get_w(), priority_.get_h()),
     slider_was_moved_(nullptr),
     collapsed_(false),
     nr_icons_(queue_ ?
                  queue_->get_max_size() :
                  type_ == Widelands::wwWORKER ? settings_->worker_queues.at(index_).max_fill :
                                                 settings_->ware_queues.at(index_).max_fill),
     icons_(nr_icons_, nullptr),
     fill_index_under_mouse_(-1) {

	assert((queue_ == nullptr) ^ (settings_ == nullptr));

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

	vbox_.add_space(kButtonSize / 4);

	hbox_.add(&b_decrease_real_fill_);
	hbox_.add(&b_decrease_desired_fill_);

	for (size_t i = 0; i < nr_icons_; ++i) {
		icons_[i] = new UI::Icon(&hbox_, UI::PanelStyle::kWui, 0, 0, kButtonSize, kButtonSize,
		                         type_ == Widelands::wwWARE ?
		                            building_.owner().tribe().get_ware_descr(index_)->icon() :
		                            building_.owner().tribe().get_worker_descr(index_)->icon());
		hbox_.add(icons_[i]);
	}

	hbox_.add_inf_space();
	hbox_.add(&b_increase_desired_fill_);
	hbox_.add(&b_increase_real_fill_);

	priority_.set_cursor_fixed_height(kButtonSize * 2 / 3);

	// To make sure the fill buttons are aligned even when some queues
	// have priority buttons and some don't (e.g. in barracks)
	hbox_.add_space(kButtonSize / 4);
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
			const bool c = !collapsed_;
			recurse([c](InputQueueDisplay& i) { i.set_collapsed(c); });
		});

		b_decrease_desired_fill_.sigclicked.connect([this]() {
			if (SDL_GetModState() & KMOD_SHIFT) {
				recurse([](InputQueueDisplay& i) { i.clicked_desired_fill(-1); });
			} else {
				clicked_desired_fill(-1);
			}
		});
		b_increase_desired_fill_.sigclicked.connect([this]() {
			if (SDL_GetModState() & KMOD_SHIFT) {
				recurse([](InputQueueDisplay& i) { i.clicked_desired_fill(1); });
			} else {
				clicked_desired_fill(1);
			}
		});
		b_decrease_real_fill_.sigclicked.connect([this]() {
			if (SDL_GetModState() & KMOD_SHIFT) {
				recurse([](InputQueueDisplay& i) { i.clicked_real_fill(-1); });
			} else {
				clicked_real_fill(-1);
			}
		});
		b_increase_real_fill_.sigclicked.connect([this]() {
			if (SDL_GetModState() & KMOD_SHIFT) {
				recurse([](InputQueueDisplay& i) { i.clicked_real_fill(1); });
			} else {
				clicked_real_fill(1);
			}
		});
		priority_.changedto.connect([this](size_t i) {
			const Widelands::WarePriority& p = index_to_priority(i);
			slider_was_moved_ = &p;
			if (SDL_GetModState() & KMOD_SHIFT) {
				recurse([&p](InputQueueDisplay& iqd) { iqd.set_priority(p); });
			} else {
				set_priority(p);
			}
		});
	} else {
		collapse_.set_visible(false);
		set_collapsed(true);
	}

	set_tooltip(type_ == Widelands::wwWARE ?
	               building_.owner().tribe().get_ware_descr(index_)->descname() :
	               building_.owner().tribe().get_worker_descr(index_)->descname());

	// Initialize tooltips, icon colours and stuff
	think();
}

void InputQueueDisplay::recurse(const std::function<void(InputQueueDisplay&)>& functor) {
	for (UI::Panel* p = get_parent()->get_first_child(); p; p = p->get_next_sibling()) {
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

	if (SDL_GetModState() & KMOD_SHIFT) {
		recurse([fill](InputQueueDisplay& i) { i.set_desired_fill(fill); });
	} else {
		set_desired_fill(fill);
	}

	return true;
}

bool InputQueueDisplay::handle_mousemove(
   uint8_t, const int32_t x, const int32_t y, int32_t, int32_t) {
	fill_index_under_mouse_ = fill_index_at(x, y);
	return true;
}

void InputQueueDisplay::set_priority(const Widelands::WarePriority& priority) {
	if (!can_act_ || !has_priority_ ||
	    priority == (queue_ ? building_.get_priority(type_, index_) : get_setting()->priority)) {
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

void InputQueueDisplay::clicked_desired_fill(const int8_t delta) {
	assert(delta == 1 || delta == -1);

	const unsigned desired_fill = queue_ ? queue_->get_max_fill() : get_setting()->desired_fill;
	const unsigned max_fill = queue_ ? queue_->get_max_size() : get_setting()->max_fill;
	assert(desired_fill <= max_fill);

	if (!can_act_ || desired_fill == (delta < 0 ? 0 : max_fill)) {
		return;
	}

	const bool ctrl_down = SDL_GetModState() & KMOD_CTRL;
	const unsigned new_fill = ctrl_down ? delta < 0 ? 0 : max_fill : desired_fill + delta;

	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_input_max_fill(
		   building_, index_, type_, new_fill, settings_ != nullptr);
	} else {
		if (queue_) {
			queue_->set_max_fill(new_fill);
		} else {
			get_setting()->desired_fill = new_fill;
		}
	}
}

void InputQueueDisplay::set_desired_fill(unsigned new_fill) {
	const unsigned desired_fill = queue_ ? queue_->get_max_fill() : get_setting()->desired_fill;
	const unsigned max_fill = queue_ ? queue_->get_max_size() : get_setting()->max_fill;
	assert(desired_fill <= max_fill);
	new_fill = std::min(new_fill, max_fill);

	if (!can_act_ || desired_fill == new_fill) {
		return;
	}

	if (Widelands::Game* game = ibase_.get_game()) {
		game->send_player_set_input_max_fill(
		   building_, index_, type_, new_fill, settings_ != nullptr);
	} else {
		if (queue_) {
			queue_->set_max_fill(new_fill);
		} else {
			get_setting()->desired_fill = new_fill;
		}
	}
}

void InputQueueDisplay::clicked_real_fill(const int8_t delta) {
	assert(delta == 1 || delta == -1);

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

void InputQueueDisplay::set_collapsed(const bool c) {
	assert(collapsed_ != c);
	collapsed_ = c;
	priority_.set_visible(has_priority_ && !collapsed_);
	spacer_.set_visible(!has_priority_ && !collapsed_);
	b_decrease_desired_fill_.set_visible(!collapsed_ && !show_only_);
	b_increase_desired_fill_.set_visible(!collapsed_ && !show_only_);
	b_decrease_real_fill_.set_visible(!collapsed_ && ibase_.omnipotent());
	b_increase_real_fill_.set_visible(!collapsed_ && ibase_.omnipotent());
}

inline Widelands::ProductionsiteSettings::InputQueueSetting*
InputQueueDisplay::get_setting() const {
	return settings_ ? type_ == Widelands::wwWARE ? &settings_->ware_queues.at(index_) :
	                                                &settings_->worker_queues.at(index_) :
	                   nullptr;
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
	const unsigned nr_coming =
	   queue_ && real_fill < desired_fill ? desired_fill - real_fill - nr_missing : 0;

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
		const Widelands::WarePriority& p =                                            // NOLINT
		   queue_ ? building_.get_priority(type_, index_) : get_setting()->priority;  // NOLINT
		// The purpose of this check is to prevent the slider from snapping back directly after
		// the user dragged it, because the playercommand is not executed immediately of course
		if (!slider_was_moved_ || *slider_was_moved_ == p) {
			priority_.set_value(priority_to_index(p));
			slider_was_moved_ = nullptr;
		}
		priority_.set_tooltip(priority_tooltip(priority_.get_value()));
	}

	collapse_.set_tooltip(collapsed_ ? _("Show controls") : _("Hide controls"));
	collapse_.set_pic(g_image_cache->get(collapsed_ ? "images/ui_basic/scrollbar_right.png" :
	                                                  "images/ui_basic/scrollbar_left.png"));
}

static const RGBAColor kPriorityColors[] = {RGBAColor(255, 0, 0, 127), RGBAColor(255, 127, 0, 127),
                                            RGBAColor(255, 255, 0, 127), RGBAColor(0, 255, 0, 127),
                                            RGBAColor(0, 127, 255, 127)};

void InputQueueDisplay::draw(RenderTarget& r) {
	// Draw priority indicator
	if (has_priority_ && !collapsed_) {
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
	// Draw max fill indicator
	if (!show_only_) {
		assert(nr_icons_);
		const unsigned desired_fill = queue_ ? queue_->get_max_fill() : get_setting()->desired_fill;
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

		if (can_act_ && fill_index_under_mouse_ >= 0) {
			r.blitrect_scale(Rectf(calc_xpos(fill_index_under_mouse_), ypos,
			                       max_fill_indicator_.width(), max_fill_indicator_.height()),
			                 &max_fill_indicator_,
			                 Recti(0, 0, max_fill_indicator_.width(), max_fill_indicator_.height()),
			                 0.4f, BlendMode::Default);
		}
	}

	// Draw priority indicator
	if (has_priority_ && collapsed_) {
		const size_t p = priority_to_index(queue_ ? building_.get_priority(type_, index_) :
		                                            get_setting()->priority);
		const int w = kButtonSize / 5;
		const int x = hbox_.get_x() + collapse_.get_x() - w;
		r.brighten_rect(Recti(x, hbox_.get_y(), w, kButtonSize), -32);
		r.fill_rect(Recti(x, hbox_.get_y() + (4 - p) * kButtonSize / 5, w, kButtonSize / 5),
		            kPriorityColors[p], BlendMode::Copy);
	}

	UI::Box::draw_overlay(r);
}
