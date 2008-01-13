/*
 * Copyright (C) 2002-2005, 2007-2008 by the Widelands Development Team
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

#include "trigger.h"

#include "filesystem.h"
#include "game.h"
#include "map.h"
#include "trigger_referencer.h"

namespace Widelands {

/*
 * Trigger himself
 */
/*
 * reference or unrefereence this Trigger
 */
void Trigger::reference(TriggerReferencer* ref) {
	++m_referencers[ref];
}
void Trigger::unreference(TriggerReferencer* ref) {
   std::map<TriggerReferencer*, uint32_t>::iterator cur = m_referencers.find(ref);
	if (cur != m_referencers.end()) {
        cur->second--;
		if (cur->second == 0)
           m_referencers.erase(cur);
	}
}

};
