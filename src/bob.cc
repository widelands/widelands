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

#include "bob.h"
#include "world.h"
#include "os.h"

Bob::Bob(BobDesc* bob, World* w)
{
	this->world = w;
	this->desc = *bob;
}

Pic* Bob::get_pic()
{

	int key = 0;
	switch (desc.animKey)
	{
	case STOCK:
		key = desc.stock;
//	case TIME:
//		key = 0167254123;
	}
	Anim* anim = world->get_anim(desc.anim);
	key %= anim->pics;
	return world->get_texture(anim->pic[key]);
	
	return 0;
}

Bob::~Bob()
{
}

int Bob::consume()
{
	if (!desc.stock)
		return 0;
	return --desc.stock;
}

Bob* Bob::die()
{
	if (desc.heir < 0)
		return NULL;
	return world->create_bob(desc.heir);
}