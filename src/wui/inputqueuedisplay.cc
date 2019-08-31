/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#include <algorithm>

#include <boost/format.hpp>

#include "economy/input_queue.h"
#include "economy/request.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "logic/player.h"
#include "wui/interactive_gamebase.h"

static char const* pic_priority_low = "images/wui/buildings/low_priority_button.png";
static char const* pic_priority_normal = "images/wui/buildings/normal_priority_button.png";
static char const* pic_priority_high = "images/wui/buildings/high_priority_button.png";
static char const* pic_max_fill_indicator = "images/wui/buildings/max_fill_indicator.png";

InputQueueDisplay::InputQueueDisplay(UI::Panel* const parent,
                                     int32_t const x,
                                     int32_t const y,
                                     InteractiveGameBase& igb,
                                     Widelands::Building& building,
                                     const Widelands::InputQueue& queue,
                                     bool show_only)
   : UI::Panel(parent, x, y, 0, 28),
     igb_(igb),
     building_(building),
     queue_(&queue),
     settings_(nullptr),
     priority_radiogroup_(nullptr),
     increase_max_fill_(nullptr),
     decrease_max_fill_(nullptr),
     index_(queue.get_index()),
     type_(queue.get_type()),
     max_fill_indicator_(g_gr->images().get(pic_max_fill_indicator)),
     cache_size_(queue.get_max_size()),
     cache_max_fill_(queue.get_max_fill()),
     total_height_(0),
     show_only_(show_only) {
	if (type_ == Widelands::wwWARE) {
		const Widelands::WareDescr& ware = *queue.owner().tribe().get_ware_descr(queue_->get_index());
		set_tooltip(ware.descname().c_str());
		icon_ = ware.icon();
	} else {
		const Widelands::WorkerDescr& worker =
		   *queue.owner().tribe().get_worker_descr(queue_->get_index());
		set_tooltip(worker.descname().c_str());
		icon_ = worker.icon();
	}

	uint16_t ph = max_fill_indicator_->height();

	uint32_t priority_button_height = show_only ? 0 : 3 * PriorityButtonSize;
	uint32_t image_height =
	   show_only ? kWareMenuPicHeight : std::max<int32_t>(kWareMenuPicHeight, ph);

	total_height_ = std::max(priority_button_height, image_height) + 2 * Border;

	max_size_changed();

	set_thinks(true);
}

InputQueueDisplay::InputQueueDisplay(UI::Panel* const parent,
                                     int32_t const x,
                                     int32_t const y,
                                     InteractiveGameBase& igb,
                                     Widelands::ConstructionSite& building,
                                     Widelands::WareWorker ww,
                                     Widelands::DescriptionIndex di,
                                     bool show_only)
   : UI::Panel(parent, x, y, 0, 28),
     igb_(igb),
     building_(building),
     queue_(nullptr),
     settings_(dynamic_cast<const Widelands::ProductionsiteSettings*>(building.get_settings())),
     priority_radiogroup_(nullptr),
     increase_max_fill_(nullptr),
     decrease_max_fill_(nullptr),
     index_(di),
     type_(ww),
     max_fill_indicator_(g_gr->images().get(pic_max_fill_indicator)),
     total_height_(0),
     show_only_(show_only) {
	cache_size_ = check_max_size();
	cache_max_fill_ = check_max_fill();
	if (type_ == Widelands::wwWARE) {
		const Widelands::WareDescr& ware = *building.owner().tribe().get_ware_descr(index_);
		set_tooltip(ware.descname().c_str());
		icon_ = ware.icon();
	} else {
		const Widelands::WorkerDescr& worker = *building.owner().tribe().get_worker_descr(index_);
		set_tooltip(worker.descname().c_str());
		icon_ = worker.icon();
	}

	uint16_t ph = max_fill_indicator_->height();

	uint32_t priority_button_height = show_only ? 0 : 3 * PriorityButtonSize;
	uint32_t image_height =
	   show_only ? kWareMenuPicHeight : std::max<int32_t>(kWareMenuPicHeight, ph);

	total_height_ = std::max(priority_button_height, image_height) + 2 * Border;

	max_size_changed();

	set_thinks(true);
}

