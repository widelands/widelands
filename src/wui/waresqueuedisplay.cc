/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "waresqueuedisplay.h"

#include "economy/request.h"
#include "economy/wares_queue.h"
#include "graphic/rendertarget.h"
#include "interactive_gamebase.h"
#include "logic/player.h"

#include "upcast.h"

static char const * pic_queue_background = "pics/queue_background.png";

static char const * pic_priority_low_flat       = "pics/low_priority_button_flat.png";
static char const * pic_priority_normal_flat    = "pics/normal_priority_button_flat.png";
static char const * pic_priority_high_flat      = "pics/high_priority_button_flat.png";

WaresQueueDisplay::WaresQueueDisplay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y, uint32_t const maxw,
	 Interactive_GameBase  & igb,
	 Widelands::Building   & building,
	 Widelands::WaresQueue * const queue)
:
UI::Panel(parent, x, y, 0, Height),
m_igb(igb),
m_building(building),
m_queue(queue),
m_ware_index(queue->get_ware()),
m_ware_type(Widelands::Request::WARE),
m_max_width(maxw - Height / 3),
m_pic_background(g_gr->get_picture(PicMod_Game, pic_queue_background)),
m_cache_size(queue->get_size()),
m_cache_filled(queue->get_filled()),
m_display_size(0)
{
	const Widelands::Item_Ware_Descr & ware =
		*queue->owner().tribe().get_ware_descr(m_queue->get_ware());
	set_tooltip(ware.descname().c_str());

	m_icon = ware.icon();
	m_pic_background = g_gr->create_grayed_out_pic(m_icon);

	update_desired_size();

	set_think(true);

	Point pos = Point(m_cache_size * CellWidth + Border, 0);

	m_radiogroup.add_button
			(this,
			pos,
			g_gr->get_picture(PicMod_Game,  pic_priority_high_flat),
			_("Highest priority"));
	pos.y += Height / 3;
	m_radiogroup.add_button
			(this,
			pos,
			g_gr->get_picture(PicMod_Game,  pic_priority_normal_flat),
			_("Normal priority"));
	pos.y += Height / 3;
	m_radiogroup.add_button
			(this,
			pos,
			g_gr->get_picture(PicMod_Game,  pic_priority_low_flat),
			_("Lowest priority"));


	int32_t priority = m_building.get_priority(m_ware_type, m_ware_index, false);
	switch (priority) {
	case HIGH_PRIORITY:
		m_radiogroup.set_state(0);
		break;
	case DEFAULT_PRIORITY:
		m_radiogroup.set_state(1);
		break;
	case LOW_PRIORITY:
		m_radiogroup.set_state(2);
		break;
	default:
		break;
	}

	m_radiogroup.changedto.set
			(this, &WaresQueueDisplay::radiogroup_changed);
}

WaresQueueDisplay::~WaresQueueDisplay()
{
}

/**
 * Recalculate the panel's size based on the size of the queue.
 *
 * This is useful for construction sites, whose queues shrink over time.
 */
void WaresQueueDisplay::update_desired_size()
{
	m_display_size = (m_max_width - 2 * Border) / CellWidth;

	m_cache_size = m_queue->get_size();

	if (m_cache_size < m_display_size)
		m_display_size = m_cache_size;

	set_desired_size(m_display_size * CellWidth + Height / 3 + 2 * Border, Height);
}

/**
 * Compare the current WaresQueue state with the cached state; update if necessary.
 */
void WaresQueueDisplay::think()
{
	if (static_cast<uint32_t>(m_queue->get_size()) != m_cache_size)
		update_desired_size();

	if (static_cast<uint32_t>(m_queue->get_filled()) != m_cache_filled)
		update();
}

/**
 * Render the current WaresQueue state.
 */
void WaresQueueDisplay::draw(RenderTarget & dst)
{
	if (!m_display_size)
		return;

	m_cache_filled = m_queue->get_filled();

	uint32_t nr_wares_to_draw = std::min(m_cache_filled, m_display_size);
	uint32_t nr_empty_to_draw = m_display_size - nr_wares_to_draw;
	Point point(Border, Border);
	for (; nr_wares_to_draw; --nr_wares_to_draw, point.x += CellWidth)
		dst.blit(point, m_icon);
	for (; nr_empty_to_draw; --nr_empty_to_draw, point.x += CellWidth)
		dst.blit(point, m_pic_background);
}

/*
 * Update priority when radiogroup has changed
 */
void WaresQueueDisplay::radiogroup_changed(int32_t state) {
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
 * Allocate a new panel that displays the given wares queue and shows
 * priority buttons that can be manipulated if appropriate.
 */
UI::Panel * create_wares_queue_display
	(UI::Panel * parent,
	 Interactive_GameBase & igb,
	 Widelands::Building & b,
	 Widelands::WaresQueue * const wq,
	 int32_t width)
{
	WaresQueueDisplay & wqd = *new WaresQueueDisplay(parent, 0, 0, width, igb, b, wq);
	upcast(UI::Panel, panel, &wqd);
	return panel;
}
