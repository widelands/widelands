/*
 * Copyright (C) 2002-2004 by Widelands Development Team
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
/*
This file contains the options windows that are displayed when you click on
a building, plus the necessary hook function(s) in the class Building itself.

This is seperated out because options windows should _never_ manipulate
buildings directly. Instead, they must send a player command through the Game
class.
*/

#include "building.h"
#include "cmd_queue.h"
#include "constructionsite.h"
#include "error.h"
#include "font_handler.h"
#include "game_debug_ui.h"
#include "i18n.h"
#include "interactive_player.h"
#include "militarysite.h"
#include "player.h"
#include "productionsite.h"
#include "rendertarget.h"
#include "soldier.h"
#include "trainingsite.h"
#include "transport.h"
#include "tribe.h"
#include "ui_box.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_progressbar.h"
#include "ui_table.h"
#include "ui_tabpanel.h"
#include "ui_textarea.h"
#include "ui_window.h"
#include "warehouse.h"
#include "waresdisplay.h"
#include "wexception.h"
#include "worker.h"

static const char* pic_ok = "pics/menu_okay.png";
static const char* pic_cancel = "pics/menu_abort.png";
static const char* pic_debug = "pics/menu_debug.png";

static const char* pic_bulldoze = "pics/menu_bld_bulldoze.png";
static const char* pic_queue_background = "pics/queue_background.png";

static const char* pic_list_worker = "pics/menu_list_workers.png";

static const char* pic_tab_military = "pics/menu_tab_military.png";
static const char* pic_tab_training = "pics/menu_tab_training.png";
static const char* pic_up_train = "pics/menu_up_train.png";
static const char* pic_down_train = "pics/menu_down_train.png";
static const char* pic_train_options = "pics/menu_train_options.png";
static const char* pic_drop_soldier = "pics/menu_drop_soldier.png";

/*
==============================================================================

Building UI IMPLEMENTATION

==============================================================================
*/

/*
===============
Building::show_options

Create the building's options window if necessary.
===============
*/
void Building::show_options(Interactive_Player *plr)
{
	if (m_optionswindow)
		m_optionswindow->move_to_top();
	else
		create_options_window(plr, &m_optionswindow);
}

/*
===============
Building::hide_options

Force the destruction of the options window.
===============
*/
void Building::hide_options()
{
	if (m_optionswindow)
		delete m_optionswindow;
}


/*
==============================================================================

class BulldozeConfirm

==============================================================================
*/

/*
class BulldozeConfirm
---------------------
Confirm the bulldoze request for a building.
*/
class BulldozeConfirm : public UIWindow {
public:
	BulldozeConfirm(Interactive_Base* parent, Building* building, PlayerImmovable* todestroy = 0);
	virtual ~BulldozeConfirm();

	virtual void think();

private:
	void bulldoze();

private:
	Interactive_Base*	   m_iabase;
	Object_Ptr				m_building;
	Object_Ptr				m_todestroy;
};


/*
===============
BulldozeConfirm::BulldozeConfirm

Create the panels.
If todestroy is 0, the building will be destroyed when the user confirms it.
Otherwise, todestroy is destroyed when the user confirms it. This is useful to
confirm building destruction when the building's base flag is removed.
===============
*/
BulldozeConfirm::BulldozeConfirm(Interactive_Base* parent, Building* building, PlayerImmovable* todestroy)
	: UIWindow(parent, 0, 0, 160, 90, _("Destroy building?"))
{
	UIButton* btn;
	std::string text;

	m_iabase = parent;
	m_building = building;

	if (!todestroy)
		m_todestroy = building;
	else
		m_todestroy = todestroy;

	text = _("Do you really want to destroy this ");
	text += building->get_name();
	text += "?";
	new UITextarea(this, 0, 0, 160, 44, text, Align_Center, true);

	btn = new UIButton(this, 6, 50, 60, 34, 4);
	btn->clicked.set(this, &BulldozeConfirm::bulldoze);
	btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_ok ));

	btn = new UIButton(this, 94, 50, 60, 34, 4);
	btn->clicked.set(this, &BulldozeConfirm::die);
	btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_cancel ));

	btn->center_mouse();
}


/*
===============
BulldozeConfirm::~BulldozeCOnfirm
===============
*/
BulldozeConfirm::~BulldozeConfirm()
{
}


/*
===============
BulldozeConfirm::think

Make sure the building still exists and can in fact be bulldozed.
===============
*/
void BulldozeConfirm::think()
{
	Editor_Game_Base* egbase = m_iabase->get_egbase();
	Building* building = (Building*)m_building.get(egbase);
	PlayerImmovable* todestroy = (PlayerImmovable*)m_todestroy.get(egbase);

	if (!todestroy || !building ||
	    !(building->get_playercaps() & (1 << Building::PCap_Bulldoze)))
		die();
}


/*
===============
BulldozeConfirm::bulldoze

Issue the CMD_BULLDOZE command for this building.
===============
*/
void BulldozeConfirm::bulldoze()
{
	Editor_Game_Base* egbase = m_iabase->get_egbase();
	Building* building = (Building*)m_building.get(egbase);
	PlayerImmovable* todestroy = (PlayerImmovable*)m_todestroy.get(egbase);

	if (todestroy && building && building->get_playercaps() & (1 << Building::PCap_Bulldoze)) {
      if(egbase->is_game()) {
         // Game
         Game* g=static_cast<Game*>(egbase);
         g->send_player_bulldoze (todestroy);
         m_iabase->need_complete_redraw();
      } else {
         // Editor
         Player* plr=todestroy->get_owner();
         plr->bulldoze(todestroy);
         m_iabase->need_complete_redraw();
      }
   }

	die();
}


/*
===============
show_bulldoze_confirm

Create a BulldozeConfirm window.
building is the building that the confirmation dialog displays.
todestroy is the immovable that will be bulldozed if the user confirms the
dialog.
===============
*/
void show_bulldoze_confirm(Interactive_Base* player, Building* building, PlayerImmovable* todestroy)
{
	new BulldozeConfirm(player, building, todestroy);
}


/*
==============================================================================

class WaresQueueDisplay

==============================================================================
*/

/*
class WaresQueueDisplay
-----------------------
This passive class displays the status of a WaresQueue.
It updates itself automatically through think().
*/
class WaresQueueDisplay : public UIPanel {
public:
	enum {
		CellWidth = 24,
		CellHeight = 24,
		Border = 4,

		Height = CellHeight + 2 * Border,

		BG_LeftBorderX = 0,
		BG_CellX = BG_LeftBorderX + Border,
		BG_RightBorderX = BG_CellX + CellWidth,
		BG_ContinueCellX = BG_RightBorderX + Border,
		BG_ContinueBorderX = BG_ContinueCellX + CellWidth,
	};

public:
	WaresQueueDisplay(UIPanel* parent, int x, int y, uint maxw, WaresQueue* queue, Game* g);
	~WaresQueueDisplay();

