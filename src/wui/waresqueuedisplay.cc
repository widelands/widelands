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
#include "logic/item_ware_descr.h"
#include "logic/player.h"
#include "ui_basic/button.h"


static char const * pic_queue_background = "pics/queue_background.png";

static char const * pic_priority_low       = "pics/low_priority_button.png";
static char const * pic_priority_normal    = "pics/normal_priority_button.png";
static char const * pic_priority_high      = "pics/high_priority_button.png";
static char const * pic_priority_low_on    = "pics/low_priority_on.png";
static char const * pic_priority_normal_on = "pics/normal_priority_on.png";
static char const * pic_priority_high_on   = "pics/high_priority_on.png";

/**
 * This passive class displays the status of a WaresQueue.
 * It updates itself automatically through think().
 */
struct WaresQueueDisplay : public UI::Panel {
	enum {
		CellWidth = WARE_MENU_PIC_WIDTH,
		Border = 4,
		Height = WARE_MENU_PIC_HEIGHT + 2 * Border,
	};

public:
	WaresQueueDisplay
		(UI::Panel             * parent,
		 int32_t x, int32_t y,
		 uint32_t                maxw,
		 Widelands::WaresQueue *);
	~WaresQueueDisplay();

	virtual void think();
	virtual void draw(RenderTarget &);

private:
	void recalc_size();

private:
	Widelands::WaresQueue * m_queue;
	uint32_t         m_max_width;
	PictureID        m_icon;            //< Index to ware's picture
	PictureID        m_pic_background;

	uint32_t         m_cache_size;
	uint32_t         m_cache_filled;
	uint32_t         m_display_size;
};

WaresQueueDisplay::WaresQueueDisplay
	(UI::Panel * const parent,
	 int32_t const x, int32_t const y, uint32_t const maxw,
	 Widelands::WaresQueue * const queue)
:
UI::Panel(parent, x, y, 0, Height),
m_queue(queue),
m_max_width(maxw),
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

	recalc_size();

	set_think(true);
}

WaresQueueDisplay::~WaresQueueDisplay() {
	g_gr->free_picture_surface(m_pic_background);
}

/**
 * Recalculate the panel's size based on the size of the queue.
 *
 * This is useful for construction sites, whose queues shrink over time.
 */
void WaresQueueDisplay::recalc_size()
{
	m_display_size = (m_max_width - 2 * Border) / CellWidth;

	m_cache_size = m_queue->get_size();

	if (m_cache_size < m_display_size)
		m_display_size = m_cache_size;

	set_size(m_display_size * CellWidth + 2 * Border, Height);
}

/**
 * Compare the current WaresQueue state with the cached state; update if necessary.
 */
