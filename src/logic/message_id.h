/*
 * Copyright (C) 2010-2024 by the Widelands Development Team
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

#ifndef WL_LOGIC_MESSAGE_ID_H
#define WL_LOGIC_MESSAGE_ID_H

#include <cstdint>

namespace Widelands {

/// Each message has an id that is unique per message queue. (Each player has
/// his own message queue.) Ids are not reused while the simulation is running.
/// But when the game is saved, Ids are of course not saved with it. Instead,
/// all messages are just saved in sequence. When the game is loaded again,
/// messages will get new ids starting at 1. The value 0 is a special null
/// value that can be used to represent an unset reference to a message.
///
/// Because of this renumbering, the code that saves a message must translate
/// the id that it has to the sequence number in the savegame of that message.
/// MapMessageSaver does that.
struct MessageId {
	MessageId() = default;
	explicit MessageId(uint32_t const init_id) : id_(init_id) {
	}

	/// Constant value for no message.
	[[nodiscard]] static MessageId null() {
		return MessageId(0);
	}

	[[nodiscard]] bool operator==(const MessageId& other) const {
		return id_ == other.id_;
	}
	[[nodiscard]] bool operator!=(const MessageId& other) const {
		return id_ != other.id_;
	}
	[[nodiscard]] bool operator<(const MessageId& other) const {
		return id_ < other.id_;
	}
	[[nodiscard]] bool valid() const {
		return *this != null();
	}
	[[nodiscard]] uint32_t value() const {
		return id_;
	}

private:
	friend struct MapMessageSaver;
	friend struct MessageQueue;
	MessageId operator++() {
		++id_;
		return *this;
	}
	uint32_t id_{0U};
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MESSAGE_ID_H
