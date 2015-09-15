/*
 * Copyright (C) 2002-2010 by the Widelands Development Team
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

#include <string>
#include <boost/signals2.hpp>

#include "logic/widelands.h"
#include "logic/widelands_geometry.h"

namespace Widelands {

struct Message {
	enum class Status: uint8_t {kNew, kRead, kArchived};
	enum class Type: uint8_t {
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
		kEconomy,      // economy
		kEconomySiteOccupied, // economy
		kWarfare,     // everything starting from here is warfare
		kWarfareSiteDefeated,
		kWarfareSiteLost,
		kWarfareUnderAttack
	};

	/**
	 * A new message to be displayed to the player
	 * \param msgtype The type of message (economy, geologists, etc.)
	 * \param sent_time The (game) time at which the message is sent
	 * \param t The message title
	 * \param b The message body
	 * \param c The message coords. The player will be able to taken there.
	 * Defaults to Coords::null()
	 * \param ser A MapObject serial. If non null, the message will be deleted once
	 * the object is removed from the game. Defaults to 0
	 * \param s The message status. Defaults to Status::New
	 */
	Message
		(Message::Type             msgtype,
		 uint32_t                  sent_time,
		 const std::string &       t,
		 const std::string &       b,
		 Widelands::Coords   const c = Coords::null(),
		 Widelands::Serial         ser = 0,
		 Status                    s = Status::kNew)
		:
		m_type    (msgtype),
		m_title   (t),
		m_body    (b),
		m_sent    (sent_time),
		m_position(c),
		m_serial  (ser),
		m_status  (s)
	{}

	Message::Type         type    () const   {return m_type;}
	uint32_t              sent    () const   {return m_sent;}
	const std::string &   title   () const   {return m_title;}
	const std::string &   body    () const   {return m_body;}
	Widelands::Coords     position() const   {return m_position;}
	Widelands::Serial     serial  () const   {return m_serial;}
	Status                status  () const   {return m_status;}
	Status set_status(Status const s)        {return m_status = s;}

	/**
	 * Returns the main type for the message's sub type
	 */
	Message::Type message_type_category() const {
		if (m_type >=  Widelands::Message::Type::kWarfare) {
			return Widelands::Message::Type::kWarfare;

		} else if (m_type >= Widelands::Message::Type::kEconomy &&
					  m_type <= Widelands::Message::Type::kEconomySiteOccupied) {
			return Widelands::Message::Type::kEconomy;
		} else if (m_type >= Widelands::Message::Type::kGeologists &&
					 m_type <= Widelands::Message::Type::kGeologistsWater) {
		  return Widelands::Message::Type::kGeologists;
	  }
		return m_type;
	}

private:
	Message::Type     m_type;
	std::string       m_title;
	std::string       m_body;
	uint32_t          m_sent;
	Widelands::Coords m_position;
	Widelands::Serial m_serial; // serial to map object
	Status            m_status;
};

}

#endif  // end of include guard: WL_LOGIC_MESSAGE_H
