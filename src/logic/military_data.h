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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef MILITARY_DATA_H
#define MILITARY_DATA_H

#include <map>
#include <vector>

#include "logic/bob.h"
#include "logic/building.h"
#include "graphic/animation.h"
#include "logic/immovable.h"
#include "io/filewrite.h"
#include "profile/profile.h"
#include "logic/worker.h"

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
	uint8_t        get_max_retreat() const {return m_max_retreat;}
	uint8_t        get_min_retreat() const {return m_min_retreat;}
	uint8_t        get_retreat()     const {return m_retreat;}
private:
	uint8_t              m_min_retreat;
	uint8_t              m_max_retreat;
	uint8_t              m_retreat;
};

}

#endif
