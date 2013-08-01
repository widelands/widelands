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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WIDELANDS_MAP_MESSAGE_SAVER_H
#define WIDELANDS_MAP_MESSAGE_SAVER_H

#include <cassert>
#include <map>

#include <stdint.h>

#include "logic/message_id.h"

namespace Widelands {

/// A map used during save to store the sequence number in the saved file of
/// each message. (This is not equal to the message id, because when a message
/// is deleted, its id is never reused, while the sequence of messages saved to
/// file obviously has continuous numbers.) When the game is loaded, each
/// message will get its sequence number as id.
///
/// When saving a PlayerMessageCommand (Cmd_MarkMessageAsRead or
/// Cmd_DeleteMessage) that refers to a message by id, use this map to
/// translate from the id that is stored in the command to the sequence number
/// that will be used as the id of the message when the game is loaded.
struct Map_Message_Saver : private std::map<Message_Id, Message_Id> {
	Map_Message_Saver() : counter(0) {}
	void add(Message_Id const id) {
		assert(find(id) == end());
		insert(std::pair<Message_Id, Message_Id>(id, ++counter));
	}
	Message_Id operator[](Message_Id const id) const {
		return find(id) != end() ? find(id)->second : Message_Id::Null();
	}
private:
	Message_Id counter;
};

}

#endif
