/*
 * Copyright (C) 2002 by the Widelands Development Team
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

#include "widelands.h"
#include "ui.h"
#include "game.h"
#include "map.h"
#include "IntPlayer.h"
#include "player.h"
#include "transport.h"
#include "fieldaction.h"
#include "watchwindow.h"

#include "ui_tabpanel.h"
#include "ui_box.h"


/*
==============================================================================

BuildGrid IMPLEMENTATION

==============================================================================
*/

#define BG_CELL_WIDTH		34 // extents of one cell
#define BG_CELL_HEIGHT		34


// The BuildGrid presents a selection of buildable buildings
class BuildGrid : public Panel {
public:
	BuildGrid(Panel* parent, Tribe_Descr* tribe, int x, int y, int cols);
	
	UISignal1<int> clicked;
	
	void add(int idx);

protected:
	void draw(RenderTarget* dst);
	
	int index_for_point(int x, int y);
	void update_for_index(int idx);
	
	void handle_mousein(bool inside);
	void handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns);
	bool handle_mouseclick(uint btn, bool down, int x, int y);

private:
	Tribe_Descr*		m_tribe;
	std::vector<int>	m_buildings;
	
	int	m_cols;			// max # of columns in the grid
	int	m_highlight;	// currently highlight building idx (-1 = no highlight)
	int	m_clicked;		// building that was clicked (only while LMB is down)
};


/*
===============
BuildGrid::BuildGrid

Initialize the grid
===============
*/
BuildGrid::BuildGrid(Panel* parent, Tribe_Descr* tribe, int x, int y, int cols)
	: Panel(parent, x, y, 0, 0)
{
	m_tribe = tribe;
	m_cols = cols;
	
	m_highlight = -1;
	m_clicked = -1;
}


/*
===============
BuildGrid::add

Add a new building to the list of buildable buildings and resize appropriately.
===============
*/
void BuildGrid::add(int idx)
{
	m_buildings.push_back(idx);
	
	if ((int)m_buildings.size() < m_cols)
		set_size(BG_CELL_WIDTH * m_buildings.size(), BG_CELL_HEIGHT);
	else
		set_size(BG_CELL_WIDTH * m_cols, BG_CELL_HEIGHT * (m_buildings.size()+m_cols-1) / m_cols);
}


/*
===============
BuildGrid::draw

Draw the building symbols
===============
*/
void BuildGrid::draw(RenderTarget* dst)
{
	int x, y;

	// First of all, draw the highlight
	if (m_highlight >= 0 && (m_clicked < 0 || m_clicked == m_highlight))
		dst->brighten_rect((m_highlight % m_cols) * BG_CELL_WIDTH, (m_highlight / m_cols) * BG_CELL_HEIGHT,
		                   BG_CELL_WIDTH, BG_CELL_HEIGHT, MOUSE_OVER_BRIGHT_FACTOR);

	// Draw the symbols
	x = 0;
	y = 0;
	
	for(uint idx = 0; idx < m_buildings.size(); idx++)
		{
		Building_Descr* descr = m_tribe->get_building_descr(m_buildings[idx]);
		uint picid = descr->get_buildicon();
		int w, h;
		
		g_gr->get_picture_size(picid, &w, &h);
		
		dst->blit(x + (BG_CELL_WIDTH-w)/2, y + (BG_CELL_HEIGHT-h)/2, picid);
				
		x += BG_CELL_WIDTH;
		if (x >= get_w())
			{
			x = 0;
			y += BG_CELL_HEIGHT;
			}
		}
}


/*
===============
BuildGrid::index_for_point

Return the building index for a given point inside the BuildGrid.
Returns -1 if no building is below the point.
===============
*/
int BuildGrid::index_for_point(int x, int y)
{
	if (x < 0 || x >= m_cols * BG_CELL_HEIGHT || y < 0)
		return -1;
	
	int index = m_cols * (y / BG_CELL_HEIGHT) + (x / BG_CELL_WIDTH);
	
	if (index < 0 || index >= (int)m_buildings.size())
		return -1;
	
	return index;
}


/*
===============
BuildGrid::update_for_index

Issue an update() call for the cell with the given idx.
===============
*/
void BuildGrid::update_for_index(int idx)
{
	if (idx >= 0 && idx < (int)m_buildings.size())
		update((idx % m_cols) * BG_CELL_WIDTH, (idx / m_cols) * BG_CELL_HEIGHT,
		       BG_CELL_WIDTH, BG_CELL_HEIGHT);
}


/*
===============
BuildGrid::handle_mousein

Turn highlight off when the mouse leaves the grid
===============
*/
void BuildGrid::handle_mousein(bool inside)
{
	if (!inside)
		{
		if (m_highlight != -1)
			update_for_index(m_highlight);
		
		m_highlight = -1;
		}
}


