/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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

#include "worker.h"


/**
 * Carrier is a worker who is employed by a Road.
 */
struct Carrier : public Worker {
	friend class Widelands_Map_Bobdata_Data_Packet; // Writes this to disk

	struct Descr : public Worker_Descr {
			Descr(const Tribe_Descr &t, const std::string & carrier_name):
					Worker_Descr(t, carrier_name) {};

			virtual Worker_Type get_worker_type() const {return CARRIER;}

	protected:
				virtual Bob * create_object() const {return new Carrier(*this);}
				virtual void parse(const char *directory, Profile *prof,
								   const EncodeData *encdata)
						{Worker_Descr::parse(directory, prof, encdata);}
	};


	Carrier(const Descr & carrier_descr): Worker(carrier_descr), m_acked_ware(-1)
	{}
		virtual ~Carrier() {};

		bool notify_ware(Game* g, int32_t flag);

		void start_task_road();
		void update_task_road(Game* g);
		void start_task_transport(int32_t fromflag);
		bool start_task_walktoflag(Game* g, int32_t flag, bool offset = false);


private:
		MO_DESCR(Descr);

		void find_pending_item(Game* g);
		int32_t find_closest_flag(Game* g);

		// internal task stuff
		void road_update(Game* g, State* state);
		void road_signal(Game* g, State* state);

		void transport_update(Game* g, State* state);
		void transport_signal(Game* g, State* state);

		static Task taskRoad;
		static Task taskTransport;

		void deliver_to_building(Game* g, State* state);
		void pickup_from_flag(Game *g, State* s);
		void drop_item(Game* g, State* s);
		void enter_building(Game* g, State* s);
		bool swap_or_wait(Game*g, State* s);

		/// -1: no ware acked; 0/1: acked ware for start/end flag of road
		int32_t m_acked_ware;
};
