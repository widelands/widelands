/*
 * Copyright (C) 2002-2004 by The Widelands Development Team
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

#include "IntPlayer.h"
#include "building.h"
#include "cmd_queue.h"
#include "fieldaction.h"
#include "font_handler.h"
#include "game_saver.h"
#include "keycodes.h"
#include "immovable.h"
#include "mapview.h"
#include "player.h"
#include "ui_editbox.h"
#include "ui_button.h"
#include "ui_textarea.h"
#include "ui_unique_window.h"
#include "overlay_manager.h"

class Game_Main_Menu_Save_Game : public UIUniqueWindow {
   public:
      Game_Main_Menu_Save_Game(Interactive_Player* plr, UIUniqueWindowRegistry* registry) ;
      virtual ~Game_Main_Menu_Save_Game(void);

   private:
      void clicked(int);
      Interactive_Player* m_parent;
      UIEdit_Box* m_editbox;
};

Game_Main_Menu_Save_Game::Game_Main_Menu_Save_Game(Interactive_Player* plr, UIUniqueWindowRegistry* registry) :
UIUniqueWindow(plr,registry,105,140,"Save_Game") {

   m_parent=plr;

   m_editbox=new UIEdit_Box(this, 5, 5, get_inner_w()-10, 20, 0, 0);

   UIButton* b=new UIButton(this, (get_inner_w()-60)/2, get_inner_h()-30, 60, 20, 0, 1);
   b->set_title("OK");
   b->clickedid.set(this, &Game_Main_Menu_Save_Game::clicked);

   if(get_usedefaultpos())
      center_to_parent();
}

Game_Main_Menu_Save_Game::~Game_Main_Menu_Save_Game(void) {
}

void Game_Main_Menu_Save_Game::clicked(int) {
   std::string t=m_editbox->get_text();

   if(t.size()) {
      Game_Saver gs(m_editbox->get_text(), m_parent->get_game());
      gs.save();
      die();
   }
}

class Game_Main_Menu_Load_Game : public UIUniqueWindow {
   public:
      Game_Main_Menu_Load_Game(Interactive_Player* plr, UIUniqueWindowRegistry* registry) ;
      virtual ~Game_Main_Menu_Load_Game(void);

   private:
      void clicked(int);
      Interactive_Player* m_parent;
      UIEdit_Box* m_editbox;
};

Game_Main_Menu_Load_Game::Game_Main_Menu_Load_Game(Interactive_Player* plr, UIUniqueWindowRegistry* registry) :
UIUniqueWindow(plr,registry,105,140,"Load_Game") {

   m_parent=plr;

   m_editbox=new UIEdit_Box(this, 5, 5, get_inner_w()-10, 20, 0, 0);

   UIButton* b=new UIButton(this, (get_inner_w()-60)/2, get_inner_h()-30, 60, 20, 0, 1);
   b->set_title("OK");
   b->clickedid.set(this, &Game_Main_Menu_Load_Game::clicked);

   if(get_usedefaultpos())
      center_to_parent();
}

Game_Main_Menu_Load_Game::~Game_Main_Menu_Load_Game(void) {
}

void Game_Main_Menu_Load_Game::clicked(int) {
   std::string t=m_editbox->get_text();

   if(t.size()) {
      m_parent->get_game()->cleanup_for_load(true,true); // TODO: this should really clean up all
      // Load Game
      Game_Saver gs(m_editbox->get_text(), m_parent->get_game());
      gs.load();
      m_parent->get_game()->postload();
      m_parent->get_game()->load_graphics();
      die();
   }
}
/*
==============================================================================

GameMainMenu IMPLEMENTATION

==============================================================================
*/

// The GameMainMenu is a rather dumb window with lots of buttons
class GameMainMenu : public UIUniqueWindow {
public:
	GameMainMenu(Interactive_Player *plr, UIUniqueWindowRegistry *registry);
	virtual ~GameMainMenu();

private:
   UIUniqueWindowRegistry m_saveload;
	Interactive_Player	*m_player;
   void clicked(int);
};

/*
===============
GameMainMenu::GameMainMenu

Create all the buttons etc...
===============
*/
GameMainMenu::GameMainMenu(Interactive_Player *plr, UIUniqueWindowRegistry *registry)
	: UIUniqueWindow(plr, registry, 102, 136, "Menu")
{
   m_player=plr;

   UIButton* b=new UIButton(this, 5, 5, get_inner_w()-10, 20, 0, 1);
   b->set_title("Save Game");
   b->clickedid.set(this, &GameMainMenu::clicked);

   b=new UIButton(this, 5, 30, get_inner_w()-10, 20, 0, 2);
   b->set_title("Load Game");
   b->clickedid.set(this, &GameMainMenu::clicked);
  
	if (get_usedefaultpos())
		center_to_parent();
}

void GameMainMenu::clicked(int n) {
   if(n==1) {
      // Save
      new Game_Main_Menu_Save_Game(m_player, &m_saveload);
   } else {
      // Load
      new Game_Main_Menu_Load_Game(m_player, &m_saveload);
   }
}

/*
===============
GameMainMenu::~GameMainMenu
===============
*/
GameMainMenu::~GameMainMenu()
{
}


/*
==============================================================================

Interactive_Player IMPLEMENTATION

==============================================================================
*/

