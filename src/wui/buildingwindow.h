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

#ifndef _BUILDINGWINDOW_H_
#define _BUILDINGWINDOW_H_

#include <cstdlib>

#include "interactive_gamebase.h"
#include "ui_basic/button.h"
#include "ui_basic/window.h"

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

	virtual void draw(RenderTarget &);
	virtual void think();

	UI::Panel * create_capsbuttons(UI::Panel * parent);

protected:
	void setup_capsbuttons();

	void act_bulldoze();
	void act_debug();
	void toggle_workarea();
	void act_start_stop();
	void act_enhance(Widelands::Building_Index);
	void act_drop_soldier(uint32_t);
	void act_change_soldier_capacity(int32_t);

	void create_ware_queue_panel
		(UI::Box *, Widelands::Building &, Widelands::WaresQueue *);

protected:
	UI::Window * & m_registry;
private:
	Widelands::Building & m_building;

	UI::Panel * m_capsbuttons; //  UI::Panel that contains capabilities buttons
	UI::Callback_Button<Building_Window> * m_toggle_workarea;

	//  capabilities that were last used in setting up the caps panel
	uint32_t m_capscache;
	Widelands::Player_Number m_capscache_player_number;

	Overlay_Manager::Job_Id m_workarea_job_id;
	PictureID workarea_cumulative_picid[NUMBER_OF_WORKAREA_PICS];
};

#endif // _BUILDINGWINDOW_H_
