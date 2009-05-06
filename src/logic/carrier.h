/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#ifndef CARRIER_H
#define CARRIER_H

#include "worker.h"

namespace Widelands {

/**
 * Carrier is a worker who is employed by a Road.
 */
struct Carrier : public Worker {
	friend struct Map_Bobdata_Data_Packet;

	struct Descr : public Worker_Descr {
		Descr
			(char const * const _name, char const * const _descname,
			 std::string const & directory, Profile & prof, Section & global_s,
			 Tribe_Descr const & _tribe, EncodeData const * encdata) :
				Worker_Descr
					(_name, _descname, directory,
					 prof, global_s, _tribe, encdata)
		{};

		virtual Worker_Type get_worker_type() const {return CARRIER;}

	protected:
		virtual Bob & create_object() const {return *new Carrier(*this);}
	};


	Carrier(const Descr & carrier_descr)
		: Worker(carrier_descr), m_acked_ware(-1)
	{}
	virtual ~Carrier() {};

	bool notify_ware(Game &, int32_t flag);

	void start_task_road(Game &);
	void update_task_road(Game &);
	void start_task_transport(Game &, int32_t fromflag);
	bool start_task_walktoflag(Game &, int32_t flag, bool offset = false);


private:
	MO_DESCR(Descr);

	void find_pending_item(Game &);
	int32_t find_closest_flag(Game &);

	// internal task stuff
	void road_update        (Game &, State &);
	void transport_update   (Game &, State &);

	static Task taskRoad;
	static Task taskTransport;

	void deliver_to_building(Game &, State &);
	void pickup_from_flag   (Game &, State &);
	void drop_item          (Game &, State &);
	void enter_building     (Game &, State &);
	bool swap_or_wait       (Game &, State &);

	/// -1: no ware acked; 0/1: acked ware for start/end flag of road
	int32_t m_acked_ware;
};

};

#endif