	virtual void think();
	virtual void draw(RenderTarget* dst);

private:
	void recalc_size();

private:
	WaresQueue*		m_queue;
	uint				m_max_width;
	uint				m_pic_empty;
	uint				m_pic_full;
	uint				m_pic_background;

	uint				m_cache_size;
	uint				m_cache_filled;
	uint				m_display_size;
};


/*
===============
WaresQueueDisplay::WaresQueueDisplay

Initialize the panel.
===============
*/
WaresQueueDisplay::WaresQueueDisplay(UIPanel* parent, int x, int y, uint maxw, WaresQueue* queue, Game* g)
	: UIPanel(parent, x, y, 0, Height)
{
	Item_Ware_Descr* waredescr;

	m_queue = queue;
	m_max_width = maxw;

	waredescr = queue->get_owner()->get_tribe()->get_ware_descr(m_queue->get_ware());

	set_tooltip(waredescr->get_descname());

	m_pic_empty = waredescr->get_pic_queue_empty();
	m_pic_full = waredescr->get_pic_queue_full();

	m_cache_size = m_queue->get_size();
	m_cache_filled = m_queue->get_filled();
	m_display_size = 0;

	m_pic_background = g_gr->get_picture( PicMod_Game,  pic_queue_background );

	recalc_size();

	set_think(true);
}


/*
===============
WaresQueueDisplay::~WaresQueueDisplay

Cleanup
===============
*/
WaresQueueDisplay::~WaresQueueDisplay()
{
}


/*
===============
WaresQueueDisplay::recalc_size

Recalculate the panel's size.
===============
*/
void WaresQueueDisplay::recalc_size()
{
	m_display_size = (m_max_width - 2*Border) / CellWidth;

	m_cache_size = m_queue->get_size();

	if (m_cache_size < m_display_size)
		m_display_size = m_cache_size;

	set_size(m_display_size*CellWidth + 2*Border, Height);
}


/*
===============
WaresQueueDisplay::think

Compare the current WaresQueue state with the cached state; update if necessary.
===============
*/
void WaresQueueDisplay::think()
{
	if ((uint)m_queue->get_size() != m_cache_size)
		recalc_size();

	if ((uint)m_queue->get_filled() != m_cache_filled)
		update(0, 0, get_w(), get_h());
}


/*
===============
WaresQueueDisplay::draw

Render the current WaresQueue state.
===============
*/
void WaresQueueDisplay::draw(RenderTarget* dst)
{
	int x;

	if (!m_display_size)
		return;

	m_cache_filled = m_queue->get_filled();

	// Draw it
	dst->blitrect(0, 0, m_pic_background, BG_LeftBorderX, 0, Border, Height);

	x = Border;

	for(uint cells = 0; cells < m_display_size; cells++) {
		uint pic;

		if (cells+1 == m_display_size && m_cache_size > m_display_size)
			dst->blitrect(x, 0, m_pic_background, BG_ContinueCellX, 0, CellWidth, Height);
		else
			dst->blitrect(x, 0, m_pic_background, BG_CellX, 0, CellWidth, Height);

		if (cells < m_cache_filled)
			pic = m_pic_full;
		else
			pic = m_pic_empty;

		dst->blit(x, Border, pic);

		x += CellWidth;
	}

	if (m_cache_size > m_display_size)
		dst->blitrect(x, 0, m_pic_background, BG_ContinueBorderX, 0, Border, Height);
	else
		dst->blitrect(x, 0, m_pic_background, BG_RightBorderX, 0, Border, Height);
}


/*
==============================================================================

class Building_Window

==============================================================================
*/

/*
class Building_Window
---------------------
Baseclass providing common tools for building windows.
*/
class Building_Window : public UIWindow {
	friend class TrainingSite_Window;
	friend class MilitarySite_Window;
public:
	enum {
		Width = 136		// 4*34, 4 normally sized buttons
	};

public:
	Building_Window(Interactive_Player* parent, Building* building, UIWindow** registry);
	virtual ~Building_Window();

	Interactive_Player* get_player() { return m_player; }
	Building* get_building() { return m_building; }

	virtual void draw(RenderTarget* dst);
	virtual void think();

	UIPanel* create_capsbuttons(UIPanel* parent);

protected:
	void setup_capsbuttons();

	void act_bulldoze();
	void act_debug();
	void act_start_stop();
   void act_enhance(int);
   void act_drop_soldier(uint);
	void act_change_soldier_capacity(int);

private:
	UIWindow**				m_registry;
	Interactive_Player*	m_player;
	Building*				m_building;

	UIPanel*	m_capsbuttons;		// UIPanel that contains capabilities buttons
	uint		m_capscache;		// capabilities that were last used in setting up the caps panel
};


/*
===============
Building_Window::Building_Window

Create the window, add it to the registry.
===============
*/
Building_Window::Building_Window(Interactive_Player* parent, Building* building, UIWindow** registry)
	: UIWindow(parent, 0, 0, Width, 0, building->get_descname())
{
	m_registry = registry;
	if (*m_registry)
		delete *m_registry;
	*m_registry = this;

	m_player = parent;
	m_building = building;

	m_capsbuttons = 0;
	m_capscache = 0;

	move_to_mouse();

	set_think(true);
}


/*
===============
Building_Window::~Building_Window

Add to registry
===============
*/
Building_Window::~Building_Window()
{
	*m_registry = 0;
}


/*
===============
Building_Window::draw

Draw a picture of the building in the background.
===============
*/
void Building_Window::draw(RenderTarget* dst)
{
	uint anim = get_building()->get_ui_anim();

	dst->drawanim(get_inner_w() / 2, get_inner_h() / 2, anim, 0, 0);

	// Draw all the panels etc. above the background
	UIWindow::draw(dst);
}


/*
===============
Building_Window::think

Check the capabilities and setup the capsbutton panel in case they've changed.
===============
*/
void Building_Window::think()
{
	if (m_capsbuttons) {
		if (get_building()->get_playercaps() != m_capscache)
			setup_capsbuttons();
	}

	UIWindow::think();
}


/*
===============
Building_Window::create_capsbuttons

Create the capsbuttons panel with the given parent window, set it up and return
it.
===============
*/
UIPanel* Building_Window::create_capsbuttons(UIPanel* parent)
{
	if (m_capsbuttons)
		delete m_capsbuttons;

	m_capsbuttons = new UIPanel(parent, 0, 0, Width, 34);
	setup_capsbuttons();

	return m_capsbuttons;
}


