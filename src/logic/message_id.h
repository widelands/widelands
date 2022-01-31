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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_LOGIC_MESSAGE_ID_H
#define WL_LOGIC_MESSAGE_ID_H

#include <cstdint>

#include "base/macros.h"

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
	MessageId() : id(0) {
	}
	explicit MessageId(uint32_t const init_id) : id(init_id) {
	}

	/// Constant value for no message.
	static MessageId null() {
		MessageId result;
		result.id = 0;
		return result;
	}

	bool operator==(const MessageId& other) const {
		return id == other.id;
	}
	bool operator!=(const MessageId& other) const {
		return id != other.id;
	}
	bool operator<(const MessageId& other) const {
		return id < other.id;
	}
	operator bool() const {
		return *this != null();
	}
	uint32_t value() const {
		return id;
	}

private:
	//  This is needed to prevent operator bool from being applied when someone
	//  tries to use a MessageId where an int*_t is needed. It will try to use
	//  this operator instead and fail because it is private. As an extra line
	//  of defense, it is marked as deprectated. In any case, the linking will
	//  fail because the function body is missing.
	operator int8_t() const __attribute__((deprecated));
	operator int16_t() const __attribute__((deprecated));
	operator int32_t() const __attribute__((deprecated));
	operator int64_t() const __attribute__((deprecated));

	friend struct MapMessageSaver;
	friend struct MessageQueue;
	MessageId operator++() {
		++id;
		return *this;
	}
	uint32_t id;
};
}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_MESSAGE_ID_H