void WaresQueueDisplay::think()
{
	if (static_cast<uint32_t>(m_queue->get_size()) != m_cache_size)
		recalc_size();

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

/**
 * This button is used to change the priority of a wares queue.
 *
 * It maintains its enabled state automatically.
 */
struct WareQueuePriorityButton : UI::Button {
	WareQueuePriorityButton
		(UI::Panel * parent,
		 int32_t x, int32_t y, uint32_t w, uint32_t h,
		 const char * picture_enabled, const char * picture_disabled,
		 const std::string & tooltip,
		 Interactive_GameBase & igb,
		 Widelands::Building & building,
		 int32_t ware_type,
		 Widelands::Ware_Index ware_index,
		 int32_t priority);

private:
	void clicked();
	void think();

	void update_enabled();

	Interactive_GameBase & m_igb;
	Widelands::Building & m_building;
	int32_t m_ware_type;
	Widelands::Ware_Index m_ware_index;
	int32_t m_priority;

	PictureID m_picture_enabled;
	PictureID m_picture_disabled;
};

WareQueuePriorityButton::WareQueuePriorityButton
	(UI::Panel* parent,
	 int32_t x, int32_t y, uint32_t w, uint32_t h,
	 const char* picture_enabled, const char* picture_disabled,
	 const std::string& tooltip,
	 Interactive_GameBase& igb,
	 Widelands::Building& building,
	 int32_t ware_type,
	 Widelands::Ware_Index ware_index,
	 int32_t priority)
:
UI::Button
	(parent, "priority_button",
	 x, y, w, h,
	 g_gr->get_picture(PicMod_UI, "pics/but4.png"),
	 g_gr->get_no_picture(),
	 tooltip),
m_igb(igb),
m_building(building),
m_ware_type(ware_type),
m_ware_index(ware_index),
m_priority(priority),
m_picture_enabled
	(g_gr->get_resized_picture
		(g_gr->get_picture(PicMod_Game,  picture_enabled),
		 w, h, Graphic::ResizeMode_Clip)),
m_picture_disabled
	(g_gr->get_resized_picture
		(g_gr->get_picture(PicMod_Game,  picture_disabled),
		 w, h, Graphic::ResizeMode_Clip))
{
	update_enabled();

	set_think(true);
}

void WareQueuePriorityButton::clicked()
{
	m_igb.game().send_player_set_ware_priority
		(m_building, m_ware_type, m_ware_index, m_priority);
	set_think(true);
}

void WareQueuePriorityButton::think()
{
	update_enabled();
}

void WareQueuePriorityButton::update_enabled()
{
	bool allow_changes = m_igb.can_act(m_building.owner().player_number());
	bool enabled = m_building.get_priority(m_ware_type, m_ware_index, false) != m_priority;

	set_enabled(allow_changes && enabled);
	set_pic(enabled ? m_picture_enabled : m_picture_disabled);
}

/**
 * Allocate a new panel that displays the given wares queue and shows
 * priority buttons that can be manipulated if appropriate.
 */
UI::Panel* create_wares_queue_display
	(UI::Panel * parent,
	 Interactive_GameBase & igb,
	 Widelands::Building & b,
	 Widelands::WaresQueue * const wq,
	 int32_t width)
{
	const int32_t priority_buttons_width = WaresQueueDisplay::Height / 3;
	UI::Box * hbox = new UI::Box (parent, 0, 0, UI::Box::Horizontal);
	WaresQueueDisplay & wqd =
		*new WaresQueueDisplay(hbox, 0, 0, width - priority_buttons_width, wq);

	hbox->add(&wqd, UI::Box::AlignTop);

	if (wq->get_ware()) {
		// Add priority buttons
		UI::Box * vbox = new UI::Box (hbox, 0, 0, UI::Box::Vertical);
		int32_t ware_type = Widelands::Request::WARE;
		Widelands::Ware_Index ware_index = wq->get_ware();

		vbox->add
			(new WareQueuePriorityButton
			 	(vbox, 0, 0,
			 	 priority_buttons_width,
			 	 priority_buttons_width,
			 	 pic_priority_high,
			 	 pic_priority_high_on,
			 	 _("Highest priority"),
			 	 igb, b, ware_type, ware_index,
			 	 HIGH_PRIORITY),
			 UI::Box::AlignTop);
		vbox->add
			(new WareQueuePriorityButton
			 	(vbox, 0, 0,
			 	 priority_buttons_width,
			 	 priority_buttons_width,
			 	 pic_priority_normal,
			 	 pic_priority_normal_on,
			 	 _("Normal priority"),
			 	 igb, b, ware_type, ware_index,
			 	 DEFAULT_PRIORITY),
			 UI::Box::AlignTop);
		vbox->add
			(new WareQueuePriorityButton
			 	(vbox, 0, 0,
			 	 priority_buttons_width,
			 	 priority_buttons_width,
			 	 pic_priority_low,
			 	 pic_priority_low_on,
			 	 _("Lowest priority"),
			 	 igb, b, ware_type, ware_index,
			 	 LOW_PRIORITY),
			 UI::Box::AlignTop);

		hbox->add(vbox, UI::Box::AlignCenter);
	}

	return hbox;
}
