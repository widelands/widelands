/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "editor_noise_height_tool.h"

#include "editor_increase_height_tool.h"
#include "editor_decrease_height_tool.h"
#include "editor_set_height_tool.h"
#include "mapregion.h"
#include "field.h"
#include "editorinteractive.h"
#include <stdint.h>

/// Sets the heights to random values. Chages surrounding nodes if necessary.
int Editor_Noise_Height_Tool::handle_click_impl
(Map & map, const Node_and_Triangle<> center, Editor_Interactive & parent)
{
	uint32_t max = 0;
	MapRegion<Area<FCoords> > mr
		(map,
		 Area<FCoords>(map.get_fcoords(center.node), parent.get_sel_radius()));
	do max =
		std::max
		(max,
		 map.set_height
		 (mr.location(),
		  m_interval.min
		  +
		  static_cast<int>
		  (static_cast<double>
		   (m_interval.min - m_interval.max) * rand() / (RAND_MAX + 1.0))));
	while (mr.advance(map));
	return mr.radius() + max;
}
