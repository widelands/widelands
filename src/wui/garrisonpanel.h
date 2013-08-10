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

#ifndef GARRISON_PANEL_H
#define GARRISON_PANEL_H

#include "ui_basic/box.h"
#include "ui_basic/radiobutton.h"
#include "ui_basic/textarea.h"
#include "logic/instances.h"
#include "wui/soldierlist.h"

#include <boost/function.hpp>


namespace Widelands {
class Soldier;
class Garrison;
}

class Interactive_GameBase;
using Widelands::Soldier;

namespace UI {

/**
* List of soldiers \ref MilitarySiteWindow and \ref TrainingSiteWindow
*/
struct GarrisonPanel : Box {
	GarrisonPanel
		(Panel & parent,
		 Interactive_GameBase & igb,
		 Widelands::Garrison & garrison);

private:
	void mouseover(const Soldier * soldier);
	void eject(const Soldier * soldier);
	void set_soldier_preference(int32_t changed_to);

	Interactive_GameBase & m_igb;
	Widelands::Garrison & m_garrison;
	SoldierPanel m_soldierpanel;
	Radiogroup m_soldier_preference;
	Textarea m_infotext;
};

}

#endif
