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

#include <algorithm>

#include "waresqueuedisplay.h"

#include "economy/request.h"
#include "economy/wares_queue.h"
#include "graphic/rendertarget.h"
#include "interactive_gamebase.h"
#include "logic/player.h"

static char const * pic_queue_background = "pics/queue_background.png";

static char const * pic_priority_low     = "pics/low_priority_button.png";
static char const * pic_priority_normal  = "pics/normal_priority_button.png";
static char const * pic_priority_high    = "pics/high_priority_button.png";
static char const * pic_max_fill_indicator = "pics/max_fill_indicator.png";

WaresQueueDisplay::WaresQueueDisplay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y, uint32_t const maxw,
	 Interactive_GameBase  & igb,
	 Widelands::Building   & building,
	 Widelands::WaresQueue * const queue)
:
UI::Panel(parent, x, y, 0, 28),
m_igb(igb),
m_building(building),
m_queue(queue),
m_priority_radiogroup(0),
m_increase_max_fill(0),
m_decrease_max_fill(0),
m_ware_index(queue->get_ware()),
m_ware_type(Widelands::Request::WARE),
m_max_width(maxw - PriorityButtonSize),
m_pic_background(g_gr->get_picture(PicMod_Game, pic_queue_background)),
m_max_fill_indicator(g_gr->get_picture(PicMod_Game, pic_max_fill_indicator)),
m_cache_size(queue->get_max_size()),
m_cache_filled(queue->get_filled()),
m_display_size(0),
m_total_height(0)
{
	const Widelands::Item_Ware_Descr & ware =
		*queue->owner().tribe().get_ware_descr(m_queue->get_ware());
	set_tooltip(ware.descname().c_str());

	m_icon = ware.icon();
	m_pic_background = g_gr->create_grayed_out_pic(m_icon);

	uint32_t pw, ph;
	g_gr->get_picture_size(m_max_fill_indicator, pw, ph);

	m_total_height = std::max
		(3 * PriorityButtonSize, std::max(WARE_MENU_PIC_HEIGHT, static_cast<int32_t>(ph))) + 2 * Border;

	max_size_changed();

	set_think(true);
}

WaresQueueDisplay::~WaresQueueDisplay()
{
}

/**
 * Recalculate the panel's size based on the size of the queue.
 *
 * This is useful for construction sites, whose queues shrink over time.
 */
void WaresQueueDisplay::max_size_changed()
{
	m_display_size =
		(m_max_width - 2 * Border - PriorityButtonSize - 2 * WARE_MENU_PIC_WIDTH - 2 * CellSpacing)
								/ (CellWidth + CellSpacing);

	m_cache_size = m_queue->get_max_size();

	if (m_cache_size < m_display_size)
		m_display_size = m_cache_size;

	update_priority_buttons();
	update_max_fill_buttons();

	if (m_display_size <= 0) {
		set_desired_size(0, 0);
	} else {
		set_desired_size
			((m_display_size + 2) * (CellWidth + CellSpacing) + PriorityButtonSize + 2 * Border,
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

	uint32_t pw, ph;
	g_gr->get_picture_size(m_max_fill_indicator, pw, ph);

	Point point;
	point.x = Border + CellWidth + CellSpacing;
	point.y = Border + (m_total_height - 2 * Border - WARE_MENU_PIC_HEIGHT) / 2;

	for (; nr_wares_to_draw; --nr_wares_to_draw, point.x += CellWidth + CellSpacing)
		dst.blit(point, m_icon);
	for (; nr_empty_to_draw; --nr_empty_to_draw, point.x += CellWidth + CellSpacing)
		dst.blit(point, m_pic_background);

	point.y = Border;
	point.x = Border + CellWidth + CellSpacing +
		(m_queue->get_max_fill() * (CellWidth + CellSpacing)) - CellSpacing / 2 - pw / 2;
	dst.blit(point, m_max_fill_indicator);
}

/**
 * Updates priority buttons of the WaresQueue
 */
void WaresQueueDisplay::update_priority_buttons()
{
	delete m_priority_radiogroup;
	if (m_display_size <= 0)
		return;

	m_priority_radiogroup = new UI::Radiogroup();

	Point pos = Point(m_display_size * CellWidth + Border, 0);
	pos.x = (m_display_size + 2) * (CellWidth + CellSpacing) + Border;
	pos.y = Border + (m_total_height - 2 * Border - 3 * PriorityButtonSize) / 2;

	m_priority_radiogroup->add_button
			(this,
			pos,
			g_gr->get_resized_picture
				(g_gr->get_picture(PicMod_Game,  pic_priority_high),
				PriorityButtonSize, PriorityButtonSize, Graphic::ResizeMode_Clip),
			_("Highest priority"));
	pos.y += PriorityButtonSize;
	m_priority_radiogroup->add_button
			(this,
			pos,
			g_gr->get_resized_picture
				(g_gr->get_picture(PicMod_Game,  pic_priority_normal),
				PriorityButtonSize, PriorityButtonSize, Graphic::ResizeMode_Clip),
			_("Normal priority"));
	pos.y += PriorityButtonSize;
	m_priority_radiogroup->add_button
			(this,
			pos,
			g_gr->get_resized_picture
				(g_gr->get_picture(PicMod_Game,  pic_priority_low),
				PriorityButtonSize, PriorityButtonSize, Graphic::ResizeMode_Clip),
			_("Lowest priority"));

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

	m_priority_radiogroup->changedto.set
		(this, &WaresQueueDisplay::radiogroup_changed);
}

/**
 * Updates the desired size buttons
 */
void WaresQueueDisplay::update_max_fill_buttons() {
	delete m_increase_max_fill;
	delete m_decrease_max_fill;
	if (m_display_size <= 0)
		return;

	uint32_t x = Border;
	uint32_t y = Border + (m_total_height - 2 * Border - WARE_MENU_PIC_WIDTH) / 2;

	m_decrease_max_fill = new UI::Callback_Button
		(this, "decrease_max_fill",
		 x, y, WARE_MENU_PIC_WIDTH, WARE_MENU_PIC_HEIGHT,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_left.png"),
		 boost::bind(&WaresQueueDisplay::decrease_max_fill_clicked, boost::ref(*this)),
		 _("Decrease the number of wares you want to be stored here."));

	x = Border + (m_display_size + 1) * (CellWidth + CellSpacing);
	m_increase_max_fill = new UI::Callback_Button
		(this, "increase_max_fill",
		 x, y, WARE_MENU_PIC_WIDTH, WARE_MENU_PIC_HEIGHT,
		 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
		 g_gr->get_picture(PicMod_UI, "pics/scrollbar_right.png"),
		 boost::bind(&WaresQueueDisplay::increase_max_fill_clicked, boost::ref(*this)),
		 _("Increase the number of wares you want to be stored here."));

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
void WaresQueueDisplay::decrease_max_fill_clicked() {
	int32_t cur = m_queue->get_max_fill();
	m_igb.game().send_player_set_ware_max_fill
			(m_building, m_ware_index, cur - 1);
}
void WaresQueueDisplay::increase_max_fill_clicked() {
	int32_t cur = m_queue->get_max_fill();
	m_igb.game().send_player_set_ware_max_fill
			(m_building, m_ware_index, cur + 1);
}

