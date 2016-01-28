/*
 * Copyright (C) 2002-2016 by the Widelands Development Team
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

#ifndef WL_WUI_BUILDINGWINDOW_H
#define WL_WUI_BUILDINGWINDOW_H

#include <cstdlib>

#include "ui_basic/button.h"
#include "ui_basic/window.h"
#include "wui/field_overlay_manager.h"
#include "wui/interactive_gamebase.h"
#include "wui/waresdisplay.h"

/**
 * Base class for all building windows.
 *
 * This class is sub-classed for all building types to provide something useful.
 */
struct BuildingWindow : public UI::Window {
	friend struct TrainingSiteWindow;
	friend struct MilitarySiteWindow;
	enum {
		Width = 4 * 34 //  4 normally sized buttons
	};

	BuildingWindow
		(InteractiveGameBase & parent, Widelands::Building &, UI::Window * & registry);

	virtual ~BuildingWindow();

	Widelands::Building & building() {return building_;}

	InteractiveGameBase & igbase() const {
		return dynamic_cast<InteractiveGameBase&>(*get_parent());
	}

	void draw(RenderTarget &) override;
	void think() override;
	void set_avoid_fastclick(bool afc) {avoid_fastclick_ = afc;}

protected:
	UI::TabPanel * get_tabs() {return tabs_;}

	void act_bulldoze();
	void act_dismantle();
	void act_debug();
	void show_workarea();
	void hide_workarea();
	void toggle_workarea();
	void configure_workarea_button();
	void act_start_stop();
	void act_start_or_cancel_expedition();
	void act_enhance(Widelands::DescriptionIndex);
	void clicked_goto();

	void create_ware_queue_panel
		(UI::Box *, Widelands::Building &, Widelands::WaresQueue *, bool = false);

	virtual void create_capsbuttons(UI::Box * buttons);

	UI::Window * & registry_;

private:
	Widelands::Building& building_;

	UI::TabPanel * tabs_;

	UI::Box * capsbuttons_; ///< \ref UI::Box that contains capabilities buttons
	UI::Button * toggle_workarea_;

	//  capabilities that were last used in setting up the caps panel
	uint32_t capscache_;
	Widelands::PlayerNumber capscache_player_number_;
	bool caps_setup_;

	FieldOverlayManager::OverlayId workarea_overlay_id_;
	bool avoid_fastclick_;
};

#endif  // end of include guard: WL_WUI_BUILDINGWINDOW_H
