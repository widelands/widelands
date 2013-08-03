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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "wui/buildingwindow.h"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "logic/building.h"
#include "ui_basic/window.h"
#include "upcast.h"

using Widelands::Building;

/**
 * Create the building's options window if necessary and bring it to
 * the top to be seen by the player.
 */
void Building::show_options(Interactive_GameBase & igbase, bool avoid_fastclick, Point pos)
{
	if (m_optionswindow) {
		if (m_optionswindow->is_minimal())
			m_optionswindow->restore();
		m_optionswindow->move_to_top();
	} else {
		create_options_window(igbase, m_optionswindow);
		if (upcast(Building_Window, bw, m_optionswindow)) {
			bw->set_avoid_fastclick(avoid_fastclick);
		}
		// Run a first think here so that certain things like caps buttons
		// get properly initialized
		m_optionswindow->think();
	}
	if (pos.x >= 0 && pos.y >= 0) {
		m_optionswindow->set_pos(pos);
	}
}

/**
 * Remove the building's options window.
 */
void Building::hide_options()
{
	BOOST_FOREACH(boost::signals2::connection& c, options_window_connections)
		c.disconnect();
	delete m_optionswindow;
	m_optionswindow = nullptr;
}

/**
 * refreshs the option window of a building - useful if some ui elements have to be removed or added
 */
void Building::refresh_options(Interactive_GameBase & igb) {
	// Only do something if there is actually a window
	if (m_optionswindow) {
		Point window_position = m_optionswindow->get_pos();
		hide_options();
		show_options(igb, true);
		m_optionswindow->set_pos(window_position);
	}
}
