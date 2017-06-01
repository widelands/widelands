/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "economy/input_queue.h"
#include "economy/request.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
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
                                     Widelands::InputQueue* const queue,
                                     bool show_only)
   : UI::Panel(parent, x, y, 0, 28),
     igb_(igb),
     building_(building),
     queue_(queue),
     priority_radiogroup_(nullptr),
     increase_max_fill_(nullptr),
     decrease_max_fill_(nullptr),
     index_(queue->get_index()),
     type_(queue->get_type()),
     max_fill_indicator_(g_gr->images().get(pic_max_fill_indicator)),
     cache_size_(queue->get_max_size()),
     cache_max_fill_(queue->get_max_fill()),
     total_height_(0),
     show_only_(show_only) {
	if (type_ == Widelands::wwWARE) {
		const Widelands::WareDescr& ware =
		   *queue->owner().tribe().get_ware_descr(queue_->get_index());
		set_tooltip(ware.descname().c_str());
		icon_ = ware.icon();
	} else {
		const Widelands::WorkerDescr& worker =
		   *queue->owner().tribe().get_worker_descr(queue_->get_index());
		set_tooltip(worker.descname().c_str());
		icon_ = worker.icon();
	}

	uint16_t ph = max_fill_indicator_->height();

	uint32_t priority_button_height = show_only ? 0 : 3 * PriorityButtonSize;
	uint32_t image_height =
	   show_only ? WARE_MENU_PIC_HEIGHT : std::max<int32_t>(WARE_MENU_PIC_HEIGHT, ph);

	total_height_ = std::max(priority_button_height, image_height) + 2 * Border;

	max_size_changed();

	set_thinks(true);
}

InputQueueDisplay::~InputQueueDisplay() {
	delete priority_radiogroup_;
}

/**
 * Recalculate the panel's size based on the size of the queue.
 *
 * This is useful for construction sites, whose queues shrink over time.
 */
void InputQueueDisplay::max_size_changed() {
	uint32_t pbs = show_only_ ? 0 : PriorityButtonSize;
	uint32_t ctrl_b_size = show_only_ ? 0 : 2 * WARE_MENU_PIC_WIDTH;

	cache_size_ = queue_->get_max_size();

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
	if (static_cast<uint32_t>(queue_->get_max_size()) != cache_size_)
		max_size_changed();

	// TODO(sirver): It seems cache_max_fill_ is not really useful for anything.
	if (static_cast<uint32_t>(queue_->get_max_fill()) != cache_max_fill_) {
		cache_max_fill_ = queue_->get_max_fill();
		compute_max_fill_buttons_enabled_state();
	}
}

/**
 * Render the current InputQueue state.
 */
void InputQueueDisplay::draw(RenderTarget& dst) {
	if (!cache_size_)
		return;

	cache_max_fill_ = queue_->get_max_fill();

	uint32_t nr_inputs_to_draw = std::min(queue_->get_filled(), cache_size_);
	uint32_t nr_empty_to_draw = cache_size_ - nr_inputs_to_draw;

	Vector2i point = Vector2i::zero();
	point.x = Border + (show_only_ ? 0 : CellWidth + CellSpacing);
	point.y = Border + (total_height_ - 2 * Border - WARE_MENU_PIC_HEIGHT) / 2;

	for (; nr_inputs_to_draw; --nr_inputs_to_draw, point.x += CellWidth + CellSpacing) {
		dst.blitrect(Vector2i(point.x, point.y), icon_, Recti(0, 0, icon_->width(), icon_->height()),
		             BlendMode::UseAlpha);
	}
	for (; nr_empty_to_draw; --nr_empty_to_draw, point.x += CellWidth + CellSpacing) {
		dst.blitrect_scale_monochrome(Rectf(point.x, point.y, icon_->width(), icon_->height()), icon_,
		                              Recti(0, 0, icon_->width(), icon_->height()),
		                              RGBAColor(166, 166, 166, 127));
	}

	if (!show_only_) {
		uint16_t pw = max_fill_indicator_->width();
		point.y = Border;
		point.x = Border + CellWidth + CellSpacing +
		          (queue_->get_max_fill() * (CellWidth + CellSpacing)) - CellSpacing / 2 - pw / 2;
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

	int32_t priority = building_.get_priority(type_, index_, false);
	switch (priority) {
	case HIGH_PRIORITY:
		priority_radiogroup_->set_state(0);
		break;
	case DEFAULT_PRIORITY:
		priority_radiogroup_->set_state(1);
		break;
	case LOW_PRIORITY:
		priority_radiogroup_->set_state(2);
		break;
	default:
		break;
	}

	priority_radiogroup_->changedto.connect(
	   boost::bind(&InputQueueDisplay::radiogroup_changed, this, _1));

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
	uint32_t y = Border + (total_height_ - 2 * Border - WARE_MENU_PIC_WIDTH) / 2;

	decrease_max_fill_ =
	   new UI::Button(this, "decrease_max_fill", x, y, WARE_MENU_PIC_WIDTH, WARE_MENU_PIC_HEIGHT,
	                  g_gr->images().get("images/ui_basic/but4.png"),
	                  g_gr->images().get("images/ui_basic/scrollbar_left.png"),
	                  _("Decrease the number of wares you want to be stored here."));
	decrease_max_fill_->sigclicked.connect(
	   boost::bind(&InputQueueDisplay::decrease_max_fill_clicked, boost::ref(*this)));

	x = Border + (cache_size_ + 1) * (CellWidth + CellSpacing);
	increase_max_fill_ =
	   new UI::Button(this, "increase_max_fill", x, y, WARE_MENU_PIC_WIDTH, WARE_MENU_PIC_HEIGHT,
	                  g_gr->images().get("images/ui_basic/but4.png"),
	                  g_gr->images().get("images/ui_basic/scrollbar_right.png"),
	                  _("Increase the number of wares you want to be stored here."));
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

	int32_t priority = 0;

	switch (state) {
	case 0:
		priority = HIGH_PRIORITY;
		break;
	case 1:
		priority = DEFAULT_PRIORITY;
		break;
	case 2:
		priority = LOW_PRIORITY;
		break;
	default:
		return;
	}

	igb_.game().send_player_set_ware_priority(building_, type_, index_, priority);
}

/**
 * One of the buttons to increase or decrease the amount of wares
 * stored here has been clicked
 */
void InputQueueDisplay::decrease_max_fill_clicked() {
	assert(cache_max_fill_ > 0);

	igb_.game().send_player_set_input_max_fill(building_, index_, type_, cache_max_fill_ - 1);
}

void InputQueueDisplay::increase_max_fill_clicked() {

	assert(cache_max_fill_ < queue_->get_max_size());

	igb_.game().send_player_set_input_max_fill(building_, index_, type_, cache_max_fill_ + 1);
}

void InputQueueDisplay::compute_max_fill_buttons_enabled_state() {

	// Disable those buttons for replay watchers
	bool const can_act = igb_.can_act(building_.owner().player_number());
	if (!can_act) {
		if (increase_max_fill_)
			increase_max_fill_->set_enabled(false);
		if (decrease_max_fill_)
			decrease_max_fill_->set_enabled(false);
	} else {

		if (decrease_max_fill_)
			decrease_max_fill_->set_enabled(cache_max_fill_ > 0);
		if (increase_max_fill_)
			increase_max_fill_->set_enabled(cache_max_fill_ < queue_->get_max_size());
	}
}
