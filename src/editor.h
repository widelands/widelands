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

#ifndef __S__EDITOR_H
#define __S__EDITOR_H

#include "editorinteractive.h"
#include "map.h"

/* 
 * This class defines the Map/Mission Editor of widelands. It has 
 * nearly the same functionality as the Game Object. But some things 
 * are still different (like Objects do not move (but Animate), there 
 * are no players, no AI).
 */
class Editor {
   public:
      // Editor(Map*);
      Editor();
      ~Editor();

      bool run();
      void think();

   private:
      Map* m_map;
      Editor_Interactive* m_editorinteractive;
};
#endif // __S__EDITOR_H
