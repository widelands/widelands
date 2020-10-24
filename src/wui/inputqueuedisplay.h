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

#ifndef WL_WUI_INPUTQUEUEDISPLAY_H
#define WL_WUI_INPUTQUEUEDISPLAY_H

#include "logic/map_objects/tribes/ware_descr.h"
#include "logic/map_objects/tribes/wareworker.h"
#include "ui_basic/button.h"
#include "ui_basic/panel.h"
#include "ui_basic/radiobutton.h"

class InteractiveBase;

namespace Widelands {
class Building;
class ConstructionSite;
struct ProductionsiteSettings;
class InputQueue;
}  // namespace Widelands

/**
 * This passive class displays the status of an InputQueue
 * and shows priority buttons that can be manipulated.
 * It updates itself automatically through think().
 */
class InputQueueDisplay : public UI::Panel {
public:
	enum { CellWidth = kWareMenuPicWidth, CellSpacing = 2, Border = 4, PriorityButtonSize = 10 };

	// Constructor for real queues (e.g. in ProductionSites)
	InputQueueDisplay(UI::Panel* parent,
	                  int32_t x,
	                  int32_t y,
	                  InteractiveBase& igb,
	                  Widelands::Building& building,
	                  const Widelands::InputQueue& queue,
	                  bool no_capacity_buttons = false,
	                  bool no_priority_buttons = false);
	// Constructor for fake queues (e.g. in ConstructionSite settings)
	InputQueueDisplay(UI::Panel* parent,
	                  int32_t x,
	                  int32_t y,
	                  InteractiveBase&,
	                  Widelands::ConstructionSite&,
	                  Widelands::WareWorker,
	                  Widelands::DescriptionIndex,
	                  bool no_capacity_buttons = false,
	                  bool no_priority_buttons = false);
	~InputQueueDisplay() override;

	void think() override;
	void draw(RenderTarget&) override;

private:
	InteractiveBase& interactive_base_;
	Widelands::Building& building_;
	const Widelands::InputQueue* queue_;
	const Widelands::ProductionsiteSettings* settings_;
	UI::Radiogroup* priority_radiogroup_;
	UI::Button* increase_max_fill_;
	UI::Button* decrease_max_fill_;
	UI::Button* increase_real_fill_;
	UI::Button* decrease_real_fill_;
	Widelands::DescriptionIndex index_;
	Widelands::WareWorker type_;
	const Image* icon_;  //< Index to ware's picture
	const Image* max_fill_indicator_;

	uint32_t cache_size_;
	uint32_t cache_max_fill_;
	uint32_t total_height_;
	bool no_capacity_buttons_;
	bool no_priority_buttons_;

	virtual void max_size_changed();
	void update_priority_buttons();
	void update_max_fill_buttons();
	void decrease_max_fill_clicked();
	void increase_max_fill_clicked();
	void decrease_real_fill_clicked();
	void increase_real_fill_clicked();
	void radiogroup_changed(int32_t);
	void radiogroup_clicked();
	void update_siblings_priority(int32_t);
	void update_siblings_fill(int32_t);

	uint32_t check_max_size() const;
	uint32_t check_max_fill() const;

	void compute_max_fill_buttons_enabled_state();
};

#endif  // end of include guard: WL_WUI_INPUTQUEUEDISPLAY_H
