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

#include "IntPlayer.h"
#include "building.h"
#include "cmd_queue.h"
#include "constructionsite.h"
#include "error.h"
#include "font_handler.h"
#include "game_debug_ui.h"
#include "militarysite.h"
#include "productionsite.h"
#include "rendertarget.h"
#include "transport.h"
#include "ui_box.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_progressbar.h"
#include "ui_table.h"
#include "ui_textarea.h"
#include "ui_window.h"
#include "warehouse.h"
#include "waresdisplay.h"
#include "wexception.h"
#include "worker.h"
#include "player.h"
#include "soldier.h"
#include "tribe.h"

static const char* pic_ok = "pics/menu_okay.png";
static const char* pic_cancel = "pics/menu_abort.png";
static const char* pic_debug = "pics/menu_debug.png";

static const char* pic_bulldoze = "pics/menu_bld_bulldoze.png";
static const char* pic_queue_background = "pics/queue_background.png";

static const char* pic_list_worker = "pics/menu_list_workers.png";

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
	: UIWindow(parent, 0, 0, 160, 90, "Destroy building?")
{
	UIButton* btn;
	std::string text;

	m_iabase = parent;
	m_building = building;

	if (!todestroy)
		m_todestroy = building;
	else
		m_todestroy = todestroy;

	text = "Do you really want to destroy this ";
	text += building->get_name();
	text += "?";
	new UITextarea(this, 0, 0, 160, 44, text, Align_Center, true);

	btn = new UIButton(this, 6, 50, 60, 34, 2);
	btn->clicked.set(this, &BulldozeConfirm::bulldoze);
	btn->set_pic(g_gr->get_picture(PicMod_Game, pic_ok, true));

	btn = new UIButton(this, 94, 50, 60, 34, 2);
	btn->clicked.set(this, &BulldozeConfirm::die);
	btn->set_pic(g_gr->get_picture(PicMod_Game, pic_cancel, true));

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
         Interactive_Player* player=static_cast<Interactive_Player*>(m_iabase);
	 g->send_player_bulldoze (todestroy);
      } else {
         // Editor
         Player* plr=todestroy->get_owner();
         plr->bulldoze(todestroy);
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
	Ware_Descr* descr;
	Item_Ware_Descr* waredescr;

	m_queue = queue;
	m_max_width = maxw;

	descr = g->get_ware_description(m_queue->get_ware());
	if (descr->is_worker())
		throw wexception("WaresQueueDisplay: not implemented for workers");

	waredescr = (Item_Ware_Descr*)descr;
	m_pic_empty = waredescr->get_pic_queue_empty();
	m_pic_full = waredescr->get_pic_queue_full();

	m_cache_size = m_queue->get_size();
	m_cache_filled = m_queue->get_filled();
	m_display_size = 0;

	m_pic_background = g_gr->get_picture(PicMod_Game, pic_queue_background, false);

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

private:
	void setup_capsbuttons();

	void act_bulldoze();
	void act_debug();
	void act_start_stop();
   void act_enhance(int);

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
	: UIWindow(parent, 0, 0, Width, 0, building->get_name())
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
		UIButton* btn = new UIButton(m_capsbuttons, x, 0, 34, 34, 2);
		btn->clicked.set(this, &Building_Window::act_start_stop);
		btn->set_pic(g_gr->get_picture(PicMod_Game, icon.c_str(), true));
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

         UIButton* btn = new UIButton(m_capsbuttons, x, 0, 34, 34, 2, id); // Button id == building id
         btn->clickedid.set(this, &Building_Window::act_enhance);
         btn->set_pic(m_player->get_player()->get_tribe()->get_building_descr(id)->get_buildicon());
         x += 34;
      }
   }

	if (m_capscache & (1 << Building::PCap_Bulldoze)) {
		UIButton* btn = new UIButton(m_capsbuttons, x, 0, 34, 34, 2);
		btn->clicked.set(this, &Building_Window::act_bulldoze);
		btn->set_pic(g_gr->get_picture(PicMod_Game, pic_bulldoze, true));
		x += 34;
	}

	if (m_player->get_display_flag(Interactive_Base::dfDebug)) {
		UIButton* btn = new UIButton(m_capsbuttons, x, 0, 34, 34, 2);
		btn->clicked.set(this, &Building_Window::act_debug);
		btn->set_pic(g_gr->get_picture(PicMod_Game, pic_debug,true));
		x += 34;
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
	WaresDisplay*			m_waresdisplay;
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
	UIBox* box = new UIBox(this, 0, 0, UIBox::Vertical);

	// Add wares display
	m_waresdisplay = new WaresDisplay(box, 0, 0, parent->get_game(), parent->get_player());
	box->add(m_waresdisplay, UIBox::AlignCenter);

	// Add caps buttons
	box->add(create_capsbuttons(box), UIBox::AlignCenter);

	fit_inner(box);
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
===============
Warehouse_Window::think

Push the current wares status to the WaresDisplay.
===============
*/
void Warehouse_Window::think()
{
	m_waresdisplay->set_wares(get_warehouse()->get_wares());
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
   : UIWindow(parent, 0, 0, 320, 125, "Worker Listing") {
   m_ps=ps;
   m_ps_location=ps->get_position();
   m_parent=parent;
   
   // Caption
   UITextarea* tt=new UITextarea(this, 0, 0, "Worker Listing", Align_Left);
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
   new UITextarea(this, posx, posy, 150, 20, "Type: ", Align_CenterLeft);
   m_type=new UITextarea(this, posx+80, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Experience 
   new UITextarea(this, posx, posy, 150, 20, "Experience: ", Align_CenterLeft);
   m_experience=new UITextarea(this, posx+80, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // is working to become 
   new UITextarea(this, posx, posy, 70, 20, "Trying to become: ", Align_CenterLeft);
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
      m_ls->add_entry(worker->get_name().c_str(), worker, false, worker->get_menu_pic());
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

      sprintf(buffer, "%s", worker->get_name().c_str());
      m_type->set_text(buffer);

      if(worker->get_current_experience()!=-1 && worker->get_needed_experience()!=-1) {
         sprintf(buffer, "%i/%i", worker->get_current_experience(), worker->get_needed_experience());
         m_experience->set_text(buffer);
         sprintf(buffer, "%s", worker->get_becomes());
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
   
   void list_worker_clicked(void);
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

	UIBox* box = new UIBox(this, 0, 0, UIBox::Vertical);

	// Add the wares queue
   std::vector<WaresQueue*>* warequeues=ps->get_warequeues();
	for(uint i = 0; i < warequeues->size(); i++)
	{
		WaresQueueDisplay* wqd = new WaresQueueDisplay(box, 0, 0, get_w(),
					(*warequeues)[i], parent->get_game());

		box->add(wqd, UIBox::AlignLeft);
	}

	box->add_space(8);

	// Add caps buttons
	box->add(create_capsbuttons(box), UIBox::AlignCenter);

   // Add list worker button
   UIButton* b=new UIButton(box, 0,0,32,32,2,100);
   b->set_pic(g_gr->get_picture(PicMod_Game, pic_list_worker, true)); 
   b->clicked.set(this, &ProductionSite_Window::list_worker_clicked);
   box->add(b, UIBox::AlignLeft);

	fit_inner(box);
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
   
   Coords          m_ms_location;
   Interactive_Player* m_parent;
   UIWindow** m_reg;
   UITable* m_table;
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
   m_table=new UITable(box, 0, 0, 450, 200, Align_Left, UITable::UP);
   m_table->add_colum("Name", UITable::STRING, 100);
   m_table->add_colum("HP Level", UITable::STRING, 75);
   m_table->add_colum("AT Level", UITable::STRING, 75);
   m_table->add_colum("DE Level", UITable::STRING, 75);
   m_table->add_colum("EV Level", UITable::STRING, 75);
   m_table->add_colum("Level", UITable::STRING, 100); // enough space for scrollbar

   box->add(m_table, UIBox::AlignCenter);
   
	box->add_space(8);

	// Add the caps button
	box->add(create_capsbuttons(box), UIBox::AlignCenter);

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
      
      e->set_string(0, s->get_name().c_str());
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