/*
===============
BuildGrid::handle_mousemove

Update highlight under the mouse
===============
*/
void BuildGrid::handle_mousemove(int x, int y, int xdiff, int ydiff, uint btns)
{
	int hl = index_for_point(x, y);
	
	if (hl != m_highlight)
		{
		update_for_index(m_highlight);
		update_for_index(hl);
		
		m_highlight = hl;
		}
}


/*
===============
BuildGrid::handle_mouseclick

Left mouse down selects the building, left mouse up acknowledges and sends the
signal.
===============
*/
bool BuildGrid::handle_mouseclick(uint btn, bool down, int x, int y)
{
	if (btn != 0)
		return false;

	if (down)
		{
		grab_mouse(true);
		m_clicked = index_for_point(x, y);
		}
	else
		{
		if (m_clicked >= 0)
			{
			int hl = index_for_point(x, y);
			
			grab_mouse(false);
			
			if (hl == m_clicked)
				clicked.call(m_buildings[m_clicked]);
			
			m_clicked = -1;
			}
		}

	return true;
}


/*
==============================================================================

FieldActionWindow IMPLEMENTATION

==============================================================================
*/
class FieldActionWindow : public Window {
public:
	FieldActionWindow(Interactive_Player *plr, UniqueWindow *registry);
	~FieldActionWindow();

	void init();
	void add_buttons_auto();
	void add_buttons_build(int buildcaps);
	void add_buttons_road(bool flag);
	
	// Action handlers
	void act_watch();
	void act_buildflag();
	void act_ripflag();
	void act_buildroad();
	void act_abort_buildroad();
	void act_removeroad();
	void act_build(int idx);

private:
   void add_tab(const char* picname, Panel* panel);
   void add_button(Box* box, const char* picname, void (FieldActionWindow::*fn)());
	void okdialog();
	
	Interactive_Player	*m_player;
	Map						*m_map;
	UniqueWindow			*m_registry;

	FCoords		m_field;

	TabPanel*	m_tabpanel;
};

static const char* pic_tab_buildroad = "pics/menu_tab_buildroad.bmp";
static const char* pic_tab_watch = "pics/menu_tab_watch.bmp";
static const char* pic_tab_buildhouse[3] = {
	"pics/menu_tab_buildsmall.bmp",
	"pics/menu_tab_buildmedium.bmp",
	"pics/menu_tab_buildbig.bmp"
};
static const char* pic_tab_buildmine = "pics/menu_tab_buildmine.bmp";

static const char* pic_buildroad = "pics/menu_build_way.bmp";
static const char* pic_remroad = "pics/menu_rem_way.bmp";
static const char* pic_buildflag = "pics/menu_build_flag.bmp";
static const char* pic_ripflag = "pics/menu_rip_flag.bmp";
static const char* pic_watchfield = "pics/menu_watch_field.bmp";
static const char* pic_abort = "pics/menu_abort.bmp";


/*
===============
FieldActionWindow::FieldActionWindow

Initialize a field action window, creating the appropriate buttons.
===============
*/
FieldActionWindow::FieldActionWindow(Interactive_Player *plr, UniqueWindow *registry)
	: Window(plr, plr->get_w()/2, plr->get_h()/2, 68, 34, "Action")
{
	// Hooks into the game classes
	m_player = plr;
	m_map = m_player->get_game()->get_map();
	m_registry = registry;
	if (registry->window)
		delete registry->window;
	registry->window = this;
	
	Field *f = m_map->get_field(m_player->get_fieldsel());
	m_field = FCoords(m_player->get_fieldsel(), f);

	m_player->set_fieldsel_freeze(true);

	//
	m_tabpanel = new TabPanel(this, 0, 0, 1);
	m_tabpanel->set_snapparent(true);
}

/*
===============
FieldActionWindow::~FieldActionWindow

Free allocated resources, remove from registry.
===============
*/
FieldActionWindow::~FieldActionWindow()
{
	m_player->set_fieldsel_freeze(false);
	m_registry->window = 0;
}


/*
===============
FieldActionWindow::init

Initialize after buttons have been registered.
This mainly deals with mouse placement
===============
*/
void FieldActionWindow::init()
{
	m_tabpanel->resize();

	// Move the window away from the current mouse position, i.e.
	// where the field is, to allow better view
	int mousex = get_mouse_x();
	int mousey = get_mouse_y();
	
	if (mousex >= 0 && mousex < get_w() &&
	    mousey >= 0 && mousey < get_h()) {
		if (mousey < get_h()/2)
			set_pos(get_x(), get_y()+get_h());
		else
			set_pos(get_x(), get_y()-get_h());
	}
	
	// Now force the mouse onto the first button
	// TODO: should be on first tab button if we're building
	set_mouse_pos(17, 51);
}


