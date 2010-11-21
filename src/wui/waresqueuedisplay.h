/*
 * Copyright (C) 2010 by the Widelands Development Team
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

#ifndef _WARESQUEUEDISPLAY_H_
#define _WARESQUEUEDISPLAY_H_

#include <cstdlib>
#include <stdint.h>

struct Interactive_GameBase;

namespace UI {
struct Panel;
}

namespace Widelands {
struct Building;
struct WaresQueue;
}

UI::Panel * create_wares_queue_display
	(UI::Panel * parent,
	 Interactive_GameBase & igb,
	 Widelands::Building & b,
	 Widelands::WaresQueue * const wq,
	 int32_t width);

#endif // _WARESQUEUEDISPLAY_H_
