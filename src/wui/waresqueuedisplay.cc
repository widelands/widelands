/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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

#include "wui/waresqueuedisplay.h"

#include <algorithm>

#include "economy/request.h"
#include "economy/wares_queue.h"
#include "graphic/graphic.h"
#include "graphic/image_transformations.h"
#include "graphic/rendertarget.h"
#include "logic/player.h"
#include "wui/interactive_gamebase.h"

static char const * pic_priority_low     = "pics/low_priority_button.png";
static char const * pic_priority_normal  = "pics/normal_priority_button.png";
static char const * pic_priority_high    = "pics/high_priority_button.png";
static char const * pic_max_fill_indicator = "pics/max_fill_indicator.png";

WaresQueueDisplay::WaresQueueDisplay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y,
	 Interactive_GameBase  & igb,
	 Widelands::Building   & building,
	 Widelands::WaresQueue * const queue,
	 bool show_only)
:
UI::Panel(parent, x, y, 0, 28),
m_igb(igb),
m_building(building),
m_queue(queue),
m_priority_radiogroup(nullptr),
m_increase_max_fill(nullptr),
m_decrease_max_fill(nullptr),
m_ware_index(queue->get_ware()),
m_ware_type(Widelands::wwWARE),
m_max_fill_indicator(g_gr->images().get(pic_max_fill_indicator)),
m_cache_size(queue->get_max_size()),
m_cache_filled(queue->get_filled()),
m_cache_max_fill(queue->get_max_fill()),
m_total_height(0),
m_show_only(show_only)
{
	const Widelands::WareDescr & ware =
		*queue->owner().tribe().get_ware_descr(m_queue->get_ware());
	set_tooltip(ware.descname().c_str());

	m_icon = ware.icon();
	m_icon_grey = ImageTransformations::change_luminosity(ImageTransformations::gray_out(m_icon), 0.65, false);

	uint16_t ph = m_max_fill_indicator->height();

	uint32_t priority_button_height = show_only ? 0 : 3 * PriorityButtonSize;
	uint32_t image_height = show_only ? WARE_MENU_PIC_HEIGHT : std::max<int32_t>(WARE_MENU_PIC_HEIGHT, ph);

	m_total_height = std::max(priority_button_height, image_height) + 2 * Border;

	max_size_changed();

	set_think(true);
}

WaresQueueDisplay::~WaresQueueDisplay()
{
	delete m_priority_radiogroup;
}

/**
 * Recalculate the panel's size based on the size of the queue.
 *
 * This is useful for construction sites, whose queues shrink over time.
 */
void WaresQueueDisplay::max_size_changed()
{
	uint32_t pbs = m_show_only ? 0 : PriorityButtonSize;
	uint32_t ctrl_b_size = m_show_only ? 0 : 2 * WARE_MENU_PIC_WIDTH;

	m_cache_size = m_queue->get_max_size();

	update_priority_buttons();
	update_max_fill_buttons();

	if (m_cache_size <= 0) {
		set_desired_size(0, 0);
	} else {
		set_desired_size
			(m_cache_size * (CellWidth + CellSpacing) + pbs + ctrl_b_size + 2 * Border,
			 m_total_height);
	}
}

/**
 * Compare the current WaresQueue state with the cached state; update if necessary.
 */
void WaresQueueDisplay::think()
{
	if (static_cast<uint32_t>(m_queue->get_max_size()) != m_cache_size)
		max_size_changed();

	if (static_cast<uint32_t>(m_queue->get_filled()) != m_cache_filled)
		update();

	if (static_cast<uint32_t>(m_queue->get_max_fill()) != m_cache_max_fill) {
		m_cache_max_fill = m_queue->get_max_fill();
		compute_max_fill_buttons_enabled_state();
		update();
	}

}

/**
 * Render the current WaresQueue state.
 */
void WaresQueueDisplay::draw(RenderTarget & dst)
{
	if (!m_cache_size)
		return;

	m_cache_filled = m_queue->get_filled();
	m_cache_max_fill = m_queue->get_max_fill();

	uint32_t nr_wares_to_draw = std::min(m_cache_filled, m_cache_size);
	uint32_t nr_empty_to_draw = m_cache_size - nr_wares_to_draw;

	Point point;
	point.x = Border + (m_show_only ? 0 : CellWidth + CellSpacing);
	point.y = Border + (m_total_height - 2 * Border - WARE_MENU_PIC_HEIGHT) / 2;

	for (; nr_wares_to_draw; --nr_wares_to_draw, point.x += CellWidth + CellSpacing)
		dst.blit(point, m_icon);
	for (; nr_empty_to_draw; --nr_empty_to_draw, point.x += CellWidth + CellSpacing)
		dst.blit(point, m_icon_grey);

	if (not m_show_only) {
		uint16_t pw = m_max_fill_indicator->width();
		point.y = Border;
		point.x = Border + CellWidth + CellSpacing +
			(m_queue->get_max_fill() * (CellWidth + CellSpacing)) - CellSpacing / 2 - pw / 2;
		dst.blit(point, m_max_fill_indicator);
	}
}

/**
 * Updates priority buttons of the WaresQueue
 */