/*
===============
FieldActionWindow::add_buttons_auto

Add the buttons you normally get when clicking on a field.
===============
*/
void FieldActionWindow::add_buttons_auto()
{
	Box* buildbox = 0;
	Box* watchbox;
	
	watchbox = new Box(m_tabpanel, 0, 0);
	
	// Add road-building actions
	if (m_field.field->get_owned_by() == m_player->get_player_number())
	{
		BaseImmovable *imm = m_map->get_immovable(m_field);
	
		// The box with road-building buttons
		buildbox = new Box(m_tabpanel, 0, 0);
		
		if (imm && imm->get_type() == Map_Object::FLAG)
		{
			// Add flag actions
			Flag *flag = (Flag*)imm;

			add_button(buildbox, pic_buildroad, &FieldActionWindow::act_buildroad);

			Building *building = flag->get_building();

			if (!building || strcasecmp(building->get_name(), "headquarters"))
				add_button(buildbox, pic_ripflag, &FieldActionWindow::act_ripflag);
		}
		else
		{
			int buildcaps = m_player->get_player()->get_buildcaps(m_field);
			
			// Add house building
			if ((buildcaps & BUILDCAPS_SIZEMASK) || (buildcaps & BUILDCAPS_MINE))
				add_buttons_build(buildcaps);
			
			// Add build actions
			if (buildcaps & BUILDCAPS_FLAG)
				add_button(buildbox, pic_buildflag, &FieldActionWindow::act_buildflag);
			
			if (imm && imm->get_type() == Map_Object::ROAD)
				add_button(buildbox, pic_remroad, &FieldActionWindow::act_removeroad);
		}
	}
	
	// Watch actions
	add_button(watchbox, pic_watchfield, &FieldActionWindow::act_watch);
	
	
	// Add tabs
	if (buildbox && buildbox->get_nrpanels())
		{
		buildbox->resize();
		add_tab(pic_tab_buildroad, buildbox);
		}
	
	watchbox->resize();
	add_tab(pic_tab_watch, watchbox);
}


/*
===============
FieldActionWindow::add_buttons_build

Add buttons for house building.
===============
*/
void FieldActionWindow::add_buttons_build(int buildcaps)
{
	BuildGrid* bbg_house[3] = { 0, 0, 0 };
	BuildGrid* bbg_mine = 0;
	
	Tribe_Descr* tribe = m_player->get_player()->get_tribe();
	
	for(int id = 0; id < tribe->get_nrbuildings(); id++)
		{
		Building_Descr* descr = tribe->get_building_descr(id);
		BuildGrid** ppgrid;
		
		// Some buildings cannot be built (i.e. construction site, HQ)
		if (!descr->get_buildable())
			continue;
		
		// Figure out if we can build it here, and in which tab it belongs
		if (descr->get_ismine())
			{
			if (!(buildcaps & BUILDCAPS_MINE))
				continue;
			
			ppgrid = &bbg_mine;
			}
		else
			{
			int size = descr->get_size() - BaseImmovable::SMALL;
			
			if ((buildcaps & BUILDCAPS_SIZEMASK) < (size+1))
				continue;
			
			ppgrid = &bbg_house[size];
			}
		
		// Allocate the tab's grid if necessary
		if (!*ppgrid)
			{
			*ppgrid = new BuildGrid(m_tabpanel, tribe, 0, 0, 5);
			(*ppgrid)->clicked.set(this, &FieldActionWindow::act_build);
			}
		
		// Add it to the grid
		(*ppgrid)->add(id);
		}
	
	// Add all necessary tabs
	for(int i = 0; i < 3; i++)
		if (bbg_house[i])
			add_tab(pic_tab_buildhouse[i], bbg_house[i]);

	if (bbg_mine)
		add_tab(pic_tab_buildmine, bbg_mine);
}


/*
===============
FieldActionWindow::add_buttons_road

Buttons used during road building: Set flag here and Abort
===============
*/
void FieldActionWindow::add_buttons_road(bool flag)
{
	Box* buildbox = new Box(m_tabpanel, 0, 0);

	if (flag)
		add_button(buildbox, pic_buildflag, &FieldActionWindow::act_buildflag);
	
	add_button(buildbox, pic_abort, &FieldActionWindow::act_abort_buildroad);
	
	// Add the box as tab
	buildbox->resize();
	add_tab(pic_tab_buildroad, buildbox);
}


/*
===============
FieldActionWindow::add_tab

Convenience function: Adds a new tab to the main tab panel
===============
*/
void FieldActionWindow::add_tab(const char* picname, Panel* panel)
{
	m_tabpanel->add(g_gr->get_picture(PicMod_Game, picname, RGBColor(0,0,255)), panel);
}


