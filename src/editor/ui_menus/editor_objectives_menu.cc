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

#include "editorinteractive.h"
#include "editor_objectives_menu.h"
#include "error.h"
#include "map.h"
#include "map_objective_manager.h"
#include "system.h"
#include "trigger_null.h"
#include "ui_button.h"
#include "ui_checkbox.h"
#include "ui_editbox.h"
#include "ui_modal_messagebox.h"
#include "ui_multilineeditbox.h"
#include "ui_table.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"
#include "ui_listselect.h"
#include "util.h"

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
class Edit_Objective_Window : public UIWindow {
   public:
      Edit_Objective_Window(Editor_Interactive*, UITable_Entry*);

      bool handle_mouseclick(uint btn, bool down, int mx, int my);

   private:
      Editor_Interactive  *m_parent;
      UITable_Entry       *m_te;
      UIEdit_Box          *m_name;
      UIMultiline_Editbox *m_descr;
      UICheckbox          *m_visible;
      UICheckbox          *m_optional;

   private:
      void clicked( int );
};

Edit_Objective_Window::Edit_Objective_Window(Editor_Interactive* parent, UITable_Entry* te)
   : UIWindow(parent, 0, 0, 250, 85, "Edit Objective") {
  
   m_parent=parent;
   m_te = te;

   int spacing = 5;
   int posy = 5;
  
   MapObjective* obj = static_cast<MapObjective*>(te->get_user_data());
   
   // What type
   new UITextarea(this, 5, 5, 120, 20, "Name", Align_CenterLeft);
   m_name = new UIEdit_Box( this, 120, 5, 120, 20, 0, 0);
   m_name->set_text( narrow_string(obj->get_name()).c_str() );
   posy += 20 + spacing;
   
   new UITextarea(this, 5, posy, 120, STATEBOX_HEIGHT, "Visible at Begin: ", Align_CenterLeft);
   m_visible = new UICheckbox(this, get_inner_w() - STATEBOX_WIDTH - spacing, posy);
   m_visible->set_state( obj->get_is_visible() );
   posy += STATEBOX_HEIGHT+ spacing;
   
   new UITextarea(this, 5, posy, 120, STATEBOX_HEIGHT, "Optional Objective: ", Align_CenterLeft);
   m_optional = new UICheckbox(this, get_inner_w() - STATEBOX_WIDTH - spacing, posy);
   m_optional->set_state( obj->get_is_optional() );
   posy += STATEBOX_HEIGHT+ spacing;
  
   // Multiline editbox
   new UITextarea(this, 5, posy, 120, STATEBOX_HEIGHT, "Objective text: ", Align_CenterLeft);
   posy += 20 + spacing;
   
   const int editbox_height = 140;
   m_descr = new UIMultiline_Editbox(this, 5, posy, get_inner_w()-2*spacing, editbox_height, narrow_string(obj->get_descr()).c_str() );
   posy+= editbox_height + spacing + spacing;
   
   // back button
   UIButton* b = new UIButton( this, get_inner_w()/2-80-spacing, posy, 80, 20, 1, 0);
   b->set_title("Ok");
   b->clickedid.set(this, &Edit_Objective_Window::clicked);
   b = new UIButton( this, get_inner_w()/2 + spacing, posy, 80, 20, 1, 1);
   b->set_title("Back");
   b->clickedid.set(this, &Edit_Objective_Window::clicked);
   posy += 20 + spacing;

   set_inner_size( get_inner_w(), posy);

   center_to_parent();
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 */
bool Edit_Objective_Window::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable
}

/*
 * a button has been clicked
 */
void Edit_Objective_Window::clicked(int i) {
   // Get the a name

   if( i )  {
      // Back
      end_modal(0);
      return;
   } 

   // Ok
   
   // Extract value
   MapObjective* obj = static_cast<MapObjective*>(m_te->get_user_data());

   obj->set_name( widen_string(m_name->get_text()).c_str() );
   obj->set_is_optional( m_optional->get_state() );
   obj->set_is_visible( m_visible->get_state() );
   obj->set_descr( widen_string(m_name->get_text()).c_str() );
   m_te->set_string(0, narrow_string(obj->get_name()).c_str()); 
   m_te->set_string(1, obj->get_is_optional() ? "Yes" : "No");
   m_te->set_string(2, obj->get_is_visible() ? "Yes" : "No");
  
   // Set the triggers name
   obj->get_trigger()->set_name( m_name->get_text());

   end_modal(1); 
}


