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
#ifndef __BOB_H
#define __BOB_H

#include "worldfiletypes.h"
#include "graphic.h"

class World;
//class Pic;

/** class Bob
  *
  * This class represents a bob.
  * Depends on World, Pic, world file types.
  */
class Bob
{
	friend class World;
	BobDesc desc;
	World* world;
	int lastAct;

	/** Bob(BobDesc*, World*)
	  * This constructor creates a bob. Only worlds will create bobs.
	  */
	Bob(BobDesc*, World*);
public:
	~Bob();

	/** Pic* get_pic(int timekey)
	  * Returns the actual animation frame needed to paint the bob.
	  */
	Pic* get_pic(int timekey);

	/** int consume()
	  * Decreases the resources in the bob's stock by 1.
	  * Returns the new stock size.
	  */
	int consume();

	/** Bob* act(int timekey)
	  * Performs bob action. For now, this is nothing but occasional dying.
	  * Returns the bob to take the place of this bob (usually that's just
	  * this bob, but it may be its heir or NULL).
	  */
	Bob* act(int timekey);
};

#endif