/*
===============
Building_Window::setup_capsbuttons

Clear the capsbuttons panel and re-setup.
===============
*/
void Building_Window::setup_capsbuttons()
{
	int x;

	assert(m_capsbuttons);

	m_capsbuttons->free_children();
	m_capscache = get_building()->get_playercaps();

	x = 0;

	if (m_capscache & (1 << Building::PCap_Stopable)) {
		std::string icon;
		if (m_building->get_stop())
			icon = m_building->get_continue_icon();
		else
			icon = m_building->get_stop_icon();
		UIButton* btn = new UIButton(m_capsbuttons, x, 0, 34, 34, 4);
		btn->clicked.set(this, &Building_Window::act_start_stop);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  icon.c_str() ));
		x += 34;
	}

   if(m_capscache & (1 << Building::PCap_Enhancable)) {
      const std::vector<char*>* buildings=m_building->get_enhances_to();
      for(uint i=0; i<buildings->size(); i++) {
         int id=m_player->get_player()->get_tribe()->get_building_index((*buildings)[i]);
         if(id==-1)
            throw wexception("Should enhance to unknown building: %s\n", (*buildings)[i]);

         if(!m_player->get_player()->is_building_allowed(id)) {
            // This buildings is disabled for this scenario, sorry.
            // Try again later!!
            continue;
         }

         UIButton* btn = new UIButton(m_capsbuttons, x, 0, 34, 34, 4, id); // Button id == building id
         btn->clickedid.set(this, &Building_Window::act_enhance);
         btn->set_pic(m_player->get_player()->get_tribe()->get_building_descr(id)->get_buildicon());
         x += 34;
      }
   }

	if (m_capscache & (1 << Building::PCap_Bulldoze)) {
		UIButton* btn = new UIButton(m_capsbuttons, x, 0, 34, 34, 4);
		btn->clicked.set(this, &Building_Window::act_bulldoze);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_bulldoze ));
		x += 34;
	}

	if (m_player->get_display_flag(Interactive_Base::dfDebug)) {
		UIButton* btn = new UIButton(m_capsbuttons, x, 0, 34, 34, 4);
		btn->clicked.set(this, &Building_Window::act_debug);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_debug ));
		x += 34;
	}

   if( x == 0 ) {
      // no capsbutton is in this window
      // resize us, so that we do not take space
      m_capsbuttons->set_inner_size(0,0);
   }
}


/*
===============
Building_Window::act_bulldoze

Callback for bulldozing request
===============
*/
void Building_Window::act_bulldoze()
{
	new BulldozeConfirm(m_player, m_building);
}

void Building_Window::act_start_stop() {
	Game* g = m_player->get_game();
	if (m_building && m_building->get_playercaps() & (1 << Building::PCap_Stopable))
		g->send_player_start_stop_building (m_building);

	die();
}

/*
===============
Building_Window::act_bulldoze

Callback for bulldozing request
===============
*/
void Building_Window::act_enhance(int id)
{
	Game* g = m_player->get_game();
	if (m_building && m_building->get_playercaps() & (1 << Building::PCap_Enhancable))
		g->send_player_enhance_building (m_building, id);

	die();
}

/*
===============
Building_Window::act_drop_soldier

Callback for bulldozing request
===============
*/
void Building_Window::act_drop_soldier(uint serial) {
	 Game* g = m_player->get_game();
log ("m_building = %d\n", m_building ? 1 : 0);
log ("sold serial = %d\n", serial);

	 if (m_building && (serial > 0))
		  g->send_player_drop_soldier (m_building, serial);
}

/*
===============
Building_Window::act_change_soldier_capacity

Do a modification at soldier capacity on military and training sites.
TODO: Check that building is a military or a training site.
===============
*/
void Building_Window::act_change_soldier_capacity(int value)
{
	Game* g = m_player->get_game();
	if (m_building)
		g->send_player_change_soldier_capacity (m_building, value);

}

/*
===============
Building_Window::act_debug

Callback for debug window
===============
*/
void Building_Window::act_debug()
{
	show_mapobject_debug(m_player, m_building);
}


/*
==============================================================================

ConstructionSite UI IMPLEMENTATION

==============================================================================
*/

class ConstructionSite_Window : public Building_Window {
public:
	ConstructionSite_Window(Interactive_Player* parent, ConstructionSite* cs, UIWindow** registry);
	virtual ~ConstructionSite_Window();

	ConstructionSite* get_constructionsize() { return (ConstructionSite*)get_building(); }

	virtual void think();

private:
	UIProgress_Bar*	m_progress;
};


/*
===============
ConstructionSite_Window::ConstructionSite_Window

Create the window and its panels
===============
*/
ConstructionSite_Window::ConstructionSite_Window(Interactive_Player* parent, ConstructionSite* cs,
                                                 UIWindow** registry)
	: Building_Window(parent, cs, registry)
{
	UIBox* box = new UIBox(this, 0, 0, UIBox::Vertical);

	// Add the progress bar
	m_progress = new UIProgress_Bar(box, 0, 0, UIProgress_Bar::DefaultWidth, UIProgress_Bar::DefaultHeight,
							UIProgress_Bar::Horizontal);
	m_progress->set_total(1 << 16);
	box->add(m_progress, UIBox::AlignCenter);

	box->add_space(8);

	// Add the wares queue
	for(uint i = 0; i < cs->get_nrwaresqueues(); i++)
	{
		WaresQueueDisplay* wqd = new WaresQueueDisplay(box, 0, 0, get_w(),
					cs->get_waresqueue(i), parent->get_game());

		box->add(wqd, UIBox::AlignLeft);
	}

	box->add_space(8);

	// Add the caps button
	box->add(create_capsbuttons(box), UIBox::AlignCenter);

	fit_inner(box);
}


/*
===============
ConstructionSite_Window::~ConstructionSite_Window

Deinitialize
===============
*/
ConstructionSite_Window::~ConstructionSite_Window()
{
}


/*
===============
ConstructionSite_Window::think

Make sure the window is redrawn when necessary.
===============
*/
void ConstructionSite_Window::think()
{
	Building_Window::think();

	m_progress->set_state(((ConstructionSite*)get_building())->get_built_per64k());
}


/*
===============
ConstructionSite::create_options_window

Create the status window describing the construction site.
===============
*/
UIWindow *ConstructionSite::create_options_window(Interactive_Player *plr, UIWindow **registry)
{
	return new ConstructionSite_Window(plr, this, registry);
}


/*
==============================================================================

Warehouse UI IMPLEMENTATION

==============================================================================
*/

class Warehouse_Window : public Building_Window {
public:
	Warehouse_Window(Interactive_Player *parent, Warehouse *wh, UIWindow **registry);
	virtual ~Warehouse_Window();

	Warehouse* get_warehouse() { return (Warehouse*)get_building(); }

	virtual void think();

private:
   void clicked(int);
   void switch_page( void );

private:
	WaresDisplay*			m_waresdisplay;
   Interactive_Player*  m_parent;
   int                  m_curpage;
};