/*
===============
Editor_Objectives_Menu::Editor_Objectives_Menu

Create all the buttons etc...
===============
*/
Editor_Objectives_Menu::Editor_Objectives_Menu(Editor_Interactive *parent, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(parent, registry, 410, 330, "Objectives Menu")
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Objectives Menu", Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int spacing=5;
   m_table = new UITable(this, 5, 25, get_inner_w()-2*spacing, get_inner_h() - 60);
   m_table->add_column("Name", UITable::STRING, 270);
   m_table->add_column("Optional", UITable::STRING, 70);
   m_table->add_column("Visible", UITable::STRING, 60);
   m_table->selected.set(this, &Editor_Objectives_Menu::table_selected);
   m_table->double_clicked.set(this, &Editor_Objectives_Menu::table_dblclicked);

   // Buttons
   int posx=spacing;

   UIButton* nbutton = new UIButton( this, spacing, get_inner_h() - 30, 60, 20, 0, 0);
   nbutton->set_title("New");
   nbutton->clickedid.set(this, &Editor_Objectives_Menu::clicked);
   posx += 60 + spacing;
   m_edit_button = new UIButton( this, posx, get_inner_h() - 30, 60, 20, 0, 1);
   m_edit_button->set_title("Edit");
   m_edit_button->set_enabled(false);
   m_edit_button->clickedid.set(this, &Editor_Objectives_Menu::clicked);
   posx += 60 + spacing;
   m_delete_button = new UIButton( this, posx, get_inner_h() - 30, 60, 20, 0, 2);
   m_delete_button->set_title("Delete");
   m_delete_button->set_enabled(false);
   m_delete_button->clickedid.set(this, &Editor_Objectives_Menu::clicked);
   posx += 60 + spacing;

   // Trigger name
   new UITextarea( this, posx, get_inner_h() - 30, 80, 20, "Trigger: ", Align_CenterLeft);
   posx += 45 + spacing;
   m_trigger = new UITextarea( this, posx, get_inner_h() - 30, 100, 20, "-", Align_CenterLeft);

   // Add all variables
   MapObjectiveManager* mom = m_parent->get_egbase()->get_map()->get_mom();
   for(int i = 0; i < mom->get_nr_objectives(); i++) {
      insert_objective( mom->get_objective_by_nr( i ));
   }
   
	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/*
===============
Editor_Objectives_Menu::~Editor_Objectives_Menu

Unregister from the registry pointer
===============
*/
Editor_Objectives_Menu::~Editor_Objectives_Menu()
{
}
      
/*
 * A Button has been clicked
 */
void Editor_Objectives_Menu::clicked( int n ) { 
   switch( n ) {
      case 0: 
      {
         // Get the a name
         wchar_t buffer[256];

         int n = 1;
         while( 1 ) {
            swprintf(buffer, sizeof(buffer), L"Unnamed%i", n);
            if( !m_parent->get_egbase()->get_map()->get_mom()->get_objective( buffer ))
               break;
            ++n;
         }
         // Create a new objective
         MapObjective* mo = new MapObjective;
         mo->set_name( buffer );
         m_parent->get_egbase()->get_map()->get_mom()->register_new_objective( mo );
         // Create a null trigger for this
         Trigger_Null* trig = new Trigger_Null();
         trig->set_name( narrow_string(buffer) );
         trig->incr_reference();
         mo->set_trigger( trig );
         m_parent->get_egbase()->get_map()->register_new_trigger( trig );

         insert_objective( mo );
      }
      // Fallthrough to edit
      
      case 1:
      {
         // Edit selected variable
         Edit_Objective_Window* evw = new Edit_Objective_Window( m_parent, m_table->get_entry(m_table->get_selection_index()) );
         if( evw->run() ) { 
            m_table->sort();
            m_trigger->set_text(
                  static_cast<MapObjective*>(m_table->get_entry(m_table->get_selection_index())->get_user_data())
                  ->get_trigger()->get_name());
         }
         delete evw;
      }
      break;

      case 2: 
      {
         // Delete selected variable
      /*   int n =  m_table->get_selection_index();
         MapObjective* mv = static_cast<MapObjective*>( m_table->get_entry( n )->get_user_data() );
         
         // Otherwise, delete button should be disabled
         assert( !mv->is_delete_protected());

         m_parent->get_egbase()->get_map()->get_mvm()->delete_variable( mv->get_name() );
*/         m_table->remove_entry( n );
         m_table->sort();

         m_edit_button->set_enabled( false );
         m_delete_button->set_enabled( false );
         
      }
      break;
      
   }
}

/*
 * The table has been selected
 */
void Editor_Objectives_Menu::table_selected( int n ) {
   m_edit_button->set_enabled( true );
   m_delete_button->set_enabled( true );

   MapObjective* obj = static_cast<MapObjective*>(m_table->get_entry( n )->get_user_data());
   // Baad stuff will happen, if trigger got deleted
   m_trigger->set_text( obj->get_trigger()->get_name() );
}

/*
 * Table has been doubleclicked
 */
void Editor_Objectives_Menu::table_dblclicked( int ) {
   // like a click on edit
   clicked( 1 );
}

/*
 * Insert this map variable into the table
 */
void Editor_Objectives_Menu::insert_objective( MapObjective* var ) {
   UITable_Entry* t = new UITable_Entry(m_table, var, -1, true);

   t->set_string(0, narrow_string(var->get_name()).c_str()); 
   t->set_string(1, var->get_is_optional() ? "Yes" : "No");
   t->set_string(2, var->get_is_visible() ? "Yes" : "No");

   m_table->sort();
}