/*
===============
FieldActionWindow::add_button
===============
*/
void FieldActionWindow::add_button(Box* box, const char* picname, void (FieldActionWindow::*fn)())
{
	Button *b = new Button(box, 0, 0, 34, 34, 2);
	b->clicked.set(this, fn);
	b->set_pic(g_gr->get_picture(PicMod_Game, picname, RGBColor(0,0,255)));
	
	box->add(b);
}

/*
===============
FieldActionWindow::okdialog

Call this from the button handlers.
It resets the mouse to its original position and closes the window
===============
*/
void FieldActionWindow::okdialog()
{
	m_player->warp_mouse_to_field(m_field);
	die();
}

/*
===============
FieldActionWindow::act_watch()

Open a watch window for the given field and delete self.
===============
*/
void FieldActionWindow::act_watch()
{
	show_watch_window(m_player, m_field.x, m_field.y);
	okdialog();
}

/*
===============
FieldActionWindow::act_buildflag

Build a flag at this field
===============
*/
void FieldActionWindow::act_buildflag()
{
	Game *g = m_player->get_game();
	
	g->send_player_command(m_player->get_player_number(), CMD_BUILD_FLAG, m_field.x, m_field.y);
	
	if (m_player->is_building_road())
		m_player->finish_build_road();
	
	okdialog();
}

/*
===============
FieldActionWindow::act_ripflag

Remove the flag at this field
===============
*/
void FieldActionWindow::act_ripflag()
{
	Game *g = m_player->get_game();
	
	g->send_player_command(m_player->get_player_number(), CMD_RIP_FLAG, m_field.x, m_field.y);

	okdialog();
}

/*
===============
FieldActionWindow::act_buildroad

Start road building.
===============
*/
void FieldActionWindow::act_buildroad()
{
	m_player->start_build_road(m_field);
	okdialog();
}

/*
===============
FieldActionWindow::act_abort_buildroad

Abort building a road.
===============
*/
void FieldActionWindow::act_abort_buildroad()
{
	m_player->abort_build_road();
	okdialog();
}

/*
===============
FieldActionWindow::act_removeroad

Remove the road at the given field
===============
*/
void FieldActionWindow::act_removeroad()
{
	Game *g = m_player->get_game();
	BaseImmovable *imm = g->get_map()->get_immovable(m_field);
	
	if (imm && imm->get_type() == Map_Object::ROAD)
		g->send_player_command(m_player->get_player_number(), CMD_REMOVE_ROAD, imm->get_serial());
	
	okdialog();
}


/*
===============
FieldActionWindow::act_build

Start construction of the building with the give description index
===============
*/
void FieldActionWindow::act_build(int idx)
{
	Game *g = m_player->get_game();
	
	g->send_player_command(m_player->get_player_number(), CMD_BUILD, m_field.x, m_field.y, idx);
	
	okdialog();
}


/*
===============
show_field_action

Perform a field action (other than building options).
Bring up a field action window or continue road building.
===============
*/
void show_field_action(Interactive_Player *parent, UniqueWindow *registry)
{
	FieldActionWindow *faw;

	if (!parent->is_building_road()) {
		faw = new FieldActionWindow(parent, registry);
		faw->add_buttons_auto();
		faw->init();
		return;
	}

	// we're building a road right now	
	Map *map = parent->get_game()->get_map();
	Coords target = parent->get_fieldsel();
	
	// if user clicked on the same field again, build a flag	
	if (target == parent->get_build_road_end()) {
		faw = new FieldActionWindow(parent, registry);
		
		bool flag = false;
		if (target != parent->get_build_road_start() && 
		    parent->get_player()->get_buildcaps(target) & BUILDCAPS_FLAG)
			flag = true;
		faw->add_buttons_road(flag);
		faw->init();
		return;
	}

	// append or take away from the road
	if (!parent->append_build_road(target)) {
		faw = new FieldActionWindow(parent, registry);
		faw->add_buttons_road(false);
		faw->init();
		return;
	}
	
	// did he click on a flag or a road where a flag can be built?
	BaseImmovable *imm = map->get_immovable(target);
	
	if (imm) {
		switch(imm->get_type()) {
		case Map_Object::ROAD:
			if (!(parent->get_player()->get_buildcaps(target) & BUILDCAPS_FLAG))
				break;
			parent->get_game()->send_player_command(parent->get_player_number(), CMD_BUILD_FLAG, target.x, target.y);
			// fall through, there is a flag now
				
		case Map_Object::FLAG:
			parent->finish_build_road();
			break;
		}
	}
}
