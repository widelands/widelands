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

/** Bob(BobDesc*, World*)
  * This constructor creates a bob. Only worlds will create bobs.
  */
Bob::Bob(BobDesc* bob, World* w)
{
	this->world = w;
	this->desc = *bob;
	this->lastAct = 0;
}

/** Pic* get_pic(int timekey)
  * Returns the actual animation frame needed to paint the bob.
  */
Pic* Bob::get_pic(int timekey)
{
	int key = NONE;
	switch (desc.animKey)
	{
	case STOCK:
		key = desc.stock;
		break;
	case TIME:
		// this scale allows 1 per second as slowest animation; hm.
		// are there slower anims?
		key = (timekey * desc.animFactor) / 1000;
	}
	Anim* anim = world->get_anim(desc.anim);
	key %= anim->pics;
	return world->get_texture(anim->pic[key]);
}

/** ~Bob()
  * Does nothing.
  */
Bob::~Bob()
{
}

/** int consume()
  * Decreases the resources in the bob's stock by 1.
  * Returns the new stock size.
  */
int Bob::consume()
{
	if (!desc.stock)
		return 0;
	return --desc.stock;
}

/** Bob* act(int timekey)
  * Performs bob action. For now, this is nothing but occasional dying.
  * Returns the bob to take the place of this bob (usually that's just
  * this bob, but it may be its heir or NULL).
  */
Bob* Bob::act(int timekey)
{
	if (desc.lifetime)
	{
		int secondsAfterUpdate = (lastAct - timekey) / 1000;
		if ((desc.lifetime -= secondsAfterUpdate) <= 0)
		{
			Bob* bob = world->create_bob(desc.heir);
			delete this;
			return bob;
		}
	}
	if (desc.stock == 0)
	{
			Bob* bob = world->create_bob(desc.heir);
			delete this;
			return bob;
	}
	return this;
}