/*
 * Copyright (C) 2010-2022 by the Widelands Development Team
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

#ifndef WL_MAP_IO_MAP_MESSAGE_SAVER_H
#define WL_MAP_IO_MAP_MESSAGE_SAVER_H

#include <cassert>
#include <map>

#include "logic/message_id.h"

namespace Widelands {

/// A map used during save to store the sequence number in the saved file of
/// each message. (This is not equal to the message id, because when a message
/// is deleted, its id is never reused, while the sequence of messages saved to
/// file obviously has continuous numbers.) When the game is loaded, each
/// message will get its sequence number as id.
///
/// When saving a PlayerMessageCommand (CmdMarkMessageAsRead or
/// CmdDeleteMessage) that refers to a message by id, use this map to
/// translate from the id that is stored in the command to the sequence number
/// that will be used as the id of the message when the game is loaded.
struct MapMessageSaver {
	MapMessageSaver() : counter_(0) {
	}
	void add(const MessageId& id) {
		assert(messages_.find(id) == messages_.end());
		messages_.insert(std::make_pair(id, ++counter_));
	}
	MessageId operator[](const MessageId& id) const {
		return messages_.find(id) != messages_.end() ? messages_.find(id)->second : MessageId::null();
	}

private:
	std::map<MessageId, MessageId> messages_;
	MessageId counter_;
};
}  // namespace Widelands

#endif  // end of include guard: WL_MAP_IO_MAP_MESSAGE_SAVER_H
