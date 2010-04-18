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

#include "logic/building.h"
#include "ui_basic/window.h"

using Widelands::Building;

/**
 * Create the building's options window if necessary and bring it to
 * the top to be seen by the player.
 */
void Building::show_options(Interactive_GameBase & igbase)
{
	if (m_optionswindow)
		m_optionswindow->move_to_top();
	else
		create_options_window(igbase, m_optionswindow);
}

/**
 * Remove the building's options window.
 */
void Building::hide_options()
{
	delete m_optionswindow;
}

