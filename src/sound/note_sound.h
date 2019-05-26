/*
 * Copyright (C) 2017-2019 by the Widelands Development Team
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

#ifndef WL_SOUND_NOTE_SOUND_H
#define WL_SOUND_NOTE_SOUND_H

#include <string>

#include "logic/widelands_geometry.h"
#include "notifications/note_ids.h"
#include "notifications/notifications.h"
#include "sound/constants.h"

struct NoteSound {
	CAN_BE_SENT_AS_NOTE(NoteId::Sound)
	const SoundType type;
	const FxId fx;
	const Widelands::Coords coords;
	const uint8_t priority;

	NoteSound(SoundType init_type,
	          FxId init_fx,
	          Widelands::Coords init_coords,
	          uint8_t init_priority)
	   : type(init_type), fx(init_fx), coords(init_coords), priority(init_priority) {
	}
};

#endif  // end of include guard: WL_SOUND_NOTE_SOUND_H