InputQueueDisplay::~InputQueueDisplay() {
	delete priority_radiogroup_;
}

uint32_t InputQueueDisplay::check_max_size() const {
	if (queue_) {
		return queue_->get_max_size();
	}
	assert(settings_);
	for (const auto& pair :
	     type_ == Widelands::wwWARE ? settings_->ware_queues : settings_->worker_queues) {
		if (pair.first == index_) {
			return pair.second.max_fill;
		}
	}
	NEVER_HERE();
}

uint32_t InputQueueDisplay::check_max_fill() const {
	if (queue_) {
		return queue_->get_max_fill();
	}
	assert(settings_);
	for (const auto& pair :
	     type_ == Widelands::wwWARE ? settings_->ware_queues : settings_->worker_queues) {
		if (pair.first == index_) {
			return pair.second.desired_fill;
		}
	}
	NEVER_HERE();
}

/**
 * Recalculate the panel's size based on the size of the queue.
 *
 * This is useful for construction sites, whose queues shrink over time.
 */
void InputQueueDisplay::max_size_changed() {
	uint32_t pbs = show_only_ ? 0 : PriorityButtonSize;
	uint32_t ctrl_b_size = show_only_ ? 0 : 2 * kWareMenuPicWidth;

	cache_size_ = check_max_size();

	update_priority_buttons();
	update_max_fill_buttons();

	if (cache_size_ <= 0) {
		set_desired_size(0, 0);
	} else {
		set_desired_size(
		   cache_size_ * (CellWidth + CellSpacing) + pbs + ctrl_b_size + 2 * Border, total_height_);
	}
}

/**
 * Compare the current InputQueue state with the cached state; update if necessary.
 */
void InputQueueDisplay::think() {
	if (static_cast<uint32_t>(check_max_size()) != cache_size_)
		max_size_changed();

	// TODO(sirver): It seems cache_max_fill_ is not really useful for anything.
	if (static_cast<uint32_t>(check_max_fill()) != cache_max_fill_) {
		cache_max_fill_ = check_max_fill();
		compute_max_fill_buttons_enabled_state();
	}
}

/**
 * Render the current InputQueue state.
 */
void InputQueueDisplay::draw(RenderTarget& dst) {
	if (!cache_size_)
		return;

	cache_max_fill_ = check_max_fill();

	uint32_t nr_inputs_to_draw =
	   queue_ ? std::min(queue_->get_filled(), cache_size_) : cache_max_fill_;
	uint32_t nr_missing_to_draw =
	   queue_ ? std::min(queue_->get_missing(), cache_max_fill_) + cache_size_ - cache_max_fill_ :
	            cache_size_ - cache_max_fill_;
	if (nr_inputs_to_draw > cache_max_fill_) {
		nr_missing_to_draw -= nr_inputs_to_draw - cache_max_fill_;
	}
	uint32_t nr_coming_to_draw = cache_size_ - nr_inputs_to_draw - nr_missing_to_draw;
	assert(nr_inputs_to_draw + nr_missing_to_draw + nr_coming_to_draw == cache_size_);

	Vector2i point = Vector2i::zero();
	point.x = Border + (show_only_ ? 0 : CellWidth + CellSpacing);
	point.y = Border + (total_height_ - 2 * Border - kWareMenuPicHeight) / 2;

	for (; nr_inputs_to_draw; --nr_inputs_to_draw, point.x += CellWidth + CellSpacing) {
		dst.blitrect(Vector2i(point.x, point.y), icon_, Recti(0, 0, icon_->width(), icon_->height()),
		             BlendMode::UseAlpha);
	}
	for (; nr_coming_to_draw; --nr_coming_to_draw, point.x += CellWidth + CellSpacing) {
		dst.blitrect_scale_monochrome(Rectf(point.x, point.y, icon_->width(), icon_->height()), icon_,
		                              Recti(0, 0, icon_->width(), icon_->height()),
		                              RGBAColor(127, 127, 127, 191));
	}
	for (; nr_missing_to_draw; --nr_missing_to_draw, point.x += CellWidth + CellSpacing) {
		dst.blitrect_scale_monochrome(Rectf(point.x, point.y, icon_->width(), icon_->height()), icon_,
		                              Recti(0, 0, icon_->width(), icon_->height()),
		                              RGBAColor(191, 191, 191, 127));
	}

	if (!show_only_) {
		uint16_t pw = max_fill_indicator_->width();
		point.y = Border;
		point.x = Border + CellWidth + CellSpacing + (cache_max_fill_ * (CellWidth + CellSpacing)) -
		          CellSpacing / 2 - pw / 2;
		dst.blit(point, max_fill_indicator_);
	}
}

