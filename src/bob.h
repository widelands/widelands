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
#ifndef __BOB_H
#define __BOB_H

#include "worldfiletypes.h"
#include "graphic.h"
#include "pic.h"

class World;
//class Pic;

class Animation;

struct Animation_Pic {
   ushort *data;
   Animation* parent;
};

class Animation {
   public:
      enum {
         HAS_TRANSPARENCY = 1,
         HAS_SHADOW = 2,
         HAS_PL_CLR = 3
      };

      Animation(void) { npics=0; pics=0;}
      ~Animation(void) { 
         if(npics) {
            uint i; 
            for(i=0; i<npics; i++) {
               free(pics[i].data);
            }
            free(pics);
         }
      }
   
      inline ushort get_w(void) { return w; }
      inline ushort get_h(void) { return h; }
      inline ushort get_hsx(void) { return hsx; }
      inline ushort get_hsy(void) { return hsy; }
      void add_pic(ushort size, ushort* data) {
         if(!npics) {
            npics=1;
            pics=(Animation_Pic*) malloc(sizeof(Animation_Pic));
         } else {
            npics++;
            pics=(Animation_Pic*) realloc(pics, sizeof(Animation_Pic*)*npics);
         }
         pics[npics-1].data=(ushort*)malloc(size);
         pics[npics-1].parent=this;
         memcpy(pics[npics-1].data, data, size);
      }

      void set_flags(uint mflags) { flags=mflags; }
      void set_dimensions(ushort mw, ushort mh) { w=mw; h=mh; }
      void set_hotspot(ushort x, ushort y) { hsx=x; hsy=y; }

      // TEMP 
      inline Animation_Pic* get_pic(ushort) { return &pics[0]; }

      // TEMP ENDS
   private:
      uint flags;
      ushort w, h;
      ushort hsx, hsy;
      ushort npics;
      Animation_Pic *pics;
};

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

