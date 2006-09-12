/*
 * Copyright (C) 2002-4 by the Widelands Development Team
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

#ifndef __S__BUILDING_STATISTICS_MENU_H
#define __S__BUILDING_STATISTICS_MENU_H

#include "filesystem.h"
#include "ui_unique_window.h"

class Interactive_Player;
class UIButton;
class UIProgress_Bar;
class UITextarea;
class UITable;

class Building_Statistics_Menu : public UIUniqueWindow {
   public:
      Building_Statistics_Menu(Interactive_Player* plr, UIUniqueWindowRegistry* registry) ;
      virtual ~Building_Statistics_Menu(void);

      void think(void);
      void draw(RenderTarget*);

   private:
      UITable*            m_table;
      Interactive_Player* m_parent;
      UIProgress_Bar*     m_progbar;
      UITextarea*         m_owned, *m_build;
      uint                m_anim;
      uint                m_lastupdate;
      uint                m_end_of_table_y;
      int                 m_selected;
      UIButton*           m_btn[6];
      int                 m_last_building_index;

   private:
      void clicked(int);
      void table_changed(int);
      void update();
      int validate_pointer(int*,int);
};

#endif