/**
 * Updates priority buttons of the InputQueue
 */
void InputQueueDisplay::update_priority_buttons() {

	if (type_ != Widelands::wwWARE)
		return;

	if (cache_size_ <= 0 || show_only_) {
		delete priority_radiogroup_;
		priority_radiogroup_ = nullptr;
	}

	Vector2i pos = Vector2i(cache_size_ * CellWidth + Border, 0);
	pos.x = (cache_size_ + 2) * (CellWidth + CellSpacing) + Border;
	pos.y = Border + (total_height_ - 2 * Border - 3 * PriorityButtonSize) / 2;

	if (priority_radiogroup_) {
		pos.y += 2 * PriorityButtonSize;
		for (UI::Radiobutton* btn = priority_radiogroup_->get_first_button(); btn;
		     btn = btn->next_button()) {
			btn->set_pos(pos);
			pos.y -= PriorityButtonSize;
		}
	} else {
		priority_radiogroup_ = new UI::Radiogroup();

		priority_radiogroup_->add_button(
		   this, pos, g_gr->images().get(pic_priority_high), _("Highest priority"));
		pos.y += PriorityButtonSize;
		priority_radiogroup_->add_button(
		   this, pos, g_gr->images().get(pic_priority_normal), _("Normal priority"));
		pos.y += PriorityButtonSize;
		priority_radiogroup_->add_button(
		   this, pos, g_gr->images().get(pic_priority_low), _("Lowest priority"));
	}

	int32_t priority = -1;
	if (settings_) {
		for (const auto& pair : settings_->ware_queues) {
			if (pair.first == index_) {
				priority = pair.second.priority;
				break;
			}
		}
	} else {
		priority = building_.get_priority(type_, index_, false);
	}
	switch (priority) {
	case Widelands::kPriorityHigh:
		priority_radiogroup_->set_state(0);
		break;
	case Widelands::kPriorityNormal:
		priority_radiogroup_->set_state(1);
		break;
	case Widelands::kPriorityLow:
		priority_radiogroup_->set_state(2);
		break;
	default:
		NEVER_HERE();
	}

	priority_radiogroup_->changedto.connect(
	   boost::bind(&InputQueueDisplay::radiogroup_changed, this, _1));
	priority_radiogroup_->clicked.connect(boost::bind(&InputQueueDisplay::radiogroup_clicked, this));

	bool const can_act = igb_.can_act(building_.owner().player_number());
	if (!can_act)
		priority_radiogroup_->set_enabled(false);
}

/**
 * Updates the desired size buttons
 */