// This function is the callback for recalculation of field overlays
int Int_Player_overlay_callback_function(FCoords& fc, void* data, int) {
   Interactive_Player* plr=static_cast<Interactive_Player*>(data);

   return plr->get_player()->get_buildcaps(fc);
}


/*
===============
Interactive_Player::Interactive_Player

Initialize
===============
*/
Interactive_Player::Interactive_Player(Game *g, uchar plyn) : Interactive_Base(g)
{
   // Setup all screen elements
	m_game = g;
	set_player_number(plyn);

	Map_View* mview;
   mview = new Map_View(this, 0, 0, get_w(), get_h(), this);
	mview->warpview.set(this, &Interactive_Player::mainview_move);
	mview->fieldclicked.set(this, &Interactive_Player::field_action);
   set_mapview(mview);

	// user interface buttons
	int x = (get_w() - (4*34)) >> 1;
	int y = get_h() - 34;
	UIButton *b;

	// temp (should be toggle messages)
	b = new UIButton(this, x, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::exit_game_btn);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_exit_game.png", true));
	// temp

	b = new UIButton(this, x+34, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::main_menu_btn);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_menu.png", true));

	b = new UIButton(this, x+68, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::toggle_minimap);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_minimap.png", true));

	b = new UIButton(this, x+102, y, 34, 34, 2);
	b->clicked.set(this, &Interactive_Player::toggle_buildhelp);
	b->set_pic(g_gr->get_picture(PicMod_Game, "pics/menu_toggle_buildhelp.png", true));

	// Speed info
	m_label_speed = new UITextarea(this, get_w(), 0, 0, 0, "", Align_TopRight);

}

/*
===============
Interactive_Player::~Interactive_Player

cleanups
===============
*/
Interactive_Player::~Interactive_Player(void)
{
}


/*
===============
Interactive_Player::think

Update the speed display.
===============
*/
void Interactive_Player::think()
{
	Interactive_Base::think();

	// Draw speed display
	int speed = m_game->get_speed();
	char buf[32] = "";

	if (!speed)
		strcpy(buf, "PAUSE");
	else if (speed > 1)
		snprintf(buf, sizeof(buf), "%ix", speed);

	m_label_speed->set_text(buf);
}


/*
===============
Interactive_Player::start

Called just before the game starts, after postload, init and gfxload
===============
*/
void Interactive_Player::start()
{
	int mapw;
	int maph;

	mapw = 0;
	maph = 0;

   get_map()->get_overlay_manager()->show_buildhelp(false);
   get_map()->get_overlay_manager()->register_overlay_callback_function(&Int_Player_overlay_callback_function, static_cast<void*>(this));

   // Recalc whole map for changed owner stuff
   get_map()->recalc_whole_map();
}



/** Interactive_Player::exit_game_btn(void *a)
 *
 * Handle exit button
 */
void Interactive_Player::exit_game_btn()
{
	end_modal(0);
}

/*
===============
Interactive_Player::main_menu_btn

Bring up or close the main menu
===============
*/
void Interactive_Player::main_menu_btn()
{
	if (m_mainmenu.window)
		delete m_mainmenu.window;
	else
		new GameMainMenu(this, &m_mainmenu);
}

//
// Toggles buildhelp rendering in the main MapView
//
void Interactive_Player::toggle_buildhelp(void)
{
   get_map()->get_overlay_manager()->toggle_buildhelp();
}

/*
===============
Interactive_Player::field_action

Player has clicked on the given field; bring up the context menu.
===============
*/
void Interactive_Player::field_action()
{
	if (!get_player()->is_field_seen(get_fieldsel_pos()))
		return;

	// Special case for buildings
	BaseImmovable *imm = m_game->get_map()->get_immovable(get_fieldsel_pos());

	if (imm && imm->get_type() == Map_Object::BUILDING) {
		Building *building = (Building *)imm;

		if (building->get_owner()->get_player_number() == get_player_number()) {
			building->show_options(this);
			return;
		}
	}

	// everything else can bring up the temporary dialog
	show_field_action(this, get_player(), &m_fieldaction);
}

/*
===============
Interactive_Player::handle_key

Global in-game keypresses:
Space: toggles buildhelp
F5: reveal map
===============
*/
bool Interactive_Player::handle_key(bool down, int code, char c)
{
	switch(code) {
	case KEY_SPACE:
		if (down)
			toggle_buildhelp();
		return true;

	case KEY_m:
		if (down)
			toggle_minimap();
		return true;

	case KEY_c:
		if (down)
			set_display_flag(dfShowCensus, !get_display_flag(dfShowCensus));
		return true;

	case KEY_s:
		if (down)
			set_display_flag(dfShowStatistics, !get_display_flag(dfShowStatistics));
		return true;

	case KEY_PAGEUP:
		if (down) {
			int speed = m_game->get_speed();

			m_game->set_speed(speed + 1);
		}
		return true;

	case KEY_PAGEDOWN:
		if (down) {
			int speed = m_game->get_speed();

			m_game->set_speed(std::max(0, speed-1));
		}
		return true;

	case KEY_F5:
		if (down) {
         get_player()->set_see_all(!get_player()->get_see_all());
		}
		return true;
	}

	return false;
}

/*
 * set the player and the visibility to this
 * player
 */
void Interactive_Player::set_player_number(uint n) {
   m_player_number=n;
}


/*
 * Return our players visibility
 */
std::vector<bool>* Interactive_Player::get_visibility(void) {
   return m_game->get_player(m_player_number)->get_visibility(); 
}

