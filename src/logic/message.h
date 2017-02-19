/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#ifndef WL_LOGIC_MESSAGE_H
#define WL_LOGIC_MESSAGE_H

#include <boost/signals2.hpp>
#include <string>

#include "graphic/graphic.h"
#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

struct Message {
	enum class Status : uint8_t { kNew, kRead, kArchived };
	enum class Type : uint8_t {
		kNoMessages,
		kAllMessages,
		kGameLogic,
		kGeologists,
		kGeologistsCoal,
		kGeologistsGold,
		kGeologistsStones,
		kGeologistsIron,
		kGeologistsWater,
		kScenario,
		kSeafaring,
		kEconomy,              // economy
		kEconomySiteOccupied,  // economy
		kWarfare,              // everything starting from here is warfare
		kWarfareSiteDefeated,
		kWarfareSiteLost,
		kWarfareUnderAttack
	};

	/**
	 * A new message to be displayed to the player
	 * \param msgtype    The type of message (economy, geologists, etc.)
	 * \param sent_time  The (game) time at which the message is sent
	 * \param init_title The intial message title
	 * \param init_body  The initial message body
	 * \param c          The message coords. The player will be able to position its view there.
	 *                   Defaults to Coords::null()
	 * \param ser        A MapObject serial. If non null, the message will be deleted once
	 *                   the object is removed from the game. Defaults to 0
	 * \param s          The message status. Defaults to Status::New
	 */
	Message(Message::Type msgtype,
	        uint32_t sent_time,
	        const std::string& init_title,
	        const std::string& init_icon_filename,
	        const std::string& init_heading,
	        const std::string& init_body,
	        const Widelands::Coords& c = Coords::null(),
	        Widelands::Serial ser = 0,
	        Status s = Status::kNew,
	        bool new_renderer = false)
	   : type_(msgtype),
	     title_(init_title),
	     icon_filename_(init_icon_filename),
	     icon_(g_gr->images().get(init_icon_filename)),
	     heading_(init_heading),
	     body_(init_body),
	     sent_(sent_time),
	     position_(c),
	     serial_(ser),
	     status_(s),
	     use_new_renderer_(new_renderer) {
	}

	Message::Type type() const {
		return type_;
	}
	uint32_t sent() const {
		return sent_;
	}
	const std::string& title() const {
		return title_;
	}
	const std::string& icon_filename() const {
		return icon_filename_;
	}
	const Image* icon() const {
		return icon_;
	}
	const std::string& heading() const {
		return heading_;
	}
	const std::string& body() const {
		return body_;
	}
	Widelands::Coords position() const {
		return position_;
	}
	Widelands::Serial serial() const {
		return serial_;
	}
	Status status() const {
		return status_;
	}
	Status set_status(Status const s) {
		return status_ = s;
	}

	bool use_new_renderer() const {
		return use_new_renderer_;
	}

	/**
	 * Returns the main type for the message's sub type
	 */
	Message::Type message_type_category() const {
		if (type_ >= Widelands::Message::Type::kWarfare) {
			return Widelands::Message::Type::kWarfare;

		} else if (type_ >= Widelands::Message::Type::kEconomy &&
		           type_ <= Widelands::Message::Type::kEconomySiteOccupied) {
			return Widelands::Message::Type::kEconomy;
		} else if (type_ >= Widelands::Message::Type::kGeologists &&
		           type_ <= Widelands::Message::Type::kGeologistsWater) {
			return Widelands::Message::Type::kGeologists;
		}
		return type_;
	}

private:
	Message::Type type_;
	const std::string title_;
	const std::string icon_filename_;
	const Image* icon_;  // Pointer to icon into picture stack
	const std::string heading_;
	const std::string body_;
	uint32_t sent_;
	Widelands::Coords position_;
	Widelands::Serial serial_;  // serial to map object
	Status status_;
	// TODO(GunChleoc): Temporary switch while we migrate to the new font renderer
	bool use_new_renderer_;
};
}

#endif  // end of include guard: WL_LOGIC_MESSAGE_H
