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
#include "editor_variables_menu.h"
#include "error.h"
#include "map.h"
#include "map_variable_manager.h"
#include "system.h"
#include "ui_button.h"
#include "ui_editbox.h"
#include "ui_modal_messagebox.h"
#include "ui_table.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"
#include "ui_listselect.h"
#include "unicode.h"
#include "util.h"

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
class New_Variable_Window : public UIWindow {
   public:
      New_Variable_Window(Editor_Interactive*);

      bool handle_mouseclick(uint btn, bool down, int mx, int my);

      MapVariable* get_variable( void ) { return m_variable; }
   
   private:
      Editor_Interactive* m_parent;
      MapVariable* m_variable;

   private:
      void clicked( int );
};

New_Variable_Window::New_Variable_Window(Editor_Interactive* parent) :
   UIWindow(parent, 0, 0, 135, 55, _("New Variable")) {
  
   m_parent = parent;
   m_variable = 0;
   
   // What type 
   UIButton* b = new UIButton( this, 5, 5, 60, 20, 0, 0);
   b->set_title(_("Integer"));
   b->clickedid.set(this, &New_Variable_Window::clicked);
   b = new UIButton( this, 70, 5, 60, 20, 0, 1);
   b->set_title(_("String"));
   b->clickedid.set(this, &New_Variable_Window::clicked);

   // back button
   b = new UIButton( this, (get_inner_w()-80)/2, 30, 80, 20, 1, 2);
   b->set_title(_("Back"));
   b->clickedid.set(this, &New_Variable_Window::clicked);

   center_to_parent();
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 */
bool New_Variable_Window::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable
}

/*
 * a button has been clicked
 */
void New_Variable_Window::clicked(int i) {
   // Get the a name

   char buffer[256];

   int n = 1;
   while( 1 ) {
      snprintf(buffer, sizeof(buffer), "%s%i", _("Unnamed"), n);
      if( !m_parent->get_egbase()->get_map()->get_mvm()->get_variable( buffer ))
         break;
      ++n;
   }

   std::string name = buffer;
   switch( i ) {
      case 0:
         // Integer
         m_variable = new Int_MapVariable( 0 );
         m_variable->set_name( buffer);
         m_parent->get_egbase()->get_map()->get_mvm()->register_new_variable( m_variable );
         end_modal(1);
         break;

      case 1: 
         // String
         m_variable = new String_MapVariable( 0 );
         m_variable->set_name( buffer);
         m_parent->get_egbase()->get_map()->get_mvm()->register_new_variable( m_variable );
         end_modal(1);
         break;

      case 2: 
         // back
         end_modal(0);
         break;
   }

   return;
}

/*
 * This is a modal box - The user must end this first
 * before it can return
 */
class Edit_Variable_Window : public UIWindow {
   public:
      Edit_Variable_Window(Editor_Interactive*, UITable_Entry*);

      bool handle_mouseclick(uint btn, bool down, int mx, int my);

   private:
      Editor_Interactive *m_parent;
      UITable_Entry      *m_te;
      UIEdit_Box         *m_name;
      UIEdit_Box         *m_val;

   private:
      void clicked( int );
};

Edit_Variable_Window::Edit_Variable_Window(Editor_Interactive* parent, UITable_Entry* te)
   : UIWindow(parent, 0, 0, 250, 85, _("Edit Variable")) {
  
   m_parent=parent;
   m_te = te;

   int spacing = 5;

   // What type
   new UITextarea(this, 5, 5, 120, 20, _("Name"), Align_CenterLeft);
   m_name = new UIEdit_Box( this, 120, 5, 120, 20, 0, 0);
   m_name->set_text(m_te->get_string(0));
   
   new UITextarea(this, 5, 30, 120, 20, _("Value"), Align_CenterLeft);
   m_val = new UIEdit_Box( this, 120, 35, 120, 20, 0, 0);
   m_val->set_text(m_te->get_string(1));
  
   // back button
   UIButton* b = new UIButton( this, get_inner_w()/2-80-spacing, 60, 80, 20, 1, 0);
   b->set_title(_("Ok"));
   b->clickedid.set(this, &Edit_Variable_Window::clicked);
   b = new UIButton( this, get_inner_w()/2 + spacing, 60, 80, 20, 1, 1);
   b->set_title(_("Back"));
   b->clickedid.set(this, &Edit_Variable_Window::clicked);


   center_to_parent();
}

/*
 * Handle mouseclick
 *
 * we're a modal, therefore we can not delete ourself
 * on close (the caller must do this) instead
 * we simulate a cancel click
 */
bool Edit_Variable_Window::handle_mouseclick(uint btn, bool down, int mx, int my) {
   if(btn == MOUSE_RIGHT && down) {
      clicked(0);
      return true;
   } else
      return false; // we're not dragable
}

/*
 * a button has been clicked
 */
void Edit_Variable_Window::clicked(int i) {
   // Get the a name

   if( i )  {
      // Back
      end_modal(0);
      return;
   } 

   // Ok
   
   // Extract value
   MapVariable* var = static_cast<MapVariable*>(m_te->get_user_data());

   switch( var->get_type() ) {
      case MapVariable::MVT_INT: 
      {
         char* endp;
         long ivar = strtol(m_val->get_text(), &endp, 0);

         if (endp && *endp) {
            char buffer[1024];
            snprintf(buffer, sizeof(buffer), "%s %s", m_val->get_text(), _("is not a valid integer!"));
            UIModal_Message_Box* mb = new UIModal_Message_Box(m_parent, _("Parse error!"), buffer, UIModal_Message_Box::OK); 
            mb->run();
            delete mb;
            return;
         }
         char buffer[256];
         snprintf(buffer, sizeof(buffer), "%li", ivar); 
         
         static_cast<Int_MapVariable*>(var)->set_value( ivar ); 
         m_te->set_string(1, buffer );
      }
      break;

      case MapVariable::MVT_STRING:
      {
         static_cast<String_MapVariable*>(var)->set_value( m_val->get_text() );
         m_te->set_string(1, m_val->get_text() );
      }
      break;
   }
      
   var->set_name( m_name->get_text() );
   m_te->set_string(0, var->get_name() );
   
   end_modal(1); 
}


