/*
 * Copyright (C) 2002-2020 by the Widelands Development Team
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

#include <memory>

#include "economy/expedition_bootstrap.h"
#include "economy/input_queue.h"
#include "logic/map_objects/tribes/building.h"
#include "ui_basic/button.h"
#include "ui_basic/tabpanel.h"
#include "ui_basic/unique_window.h"
#include "wui/interactive_base.h"

/**
 * Base class for all building windows.
 *
 * This class is sub-classed for all building types to provide something useful.
 */
struct BuildingWindow : public UI::UniqueWindow {
	friend struct TrainingSiteWindow;
	friend struct MilitarySiteWindow;
	enum {
		Width = 4 * 34  //  4 normally sized buttons
	};

protected:
	// This constructor allows setting a building description for the help button independent of the
	// base building
	BuildingWindow(InteractiveBase& parent,
	               UI::UniqueWindow::Registry& reg,
	               Widelands::Building&,
	               const Widelands::BuildingDescr&,
	               bool avoid_fastclick);

public:
	BuildingWindow(InteractiveBase& parent,
	               UI::UniqueWindow::Registry& reg,
	               Widelands::Building&,
	               bool avoid_fastclick);

	~BuildingWindow() override;

	InteractiveBase* ibase() const {
		return parent_;
	}

	void draw(RenderTarget&) override;
	void think() override;

protected:
	virtual void init(bool avoid_fastclick, bool workarea_preview_wanted);
	void die() override;

	UI::TabPanel* get_tabs() {
		return tabs_;
	}

	void act_bulldoze();
	void act_dismantle();
	void act_debug();
	void show_workarea();
	void hide_workarea(bool configure_button);
	void toggle_workarea();
	void configure_workarea_button();
	void act_start_stop();
	void act_start_or_cancel_expedition();
	void act_enhance(Widelands::DescriptionIndex, bool is_csite);
	void clicked_goto();
	void act_mute(bool all);

	Widelands::Game* const game_;

	bool is_dying_;

	void set_building_descr_for_help(const Widelands::BuildingDescr* d) {
		building_descr_for_help_ = d;
	}

private:
	void create_capsbuttons(UI::Box* buttons, Widelands::Building* building);

	// Actions performed when a NoteBuilding is received.
	void on_building_note(const Widelands::NoteBuilding& note);

	// For ports only.
	void update_expedition_button(bool expedition_was_canceled);

	InteractiveBase* parent_;

	// The building that this window belongs to
	Widelands::OPtr<Widelands::Building> building_;

	// The building description that will be used for the help button
	const Widelands::BuildingDescr* building_descr_for_help_;

	// We require this to unregister overlays when we are closed. Since the
	// building might have been destroyed by then we have to keep a copy of its
	// position around.
	const Widelands::Coords building_position_;

	std::unique_ptr<UI::Box> vbox_;

	UI::TabPanel* tabs_;

	UI::Box* capsbuttons_;  ///< \ref UI::Box that contains capabilities buttons
	UI::Button* toggle_workarea_;

	//  capabilities that were last used in setting up the caps panel
	uint32_t capscache_;
	Widelands::PlayerNumber capscache_player_number_;
	bool caps_setup_;

	bool showing_workarea_;
	bool avoid_fastclick_;

	UI::Button* expeditionbtn_;
	UI::Button* mute_this_;
	UI::Button* mute_all_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteExpeditionCanceled>>
	   expedition_canceled_subscriber_;
	std::unique_ptr<Notifications::Subscriber<Widelands::NoteBuilding>> buildingnotes_subscriber_;
	DISALLOW_COPY_AND_ASSIGN(BuildingWindow);
};

#endif  // end of include guard: WL_WUI_BUILDINGWINDOW_H
