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

#include "widelands.h"
#include "IntPlayer.h"
#include "building.h"
#include "game.h"
#include "player.h"
#include "worker.h"
#include "fieldaction.h"
#include "transport.h"

#include "building_int.h"
#include "ui_box.h"
#include "waresdisplay.h"


static const char* pic_ok = "pics/menu_okay.png";
static const char* pic_cancel = "pics/menu_abort.png";

static const char* pic_bulldoze = "pics/menu_bld_bulldoze.png";
static const char* pic_queue_background = "pics/queue_background.png";


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
class BulldozeConfirm : public Window {
public:
	BulldozeConfirm(Interactive_Player* parent, Building* building, PlayerImmovable* todestroy = 0);
	virtual ~BulldozeConfirm();

	virtual void think();

private:
	void bulldoze();

private:
	Interactive_Player*	m_player;
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
BulldozeConfirm::BulldozeConfirm(Interactive_Player* parent, Building* building, PlayerImmovable* todestroy)
	: Window(parent, 0, 0, 160, 90, "Destroy building?")
{
	Button* btn;
	std::string text;

	m_player = parent;
	m_building = building;

	if (!todestroy)
		m_todestroy = building;
	else
		m_todestroy = todestroy;

	text = "Do you really want to destroy this ";
	text += building->get_name();
	text += "?";
	new Textarea(this, 0, 0, 160, 44, text, Align_Center, true);

	btn = new Button(this, 6, 50, 60, 34, 2);
	btn->clicked.set(this, &BulldozeConfirm::bulldoze);
	btn->set_pic(g_gr->get_picture(PicMod_Game, pic_ok, RGBColor(0,0,255)));

	btn = new Button(this, 94, 50, 60, 34, 2);
	btn->clicked.set(this, &Panel::die);
	btn->set_pic(g_gr->get_picture(PicMod_Game, pic_cancel, RGBColor(0,0,255)));

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
	Game* g = m_player->get_game();
	Building* building = (Building*)m_building.get(g);
	PlayerImmovable* todestroy = (PlayerImmovable*)m_todestroy.get(g);

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
	Game* g = m_player->get_game();
	Building* building = (Building*)m_building.get(g);
	PlayerImmovable* todestroy = (PlayerImmovable*)m_todestroy.get(g);

	if (todestroy && building && building->get_playercaps() & (1 << Building::PCap_Bulldoze))
		g->send_player_command(m_player->get_player_number(), CMD_BULLDOZE, todestroy->get_serial());

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
void show_bulldoze_confirm(Interactive_Player* player, Building* building, PlayerImmovable* todestroy)
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
class WaresQueueDisplay : public Panel {
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
	WaresQueueDisplay(Panel* parent, int x, int y, uint maxw, WaresQueue* queue, Game* g);
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
WaresQueueDisplay::WaresQueueDisplay(Panel* parent, int x, int y, uint maxw, WaresQueue* queue, Game* g)
	: Panel(parent, x, y, 0, Height)
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

	m_pic_background = g_gr->get_picture(PicMod_Game, pic_queue_background);

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
class Building_Window : public Window {
public:
	enum {
		Width = 136		// 4*34, 4 normally sized buttons
	};

public:
	Building_Window(Interactive_Player* parent, Building* building, Window** registry);
	virtual ~Building_Window();

	Interactive_Player* get_player() { return m_player; }
	Building* get_building() { return m_building; }

	virtual void draw(RenderTarget* dst);
	virtual void think();

	Panel* create_capsbuttons(Panel* parent);

private:
	void setup_capsbuttons();

	void act_bulldoze();

private:
	Window**					m_registry;
	Interactive_Player*	m_player;
	Building*				m_building;

	Panel*	m_capsbuttons;		// Panel that contains capabilities buttons
	uint		m_capscache;		// capabilities that were last used in setting up the caps panel
};


/*
===============
Building_Window::Building_Window

Create the window, add it to the registry.
===============
*/
Building_Window::Building_Window(Interactive_Player* parent, Building* building, Window** registry)
	: Window(parent, 0, 0, Width, 0, building->get_name())
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
	Window::draw(dst);
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

	Window::think();
}


/*
===============
Building_Window::create_capsbuttons

Create the capsbuttons panel with the given parent window, set it up and return
it.
===============
*/
Panel* Building_Window::create_capsbuttons(Panel* parent)
{
	if (m_capsbuttons)
		delete m_capsbuttons;

	m_capsbuttons = new Panel(parent, 0, 0, Width, 34);
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

	if (m_capscache & (1 << Building::PCap_Bulldoze)) {
		Button* btn = new Button(m_capsbuttons, x, 0, 34, 34, 2);
		btn->clicked.set(this, &Building_Window::act_bulldoze);
		btn->set_pic(g_gr->get_picture(PicMod_Game, pic_bulldoze, RGBColor(0,0,255)));
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



/*
==============================================================================

ConstructionSite UI IMPLEMENTATION

==============================================================================
*/

class ConstructionSite_Window : public Building_Window {
public:
	ConstructionSite_Window(Interactive_Player* parent, ConstructionSite* cs, Window** registry);
	virtual ~ConstructionSite_Window();

	ConstructionSite* get_constructionsize() { return (ConstructionSite*)get_building(); }

	virtual void think();

private:
};


/*
===============
ConstructionSite_Window::ConstructionSite_Window

Create the window and its panels
===============
*/
ConstructionSite_Window::ConstructionSite_Window(Interactive_Player* parent, ConstructionSite* cs,
                                                 Window** registry)
	: Building_Window(parent, cs, registry)
{
	Box* box = new Box(this, 0, 0, Box::Vertical);

	// Add the wares queue
	for(uint i = 0; i < cs->get_nrwaresqueues(); i++)
	{
		WaresQueueDisplay* wqd = new WaresQueueDisplay(box, 0, 0, get_w(),
					cs->get_waresqueue(i), parent->get_game());

		box->add(wqd, Box::AlignLeft);
	}

	box->add_space(8);

	// Add the caps button
	box->add(create_capsbuttons(box), Box::AlignCenter);

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
}


/*
===============
ConstructionSite::create_options_window

Create the status window describing the construction site.
===============
*/
Window *ConstructionSite::create_options_window(Interactive_Player *plr, Window **registry)
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
	Warehouse_Window(Interactive_Player *parent, Warehouse *wh, Window **registry);
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
Warehouse_Window::Warehouse_Window(Interactive_Player *parent, Warehouse *wh, Window **registry)
	: Building_Window(parent, wh, registry)
{
	Box* box = new Box(this, 0, 0, Box::Vertical);

	// Add wares display
	m_waresdisplay = new WaresDisplay(box, 0, 0, parent->get_game(), parent->get_player());
	box->add(m_waresdisplay, Box::AlignCenter);

	// Add caps buttons
	box->add(create_capsbuttons(box), Box::AlignCenter);

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
Window *Warehouse::create_options_window(Interactive_Player *plr, Window **registry)
{
	return new Warehouse_Window(plr, this, registry);
}


/*
==============================================================================

ProductionSite UI IMPLEMENTATION

==============================================================================
*/

class ProductionSite_Window : public Building_Window {
public:
	ProductionSite_Window(Interactive_Player* parent, ProductionSite* ps, Window** registry);
	virtual ~ProductionSite_Window();

	inline ProductionSite* get_productionsite() { return (ProductionSite*)get_building(); }

	virtual void think();
};


/*
===============
ProductionSite_Window::ProductionSite_Window

Create the window and its panels, add it to the registry.
===============
*/
ProductionSite_Window::ProductionSite_Window(Interactive_Player* parent, ProductionSite* ps, Window** registry)
	: Building_Window(parent, ps, registry)
{
	Box* box = new Box(this, 0, 0, Box::Vertical);

	// Add the wares queue
   std::vector<WaresQueue*>* warequeues=ps->get_warequeues();
	for(uint i = 0; i < warequeues->size(); i++)
	{
		WaresQueueDisplay* wqd = new WaresQueueDisplay(box, 0, 0, get_w(),
					(*warequeues)[i], parent->get_game());

		box->add(wqd, Box::AlignLeft);
	}

	box->add_space(8);

	// Add caps buttons
	box->add(create_capsbuttons(box), Box::AlignCenter);

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
Window* ProductionSite::create_options_window(Interactive_Player* plr, Window** registry)
{
	return new ProductionSite_Window(plr, this, registry);
}


/*
==============================================================================

MilitarySite UI IMPLEMENTATION

==============================================================================
*/

class MilitarySite_Window : public ProductionSite_Window {
public:
	MilitarySite_Window(Interactive_Player* parent, MilitarySite* ps, Window** registry);
	virtual ~MilitarySite_Window();

	inline MilitarySite* get_militarysite() { return (MilitarySite*)get_building(); }

	virtual void think();
};


/*
===============
MilitarySite_Window::MilitarySite_Window

Create the window and its panels, add it to the registry.
===============
*/
MilitarySite_Window::MilitarySite_Window(Interactive_Player* parent, MilitarySite* ps, Window** registry)
	: ProductionSite_Window(parent, ps, registry)
{
   // TODO
   log("TODO: military options window!. must have all functionality of productionsite + soldier options!\n");
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
}


/*
===============
MilitarySite::create_options_window

Create the production site information window.
===============
*/
Window* MilitarySite::create_options_window(Interactive_Player* plr, Window** registry)
{
	return new MilitarySite_Window(plr, this, registry);
}