/*
===============
Editor_Variables_Menu::Editor_Variables_Menu

Create all the buttons etc...
===============
*/
Editor_Variables_Menu::Editor_Variables_Menu(Editor_Interactive *parent, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(parent, registry, 410, 330, _("Variables Menu"))
{
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Variables Menu"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   const int offsx=5;
   const int offsy=25;
   const int spacing=5;
   int posx=offsx;
   int posy=offsy;

   m_table = new UITable(this, posx, posy, get_inner_w()-2*spacing, get_inner_h() - 60);
   m_table->add_column(_("Variable"), UITable::STRING, 300);
   m_table->add_column(_("Value"), UITable::STRING, 100);
   m_table->selected.set(this, &Editor_Variables_Menu::table_selected);
   m_table->double_clicked.set(this, &Editor_Variables_Menu::table_dblclicked);

   // Buttons
   UIButton* nbutton = new UIButton( this, get_inner_w()/2 - 180 - spacing, get_inner_h() - 30, 120, 20, 0, 0);
   nbutton->set_title(_("New"));
   nbutton->clickedid.set(this, &Editor_Variables_Menu::clicked);
   m_edit_button = new UIButton( this, get_inner_w()/2 - 60, get_inner_h() - 30, 120, 20, 0, 1);
   m_edit_button->set_title(_("Edit"));
   m_edit_button->set_enabled(false);
   m_edit_button->clickedid.set(this, &Editor_Variables_Menu::clicked);
   m_delete_button = new UIButton( this, get_inner_w()/2 + 60 + spacing, get_inner_h() - 30, 120, 20, 0, 2);
   m_delete_button->set_title(_("Delete"));
   m_delete_button->set_enabled(false);
   m_delete_button->clickedid.set(this, &Editor_Variables_Menu::clicked);

   // Add all variables
   MapVariableManager* mvm = m_parent->get_egbase()->get_map()->get_mvm();
   for(int i = 0; i < mvm->get_nr_variables(); i++) {
      insert_variable( mvm->get_variable_by_nr( i ));
   }
   
	// Put in the default position, if necessary
	if (get_usedefaultpos())
		center_to_parent();
}

/*
===============
Editor_Variables_Menu::~Editor_Variables_Menu

Unregister from the registry pointer
===============
*/
Editor_Variables_Menu::~Editor_Variables_Menu()
{
}
      
/*
 * A Button has been clicked
 */
void Editor_Variables_Menu::clicked( int n ) { 
   switch( n ) {
      case 0: 
      {
         // Create a new variable
         New_Variable_Window* nvw = new New_Variable_Window( m_parent );
         if( nvw->run() ) { 
            insert_variable( nvw->get_variable() );
            clicked(1);
         }
         delete nvw;
      }
      break;

      case 1:
      {
         // Edit selected variable
         Edit_Variable_Window* evw = new Edit_Variable_Window( m_parent, m_table->get_entry(m_table->get_selection_index()) );
         if( evw->run() ) 
            m_table->sort();
         delete evw;
      }
      break;

      case 2: 
      {
         // Delete selected variable
         int n =  m_table->get_selection_index();
         MapVariable* mv = static_cast<MapVariable*>( m_table->get_entry( n )->get_user_data() );
         
         // Otherwise, delete button should be disabled
         assert( !mv->is_delete_protected());

         m_parent->get_egbase()->get_map()->get_mvm()->delete_variable( mv->get_name() );
         m_table->remove_entry( n );
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
void Editor_Variables_Menu::table_selected( int n ) {
   m_edit_button->set_enabled( true );

   if( !static_cast<MapVariable*>(m_table->get_entry( n )->get_user_data())->is_delete_protected())
      m_delete_button->set_enabled( true );
   else
      m_delete_button->set_enabled( false );
}

/*
 * Table has been doubleclicked
 */
void Editor_Variables_Menu::table_dblclicked( int ) {
   // like a click on edit
   clicked( 1 );
}

/*
 * Insert this map variable into the table
 */
void Editor_Variables_Menu::insert_variable( MapVariable* var ) {
   const char* pic = 0;
   
   switch( var->get_type() ) {
      case MapVariable::MVT_INT: pic = "pics/map_variable_int.png"; break;
      case MapVariable::MVT_STRING: pic = "pics/map_variable_string.png"; break;
      default: pic = "nothing";
   };
   
   UITable_Entry* t = new UITable_Entry(m_table, var, g_gr->get_picture( PicMod_UI, pic ), true );
   t->set_string(0, var->get_name());

   std::string val;

   switch( var->get_type() ) {
      case MapVariable::MVT_INT: 
      {
         char buffer[256];
         snprintf(buffer, sizeof(buffer), "%li", static_cast<Int_MapVariable*>(var)->get_value());
         val = buffer; 
      }
      break;
      
      case MapVariable::MVT_STRING: 
      val = static_cast<String_MapVariable*>(var)->get_value();
      break;

      default: val = "";
   }
   t->set_string(0, var->get_name()); 
   t->set_string(1, val.c_str()); 

   if( var->is_delete_protected()) 
      t->set_color(RGBColor(255,0,0));
   
   m_table->sort();
}


