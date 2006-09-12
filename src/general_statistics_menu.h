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

#ifndef __S__GENERAL_STATISTICS_MENU_H
#define __S__GENERAL_STATISTICS_MENU_H

#include "constants.h"
#include "ui_unique_window.h"

class Interactive_Player;
class UICheckbox;
class UIRadiogroup;
class UITextarea;
class UIButton;
class WUIPlot_Area;

class General_Statistics_Menu : public UIUniqueWindow {
   public:
      General_Statistics_Menu(Interactive_Player* plr, UIUniqueWindowRegistry* registry) ;
      virtual ~General_Statistics_Menu(void);


   private:
      Interactive_Player* m_parent;
      WUIPlot_Area*       m_plot;
      UIRadiogroup*       m_radiogroup;
      int                 m_selected_information;
      UICheckbox*         m_cbs[MAX_PLAYERS];

   private:

      void clicked(int);
      void cb_changed_to(int, bool);
      void radiogroup_changed(int);
};

#endif
