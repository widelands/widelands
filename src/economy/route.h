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

#ifndef ROUTE_H
#define ROUTE_H

#include <vector>

// Needed for OPtr
#include "logic/instances.h"

#include "iroute.h"


namespace Widelands {

struct Flag;
struct Editor_Game_Base;
struct Map_Map_Object_Saver;
struct Map_Map_Object_Loader;
struct RoutingNode;

/**
 * Route stores a route from flag to flag.
 * The number of steps is the number of flags stored minus one.
 */
struct Route : public IRoute {
	friend struct Router;
	friend struct Request;

	Route();

	void init(int32_t);

	int32_t get_totalcost() const {return m_totalcost;}
	int32_t get_nrsteps() const {return m_route.size() - 1;}
	Flag & get_flag(Editor_Game_Base &, std::vector<Flag *>::size_type);

	void starttrim(int32_t count);
	void truncate(int32_t count);

	struct LoadData {
		std::vector<uint32_t> flags;
	};

	void load(LoadData &, FileRead &);
	void load_pointers(LoadData const &, Map_Map_Object_Loader &);
	void save(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver *);

	void insert_as_first(RoutingNode * node);

private:
	int32_t                     m_totalcost;
	std::vector<OPtr<Flag> > m_route; ///< includes start and end flags
};

}

#endif
