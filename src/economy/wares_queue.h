/*
 * Copyright (C) 2004, 2006-2010 by the Widelands Development Team
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

#ifndef WARES_QUEUE_H
#define WARES_QUEUE_H

// Needed for Ware_Index
#include "logic/widelands.h"
#include "logic/immovable.h"

namespace Widelands {

struct Economy;
struct Editor_Game_Base;
struct Game;
struct Map_Map_Object_Loader;
struct Map_Map_Object_Saver;
struct Player;
struct Request;
struct WaresQueue;
struct Worker;

/**
 * This micro storage room can hold any number of items of a fixed ware.
 *
 * You must call update() after changing the queue's size or filled state using
 * one of the set_*() functions.
 */
struct WaresQueue {
	typedef void (callback_t)
		(Game &, WaresQueue *, Ware_Index ware, void * data);

	WaresQueue(PlayerImmovable &, Ware_Index, uint8_t size, uint8_t filled = 0);

#ifndef NDEBUG
	~WaresQueue() {assert(not m_ware);}
#endif

	Ware_Index get_ware() const {return m_ware;}
	uint32_t get_size            () const throw () {return m_size;}
	uint32_t get_filled          () const throw () {return m_filled;}
	uint32_t get_consume_interval() const throw () {return m_consume_interval;}

	void cleanup();
	void update();

	void set_callback(callback_t *, void * data);

	void remove_from_economy(Economy &);
	void add_to_economy(Economy &);

	void set_size            (uint32_t) throw ();
	void set_filled          (uint32_t) throw ();
	void set_consume_interval(uint32_t) throw ();

	Player & owner() const throw () {return m_owner.owner();}

	void Write(FileWrite &, Game &, Map_Map_Object_Saver  *);
	void Read (FileRead  &, Game &, Map_Map_Object_Loader *);

private:
	static void request_callback
		(Game &, Request &, Ware_Index, Worker *, PlayerImmovable &);

	PlayerImmovable & m_owner;
	Ware_Index        m_ware;    ///< ware ID
	uint32_t m_size;             ///< nr of items that fit into the queue
	uint32_t m_filled;           ///< nr of items that are currently in the queue

	///< time in ms between consumption at full speed
	uint32_t m_consume_interval;

	Request         * m_request; ///< currently pending request

	callback_t      * m_callback_fn;
	void            * m_callback_data;
};

}

#endif