void InputQueueDisplay::update_max_fill_buttons() {
	delete increase_max_fill_;
	delete decrease_max_fill_;
	increase_max_fill_ = nullptr;
	decrease_max_fill_ = nullptr;

	if (cache_size_ <= 0 || show_only_)
		return;

	uint32_t x = Border;
	uint32_t y = Border + (total_height_ - 2 * Border - kWareMenuPicWidth) / 2;

	boost::format tooltip_format("<p>%s%s%s</p>");

	decrease_max_fill_ = new UI::Button(
	   this, "decrease_max_fill", x, y, kWareMenuPicWidth, kWareMenuPicHeight,
	   UI::ButtonStyle::kWuiMenu, g_gr->images().get("images/ui_basic/scrollbar_left.png"),
	   (tooltip_format %
	    g_gr->styles()
	       .font_style(UI::FontStyle::kTooltipHeader)
	       .as_font_tag(
	          /** TRANSLATORS: Button tooltip in in a building's wares input queue */
	          _("Decrease the number of wares you want to be stored here"))

	    %
	    as_listitem(
	       /** TRANSLATORS: Button tooltip in in a building's wares input queue - option
	          explanation */
	       _("Hold down Shift to decrease all ware types at the same time"), UI::FontStyle::kTooltip)

	    % as_listitem(
	         /** TRANSLATORS: Button tooltip in in a building's wares input queue - option
	            explanation */
	         _("Hold down Ctrl to allow none of this ware"), UI::FontStyle::kTooltip))
	      .str());
	decrease_max_fill_->sigclicked.connect(
	   boost::bind(&InputQueueDisplay::decrease_max_fill_clicked, boost::ref(*this)));

	x = Border + (cache_size_ + 1) * (CellWidth + CellSpacing);

	increase_max_fill_ = new UI::Button(
	   this, "increase_max_fill", x, y, kWareMenuPicWidth, kWareMenuPicHeight,
	   UI::ButtonStyle::kWuiMenu, g_gr->images().get("images/ui_basic/scrollbar_right.png"),
	   (tooltip_format

	    % g_gr->styles()
	         .font_style(UI::FontStyle::kTooltipHeader)
	         .as_font_tag(
	            /** TRANSLATORS: Button tooltip in a building's wares input queue */
	            _("Increase the number of wares you want to be stored here"))

	    %
	    as_listitem(
	       /** TRANSLATORS: Button tooltip in in a building's wares input queue - option
	          explanation */
	       _("Hold down Shift to increase all ware types at the same time"), UI::FontStyle::kTooltip)

	    % as_listitem(
	         /** TRANSLATORS: Button tooltip in in a building's wares input queue - option
	            explanation */
	         _("Hold down Ctrl to allow all of this ware"), UI::FontStyle::kTooltip))
	      .str());
	increase_max_fill_->sigclicked.connect(
	   boost::bind(&InputQueueDisplay::increase_max_fill_clicked, boost::ref(*this)));

	increase_max_fill_->set_repeating(true);
	decrease_max_fill_->set_repeating(true);
	compute_max_fill_buttons_enabled_state();
}

/**
 * Update priority when radiogroup has changed
 */
void InputQueueDisplay::radiogroup_changed(int32_t state) {
	assert(type_ == Widelands::wwWARE);
	if (!igb_.can_act(building_.owner().player_number())) {
		return;
	}

	int32_t priority = 0;

	switch (state) {
	case 0:
		priority = Widelands::kPriorityHigh;
		break;
	case 1:
		priority = Widelands::kPriorityNormal;
		break;
	case 2:
		priority = Widelands::kPriorityLow;
		break;
	default:
		return;
	}
	if (SDL_GetModState() & KMOD_CTRL) {
		update_siblings_priority(state);
	}
	igb_.game().send_player_set_ware_priority(
	   building_, type_, index_, priority, settings_ != nullptr);
}

void InputQueueDisplay::radiogroup_clicked() {
	// Already set option has been clicked again
	// Unimportant for this queue, but update other queues
	if (SDL_GetModState() & KMOD_CTRL) {
		update_siblings_priority(priority_radiogroup_->get_state());
	}
}

