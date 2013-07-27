/*
 * Copyright (C) 2002-2004, 2006-2008 by the Widelands Development Team
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

#ifndef IDLEWORKERSUPPLY_H
#define IDLEWORKERSUPPLY_H

#include "economy/supply.h"

namespace Widelands {
class Worker;
class Economy;

struct IdleWorkerSupply : public Supply {
	IdleWorkerSupply(Worker &);
	~IdleWorkerSupply();

	void set_economy(Economy *);
	virtual PlayerImmovable * get_position(Game &);

	virtual bool is_active() const throw ();
	virtual bool has_storage() const throw ();
	virtual void get_ware_type(WareWorker & type, Ware_Index & ware) const;
	virtual void send_to_storage(Game &, Warehouse * wh);

	virtual uint32_t nr_supplies(const Game &, const Request &) const;
	virtual WareInstance & launch_item(Game &, const Request &);
	virtual Worker & launch_worker(Game &, const Request &);

private:
	Worker  & m_worker;
	Economy * m_economy;
};

}

#endif
