/*
 * Copyright (C) 2002 by the Widelands Development Tea
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

#include "../src/worldfiletypes.h"

class Growable_Array;
class Profile;

class Conf_Reader
{
	Profile*				conf;
	WorldFileHeader			header;
	ResourceDesc*			resource;
	TerrainType*			terrain;
	Growable_Array*			pics;
	BobDesc*				bob;
	void					read_header();
	void					read_resources();
	void					read_terrains();
	void					read_bobs();
public:
							Conf_Reader(const char* filename);
							~Conf_Reader();
	const WorldFileHeader*	get_header();
	const ResourceDesc*		get_resource(int n);
	const TerrainType* 		get_terrain(int n);
	const BobDesc*	 		get_bob(int n);
	uint					textures();
	const char*				get_texture(int n);
};

#endif //__CONFREADER_H
