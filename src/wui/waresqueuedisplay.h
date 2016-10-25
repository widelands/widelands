/*
 * Copyright (C) 2010-2016 by the Widelands Development Team
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

#ifndef WL_WUI_WARESQUEUEDISPLAY_H
#define WL_WUI_WARESQUEUEDISPLAY_H

#include <cstdlib>

#include <stdint.h>

#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "ui_basic/button.h"
#include "ui_basic/panel.h"
#include "ui_basic/radiobutton.h"

class InteractiveGameBase;

namespace UI {
class Panel;
struct Radiogroup;
}

namespace Widelands {
class Building;
class WaresQueue;
}

/**
 * This passive class displays the status of a WaresQueue
 * and shows priority buttons that can be manipulated.
 * It updates itself automatically through think().
 */
class WaresQueueDisplay : public UI::Panel {
public:
	enum { CellWidth = WARE_MENU_PIC_WIDTH, CellSpacing = 2, Border = 4, PriorityButtonSize = 10 };

	WaresQueueDisplay(UI::Panel* parent,
	                  int32_t x,
	                  int32_t y,
	                  InteractiveGameBase& igb,
	                  Widelands::Building& building,
	                  Widelands::WaresQueue* queue,
	                  bool = false);
	~WaresQueueDisplay();

	void think() override;
	void draw(RenderTarget&) override;

private:
	InteractiveGameBase& igb_;
	Widelands::Building& building_;
	Widelands::WaresQueue* queue_;
	UI::Radiogroup* priority_radiogroup_;
	UI::Button* increase_max_fill_;
	UI::Button* decrease_max_fill_;
	Widelands::DescriptionIndex ware_index_;
	Widelands::WareWorker ware_type_;
	const Image* icon_;  //< Index to ware's picture
	const Image* max_fill_indicator_;

	uint32_t cache_size_;
	uint32_t cache_max_fill_;
	uint32_t total_height_;
	bool show_only_;

	virtual void max_size_changed();
	void update_priority_buttons();
	void update_max_fill_buttons();
	void decrease_max_fill_clicked();
	void increase_max_fill_clicked();
	void radiogroup_changed(int32_t);

	void compute_max_fill_buttons_enabled_state();
};

#endif  // end of include guard: WL_WUI_WARESQUEUEDISPLAY_H