/*
===============
Warehouse_Window::Warehouse_Window

Open the window, create the window buttons and add to the registry.
===============
*/
Warehouse_Window::Warehouse_Window(Interactive_Player *parent, Warehouse *wh, UIWindow **registry)
	: Building_Window(parent, wh, registry)
{
   m_parent = parent;

	// Add wares display
	m_waresdisplay = new WaresDisplay(this, 0, 0, parent->get_game(), parent->get_player());
	m_waresdisplay->add_warelist(&get_warehouse()->get_wares(), WaresDisplay::WARE);

   set_inner_size(m_waresdisplay->get_w(), 0);

   int spacing = 5;
   int nr_buttons = 4; // one more, turn page button is bigger
   int button_w = (get_inner_w() - (nr_buttons+1)*spacing) / nr_buttons;
   int posx = spacing;
   int posy = m_waresdisplay->get_h() + spacing;
   m_curpage = 0;


   UIButton* b = new UIButton(this, posx, posy, button_w, 25, 4, 100);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_help.png" ));
   b->clickedid.set(this, &Warehouse_Window::clicked);
   posx += button_w + spacing;
   b = new UIButton(this, posx, posy, button_w*2+spacing, 25, 4, 1);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/warehousewindow_switchpage.png" ));
   b->clickedid.set(this, &Warehouse_Window::clicked);
   posx += button_w*2 + 2*spacing;
   b = new UIButton(this, posx, posy, button_w, 25, 4, 101);
   b->set_pic(g_gr->get_picture( PicMod_Game,  "pics/menu_goto.png" ));
   b->clickedid.set(this, &Warehouse_Window::clicked);
   posx += button_w + spacing;
   posy += 25 + spacing;

	// Add caps buttons
   posx = 0;
   UIPanel* caps = create_capsbuttons(this);
   caps->set_pos(spacing, posy);
   if( caps->get_h() )
      posy += caps->get_h() + spacing;

   set_inner_size(get_inner_w(), posy);
}


/*
===============
Warehouse_Window::~Warehouse_Window

Deinitialize, remove from registry
===============
*/
Warehouse_Window::~Warehouse_Window()
{
}

/*
 * A button has been clicked
 */
void Warehouse_Window::clicked( int id ) {
   switch(id) {
      case 100:
      {
         // Help
         log("TODO: Implement help!\n");
         break;
      }

      case 101:
      {
         // Goto button
         m_parent->move_view_to(get_warehouse()->get_position().x, get_warehouse()->get_position().y);
         break;
      }

      case 1:
      {
         // Switch page
         switch_page();
      }

   }

}

/*
 * Switch to the next page, that is, show
 * wares -> workers -> soldier
 */
void Warehouse_Window::switch_page(void) {
   if(m_curpage == 0) {
      // Showing wares, should show workers
      m_waresdisplay->remove_all_warelists();
      m_waresdisplay->add_warelist(&get_warehouse()->get_workers(), WaresDisplay::WORKER);
      m_curpage = 1;
   } else if( m_curpage == 1) {
      // Showing workers, should show soldiers
      //TODO currently switches back to wares
      m_waresdisplay->remove_all_warelists();
      m_waresdisplay->add_warelist(&get_warehouse()->get_wares(), WaresDisplay::WARE);
      m_curpage = 0;
   }
}
/*
===============
Warehouse_Window::think

Push the current wares status to the WaresDisplay.
===============
*/
void Warehouse_Window::think()
{
}


/*
===============
Warehouse::create_options_window

Create the warehouse information window
===============
*/
UIWindow *Warehouse::create_options_window(Interactive_Player *plr, UIWindow **registry)
{
	return new Warehouse_Window(plr, this, registry);
}


/*
==============================================================================

ProductionSite UI IMPLEMENTATION

==============================================================================
*/

/*
 * ProductionSite_Window_ListWorkerWindow
 */
class ProductionSite_Window_ListWorkerWindow : public UIWindow{
   public:
      ProductionSite_Window_ListWorkerWindow(Interactive_Player*, ProductionSite* ps);
      virtual ~ProductionSite_Window_ListWorkerWindow();

      virtual void think();

   private:
      void update(void);
      void fill_list(void);

      Coords          m_ps_location;
      ProductionSite* m_ps;
      Interactive_Player* m_parent;
      UIListselect* m_ls;
      UITextarea* m_type, *m_experience, *m_becomes;
};

/*
 * Constructor
 */
