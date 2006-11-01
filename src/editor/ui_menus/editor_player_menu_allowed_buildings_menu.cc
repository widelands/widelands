/*
 * Copyright (C) 2002-2004, 2006 by the Widelands Development Team
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

#include "editor_player_menu_allowed_buildings_menu.h"
#include "i18n.h"
#include "map.h"
#include "tribe.h"
#include "ui_listselect.h"
#include "ui_textarea.h"
#include "ui_button.h"
#include "player.h"

/*
===============
Editor_Player_Menu_Allowed_Buildings_Menu::Editor_Player_Menu_Allowed_Buildings_Menu

Create all the buttons etc...
===============
*/
Editor_Player_Menu_Allowed_Buildings_Menu::Editor_Player_Menu_Allowed_Buildings_Menu(UI::Panel *parent, Player* player, UI::UniqueWindow::Registry* registry)
   : UI::UniqueWindow(parent, registry, 540, 400, _("Allowed Buildings"))
{
   m_player=player;

   // Caption
   UI::Textarea* tt=new UI::Textarea(this, 0, 0, _("Allowed Buildings Menu"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   // UI::Buttons
   const int offsy=30;
   const int spacing=3;
   int posy=offsy;

   // Allowed List
   new UI::Textarea(this, spacing, posy, get_inner_w()/2, 20, _("Allowed Buildings: "), Align_CenterLeft);
   m_allowed=new UI::Listselect<void *>(this, spacing, posy+23, get_inner_w()/2-2*spacing-20, get_inner_h()-posy-spacing-23);
   m_allowed->selected.set(this, &Editor_Player_Menu_Allowed_Buildings_Menu::allowed_selected);
   m_allowed->double_clicked.set(this,&Editor_Player_Menu_Allowed_Buildings_Menu::allowed_double_clicked);

   // Forbidden List
   new UI::Textarea(this, get_inner_w()/2+spacing, posy, get_inner_w()/2, 20, _("Forbidden Buildings: "), Align_CenterLeft);
   m_forbidden=new UI::Listselect<void *>(this, get_inner_w()/2+spacing+20, posy+23, get_inner_w()/2-2*spacing-20, get_inner_h()-posy-spacing-23);
   m_forbidden->selected.set(this, &Editor_Player_Menu_Allowed_Buildings_Menu::forbidden_selected);
   m_forbidden->double_clicked.set(this,&Editor_Player_Menu_Allowed_Buildings_Menu::forbidden_double_clicked);

   // Left to right button
   UI::Button* b=new UI::Button(this, get_inner_w()/2-20, posy+30, 40, 20, 1, 0);
   b->clickedid.set(this, &Editor_Player_Menu_Allowed_Buildings_Menu::clicked);
   b->set_title("->");
   b->set_enabled(false);
   m_ltr_button=b;

   // Right to left button
   b=new UI::Button(this, get_inner_w()/2-20, posy+55, 40, 20, 1, 1);
   b->clickedid.set(this, &Editor_Player_Menu_Allowed_Buildings_Menu::clicked);
   b->set_title("<-");
   b->set_enabled(false);
   m_rtl_button=b;

   // Fill the lists
	const Tribe_Descr* t=player->get_tribe();
   int i;
   for(i=0; i<t->get_nrbuildings(); i++) {
      Building_Descr* descr=t->get_building_descr(i);
      if(!descr->get_enhanced_building() && !descr->get_buildable()) continue;
      if(m_player->is_building_allowed(i))
         m_allowed->add_entry(descr->get_descname(), ((void*)(i)), false, descr->get_buildicon());
      else
         m_forbidden->add_entry(descr->get_descname(), ((void*)(i)), false, descr->get_buildicon());
   }
   m_forbidden->sort();
   m_allowed->sort();
   update();
}

/*
===============
Editor_Player_Menu_Allowed_Buildings_Menu::update()

Updates all UI::Textareas in the UI::Window to represent currently
set values
==============
*/
void Editor_Player_Menu_Allowed_Buildings_Menu::update(void) {
}

/*
===============
Editor_Player_Menu_Allowed_Buildings_Menu::~Editor_Player_Menu_Allowed_Buildings_Menu

Unregister from the registry pointer
===============
*/
Editor_Player_Menu_Allowed_Buildings_Menu::~Editor_Player_Menu_Allowed_Buildings_Menu()
{
}


/*
 * UI Action callback functions
 */
void Editor_Player_Menu_Allowed_Buildings_Menu::clicked(int i) {
	UI::Listselect<void *> * source, * target;
   bool set_to;

   if(i==0) {
      // Left to right button
      source=m_allowed;
      target=m_forbidden;
      set_to=false;
   } else {
      // Right to left button
      source=m_forbidden;
      target=m_allowed;
      set_to=true;
   }


   // Remove from one list
   long index=((long)source->get_selection());
   source->remove_entry(source->get_selection_index());
   source->sort();

   // Add to another
   Building_Descr* b=m_player->get_tribe()->get_building_descr(index);
   target->add_entry(b->get_descname(), ((void*)(index)), false, b->get_buildicon());
   target->sort();

   // Enable/Disable for player
   m_player->allow_building(index, set_to);
}

/*
 * Listbox selected
 */
void Editor_Player_Menu_Allowed_Buildings_Menu::allowed_selected(uint) {
   m_rtl_button->set_enabled(true);
}

void Editor_Player_Menu_Allowed_Buildings_Menu::forbidden_selected(uint) {
   m_ltr_button->set_enabled(true);
}

/*
 * Listbox doubleclicked
 */
void Editor_Player_Menu_Allowed_Buildings_Menu::allowed_double_clicked(uint) {
   // Left to right button
   clicked(0);
}

void Editor_Player_Menu_Allowed_Buildings_Menu::forbidden_double_clicked(uint) {
   // Right to left clicked
   clicked(1);
}
