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

#include "game.h"
#include "game_loader.h"
#include "game_main_menu_load_game.h"
#include "game_preload_data_packet.h"
#include "game_loader.h"
#include "i18n.h"
#include "interactive_player.h"
#include "layeredfilesystem.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_modal_messagebox.h"
#include "ui_textarea.h"

/*
===============
Game_Main_Menu_Load_Game::Game_Main_Menu_Load_Game

Create all the buttons etc...
===============
*/
Game_Main_Menu_Load_Game::Game_Main_Menu_Load_Game(Interactive_Player* parent, UIUniqueWindowRegistry* registry)
  : UIUniqueWindow(parent,registry,400,270,_("Load Game")) {
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Load Game"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   int spacing=5;
   int offsx=spacing;
   int offsy=30;
   int posx=offsx;
   int posy=offsy;

   // listselect
   m_ls=new UIListselect(this, posx, posy, get_inner_w()/2-spacing, get_inner_h()-spacing-offsy-60);
   m_ls->selected.set(this, &Game_Main_Menu_Load_Game::selected);
   m_ls->double_clicked.set(this, &Game_Main_Menu_Load_Game::double_clicked);

   // the descriptive areas
   // Name
   posx=get_inner_w()/2+spacing;
   posy+=20;
   new UITextarea(this, posx, posy, 150, 20, _("Map Name: "), Align_CenterLeft);
   m_name=new UITextarea(this, posx+90, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Author
   new UITextarea(this, posx, posy, 150, 20, _("Game Time: "), Align_CenterLeft);
   m_gametime=new UITextarea(this, posx+90, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Buttons
   posx=5;
   posy=get_inner_h()-30;
   UIButton* but= new UIButton(this, get_inner_w()/2-spacing-80, posy, 80, 20, 4, 1);
   but->clickedid.set(this, &Game_Main_Menu_Load_Game::clicked);
   but->set_title(_("OK").c_str());
   but->set_enabled(false);
   m_ok_btn=but;
   but= new UIButton(this, get_inner_w()/2+spacing, posy, 80, 20, 4, 0);
   but->clickedid.set(this, &Game_Main_Menu_Load_Game::clicked);
   but->set_title(_("Cancel").c_str());

   m_basedir="ssave";
   m_curdir="ssave";

   fill_list();

   center_to_parent();
   move_to_top();
}

/*
===============
Game_Main_Menu_Load_Game::~Game_Main_Menu_Load_Game

Unregister from the registry pointer
===============
*/
Game_Main_Menu_Load_Game::~Game_Main_Menu_Load_Game()
{
}

/*
===========
called when the ok button has been clicked
===========
*/
void Game_Main_Menu_Load_Game::clicked(int id) {
   if(id==1) {
      // Ok
      std::string filename=static_cast<const char*>(m_ls->get_selection());

      // Ok, load this map
      if(load_game(filename))
         die();
   } else if(id==0) {
      // Cancel
      die();
   }
}

/*
 * called when a item is selected
 */
void Game_Main_Menu_Load_Game::selected(int i) {
   const char* name=static_cast<const char*>(m_ls->get_selection());

   FileSystem* fs = g_fs->MakeSubFileSystem( name );

   Game_Loader gl(fs, m_parent->get_game());
   Game_Preload_Data_Packet gpdp;
   gl.preload_game(&gpdp); // This has worked before, no problem

   m_ok_btn->set_enabled(true);

   m_name->set_text(gpdp.get_mapname());

   char buf[200];
   uint gametime = gpdp.get_gametime();

   int hours = gametime / 3600000;
   gametime -= hours * 3600000;
   int minutes = gametime / 60000;

   sprintf(buf, "%02i:%02i", hours, minutes);
   m_gametime->set_text(buf);

   delete fs;
}

/*
 * An Item has been doubleclicked
 */
void Game_Main_Menu_Load_Game::double_clicked(int) {
   clicked(1);
}

/**
 * fill the file list
 */
void Game_Main_Menu_Load_Game::fill_list(void) {
   // Fill it with all files we find.
   g_fs->FindFiles(m_curdir, "*", &m_gamefiles, 1);

   Game_Preload_Data_Packet gpdp;

   for(filenameset_t::iterator pname = m_gamefiles.begin(); pname != m_gamefiles.end(); pname++) {
      const char *name = pname->c_str();


      Game_Loader* gl = 0;
      FileSystem* fs = 0;
      try {
         fs = g_fs->MakeSubFileSystem( name );
         gl = new Game_Loader(fs,m_parent->get_game());
         gl->preload_game(&gpdp);

	 char* fname = strdup(FileSystem::FS_Filename(name));
	 FileSystem::FS_StripExtension(fname);
         m_ls->add_entry(fname, reinterpret_cast<void*>(const_cast<char*>(name)));
         free(fname);

      } catch(wexception& ) {
         // we simply skip illegal entries
      }
      if( gl )
         delete gl;
      if( fs )
         delete fs;
   }

   if(m_ls->get_nr_entries())
      m_ls->select(0);
}

/*
 * The editbox was changed. Enable ok button
 */
void Game_Main_Menu_Load_Game::edit_box_changed(void) {
   m_ok_btn->set_enabled(true);
}

/*
 * Load the game
 *
 * returns true if dialog should close, false if it
 * should stay open
 */
bool Game_Main_Menu_Load_Game::load_game(std::string filename) {

   Game_Loader* gl = 0;
   FileSystem* fs = 0;
   try {
      fs = g_fs->MakeSubFileSystem( filename );
      gl=new Game_Loader(fs, m_parent->get_game());
      m_parent->get_game()->cleanup_for_load(true,true);
      gl->load_game();
      m_parent->get_game()->postload();
      m_parent->get_game()->load_graphics();
   } catch(std::exception& exe) {
      std::string s=_("Game Loading Error!\nReason given:\n");
      s+=exe.what();
      UIModal_Message_Box* mbox= new UIModal_Message_Box(m_parent, _("Load Game Error!!"), s, UIModal_Message_Box::OK);
      mbox->run();
      delete mbox;
   }
   if( gl )
      delete gl;
   if( fs )
      delete fs;
   die();

   return true;
}