void InputQueueDisplay::update_siblings_priority(int32_t state) {
	// "Release" the CTRL key to avoid recursion
	const SDL_Keymod old_modifiers = SDL_GetModState();
	SDL_SetModState(KMOD_NONE);

	Panel* sibling = get_parent()->get_first_child();
	// Well, at least we should be a child of our parent
	assert(sibling != nullptr);
	do {
		if (sibling == this) {
			// We already have been set
			continue;
		}
		InputQueueDisplay* display = dynamic_cast<InputQueueDisplay*>(sibling);
		if (display == nullptr) {
			// Cast failed. Sibling is no InputQueueDisplay
			continue;
		}
		if (display->type_ != Widelands::wwWARE) {
			// No ware, so there is no radio group
			continue;
		}
		assert(display->priority_radiogroup_ != nullptr);
		if (display->priority_radiogroup_->get_state() == state) {
			// Nothing to do for this queue
			continue;
		}
		// Calling set_state() leads to radiogroup_changed()) getting called, which does the real
		// change
		// TODO(Notabilis): When bug 1738485 is fixed probably replace with
		// send_player_set_ware_priority()
		display->priority_radiogroup_->set_state(state);
	} while ((sibling = sibling->get_next_sibling()));

	SDL_SetModState(old_modifiers);
}

/**
 * One of the buttons to increase or decrease the amount of wares
 * stored here has been clicked
 */
void InputQueueDisplay::decrease_max_fill_clicked() {
	if (!igb_.can_act(building_.owner().player_number())) {
		return;
	}

	// Update the value of this queue if required
	if (cache_max_fill_ > 0) {
		igb_.game().send_player_set_input_max_fill(
		   building_, index_, type_, ((SDL_GetModState() & KMOD_CTRL) ? 0 : cache_max_fill_ - 1),
		   settings_ != nullptr);
	}

	// Update other queues of this building
	if (SDL_GetModState() & KMOD_SHIFT) {
		// Using int16_t instead of int32_t on purpose to avoid over-/underflows
		update_siblings_fill(
		   ((SDL_GetModState() & KMOD_CTRL) ? std::numeric_limits<int16_t>::min() : -1));
	}
}

void InputQueueDisplay::increase_max_fill_clicked() {
	if (!igb_.can_act(building_.owner().player_number())) {
		return;
	}

	if (cache_max_fill_ < cache_size_) {
		igb_.game().send_player_set_input_max_fill(
		   building_, index_, type_,
		   ((SDL_GetModState() & KMOD_CTRL) ? cache_size_ : cache_max_fill_ + 1),
		   settings_ != nullptr);
	}

	if (SDL_GetModState() & KMOD_SHIFT) {
		update_siblings_fill(
		   ((SDL_GetModState() & KMOD_CTRL) ? std::numeric_limits<int16_t>::max() : 1));
	}
}

void InputQueueDisplay::update_siblings_fill(int32_t delta) {
	Panel* sibling = get_parent()->get_first_child();
	// Well, at least we should be a child of our parent
	assert(sibling != nullptr);
	do {
		if (sibling == this) {
			// We already have been set
			continue;
		}
		InputQueueDisplay* display = dynamic_cast<InputQueueDisplay*>(sibling);
		if (display == nullptr) {
			// Cast failed. Sibling is no InputQueueDisplay
			continue;
		}
		uint32_t new_fill =
		   std::max(0, std::min<int32_t>(static_cast<int32_t>(display->cache_max_fill_) + delta,
		                                 display->cache_size_));
		if (new_fill != display->cache_max_fill_) {
			igb_.game().send_player_set_input_max_fill(
			   building_, display->index_, display->type_, new_fill, settings_ != nullptr);
		}
	} while ((sibling = sibling->get_next_sibling()));
}

void InputQueueDisplay::compute_max_fill_buttons_enabled_state() {

	// Disable those buttons for replay watchers
	bool const can_act = igb_.can_act(building_.owner().player_number());
	if (!can_act) {
		if (increase_max_fill_)
			increase_max_fill_->set_enabled(false);
		if (decrease_max_fill_)
			decrease_max_fill_->set_enabled(false);
	}
}
