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

#ifndef __CONFREADER_H
#define __CONFREADER_H

#include "../src/mytypes.h"
#include "../src/tribefiletypes.h"

class Growable_Array;
class Profile;

class Conf_Reader
{
	Profile*				conf;
	TribeFileHeader			header;
	BuildingDesc*			building;
	CreatureDesc*			creature;
	Ware*					ware;
	Growable_Array*			pics;
	void					read_header();
	void					read_buildings();
	void					read_creatures();
	void					read_wares();
public:
							Conf_Reader(const char* filename);
							~Conf_Reader();
	const TribeFileHeader*	get_header();
	const BuildingDesc*		get_building(int n);
	const CreatureDesc*		get_creature(int n);
	const Ware*				get_ware(int n);
	uint					textures();
	const char*				get_texture(int n);
};

#endif //__CONFREADER_H