/*
 * Copyright (C) 2002-2004, 2006-2010 by the Widelands Development Team
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

#ifndef CARRIER_H
#define CARRIER_H

#include "logic/worker.h"

namespace Widelands {

/**
 * Carrier is a worker who is employed by a Road.
 */
struct Carrier : public Worker {
	friend struct Map_Bobdata_Data_Packet;

	struct Descr : public Worker_Descr {
		Descr
			(char const * const _name, char const * const _descname,
			 const std::string & directory, Profile & prof, Section & global_s,
			 const Tribe_Descr & _tribe)
			:
				Worker_Descr
					(_name, _descname, directory,
					 prof, global_s, _tribe)
		{};

		virtual Worker_Type get_worker_type() const override {return CARRIER;}

	protected:
		virtual Bob & create_object() const override {return *new Carrier(*this);}
	};


	Carrier(const Descr & carrier_descr)
		: Worker(carrier_descr), m_promised_pickup_to(-1)
	{}
	virtual ~Carrier() {};

	bool notify_ware(Game &, int32_t flag);

	void start_task_road(Game &);
	void update_task_road(Game &);
	void start_task_transport(Game &, int32_t fromflag);
	bool start_task_walktoflag(Game &, int32_t flag, bool offset = false);

	virtual void log_general_info(const Editor_Game_Base &) override;

private:
	MO_DESCR(Descr);

	void find_pending_ware(Game &);
	int32_t find_closest_flag(Game &);

	// internal task stuff
	void road_update        (Game &, State &);
	void road_pop           (Game &, State &);
	void transport_update   (Game &, State &);

	static Task const taskRoad;
	static Task const taskTransport;

	void deliver_to_building(Game &, State &);
	void pickup_from_flag   (Game &, State &);
	void drop_ware          (Game &, State &);
	void enter_building     (Game &, State &);
	bool swap_or_wait       (Game &, State &);

	/// -1: no ware acked; 0/1: acked ware for start/end flag of road
	// This should be an enum, but this clutters the code with too many casts
	static const int32_t NOONE = -1;
	static const int32_t START_FLAG = 0;
	static const int32_t END_FLAG = 1;
	int32_t m_promised_pickup_to;

	// saving and loading
protected:
	struct Loader : public Worker::Loader {
	public:
		Loader();

		virtual void load(FileRead &) override;

	protected:
		virtual const Task * get_task(const std::string & name) override;
	};

	virtual Loader * create_loader() override;

public:
	virtual void do_save
		(Editor_Game_Base &, Map_Map_Object_Saver &, FileWrite &) override;
};

}

#endif
