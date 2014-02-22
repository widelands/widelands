/*
 * Copyright (C) 2002-2011 by the Widelands Development Team
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

#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <cassert>
#include <map>

#include <boost/noncopyable.hpp>

#include "logic/message.h"
#include "logic/message_id.h"

namespace Widelands {

struct MessageQueue : boost::noncopyable, private std::map<Message_Id, Message *> {
	friend struct Map_Players_Messages_Data_Packet;
	// Make typedefs public so that this looks like proper
	// STL container to templated algorithms.
	typedef std::map<Message_Id, Message *> _Mybase;
	typedef _Mybase::pointer pointer;
	typedef _Mybase::const_pointer const_pointer;
	typedef _Mybase::reference reference;
	typedef _Mybase::const_reference const_reference;
	typedef _Mybase::iterator iterator;
	typedef _Mybase::const_iterator const_iterator;
	MessageQueue() { //  C++0x: MessageQueue() : m_counts({}) {}
		m_counts[Message::New]      = 0; //  C++0x:
		m_counts[Message::Read]     = 0; //  C++0x:
		m_counts[Message::Archived] = 0; //  C++0x:
	}                                   //  C++0x:

	~MessageQueue() {
		while (size()) {
			delete begin()->second;
			erase(std::map<Message_Id, Message *>::begin());
		}
	}

	//  Make some selected inherited members public.
	const_iterator begin() const {
		return std::map<Message_Id, Message *>::begin();
	}
	const_iterator end() const {
		return std::map<Message_Id, Message *>::end();
	}
	size_type count(uint32_t const i) const {
		assert_counts();
		return std::map<Message_Id, Message *>::count(Message_Id(i));
	}

	/// \returns a pointer to the message if it exists, otherwise 0.
	Message const * operator[](const Message_Id& id) const {
		assert_counts();
		const_iterator const it = find(Message_Id(id));
		return it != end() ? it->second : nullptr;
	}

	/// \returns the number of messages with the given status.
	uint32_t nr_messages(Message::Status const status) const {
		assert_counts();
		assert(status < 3);
		return m_counts[status];
	}

	/// Adds the message. Takes ownership of the message. Assumes that it has
	/// been allocated in a separate memory block (not as a component of an
	/// array or struct) with operator new, so that it can be deallocated with
	/// operator delete.
	///
	/// \returns the id of the added message.
	///
	/// \Note The caller must make sure that a command is scheduled to expire
	/// the message. Player::add_message does this and should be used for adding
	/// messages to a player during the simulation.
	///
	/// The loading code calls this function to add messages form the map file.
	/// The commands to expire messages are not saved with the map. Therefore
	/// the loading code must create them.
	Message_Id add_message(Message & message) {
		assert_counts();
		assert(message.status() < 3);
		++m_counts[message.status()];
		insert
			(std::map<Message_Id, Message *>::end(),
			 std::pair<Message_Id, Message *>(++m_current_message_id, &message));
		assert_counts();
		return m_current_message_id;
	}

	/// Sets the status of the message with the given id, if it exists.
	void set_message_status(const Message_Id& id, Message::Status const status) {
		assert_counts();
		assert(status < 3);
		iterator const it = find(id);
		if (it != end()) {
			Message & message = *it->second;
			assert(it->second->status() < 3);
			assert(m_counts[message.status()]);
			--m_counts[message.status    ()];
			++m_counts[message.set_status(status)];
		}
		assert_counts();
	}

	/// Expire the message with the given id so that it no longer exists.
	/// Assumes that a message with the given id exists.
	void expire_message(const Message_Id& id) {
		assert_counts();
		iterator const it = find(id);
		if (it == end()) {
			// Messages can be expired when the timeout runs out, or when the linked
			// Map_Object is removed, or both. In this later case, two expire commands
			// will be executed, and the message will not be present for the second one.
			// So we assume here that the message was removed from an earlier expire cmd.
			return;
		}
		Message & message = *it->second;
		assert(message.status() < 3);
		assert(m_counts[message.status()]);
		--m_counts[message.status()];
		delete &message;
		erase(it);
		assert_counts();
	}

	Message_Id current_message_id() const {return m_current_message_id;}

	/// \returns whether all messages with id 1, 2, 3, ..., current_message_id
	/// exist. This should be the case when messages have been loaded from a map
	/// file/savegame but the simulation has not started to run yet.
	bool is_continuous() const {
		assert_counts();
		return current_message_id().value() == size();
	}

private:
	/// Only for working around bugs in map loading code. If something has
	/// accidentally been added to the queue during load, it can be worked
	/// around by clearing the queue before the saved messages are loaded into
	/// it.
	void clear() {
		assert_counts();
		m_current_message_id        = Message_Id::Null();
		m_counts[Message::New]      = 0; //  C++0x: m_counts = {};
		m_counts[Message::Read]     = 0; //  C++0x:
		m_counts[Message::Archived] = 0; //  C++0x:
		std::map<Message_Id, Message *>::clear();
		assert_counts();
	}

	/// The id of the most recently added message, or null if none has been
	/// added yet.
	Message_Id m_current_message_id;

	/// Number of messages with each status (new, read, deleted).
	/// Indexed by Message::Status.
	uint32_t   m_counts[3];

	void assert_counts() const {
		assert
			(size() ==
			 m_counts[Message::New]  +
			 m_counts[Message::Read] +
			 m_counts[Message::Archived]);
	}
};

}

#endif
