/*
 * Copyright (C) 2008-2011 by the Widelands Development Team
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

#ifndef WL_CHAT_CHAT_H
#define WL_CHAT_CHAT_H

#include <ctime>
#include <string>
#include <vector>

#include "notifications/note_ids.h"
#include "notifications/notifications.h"

// A chat message as received in game.
struct ChatMessage {
	CAN_BE_SENT_AS_NOTE(NoteId::ChatMessage)

	// The (real-)time at which the message was received.
	time_t time;

	// The playercolor. Used to colorize the senders name; negative numbers
	// indicate system messages for which richtext is allowed.
	int16_t playern;

	// A string identifying the sender of the message.
	// This string is empty for system-generated messages.
	// This is a string instead of an ID because the backlog of
	// chat messages might contain chat from a player who has since left
	// the game.
	std::string sender;

	// A string identifying the recipient of the message. This string should
	// only be filled for personal messages. This is a string instead of an ID
	// because the backlog of chat messages might contain chat from a player who
	// has since left the game.
	std::string recipient;

	// The actual chat message
	std::string msg;
};

// Sends chat messages and owns the list of received chat messages.
// Base classes must broadcast a ChatMessage as notification when a
// new message is received.
struct ChatProvider {
	virtual ~ChatProvider();

	// Send the given chat message. The message may or may not
	// appear in subsequent calls to \ref get_messages.
	virtual void send(const std::string&) = 0;

	// \return a (chronological) list of received chat messages.
	// This list need not be stable or monotonic. In other words,
	// subsequent calls to this functions may return a smaller or
	// greater number of chat messages.
	virtual const std::vector<ChatMessage>& get_messages() const = 0;

	// reimplemented e.g. in internet_gaming to silence the chat if in game.
	// TODO(sirver): this does not belong here. The receiver of the
	// notifications should deal with this.
	virtual bool sound_off() {
		return false;
	}

	// The specific chat provider subclass might not have been set, e.g. due to an exception.
	virtual bool has_been_set() const {
		return false;
	}
};

#endif  // end of include guard:
