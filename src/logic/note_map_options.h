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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_NOTE_MAP_OPTIONS_H
#define WL_LOGIC_NOTE_MAP_OPTIONS_H

#include "notifications/note_ids.h"
#include "notifications/notifications.h"

struct NoteMapOptions {
	CAN_BE_SENT_AS_NOTE(NoteId::MapOptions)

	NoteMapOptions() = default;
};

#endif  // end of include guard: WL_LOGIC_NOTE_MAP_OPTIONS_H
