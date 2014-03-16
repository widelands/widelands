/*
 * Copyright (C) 2002-2004, 2006-2010, 2012 by the Widelands Development Team
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

#ifndef _BUILDINGWINDOW_H_
#define _BUILDINGWINDOW_H_

#include <cstdlib>

#include "wui/interactive_gamebase.h"
#include "ui_basic/button.h"
#include "ui_basic/window.h"
#include "wui/waresdisplay.h"

/**
 * Base class for all building windows.
 *
 * This class is sub-classed for all building types to provide something useful.
 */
struct Building_Window : public UI::Window {
	friend struct TrainingSite_Window;
	friend struct MilitarySite_Window;
	enum {
		Width = 4 * 34 //  4 normally sized buttons
	};

	Building_Window
		(Interactive_GameBase & parent, Widelands::Building &, UI::Window * & registry);

	virtual ~Building_Window();

	Widelands::Building & building() {return m_building;}

	Interactive_GameBase & igbase() const {
		return ref_cast<Interactive_GameBase, UI::Panel>(*get_parent());
	}

	virtual void draw(RenderTarget &) override;
	virtual void think() override;
	void set_avoid_fastclick(bool afc) {m_avoid_fastclick = afc;}

protected:
	UI::Tab_Panel * get_tabs() {return m_tabs;}

	void help_clicked();
	void act_bulldoze();
	void act_dismantle();
	void act_debug();
	void show_workarea();
	void hide_workarea();
	void toggle_workarea();
	void configure_workarea_button();
	void act_start_stop();
	void act_start_or_cancel_expedition();
	void act_enhance(Widelands::Building_Index);
	void clicked_goto();

	void create_ware_queue_panel
		(UI::Box *, Widelands::Building &, Widelands::WaresQueue *, bool = false);

	virtual void create_capsbuttons(UI::Box * buttons);

	UI::Window * & m_registry;

private:
	UI::UniqueWindow::Registry m_helpwindow_registry;
	Widelands::Building & m_building;

	UI::Tab_Panel * m_tabs;

	UI::Box * m_capsbuttons; ///< \ref UI::Box that contains capabilities buttons
	UI::Button * m_toggle_workarea;

	//  capabilities that were last used in setting up the caps panel
	uint32_t m_capscache;
	Widelands::Player_Number m_capscache_player_number;
	bool m_caps_setup;

	Overlay_Manager::Job_Id m_workarea_job_id;
	bool m_avoid_fastclick;
};

#endif // _BUILDINGWINDOW_H_
