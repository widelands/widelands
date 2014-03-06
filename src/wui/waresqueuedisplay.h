/*
 * Copyright (C) 2010-2013 by the Widelands Development Team
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

#ifndef _WARESQUEUEDISPLAY_H_
#define _WARESQUEUEDISPLAY_H_

#include <cstdlib>

#include <stdint.h>

#include "logic/ware_descr.h"
#include "logic/wareworker.h"
#include "ui_basic/button.h"
#include "ui_basic/panel.h"
#include "ui_basic/radiobutton.h"

class Interactive_GameBase;

namespace UI {
struct Panel;
struct Radiogroup;
}

namespace Widelands {
class Building;
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

	virtual void think() override;
	virtual void draw(RenderTarget &) override;

private:
	Interactive_GameBase  & m_igb;
	Widelands::Building   & m_building;
	Widelands::WaresQueue * m_queue;
	UI::Radiogroup        * m_priority_radiogroup;
	UI::Button   * m_increase_max_fill;
	UI::Button   * m_decrease_max_fill;
	Widelands::Ware_Index   m_ware_index;
	Widelands::WareWorker m_ware_type;
	const Image* m_icon;            //< Index to ware's picture
	const Image* m_icon_grey;
	const Image* m_max_fill_indicator;


	uint32_t         m_cache_size;
	uint32_t         m_cache_filled;
	uint32_t         m_cache_max_fill;
	uint32_t         m_total_height;
	bool             m_show_only;

	virtual void max_size_changed();
	void update_priority_buttons();
	void update_max_fill_buttons();
	void decrease_max_fill_clicked();
	void increase_max_fill_clicked();
	void radiogroup_changed(int32_t);

	void compute_max_fill_buttons_enabled_state();
};

#endif // _WARESQUEUEDISPLAY_H_