ProductionSite_Window_ListWorkerWindow::ProductionSite_Window_ListWorkerWindow(Interactive_Player* parent, ProductionSite* ps)
   : UIWindow(parent, 0, 0, 320, 125, _("Worker Listing")) {
   m_ps=ps;
   m_ps_location=ps->get_position();
   m_parent=parent;

   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, _("Worker Listing"), Align_Left);
   tt->set_pos((get_inner_w()-tt->get_w())/2, 5);

   int spacing=5;
   int offsx=spacing;
   int offsy=30;
   int posx=offsx;
   int posy=offsy;

   // listselect
   m_ls=new UIListselect(this, posx, posy, get_inner_w()/2-spacing, get_inner_h()-spacing-offsy);

   // the descriptive areas
   // Type
   posx=get_inner_w()/2+spacing;
   new UITextarea(this, posx, posy, 150, 20, _("Type: "), Align_CenterLeft);
   m_type=new UITextarea(this, posx+80, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Experience
   new UITextarea(this, posx, posy, 150, 20, _("Experience: "), Align_CenterLeft);
   m_experience=new UITextarea(this, posx+80, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // is working to become
   new UITextarea(this, posx, posy, 70, 20, _("Trying to become: "), Align_CenterLeft);
   posy+=20;
   m_becomes=new UITextarea(this, posx+25, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   center_to_parent();
   move_to_top();
}

/*
 * desctructor
 */
ProductionSite_Window_ListWorkerWindow::~ProductionSite_Window_ListWorkerWindow(void) {
}

/*
 * think()
 */
void ProductionSite_Window_ListWorkerWindow::think(void) {
   BaseImmovable* imm=m_parent->get_map()->get_field(m_ps_location)->get_immovable();
   if(imm->get_type()!=Map_Object::BUILDING
         || static_cast<Building*>(imm)->has_attribute(Map_Object::CONSTRUCTIONSITE) ) {
      // The Productionsite has been removed. Die quickly.
      die();
      return;
   }

   fill_list();
   UIWindow::think();
}

/*
 * fill list()
 */
void ProductionSite_Window_ListWorkerWindow::fill_list(void) {
   int m_last_select=m_ls->get_selection_index();
   if(m_last_select==-1) m_last_select=0;
   m_ls->clear();
   std::vector<Worker*>* workers=m_ps->get_workers();

   uint i;
   for(i=0; i<workers->size(); i++) {
      Worker* worker=(*workers)[i];
      m_ls->add_entry(worker->get_descname().c_str(), worker, false,
		                worker->get_menu_pic());
   }
   if(m_ls->get_nr_entries()>m_last_select)
      m_ls->select(m_last_select);
   else if(m_ls->get_nr_entries())
      m_ls->select(m_ls->get_nr_entries()-1);

   update();
}

/*
 * update()
 */
void ProductionSite_Window_ListWorkerWindow::update(void) {
   char buffer[250];

   Worker* worker=static_cast<Worker*>(m_ls->get_selection());
   if(worker) {

      sprintf(buffer, "%s", worker->get_descname().c_str());
      m_type->set_text(buffer);

      if(worker->get_current_experience()!=-1 && worker->get_needed_experience()!=-1) {
         sprintf(buffer, "%i/%i", worker->get_current_experience(), worker->get_needed_experience());
         m_experience->set_text(buffer);
	 sprintf(buffer, "%s", i18n::translate(worker->get_becomes())); //don't use _() ! Would tag "worker->get_becomes" for translation !
         m_becomes->set_text(buffer);
      } else {
         m_experience->set_text("");
         m_becomes->set_text("");
      }
   } else {
      m_experience->set_text("");
      m_becomes->set_text("");
      m_type->set_text("");
   }
}

class ProductionSite_Window : public Building_Window {
public:
	ProductionSite_Window(Interactive_Player* parent, ProductionSite* ps, UIWindow** registry);
	virtual ~ProductionSite_Window();

	inline ProductionSite* get_productionsite() { return (ProductionSite*)get_building(); }

	virtual void think();

private:
   Interactive_Player* m_parent;
   UIWindow** m_reg;
	UIButton* m_list_worker;
public:
   void list_worker_clicked(void);
	UIButton* get_list_button() { return m_list_worker; }
protected:
   UIBox* create_production_box(UIPanel* ptr, ProductionSite* ps);
};


/*
===============
ProductionSite_Window::ProductionSite_Window

Create the window and its panels, add it to the registry.
===============
*/
ProductionSite_Window::ProductionSite_Window(Interactive_Player* parent, ProductionSite* ps, UIWindow** registry)
	: Building_Window(parent, ps, registry)
{
   m_parent=parent;
   m_reg=registry;

   UIBox* prod_box = 0;
   if (ps->get_building_type() == Building::PRODUCTIONSITE)
   {
       prod_box = create_production_box (this, ps);
      fit_inner(prod_box);
   }
}

UIBox*
ProductionSite_Window::create_production_box (UIPanel* parent, ProductionSite* ps)
{
   UIBox* box = new UIBox (parent, 0, 0, UIBox::Vertical);

   // Add the wares queue
   std::vector<WaresQueue*>* warequeues=ps->get_warequeues();
   for(uint i = 0; i < warequeues->size(); i++)
   {
      WaresQueueDisplay* wqd = new WaresQueueDisplay(box, 0, 0, get_w(),
            (*warequeues)[i], m_parent->get_game());

      box->add(wqd, UIBox::AlignLeft);
   }

   box->add_space(8);

      // Add caps buttons
   box->add(create_capsbuttons(box), UIBox::AlignCenter);

      // Add list worker button
   m_list_worker=new UIButton(box, 0,0,32,32,4,100);
   m_list_worker->set_pic(g_gr->get_picture( PicMod_Game,  pic_list_worker ));
   m_list_worker->clicked.set(this, &ProductionSite_Window::list_worker_clicked);
   box->add(m_list_worker, UIBox::AlignLeft);

   return box;
}

/*
===============
ProductionSite_Window::~ProductionSite_Window

Deinitialize, remove from registry
===============
*/
ProductionSite_Window::~ProductionSite_Window()
{
}

/*
 * List worker button has been clicked
 */
void ProductionSite_Window::list_worker_clicked(void) {
   assert(*m_reg==this);

   *m_reg=new ProductionSite_Window_ListWorkerWindow(m_parent, get_productionsite());
   die();
}

/*
===============
ProductionSite_Window::think

Make sure the window is redrawn when necessary.
===============
*/
void ProductionSite_Window::think()
{
	Building_Window::think();
}


/*
===============
ProductionSite::create_options_window

Create the production site information window.
===============
*/
UIWindow* ProductionSite::create_options_window(Interactive_Player* plr, UIWindow** registry)
{
	return new ProductionSite_Window(plr, this, registry);
}


/*
==============================================================================

MilitarySite UI IMPLEMENTATION

==============================================================================
*/

class MilitarySite_Window : public Building_Window {
public:
	MilitarySite_Window(Interactive_Player* parent, MilitarySite* ps, UIWindow** registry);
	virtual ~MilitarySite_Window();

	inline MilitarySite* get_militarysite() { return (MilitarySite*)get_building(); }

	virtual void think();
private:
   void update();
	void drop_button_clicked ();
	void soldier_capacity_up () { act_change_soldier_capacity (1); }
	void soldier_capacity_down() { act_change_soldier_capacity(-1); }

   Coords          m_ms_location;
   Interactive_Player* m_parent;
   UIWindow** m_reg;
   UITable* m_table;
	UITextarea		*m_capacity;
};


/*
===============
MilitarySite_Window::MilitarySite_Window

Create the window and its panels, add it to the registry.
===============
*/
MilitarySite_Window::MilitarySite_Window(Interactive_Player* parent, MilitarySite* ps, UIWindow** registry)
	: Building_Window(parent, ps, registry)
{
   m_parent=parent;
   m_reg=registry;
   m_ms_location=ps->get_position();

	UIBox* box = new UIBox(this, 0, 0, UIBox::Vertical);

   // Soldiers view
	m_table=new UITable(box, 0, 0, 360, 200, Align_Left, UITable::UP);
	m_table->add_column(_("Name"), UITable::STRING, 100);
	m_table->add_column(_("HP"), UITable::STRING, 40);
	m_table->add_column(_("AT"), UITable::STRING, 40);
	m_table->add_column(_("DE"), UITable::STRING, 40);
	m_table->add_column(_("EV"), UITable::STRING, 40);
	m_table->add_column(_("Level"), UITable::STRING, 100); // enough space for scrollbar

   box->add(m_table, UIBox::AlignCenter);

	// Add drop soldier button
	UIButton* b = new UIButton (box, 0, 0, 360, 32, 4, 100);
	b->set_pic(g_gr->get_picture( PicMod_Game,  pic_drop_soldier ));
	b->clicked.set(this, &MilitarySite_Window::drop_button_clicked);
	box->add_space(8);
	box->add (b, UIBox::AlignLeft);
	box->add_space(8);

	UIPanel* pan = new UIPanel(box, 0, 34, Width+100, 34);

	// Add the caps button
	create_capsbuttons(pan);

		new UITextarea (pan, 70, 11, _("Capacity"), Align_Left);
	// Capacity buttons
	b = new UIButton (pan, 140, 4, 24, 24, 4, 2);
	b->set_pic(g_gr->get_picture( PicMod_Game,  pic_down_train ));
	b->clicked.set(this, &MilitarySite_Window::soldier_capacity_down);
	b = 0;
	b = new UIButton (pan, 188, 4, 24, 24, 4, 3);
	b->set_pic(g_gr->get_picture( PicMod_Game,  pic_up_train ));
	b->clicked.set(this, &MilitarySite_Window::soldier_capacity_up);
	b = 0;
	m_capacity =new UITextarea (pan, 170, 11, "XX", Align_Center);

	box->add(pan, UIBox::AlignLeft);


   fit_inner(box);
}


/*
===============
MilitarySite_Window::~MilitarySite_Window

Deinitialize, remove from registry
===============
*/
MilitarySite_Window::~MilitarySite_Window()
{
}


/*
===============
MilitarySite_Window::think

Make sure the window is redrawn when necessary.
===============
*/
void MilitarySite_Window::think()
{
	Building_Window::think();

   BaseImmovable* imm=m_parent->get_map()->get_field(m_ms_location)->get_immovable();
   if(imm->get_type()!=Map_Object::BUILDING
         || static_cast<Building*>(imm)->has_attribute(Map_Object::CONSTRUCTIONSITE) ) {
      // The Site has been removed. Die quickly.
      die();
      return;
   }
   update();
}

/*
==============
MilitarySite_Window::update()

Update the listselect, maybe there are new
soldiers
=============
*/
void MilitarySite_Window::update(void) {
   std::vector<Soldier*>* soldiers=get_militarysite()->get_soldiers();

   uint i;
   int sel;
   char buf[200];
	if (soldiers->size() < (uint) m_table->get_nr_entries())
		for (i = 0; (int) i < m_table->get_nr_entries(); i++)
			m_table->remove_entry(0);

   for(i=0; i<soldiers->size(); i++) {
      Soldier* s=((*soldiers)[i]);
      UITable_Entry* e=0;
      for(sel=0; sel<m_table->get_nr_entries(); sel++) {
         if(m_table->get_entry(sel)->get_user_data()==s) {
            // Soldier already in list
            e=m_table->get_entry(sel);
            break;
         }
      }
      if(!e) // add new
         e= new UITable_Entry(m_table, s);

		e->set_string(0, s->get_descname().c_str());
      sprintf(buf, "%i / %i", s->get_hp_level(), s->get_max_hp_level());
      e->set_string(1, buf);
      sprintf(buf, "%i / %i", s->get_attack_level(), s->get_max_attack_level());
      e->set_string(2, buf);
      sprintf(buf, "%i / %i", s->get_defense_level(), s->get_max_defense_level());
      e->set_string(3, buf);
      sprintf(buf, "%i / %i", s->get_evade_level(), s->get_max_evade_level());
      e->set_string(4, buf);
      sprintf(buf, "%i / %i", s->get_evade_level()+s->get_attack_level()+ s->get_defense_level()+s->get_hp_level(),
            s->get_max_evade_level()+s->get_max_defense_level()+s->get_max_attack_level()+s->get_max_hp_level());
      e->set_string(5, buf);
   }
   m_table->sort();

	std::string str;
	sprintf (buf, "%2d", ((MilitarySite *)get_building())->get_capacity());
	str = (const char *) buf;
	m_capacity->set_text (str);
}

void MilitarySite_Window::drop_button_clicked() {
	assert(*m_reg== this);
	 std::vector<Soldier*>* soldiers=get_militarysite()->get_soldiers();
	 Soldier* s = 0;

	int sel = m_table->get_selection_index();
	int i;

	if ((sel < 0) || (sel >= (int)soldiers->size()))
		return;
	 for (i = 0; i < (int) soldiers->size(); i++) {
		  s=((*soldiers)[i]);
		  if (m_table->get_entry(sel)->get_user_data() == s)
				break;
	 }
	 assert(i < (int) soldiers->size());
	 act_drop_soldier (s->get_serial());
}

/*
===============
MilitarySite::create_options_window

Create the production site information window.
===============
*/
UIWindow* MilitarySite::create_options_window(Interactive_Player* plr, UIWindow** registry)
{
	return new MilitarySite_Window(plr, this, registry);
}

/*
====================
	TrainingSite_Options_Window Implementation
====================
*/
class TrainingSite_Options_Window : public UIWindow {
public:
	TrainingSite_Options_Window(Interactive_Player* parent, TrainingSite* ps);
	virtual ~TrainingSite_Options_Window();

	inline TrainingSite* get_trainingsite() { return m_trainingsite; }

	void think();
private:
	void heros_clicked ()			{ get_trainingsite()->switch_heros(); }
	void up_hp_clicked ()			{ act_change_priority(atrHP, 1); }
	void up_attack_clicked()		{ act_change_priority(atrAttack, 1); }
	void up_defense_clicked()		{ act_change_priority(atrDefense, 1); }
	void up_evade_clicked()			{ act_change_priority(atrEvade, 1); }
	void down_hp_clicked ()			{ act_change_priority(atrHP, -1); }
	void down_attack_clicked()		{ act_change_priority(atrAttack, -1); }
	void down_defense_clicked()		{ act_change_priority(atrDefense, -1); }
	void down_evade_clicked()		{ act_change_priority(atrEvade, -1); }

	void act_change_priority (int atr, int how);

	void update();

	Coords			m_ms_location;
	Interactive_Player* m_parent;
	UIWindow** 		m_reg;
	UITextarea		*m_style_train,
					*m_hp_pri,
					*m_attack_pri,
					*m_defense_pri,
					*m_evade_pri;
	TrainingSite	*m_trainingsite;
};

TrainingSite_Options_Window::TrainingSite_Options_Window(Interactive_Player* parent, TrainingSite* ps)
		: UIWindow(parent, 0, 0, 320, 125, _("Training Options")) {

	int _bs = 22;
	int _cn = 20;
	int _cb = 100;

	m_parent=parent;
	m_trainingsite = ps;
	m_ms_location=ps->get_position();
	m_hp_pri = 0;
	m_attack_pri = 0;
	m_defense_pri = 0;
	m_evade_pri = 0;

	set_inner_size(250, _bs*9);

	// TODO: Put the capacity buttons here.

	UIButton *btn;

	// Add switch training mode button
	btn = new UIButton (this, _cb, _bs, 105, _bs, 4, 1);
	btn->clicked.set(this, &TrainingSite_Options_Window::heros_clicked);
	btn = 0;

	new UITextarea(this, _cn - 15, _bs + 2, _("Training mode : "), Align_Left);
	m_style_train = new UITextarea (this, _cb + 4, _bs+2, _("Balanced"), Align_Left);


	m_hp_pri			= new UITextarea (this, _cb+3*_bs/2, 3+(3+_bs)*2, "XX", Align_Center);
	m_attack_pri	= new UITextarea (this, _cb+3*_bs/2, 3+(3+_bs)*3, "XX", Align_Center);
	m_defense_pri	= new UITextarea (this, _cb+3*_bs/2, 3+(3+_bs)*4, "XX", Align_Center);
	m_evade_pri		= new UITextarea (this, _cb+3*_bs/2, 3+(3+_bs)*5, "XX", Align_Center);

	m_hp_pri->set_visible(false);
	m_attack_pri->set_visible(false);
	m_defense_pri->set_visible(false);
	m_evade_pri->set_visible(false);

	// Add priority buttons for every attribute
	if (ps->get__descr()->get_train_hp()) {
		// HP buttons
		btn = new UIButton (this,  _cb, 2*(_bs+2), _bs, _bs, 4, 2);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_down_train ));
		btn->clicked.set(this, &TrainingSite_Options_Window::down_hp_clicked);
		btn = 0;
		btn = new UIButton (this, _cb+2*_bs, 2*(_bs+2), _bs, _bs, 4, 3);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_up_train ));
		btn->clicked.set(this, &TrainingSite_Options_Window::up_hp_clicked);
		btn = 0;
		new UITextarea (this, _cn, (3+_bs)*2, _("Hit Points"), Align_Left);
		m_hp_pri->set_visible(true);
	}
	if (ps->get__descr()->get_train_attack()) {
		// Attack buttons
		btn = new UIButton (this, _cb, 3*(_bs+2), _bs, _bs, 4, 2);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_down_train ));
		btn->clicked.set(this, &TrainingSite_Options_Window::down_attack_clicked);
		btn = 0;
		btn = new UIButton (this, _cb+2*_bs, 3*(_bs+2), _bs, _bs, 4, 3);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_up_train ));
		btn->clicked.set(this, &TrainingSite_Options_Window::up_attack_clicked);
		btn = 0;
		new UITextarea (this, _cn, (3+_bs)*3, _("Attack"), Align_Left);
		m_attack_pri->set_visible(true);
	}
	if (ps->get__descr()->get_train_defense()) {
		// Defense buttons
		btn = new UIButton (this, _cb, 4*(_bs+2), _bs, _bs, 4, 2);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_down_train ));
		btn->clicked.set(this,&TrainingSite_Options_Window::down_defense_clicked);
		btn = 0;
		btn = new UIButton (this, _cb+2*_bs, 4*(_bs+2), _bs, _bs, 4, 3);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_up_train ));
		btn->clicked.set(this,&TrainingSite_Options_Window::up_defense_clicked);
		btn = 0;
		new UITextarea (this, _cn, (3+_bs)*4, _("Defense"), Align_Left);
		m_defense_pri->set_visible(true);
	}
	if (ps->get__descr()->get_train_evade()) {
		// Evade buttons
		btn = new UIButton (this, _cb, 5*(_bs+2), _bs, _bs, 4, 2);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_down_train ));
		btn->clicked.set(this, &TrainingSite_Options_Window::down_evade_clicked);
		btn = 0;
		btn = new UIButton (this, _cb+2*_bs, 5*(_bs+2), _bs, _bs, 4, 3);
		btn->set_pic(g_gr->get_picture( PicMod_Game,  pic_up_train ));
		btn->clicked.set(this, &TrainingSite_Options_Window::up_evade_clicked);
		btn = 0;
		new UITextarea (this, _cn, (3+_bs)*5, _("Evade"), Align_Left);
		m_evade_pri->set_visible(true);
	}

	center_to_parent();
	move_to_top();
}

