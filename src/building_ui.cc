/*
 * Copyright (C) 2002-2003 by Widelands Development Team
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

#include "building_int.h"
#include "ui_box.h"
#include "waresdisplay.h"


static const char* pic_ok = "pics/menu_okay.bmp";
static const char* pic_cancel = "pics/menu_abort.bmp";

static const char* pic_bulldoze = "pics/menu_bld_bulldoze.bmp";


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

	// Add the caps buttons
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

class ProductionSite_Window : public Window {
public:
	ProductionSite_Window(Interactive_Player* parent, ProductionSite* ps, Window** registry);
	virtual ~ProductionSite_Window();

	virtual void think();

private:
	Window**					m_registry;		// pointer to this window, cleared in destructor
	Interactive_Player*	m_player;		// player the production site belongs to
	ProductionSite*		m_productionsite;	// production site this window has been opened for
};


/*
===============
ProductionSite_Window::ProductionSite_Window

Create the window and its panels, add it to the registry.
===============
*/
ProductionSite_Window::ProductionSite_Window(Interactive_Player* parent, ProductionSite* ps, Window** registry)
	: Window(parent, 0, 0, 136, 260, ps->get_name())
{
	m_registry = registry;
	if (*m_registry)
		delete *m_registry;
	*m_registry = this;

	m_player = parent;
	m_productionsite = ps;

	move_to_mouse();

	set_think(true);
}

/*
===============
ProductionSite_Window::~ProductionSite_Window

Deinitialize, remove from registry
===============
*/
ProductionSite_Window::~ProductionSite_Window()
{
	*m_registry = 0;
}


/*
===============
ProductionSite_Window::think

Make sure the window is redrawn when necessary.
===============
*/
void ProductionSite_Window::think()
{
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
