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

#ifndef IDLEWORKERSUPPLY_H
#define IDLEWORKERSUPPLY_H

#include "transport.h"

struct IdleWorkerSupply : public Supply {
	IdleWorkerSupply(Worker *);
	~IdleWorkerSupply();

	void set_economy(Economy *);
	virtual PlayerImmovable * get_position(Game *);
	virtual int32_t get_amount(const int32_t ware) const;

	/// Idle workers are always active supplies, because they need to get
	/// into a Warehouse ASAP.
	virtual bool is_active() const throw () {return true;}

	virtual WareInstance * launch_item(Game *, int32_t ware)
		__attribute__ ((noreturn));

	virtual Worker * launch_worker(Game *, int32_t ware);
	virtual Soldier* launch_soldier(Game *, int32_t ware, Requeriments *);
	virtual int32_t get_passing_requeriments
		(Game *, int32_t ware, Requeriments *);
	virtual void mark_as_used (Game *, int32_t ware, Requeriments*);


private:
	Worker  * m_worker;
	Economy * m_economy;
};


#endif  // IDLEWORKERSUPPLY_H