TrainingSite_Options_Window::~TrainingSite_Options_Window()
{
}

void TrainingSite_Options_Window::act_change_priority (int atr, int val) {

	 if ((atr == atrHP) || (atr == atrAttack) || (atr == atrDefense) || (atr == atrEvade))
		  if (m_trainingsite)
				m_parent->get_game()->send_player_change_training_options((Building*)m_trainingsite, atr, val);
}

void TrainingSite_Options_Window::think()
{
//	Building_Window::think();

	BaseImmovable* imm=m_parent->get_map()->get_field(m_ms_location)->get_immovable();
	if(imm->get_type()!=Map_Object::BUILDING
			|| static_cast<Building*>(imm)->has_attribute(Map_Object::CONSTRUCTIONSITE) ) {
		// The Site has been removed. Die quickly.
		die();
		return;
	}
	update();
}

void TrainingSite_Options_Window::update() {
	char buf[200];
	std::string str;

	TrainingSite *ts = get_trainingsite();

	if (ts->get_build_heros())
		m_style_train->set_text(_("Make heroes"));
	else
		m_style_train->set_text(_("Balanced army"));

	sprintf (buf, "%2d", ts->get_pri(atrHP));
	str = (const char *) buf;
	m_hp_pri->set_text (str);

	sprintf (buf, "%2d", ts->get_pri(atrAttack));
	str = (const char *) buf;
	m_attack_pri->set_text (str);

	sprintf (buf, "%2d", ts->get_pri(atrDefense));
	str = (const char *) buf;
	m_defense_pri->set_text (str);

	sprintf (buf, "%2d", ts->get_pri(atrEvade));
	str = (const char *) buf;
	m_evade_pri->set_text (str);

}

