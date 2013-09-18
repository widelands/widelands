/*
 * Copyright (C) 2002-2004, 2006-2013 by the Widelands Development Team
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

#ifndef _SOLDIERCAPACITYCONTROL_H_
#define _SOLDIERCAPACITYCONTROL_H_

class Interactive_GameBase;

namespace UI {
struct Panel;
}

namespace Widelands {
class Building;
}

UI::Panel * create_soldier_capacity_control
	(UI::Panel & parent,
	 Interactive_GameBase & igb,
	 Widelands::Building & building);

#endif // _SOLDIERLIST_H_
