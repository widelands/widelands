/*
 * Copyright (C) 2002-2019 by the Widelands Development Team
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

#ifndef WL_WUI_SOLDIERLIST_H
#define WL_WUI_SOLDIERLIST_H

#include "ui_basic/box.h"
#include "ui_basic/button.h"
#include "ui_basic/slider.h"
#include "ui_basic/textarea.h"
#include "ui_basic/window.h"

class InteractiveBase;

namespace Widelands {
class Building;
class EditorGameBase;
class Soldier;
}  // namespace Widelands

struct SoldierSettings : UI::Window {
public:
	SoldierSettings(InteractiveBase& ib, Widelands::Soldier& s, bool);
	~SoldierSettings() {
	}

private:
	Widelands::EditorGameBase& egbase_;
	Widelands::Soldier& soldier_;

	UI::Box main_box_, upper_box_, label_box_, slider_box_;
	UI::HorizontalSlider health_, attack_, defense_, evade_, current_health_;
	UI::Button delete_, cancel_, ok_;
	UI::Textarea hlabel_, alabel_, dlabel_, elabel_, clabel_;

	void update_label_h();
	void update_label_a();
	void update_label_d();
	void update_label_e();
	void update_label_c();
	void health_slider_changed();
	void clicked_ok();
	void clicked_delete();
};

UI::Panel*
create_soldier_list(UI::Panel& parent, InteractiveBase& igb, Widelands::Building& building);

#endif  // end of include guard: WL_WUI_SOLDIERLIST_H