/*
==============================================================================

TrainingSite UI IMPLEMENTATION

==============================================================================
*/

class TrainingSite_Window : public ProductionSite_Window {
public:
	TrainingSite_Window(Interactive_Player* parent, TrainingSite* ps, UIWindow** registry);
	virtual ~TrainingSite_Window();

	inline TrainingSite* get_trainingsite() { return (TrainingSite*)get_building(); }

	virtual void think();
	void options_button_clicked ();
	void drop_button_clicked ();
	void soldier_capacity_up () { act_change_soldier_capacity (1); }
	void soldier_capacity_down() { act_change_soldier_capacity(-1); }
   UIBox* create_military_box (UIPanel*);
private:
	void update();

   void add_tab(const char* picname, UIPanel* panel);
//   void add_button(UIBox* box, const char* picname, void (FieldActionWindow::*fn)());

   Coords              m_ms_location;
   Interactive_Player* m_parent;
   UIWindow**          m_reg;
   UITable*            m_table;
   UIButton*           m_drop_button;
   UITextarea*         m_capacity;

   UITab_Panel*         m_tabpanel;

};


/*
===============
TrainingSite_Window::TrainingSite_Window

Create the window and its panels, add it to the registry.
===============
*/
TrainingSite_Window::TrainingSite_Window(Interactive_Player* parent, TrainingSite* ms, UIWindow** registry)
	: ProductionSite_Window(parent, ms, registry)
{
   m_parent = parent;
   m_reg = registry;
   m_ms_location = ms->get_position ();

   m_tabpanel = new UITab_Panel(this, 0, 0, 1);
   m_tabpanel->set_snapparent(true);

      // Training Box (wares and buttons related to they)
   UIBox* prod_box = create_production_box (m_tabpanel, ms);
   prod_box->resize();
   add_tab(pic_tab_training, prod_box);

      // Military Box (Soldiers and buttons related to they)
      // Training Box (wares and buttons related to they)
   UIBox* train_box = create_military_box (m_tabpanel);
   train_box->resize();
   add_tab(pic_tab_military, train_box);

   m_tabpanel->resize();
   fit_inner (m_tabpanel);
}


