/*
 * Copyright (C) 2002, 2010-2010 by the Widelands Development Team
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

#ifndef MILITARY_DATA_H
#define MILITARY_DATA_H

#include "graphic/animation.h"
#include "bob.h"
#include "building.h"
#include "descr_maintainer.h"
#include "immovable.h"
#include "item_ware_descr.h"
#include "worker.h"
#include "HTMLReferences.h"

#include "io/filewrite.h"
#include "profile/profile.h"

#include <map>
#include <vector>

namespace Widelands {

/*
Military Configuration
------
Stores and retrieve configuration for military configuration. By now only
stores info about retreat_when_injured param.
*/
struct Military_Data {

	Military_Data();

	void           parse(Section &);
	uint8_t        get_max_retreat() {return m_max_retreat;}
	uint8_t        get_min_retreat() {return m_min_retreat;}
	uint8_t        get_retreat()     {return m_retreat;}
private:
	uint8_t              m_min_retreat;
	uint8_t              m_max_retreat;
	uint8_t              m_retreat;
};

}

#endif
