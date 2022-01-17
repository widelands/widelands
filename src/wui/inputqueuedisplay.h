/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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

#include <vector>

#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/widelands.h"
#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/icon.h"
#include "ui_basic/slider.h"

namespace UI {

/**
 * \brief This class delegates handling of the wheel in the priority slider to the parent.
 */
struct PrioritySlider : public HorizontalSlider {
	PrioritySlider(Panel* const parent,
	               const int32_t x,
	               const int32_t y,
	               const uint32_t w,
	               const uint32_t h,
	               const int32_t min_value,
	               const int32_t max_value,
	               const int32_t value,
	               UI::SliderStyle style,
	               const std::string& tooltip_text = std::string(),
	               const uint32_t cursor_size = 20,
	               const bool enabled = true)
	   : HorizontalSlider(parent,
	                      x,
	                      y,
	                      w,
	                      h,
	                      min_value,
	                      max_value,
	                      value,
	                      style,
	                      tooltip_text,
	                      cursor_size,
	                      enabled) {
	}

public:
	bool handle_mousewheel(int32_t, int32_t, uint16_t) override {
		return false;
	}
	void change_value_by(int32_t change) {
		set_value(get_value() + change);
	}
};

}  // namespace UI

class InteractiveBase;

// Make the given box scrolling and set its scrollbar style and max size.
// This is not strictly required, but not preparing the box like this will
// make the layout look ugly if you add many input queue displays to it.
void ensure_box_can_hold_input_queues(UI::Box&);

class InputQueueDisplay : public UI::Box {
public:
	// For real input queues
	InputQueueDisplay(UI::Panel* parent,
	                  InteractiveBase&,
	                  Widelands::Building&,
	                  Widelands::InputQueue&,
	                  bool show_only,
	                  bool has_priority);
	// For constructionsite settings
	InputQueueDisplay(UI::Panel* parent,
	                  InteractiveBase&,
	                  Widelands::ConstructionSite&,
	                  Widelands::WareWorker,
	                  Widelands::DescriptionIndex);

	~InputQueueDisplay() override = default;

protected:
	void think() override;
	void draw(RenderTarget&) override;
	void draw_overlay(RenderTarget&) override;
	bool handle_mousepress(uint8_t, int32_t, int32_t) override;
	bool handle_mousemove(uint8_t, int32_t, int32_t, int32_t, int32_t) override;
	bool handle_mousewheel(int32_t x, int32_t y, uint16_t modstate) override;

private:
	// Common constructor
	InputQueueDisplay(UI::Panel*,
	                  InteractiveBase&,
	                  Widelands::Building&,
	                  Widelands::WareWorker,
	                  Widelands::DescriptionIndex,
	                  Widelands::InputQueue*,
	                  Widelands::ProductionsiteSettings*,
	                  bool,
	                  bool);

	InteractiveBase& ibase_;
	bool can_act_, show_only_, has_priority_;

	Widelands::OPtr<Widelands::Building> building_;

	Widelands::WareWorker type_;
	Widelands::DescriptionIndex index_;

	// Exactly one of these two is non-null
	Widelands::InputQueue* queue_;
	Widelands::ProductionsiteSettings* settings_;

	Widelands::ProductionsiteSettings::InputQueueSetting* get_setting() const;

	// Run a function on this InputQueueDisplay and all its siblings
	void recurse(const std::function<void(InputQueueDisplay&)>&);

	void clicked_desired_fill(int8_t delta);
	void change_desired_fill(int8_t delta);
	void set_desired_fill(unsigned fill);
	void clicked_real_fill(int8_t delta);
	void set_priority(const Widelands::WarePriority&);
	void set_collapsed(bool);

	const Image& max_fill_indicator_;

	UI::Box vbox_, hbox_;
	UI::Button b_decrease_desired_fill_, b_increase_desired_fill_, b_decrease_real_fill_,
	   b_increase_real_fill_, collapse_;
	UI::PrioritySlider priority_;
	UI::Panel spacer_;
	const Widelands::WarePriority* slider_was_moved_;

	bool collapsed_;

	size_t nr_icons_;
	std::vector<UI::Icon*> icons_;

	int32_t fill_index_at(int32_t, int32_t) const;
	int32_t fill_index_under_mouse_;

	void hide_from_view();
};

#endif  // end of include guard: WL_WUI_INPUTQUEUEDISPLAY_H