/*
===============
TrainingSite_Window::~TrainingSite_Window

Deinitialize, remove from registry
===============
*/
TrainingSite_Window::~TrainingSite_Window()
{
}

UIBox* TrainingSite_Window::create_military_box (UIPanel* panel)
{
   UIBox* sold_box = new UIBox (panel, 0, 0, UIBox::Vertical);

      // Soldiers view
   m_table=new UITable(sold_box, 0, 0, 360, 200, Align_Left, UITable::UP);
   m_table->add_column(_("Name"), UITable::STRING, 100);
   m_table->add_column(_("HP"), UITable::STRING, 40);
   m_table->add_column(_("AT"), UITable::STRING, 40);
   m_table->add_column(_("DE"), UITable::STRING, 40);
   m_table->add_column(_("EV"), UITable::STRING, 40);
   m_table->add_column(_("Level"), UITable::STRING, 100); // enough space for scrollbar
   sold_box->add (m_table, Align_Left);

      // Add drop soldier button
   UIButton* b = new UIButton (sold_box, 0, 0, 360, 32, 4, 100);
   b->set_pic (g_gr->get_picture( PicMod_Game,  pic_drop_soldier ));
   b->clicked.set (this, &TrainingSite_Window::drop_button_clicked);
   sold_box->add (b, Align_Left);

      // Add TrainingSite Options and Capacity  buttons
   UIBox* box = new UIBox (sold_box, 0, 0, UIBox::Horizontal);
   b = new UIButton(box, 32, 0, 32,32, 4,100);
   b->set_pic(g_gr->get_picture( PicMod_Game,  pic_train_options ));
   b->clicked.set(this, &TrainingSite_Window::options_button_clicked);
   box->add (b, Align_Top);

   box->add (new UITextarea (box, 0, 11, _("Capacity"), Align_Left), Align_Left);
      // Capacity buttons
   b = new UIButton (box, 70, 4, 24, 24, 4, 2);
   b->set_pic (g_gr->get_picture( PicMod_Game,  pic_down_train ));
   b->clicked.set (this, &TrainingSite_Window::soldier_capacity_down);
   box->add (b, Align_Top);
   b = 0;

   m_capacity = new UITextarea (box, 0, 11, _("xx"), Align_Center);
   box->add (m_capacity, Align_Top);

   b = new UIButton (box, 118, 4, 24, 24, 4, 3);
   b->set_pic (g_gr->get_picture( PicMod_Game,  pic_up_train ));
   b->clicked.set (this, &TrainingSite_Window::soldier_capacity_up);
   box->add (b, Align_Top);
   b = 0;
   sold_box->add (box, Align_Left);

   return sold_box;
}

/*
===============
TrainingSite_Window::add_tab

Convenience function: Adds a new tab to the main tab panel
===============
*/
void TrainingSite_Window::add_tab(const char* picname, UIPanel* panel)
{
	m_tabpanel->add(g_gr->get_picture(   PicMod_Game,   picname ), panel );
}


/*
===============
TrainingSite_Window::think

Make sure the window is redrawn when necessary.
===============
*/
void TrainingSite_Window::think()
{
	Building_Window::think();

	BaseImmovable* imm=m_parent->get_map()->get_field(m_ms_location)->get_immovable();
	if(imm->get_type()!=Map_Object::BUILDING
		|| static_cast<Building*>(imm)->has_attribute(Map_Object::CONSTRUCTIONSITE) ) {
		// The Site has been removed. Die quickly.
		die();
		return;
	}
	update();
}

/*
==============
TrainingSite_Window::update()

Update the listselect, maybe there are new soldiers
=============
*/
void TrainingSite_Window::update(void) {
	std::vector<Soldier*>* soldiers=get_trainingsite()->get_soldiers();

	uint i;
	int sel;
	char buf[200];
	if (soldiers->size() != (uint) m_table->get_nr_entries())
		for (i = 0; (int) i < m_table->get_nr_entries(); i++)
			m_table->remove_entry(0);

	for(i=0; i<soldiers->size(); i++) {
		Soldier* s=((*soldiers)[i]);
		UITable_Entry* e=0;
		for(sel=0; sel<m_table->get_nr_entries(); sel++) {
			if(m_table->get_entry(sel)->get_user_data()==s) {
				// Soldier already in list
				e=m_table->get_entry(sel);
				break;
			}
		}
		if(!e) // add new
			e= new UITable_Entry(m_table, s);
		int hl	= s->get_hp_level(),
			mhl	= s->get_max_hp_level(),
			al	= s->get_attack_level(),
			mal	= s->get_max_attack_level(),
			dl	= s->get_defense_level(),
			mdl	= s->get_max_defense_level(),
			el	= s->get_evade_level(),
			mel	= s->get_max_evade_level();

		e->set_string(0, s->get_name().c_str());
		sprintf(buf, "%i / %i", hl, mhl);
		e->set_string(1, buf);
		sprintf(buf, "%i / %i", al, mal);
		e->set_string(2, buf);
		sprintf(buf, "%i / %i", dl, mdl);
		e->set_string(3, buf);
		sprintf(buf, "%i / %i", el, mel);
		e->set_string(4, buf);
		sprintf(buf, "%i / %i", hl + al + dl + el, mhl + mel + mal + mdl);
		e->set_string(5, buf);
	}
	m_table->sort();

	std::string str;
	sprintf (buf, "%2d", ((TrainingSite *)get_building())->get_capacity());
	str = (const char *) buf;
	m_capacity->set_text (str);
}

/*
==============
TrainingSite_Window::options_button_clicked()

Handle the click at options buttons. Launch a train specific options window.
=============
*/
void TrainingSite_Window::options_button_clicked () {
	assert(*m_reg==this);

	*m_reg=new TrainingSite_Options_Window(m_parent, get_trainingsite());
	die();

}

/*
==============
TrainingSite_Window::drop_button_clicked()

Handle the click at drop soldier. Enqueue a command at command queue to get out selected
soldier from this training site.
=============
*/
void TrainingSite_Window::drop_button_clicked() {
	 assert(*m_reg== this);
	 std::vector<Soldier*>* soldiers=get_trainingsite()->get_soldiers();

	 int sel = m_table->get_selection_index();
	 int i;

	 if ((sel < 0) || (sel >= (int)soldiers->size()))
		  return;

	 for (i = 0; i < (int) soldiers->size(); i++) {
		  Soldier* s=((*soldiers)[i]);
		  if (m_table->get_entry(sel)->get_user_data() == s)
			  break;
	 }

	 assert(i < (int) soldiers->size());
	 act_drop_soldier ((*soldiers)[i]->get_serial());
}

/*
===============
TrainingSite::create_options_window

Create the training site information window.
===============
*/
UIWindow* TrainingSite::create_options_window(Interactive_Player* plr, UIWindow** registry)
{
	return new TrainingSite_Window(plr, this, registry);
}
