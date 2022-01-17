/*
 * Copyright (C) 2019-2022 by the Widelands Development Team
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

#ifndef WL_GRAPHIC_NOTE_GRAPHIC_RESOLUTION_CHANGED_H
#define WL_GRAPHIC_NOTE_GRAPHIC_RESOLUTION_CHANGED_H

#include "notifications/note_ids.h"
#include "notifications/notifications.h"

// Will be sent whenever the resolution changes.
struct GraphicResolutionChanged {
	CAN_BE_SENT_AS_NOTE(NoteId::GraphicResolutionChanged)

	// Old width and height in pixels.
	int old_width;
	int old_height;

	// New width and height in pixels.
	int new_width;
	int new_height;
};

#endif  // end of include guard: WL_GRAPHIC_NOTE_GRAPHIC_RESOLUTION_CHANGED_H
