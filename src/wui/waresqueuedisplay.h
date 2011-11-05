/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef _WARESQUEUEDISPLAY_H_
#define _WARESQUEUEDISPLAY_H_

#include <cstdlib>
#include <stdint.h>

#include "logic/item_ware_descr.h"
#include "ui_basic/panel.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/button.h"

struct Interactive_GameBase;

namespace UI {
struct Panel;
struct Radiogroup;
}

namespace Widelands {
struct Building;
struct WaresQueue;
}

/**
 * This passive class displays the status of a WaresQueue
 * and shows priority buttons that can be manipulated.
 * It updates itself automatically through think().
 */
struct WaresQueueDisplay : public UI::Panel {
	enum {
		CellWidth = WARE_MENU_PIC_WIDTH,
		CellSpacing = 2,
		Border = 4,
		PriorityButtonSize = 10
	};

public:
	WaresQueueDisplay
		(UI::Panel             * parent,
		 int32_t x, int32_t y,
		 Interactive_GameBase  & igb,
		 Widelands::Building   & building,
		 Widelands::WaresQueue * queue,
		 bool = false);
	~WaresQueueDisplay();

	virtual void think();
	virtual void draw(RenderTarget &);

private:
	Interactive_GameBase  & m_igb;
	Widelands::Building   & m_building;
	Widelands::WaresQueue * m_queue;
	UI::Radiogroup        * m_priority_radiogroup;
	UI::Callback_Button   * m_increase_max_fill;
	UI::Callback_Button   * m_decrease_max_fill;
	Widelands::Ware_Index   m_ware_index;
	int32_t          m_ware_type;
	PictureID        m_icon;            //< Index to ware's picture
	PictureID        m_icon_grey;
	PictureID        m_max_fill_indicator;


	uint32_t         m_cache_size;
	uint32_t         m_cache_filled;
	uint32_t         m_total_height;
	bool             m_show_only;

	virtual void max_size_changed();
	void update_priority_buttons();
	void update_max_fill_buttons();
	void decrease_max_fill_clicked();
	void increase_max_fill_clicked();
	void radiogroup_changed(int32_t);
};

#endif // _WARESQUEUEDISPLAY_H_
