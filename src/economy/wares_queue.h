/*
 * Copyright (C) 2004, 2006-2011 by the Widelands Development Team
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

#ifndef WARES_QUEUE_H
#define WARES_QUEUE_H

#include "logic/immovable.h"
#include "logic/widelands.h"

namespace Widelands {

class Economy;
class Editor_Game_Base;
class Game;
class Map_Map_Object_Loader;
struct Map_Map_Object_Saver;
class Player;
class Request;
class WaresQueue;
class Worker;

/**
 * This micro storage room can hold any number of items of a fixed ware.
 */
class WaresQueue {
public:
	typedef void (callback_t)
		(Game &, WaresQueue *, Ware_Index ware, void * data);

	WaresQueue(PlayerImmovable &, Ware_Index, uint8_t size);

#ifndef NDEBUG
	~WaresQueue() {assert(not m_ware);}
#endif

	Ware_Index get_ware()   const          {return m_ware;}
	uint32_t get_max_fill() const {return m_max_fill;}
	uint32_t get_max_size() const {return m_max_size;}
	uint32_t get_filled()   const {return m_filled;}

	void cleanup();

	void set_callback(callback_t *, void * data);

	void remove_from_economy(Economy &);
	void add_to_economy(Economy &);

	void set_max_size        (uint32_t);
	void set_max_fill        (uint32_t);
	void set_filled          (uint32_t);
	void set_consume_interval(uint32_t);

	Player & owner() const {return m_owner.owner();}

	void Read (FileRead  &, Game &, Map_Map_Object_Loader &);
	void Write(FileWrite &, Game &, Map_Map_Object_Saver  &);

private:
	static void request_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);
	void update();

	PlayerImmovable & m_owner;
	Ware_Index        m_ware;    ///< ware ID
	uint32_t m_max_size;         ///< nr of items that fit into the queue maximum
	uint32_t m_max_fill;         ///< nr of wares that should be ideally in this queue
	uint32_t m_filled;           ///< nr of items that are currently in the queue

	///< time in ms between consumption at full speed
	uint32_t m_consume_interval;

	Request         * m_request; ///< currently pending request

	callback_t      * m_callback_fn;
	void            * m_callback_data;
};

}

#endif
