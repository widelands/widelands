/*
 * Copyright (C) 2002 by the Widelands Development Team
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
#ifndef __WORLD_H
#define __WORLD_H

#include "graphic.h"

class Binary_file;
class Bob;
//class Pic;
struct Anim;
struct BobDesc;
struct ResourceDesc;
struct TerrainType;

/** class World
  *
  * This class provides information on a worldtype usable to create a map;
  * it can read a world file.
  */
class World
{
	char			author[64];
	char			name[32];
	Anim*			anim;
	BobDesc*		bob;
	Pic**			texture;
	ResourceDesc*	resource;
	TerrainType*	terrain;
	uint			animCount;
	uint			bobCount;
	uint			textureCount;
	uint			resourceCount;
	uint			terrainCount;
	void			read_header(Binary_file*);
	void			read_bobs(Binary_file*);
	void			read_textures(Binary_file*);
	void			read_anims(Binary_file*);
	void			read_terrains(Binary_file*);
	void			read_resources(Binary_file*);
public:
					World(const char* file);
					~World();
	Bob*			create_bob(uint n);
	Pic*			get_texture(uint n);
	Anim*			get_anim(uint n);
	ResourceDesc*	get_resource(uint n);
	TerrainType*	get_terrain(uint n);
	uint			resources();
	uint			terrains();
	uint			bobs();
	const char*		get_author()
	{
		return author;
	}
	const char*		get_name()
	{
		return name;
	}
};

#endif

