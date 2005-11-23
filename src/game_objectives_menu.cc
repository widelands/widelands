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

#include "game_objectives_menu.h"
#include "interactive_player.h"
#include "map_objective_manager.h"
#include "player.h"
#include "trigger_null.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_multilinetextarea.h"
#include "ui_multilineeditbox.h"
#include "ui_listselect.h"
#include "ui_textarea.h"
#include "util.h"

/*
==============================================================================

GameObjectivesMenu IMPLEMENTATION

==============================================================================
*/

/*
===============
GameObjectivesMenu::GameObjectivesMenu

Create all the buttons etc...
===============
*/
GameObjectivesMenu::GameObjectivesMenu(Interactive_Player *plr, UIUniqueWindowRegistry *registry, Game* game)
	: UIUniqueWindow(plr, registry, 340, 160, _("Objectives Menu"))
{
   int spacing = 5;
   int posy = 35;
   
   // Caption
   new UITextarea(this, 0, spacing, get_inner_w(), 20, _("Objectives Menu"), Align_Center);
 
   // Listselect with Objectives
   m_list = new UIListselect( this, spacing, posy, get_inner_w()-spacing*2, 60, Align_Left, false );
   MapObjectiveManager* mom = game->get_map()->get_mom();
   for( int i = 0; i < mom->get_nr_objectives(); i++) {
      MapObjective* obj = mom->get_objective_by_nr( i );
      if( !obj->get_is_visible() || !obj->get_trigger()->is_set())
         continue;
      
      m_list->add_entry( obj->get_name(), obj);
      if( obj->get_is_optional()) 
         m_list->set_entry_color( m_list->get_nr_entries()-1, RGBColor(255,0,0));
   }
   m_list->selected.set( this, &GameObjectivesMenu::selected);
   posy += 60 + spacing;
      
   // Objective Text 
   m_objectivetext = new UIMultiline_Textarea(this, spacing, posy, get_inner_w()-spacing*2, 150, "", Align_Left, 1); 
   posy += 150+spacing+spacing;
   
   // If any objectives, select the first one
   if(m_list->get_nr_entries())
      m_list->select(0);
   
   set_inner_size(get_inner_w(), posy+5);
	if (get_usedefaultpos())
		center_to_parent();
}

/*
===============
GameObjectivesMenu::~GameObjectivesMenu
===============
*/
GameObjectivesMenu::~GameObjectivesMenu()
{
}

/*
 * Something has been selected 
 */
void GameObjectivesMenu::selected( int t ) {
   MapObjective* obj = (MapObjective*)m_list->get_entry(t);
   
   m_objectivetext->set_text( obj->get_descr());
}
