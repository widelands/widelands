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
#include "os.h"
#include "world.h"
//#include "worldfiletypes.h"
#include "bob.h"
//#include "graphic.h"

World::World(const char* file)
{
	name[0] = 0;
	author[0] = 0;
	// load world file
	bobCount = 0;
	textureCount = 0;
	animCount = 0;
}

World::~World()
{
}

Bob* World::create_bob(uint n)
{
	// create a new instance of bob n
	if (n < bobCount)
		return new Bob(&bob[n], this);
	return NULL;
}

Pic* World::get_texture(uint n)
{
	// return a new pointer to the texture
	if (n < textureCount)
		return texture[n];
	return NULL;
}

Anim* World::get_anim(uint n)
{
	// return a new pointer to the anim
	if (n < animCount)
		return &anim[n];
	return NULL;
}