/*
 * Copyright (C) 2002 by Widelands Development Team
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

Warehouse UI IMPLEMENTATION

==============================================================================
*/

class Warehouse_Window : public Window {
public:
	Warehouse_Window(Interactive_Player *parent, Warehouse *wh, Window **registry);
	virtual ~Warehouse_Window();

	virtual void draw(Bitmap *dst, int ofsx, int ofsy);
	virtual void think();

private:
	Window					**m_registry;
	Interactive_Player	*m_player;
	Warehouse				*m_warehouse;
	WareList					m_warecache;
};

/*
===============
Warehouse_Window::Warehouse_Window

Open the window, create the window buttons and add to the registry.
===============
*/
Warehouse_Window::Warehouse_Window(Interactive_Player *parent, Warehouse *wh, Window **registry)
	: Window(parent, 0, 0, 136, 260, "Warehouse")
{
	m_registry = registry;
	if (*m_registry)
		delete *m_registry;
	*m_registry = this;
	
	m_player = parent;
	m_warehouse = wh;
	
	move_to_mouse();
	
	set_think(true);
}

/*
===============
Warehouse_Window::~Warehouse_Window

Deinitialize, remove from registry
===============
*/
Warehouse_Window::~Warehouse_Window()
{
	*m_registry = 0;

	m_warecache.clear();
}

/*
===============
Warehouse_Window::draw

Draw the wares
===============
*/
void Warehouse_Window::draw(Bitmap *dst, int ofsx, int ofsy)
{
	Game *game = m_player->get_game();
	Tribe_Descr *tribe = m_player->get_player()->get_tribe();
	int x, y;
	
	x = 2;
	y = 2;
	
	for(int id = 0; id < game->get_nrwares(); id++)	{
		Ware_Descr *wd = game->get_ware_description(id);
		Pic *pic;
		
		// Figure out the picture
		if (wd->is_worker()) {
			Worker_Descr *worker = ((Worker_Ware_Descr*)wd)->get_worker(tribe);
			pic = worker->get_menu_pic();
		} else
			pic = ((Item_Ware_Descr*)wd)->get_menu_pic();
		
		assert(pic);
		copy_pic(dst, pic, x+ofsx, y+ofsy, 0, 0, pic->get_w(), pic->get_h());
		dst->fill_rect(x+ofsx, y+ofsy+WARE_MENU_PIC_H, WARE_MENU_PIC_W, 8, pack_rgb(0,0,0));
		
		char buf[32];
		snprintf(buf, sizeof(buf), "%i", m_warehouse->get_wares().stock(id));
		
		Pic *text = g_fh.get_string(buf, 0);
		copy_pic(dst, text, x+ofsx+WARE_MENU_PIC_W-text->get_w(), y+ofsy+WARE_MENU_PIC_H,
		         0, 0, text->get_w(), text->get_h());
		delete text;
		
		if (id % 4 < 3)
			x += WARE_MENU_PIC_W + 3;
		else {
			x = 2;
			y += WARE_MENU_PIC_H+8 + 3;
		}
	}
	
	m_warecache = m_warehouse->get_wares();
}

/*
===============
Warehouse_Window::think

Check whether we need to redraw.
I use this instead of a push-model (Warehouse code calling  update() when 
necessary) because
a) it's less intrusive on the actual game logic code
b) the UI may know better whether we _really_ need to redraw (e.g. it may 
   not be necessary to redraw if the window currently shows a different "page",
	such as soldiers in the HQ).
===============
*/
void Warehouse_Window::think()
{
	if (m_warecache != m_warehouse->get_wares())
		update_inner(0, 0, get_inner_w(), get_inner_h());
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