void WaresQueueDisplay::update_priority_buttons()
{
	if (m_cache_size <= 0 or m_show_only) {
		delete m_priority_radiogroup;
		m_priority_radiogroup = nullptr;
	}

	Point pos = Point(m_cache_size * CellWidth + Border, 0);
	pos.x = (m_cache_size + 2) * (CellWidth + CellSpacing) + Border;
	pos.y = Border + (m_total_height - 2 * Border - 3 * PriorityButtonSize) / 2;

	if (m_priority_radiogroup) {
		pos.y += 2 * PriorityButtonSize;
		for (UI::Radiobutton * btn = m_priority_radiogroup->get_first_button(); btn; btn = btn->next_button()) {
			btn->set_pos(pos);
			pos.y -= PriorityButtonSize;
		}
	} else {
		m_priority_radiogroup = new UI::Radiogroup();

		m_priority_radiogroup->add_button
			(this, pos, g_gr->images().get(pic_priority_high), _("Highest priority"));
		pos.y += PriorityButtonSize;
		m_priority_radiogroup->add_button
				(this, pos, g_gr->images().get(pic_priority_normal), _("Normal priority"));
		pos.y += PriorityButtonSize;
		m_priority_radiogroup->add_button
				(this, pos, g_gr->images().get(pic_priority_low), _("Lowest priority"));
	}

	int32_t priority = m_building.get_priority(m_ware_type, m_ware_index, false);
	switch (priority) {
	case HIGH_PRIORITY:
		m_priority_radiogroup->set_state(0);
		break;
	case DEFAULT_PRIORITY:
		m_priority_radiogroup->set_state(1);
		break;
	case LOW_PRIORITY:
		m_priority_radiogroup->set_state(2);
		break;
	default:
		break;
	}

	m_priority_radiogroup->changedto.connect
		(boost::bind(&WaresQueueDisplay::radiogroup_changed, this, _1));

	bool const can_act = m_igb.can_act(m_building.owner().player_number());
	if (not can_act)
		m_priority_radiogroup->set_enabled(false);
}

/**
 * Updates the desired size buttons
 */
void WaresQueueDisplay::update_max_fill_buttons() {
	delete m_increase_max_fill;
	delete m_decrease_max_fill;
	m_increase_max_fill = nullptr;
	m_decrease_max_fill = nullptr;

	if (m_cache_size <= 0 or m_show_only)
		return;

	uint32_t x = Border;
	uint32_t y = Border + (m_total_height - 2 * Border - WARE_MENU_PIC_WIDTH) / 2;

	m_decrease_max_fill = new UI::Button
		(this, "decrease_max_fill",
		 x, y, WARE_MENU_PIC_WIDTH, WARE_MENU_PIC_HEIGHT,
		 g_gr->images().get("pics/but4.png"),
		 g_gr->images().get("pics/scrollbar_left.png"),
		 _("Decrease the number of wares you want to be stored here."));
	m_decrease_max_fill->sigclicked.connect
		(boost::bind(&WaresQueueDisplay::decrease_max_fill_clicked, boost::ref(*this)));

	x = Border + (m_cache_size + 1) * (CellWidth + CellSpacing);
	m_increase_max_fill = new UI::Button
		(this, "increase_max_fill",
		 x, y, WARE_MENU_PIC_WIDTH, WARE_MENU_PIC_HEIGHT,
		 g_gr->images().get("pics/but4.png"),
		 g_gr->images().get("pics/scrollbar_right.png"),
		 _("Increase the number of wares you want to be stored here."));
	m_increase_max_fill->sigclicked.connect
		(boost::bind(&WaresQueueDisplay::increase_max_fill_clicked, boost::ref(*this)));

	m_increase_max_fill->set_repeating(true);
	m_decrease_max_fill->set_repeating(true);
	compute_max_fill_buttons_enabled_state();

}

/**
 * Update priority when radiogroup has changed
 */
void WaresQueueDisplay::radiogroup_changed(int32_t state)
{
	int32_t priority = 0;

	switch (state) {
	case 0: priority = HIGH_PRIORITY;
		break;
	case 1: priority = DEFAULT_PRIORITY;
		break;
	case 2: priority = LOW_PRIORITY;
		break;
	default:
		return;
	}

	m_igb.game().send_player_set_ware_priority
			(m_building, m_ware_type, m_ware_index, priority);
};

/**
 * One of the buttons to increase or decrease the amount of wares
 * stored here has been clicked
 */
void WaresQueueDisplay::decrease_max_fill_clicked()
{
	assert (m_cache_max_fill > 0);

	m_igb.game().send_player_set_ware_max_fill
			(m_building, m_ware_index, m_cache_max_fill - 1);

}

void WaresQueueDisplay::increase_max_fill_clicked()
{

	assert (m_cache_max_fill < m_queue->get_max_size());

	m_igb.game().send_player_set_ware_max_fill
			(m_building, m_ware_index, m_cache_max_fill + 1);

}

void WaresQueueDisplay::compute_max_fill_buttons_enabled_state()
{

	// Disable those buttons for replay watchers
	bool const can_act = m_igb.can_act(m_building.owner().player_number());
	if (not can_act) {
		if (m_increase_max_fill) m_increase_max_fill->set_enabled(false);
		if (m_decrease_max_fill) m_decrease_max_fill->set_enabled(false);
	} else {

		if (m_decrease_max_fill) m_decrease_max_fill->set_enabled(m_cache_max_fill > 0);
		if (m_increase_max_fill) m_increase_max_fill->set_enabled(m_cache_max_fill < m_queue->get_max_size());
	}
}
