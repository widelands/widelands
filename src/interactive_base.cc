/*
 * Copyright (C) 2002 by The Widelands Development Team
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

#include "widelands.h"
#include "interactive_base.h"


/*
==============================================================================

Interactive_Base IMPLEMENTATION

==============================================================================
*/

/*
===============
Interactive_Base::Interactive_Base
 
Initialize
===============
*/
Interactive_Base::Interactive_Base(void)
{
	memset(&m_maprenderinfo, 0, sizeof(m_maprenderinfo));
	
	m_fieldsel_freeze = false;
}

/*
===============
Interactive_Base::~Interactive_Base

cleanups
===============
*/
Interactive_Base::~Interactive_Base(void)
{
	if (m_maprenderinfo.overlay_basic)
		free(m_maprenderinfo.overlay_basic);
	if (m_maprenderinfo.overlay_roads)
		free(m_maprenderinfo.overlay_roads);
}


/*
===============
Interactive_Base::set_fieldsel

Change the field selection. Does not honour the freeze!
===============
*/
void Interactive_Base::set_fieldsel(Coords c)
{
	m_maprenderinfo.fieldsel = c;
}


/*
===============
Interactive_Base::set_fieldsel_freeze

Field selection is frozen while the field action dialog is visible
===============
*/
void Interactive_Base::set_fieldsel_freeze(bool yes)
{
	m_fieldsel_freeze = yes;
}

