/*
 * Copyright (C) 2008 by the Widelands Development Team
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

#ifndef CHAT_H
#define CHAT_H

#include <string>

#include "notification.h"

/**
 * Represents one chat message.
 */
struct ChatMessage {
	/**
	 * The (real-)time at which the message was received.
	 */
	int32_t time;

	/**
	 * A string identifying the sender of the message.
	 *
	 * This string is empty for system-generated messages.
	 *
	 * \note This is a string instead of an ID because the backlog of
	 * chat messages might contain chat from a player who has since left
	 * the game.
	 */
	std::string sender;

	/**
	 * The actual chat message
	 */
	std::string msg;


	/**
	 * \return a string that can be displayed to the user.
	 */
	std::string toPrintable() const;
};


/**
 * Provides the chatting interface during a game.
 *
 * Use this interface to send chat messages and to access the list of
 * received chat messages. Note that this class is a \ref Widelands::NoteSender<ChatMessage>
 * and sends a notification every time a new message is received.
 */
struct ChatProvider : public Widelands::NoteSender<ChatMessage> {
	virtual ~ChatProvider() {}

	/**
	 * Send the given chat message.
	 *
	 * The message may or may not appear in subsequent calls to \ref getMessages.
	 */
	virtual void send(const std::string& msg) = 0;

	/**
	 * \return a (chronological) list of received chat messages.
	 * This list need not be stable or monotonic. In other words,
	 * subsequent calls to this functions may return a smaller or
	 * greater number of chat messages.
	 */
	virtual const std::vector<ChatMessage>& getMessages() const = 0;

protected:
	void send(const ChatMessage& c) {Widelands::NoteSender<ChatMessage>::send(c);}
};

#endif // CHAT_H
