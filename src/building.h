/*
 * Copyright (C) 2002 by Florian Bluemel
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

#include "tribefiletypes.h"

class Creature;
class Field;

class Building
{
	BuildingDesc desc;
	Creature* worker;
	Field* location;
	bool working;
public:
	Building();
	~Building();
	void get_pic(int timekey);
	void work(int timekey);
	void worker_idle(Creature* w);
	void destroy();
	bool add_ware(uint ware);
};
