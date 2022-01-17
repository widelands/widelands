/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#ifndef WL_UI_BASIC_NOTE_LOADING_MESSAGE_H
#define WL_UI_BASIC_NOTE_LOADING_MESSAGE_H

#include <string>

#include "notifications/note_ids.h"
#include "notifications/notifications.h"

namespace UI {
struct NoteLoadingMessage {
	CAN_BE_SENT_AS_NOTE(NoteId::LoadingMessage)

	const std::string message;

	explicit NoteLoadingMessage(const std::string& init_message) : message(init_message) {
	}
};
}  // namespace UI

#endif  // end of include guard: WL_UI_BASIC_NOTE_LOADING_MESSAGE_H
