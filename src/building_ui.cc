/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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


#include "constructionsite.h"
#include "game_debug_ui.h"
#include "graphic.h"
#include "i18n.h"
#include "interactive_player.h"
#include "maphollowregion.h"
#include "militarysite.h"
#include "soldier.h"
#include "trainingsite.h"
#include "tribe.h"
#include "warehouse.h"
#include "waresdisplay.h"

#include "ui_box.h"
#include "ui_button.h"
#include "ui_listselect.h"
#include "ui_progressbar.h"
#include "ui_table.h"
#include "ui_tabpanel.h"
#include "ui_textarea.h"
#include "ui_window.h"

static const char* pic_ok = "pics/menu_okay.png";
static const char* pic_cancel = "pics/menu_abort.png";
static const char* pic_debug = "pics/menu_debug.png";

static const char* pic_bulldoze = "pics/menu_bld_bulldoze.png";
static const char* pic_queue_background = "pics/queue_background.png";

static const char* pic_list_worker = "pics/menu_list_workers.png";

static const char* pic_priority_low = "pics/low_priority_button.png";
static const char* pic_priority_normal = "pics/normal_priority_button.png";
static const char* pic_priority_high = "pics/high_priority_button.png";
static const char* pic_priority_low_on = "pics/low_priority_on.png";
static const char* pic_priority_normal_on = "pics/normal_priority_on.png";
static const char* pic_priority_high_on = "pics/high_priority_on.png";

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
void Building::hide_options() {delete m_optionswindow;}


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
class BulldozeConfirm : public UI::Window {
public:
	BulldozeConfirm(Interactive_Base* parent, Building* building, PlayerImmovable* todestroy = 0);
	virtual ~BulldozeConfirm();

	virtual void think();

private:
	void bulldoze();

private:
	Interactive_Base * m_iabase;
	Object_Ptr         m_building;
	Object_Ptr         m_todestroy;
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
	: UI::Window(parent, 0, 0, 160, 90, _("Destroy building?").c_str())
{
	std::string text;

	center_to_parent();

	m_iabase = parent;
	m_building = building;

	if (!todestroy)
		m_todestroy = building;
	else
		m_todestroy = todestroy;

	text = _("Do you really want to destroy this ");
	text += building->name();
	text += _("?");
	new UI::Textarea(this, 0, 0, 160, 44, text, Align_Center, true);

	new UI::Button<BulldozeConfirm>
		(this,
		 6, 50, 60, 34,
		 4,
		 g_gr->get_picture(PicMod_Game, pic_ok),
		 &BulldozeConfirm::bulldoze, this);

	(new UI::Button<BulldozeConfirm>
	 (this,
	  94, 50, 60, 34,
	  4,
	  g_gr->get_picture(PicMod_Game, pic_cancel),
	  &BulldozeConfirm::die, this))
	 ->center_mouse();
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
	Editor_Game_Base * const egbase = &m_iabase->egbase();
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
	Editor_Game_Base * const egbase = &m_iabase->egbase();
	Building* building = (Building*)m_building.get(egbase);
	PlayerImmovable* todestroy = (PlayerImmovable*)m_todestroy.get(egbase);

	if (todestroy && building && building->get_playercaps() & (1 << Building::PCap_Bulldoze)) {
		if (Game * const game = dynamic_cast<Game *>(egbase)) {
			game->send_player_bulldoze (todestroy);
         m_iabase->need_complete_redraw();
		} else {// Editor
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
class WaresQueueDisplay : public UI::Panel {
public:
	enum {
		CellWidth = WARE_MENU_PIC_WIDTH,
		CellHeight = WARE_MENU_PIC_HEIGHT,
		Border = 4,

		Height = CellHeight + 2 * Border,

		BG_LeftBorderX = 0,
		BG_CellX = BG_LeftBorderX + Border,
		BG_RightBorderX = BG_CellX + CellWidth,
		BG_ContinueCellX = BG_RightBorderX + Border,
		BG_ContinueBorderX = BG_ContinueCellX + CellWidth,
	};

public:
	WaresQueueDisplay(UI::Panel* parent, int x, int y, uint maxw, WaresQueue* queue, Game* g);
	~WaresQueueDisplay();

	virtual void think();
	virtual void draw(RenderTarget* dst);

private:
	void recalc_size();

private:
	WaresQueue * m_queue;
	uint         m_max_width;
	uint         m_icon;            //< Index to ware's picture
	uint         m_fade_mask;       //< Mask to show faded version of icons
	uint         m_pic_background;

	uint         m_cache_size;
	uint         m_cache_filled;
	uint         m_display_size;
};


/*
===============
WaresQueueDisplay::WaresQueueDisplay

Initialize the panel.
===============
*/
WaresQueueDisplay::WaresQueueDisplay
(UI::Panel * parent, int x, int y, uint maxw, WaresQueue* queue, Game *)
:
UI::Panel(parent, x, y, 0, Height),
m_queue(queue),
m_max_width(maxw),
m_pic_background(g_gr->get_picture(PicMod_Game, pic_queue_background)),
m_cache_size(queue->get_size()),
m_cache_filled(queue->get_filled()),
m_display_size(0)
{
	const Item_Ware_Descr & ware =
		*queue->get_owner()->tribe().get_ware_descr(m_queue->get_ware());
	set_tooltip(ware.descname().c_str());

	m_icon = ware.get_icon();

	// Prepare a fadeout mask for undelivered wares
	Surface *s = new Surface(*g_gr->get_picture_surface(m_pic_background));

	SDL_SetAlpha(s->get_sdl_surface(), SDL_SRCALPHA, 175);
	m_fade_mask = g_gr->get_picture(PicMod_Game, *s);

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
	if (static_cast<uint>(m_queue->get_size()) != m_cache_size)
		recalc_size();

	if (static_cast<uint>(m_queue->get_filled()) != m_cache_filled)
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
	compile_assert(0 <= BG_LeftBorderX);
	dst->blitrect
		(Point(0, 0),
		 m_pic_background,
		 Rect(Point(BG_LeftBorderX, 0), Border, Height));

	x = Border;

	compile_assert(0 <= BG_ContinueCellX);
	compile_assert(0 <= BG_CellX);
	for (uint cells = 0; cells < m_display_size; ++cells, x += CellWidth) {
		dst->blitrect
			(Point(x, 0),
			 m_pic_background,
			 Rect
			 (Point
			  (cells + 1 == m_display_size and m_cache_size > m_display_size ?
			   BG_ContinueCellX : BG_CellX,
			   0),
			  CellWidth, Height));

		// Fill ware queue with ware's icon
		dst->blit(Point(x, Border), m_icon);

		// If ware is undelivered, gray it out.
		if (cells >= m_cache_filled)
			dst->blitrect(Point(x, 0), m_fade_mask,
			              Rect(Point(cells + 1 == m_display_size and
			                         m_cache_size > m_display_size ?
				                     BG_ContinueCellX : BG_CellX, 0),
				          CellWidth, Height));
	}

	compile_assert(0 <= BG_RightBorderX);
	dst->blitrect
		(Point(x, 0),
		 m_pic_background,
		 Rect
		 (Point
		  (m_cache_size > m_display_size ? BG_ContinueBorderX : BG_RightBorderX,
		   0),
		  Border, Height));
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
class Building_Window : public UI::Window {
	friend class TrainingSite_Window;
	friend class MilitarySite_Window;
public:
	enum {
		Width = 4 * 34 //  4 normally sized buttons
	};

public:
	Building_Window(Interactive_Player* parent, Building* building, UI::Window** registry);
	virtual ~Building_Window();

	Interactive_Player* get_player() {return m_player;}
	Building* get_building() {return m_building;}

	virtual void draw(RenderTarget* dst);
	virtual void think();

	UI::Panel* create_capsbuttons(UI::Panel* parent);

protected:
	void setup_capsbuttons();

	void act_bulldoze();
	void act_debug();
	void toggle_workarea();
	void act_start_stop();
	void act_enhance(const Building_Descr::Index);
   void act_drop_soldier(uint);
	void act_change_soldier_capacity(int);

private:
	UI::Window *                * m_registry;
	Interactive_Player          * m_player;
	Building                    * m_building;

	UI::Panel * m_capsbuttons; //  UI::Panel that contains capabilities buttons
	UI::Button<Building_Window> * m_toggle_workarea;

	//  capabilities that were last used in setting up the caps panel
	uint m_capscache;

	Overlay_Manager::Job_Id m_workarea_job_id;
	unsigned int workarea_cumulative_picid[NUMBER_OF_WORKAREA_PICS];
};


/*
===============
Building_Window::Building_Window

Create the window, add it to the registry.
===============
*/
Building_Window::Building_Window(Interactive_Player* parent, Building* building, UI::Window** registry)
:
UI::Window(parent, 0, 0, Width, 0, building->descname().c_str()),
m_workarea_job_id(Overlay_Manager::Job_Id::Null())
{
	m_registry = registry;
		delete *m_registry;
	*m_registry = this;

	m_player = parent;
	m_building = building;

	m_capsbuttons = 0;
	m_capscache = 0;

	move_to_mouse();

	set_think(true);

	for (Workarea_Info::size_type i = NUMBER_OF_WORKAREA_PICS; i;) {
		char filename[30];
		snprintf(filename, sizeof(filename), "pics/workarea%ucumulative.png", i);
		--i;
		workarea_cumulative_picid[i] = g_gr->get_picture(PicMod_Game, filename);
	}
}


/*
===============
Building_Window::~Building_Window

Add to registry
===============
*/
Building_Window::~Building_Window()
{
	if (not m_workarea_job_id.isNull())
		static_cast<Interactive_Player *>(get_parent())
		->egbase().map().overlay_manager().remove_overlay(m_workarea_job_id);
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

	dst->drawanim(Point(get_inner_w() / 2, get_inner_h() / 2), anim, 0, 0);

	// Draw all the panels etc. above the background
	UI::Window::draw(dst);
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

	UI::Window::think();
}


/*
===============
Building_Window::create_capsbuttons

Create the capsbuttons panel with the given parent window, set it up and return
it.
===============
*/
UI::Panel* Building_Window::create_capsbuttons(UI::Panel* parent)
{
		delete m_capsbuttons;

	m_capsbuttons = new UI::Panel(parent, 0, 0, Width, 34);
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
		const bool stopped = m_building->get_stop();
		new UI::Button<Building_Window>
			(m_capsbuttons,
			 x, 0, 34, 34,
			 4,
			 g_gr->get_picture
			 (PicMod_Game,
			  (stopped ?
			   m_building->get_continue_icon() : m_building->get_stop_icon())
			  .c_str()),
			 &Building_Window::act_start_stop, this,
			 stopped ? _("Continue") : _("Stop"));
		x += 34;
	}

   if (m_capscache & (1 << Building::PCap_Enhancable)) {
		const std::vector<char *> & buildings = m_building->enhances_to();
	   const Tribe_Descr & tribe = m_player->get_player()->tribe();
		const std::vector<char *>::const_iterator buildings_end = buildings.end();
		for
			(std::vector<char *>::const_iterator it = buildings.begin();
			 it != buildings_end;
			 ++it)
		{
         int id = tribe.get_building_index(*it);
         if (id==-1)
            throw wexception("Should enhance to unknown building: %s\n", *it);

         if (!m_player->get_player()->is_building_allowed(id)) {
            // This buildings is disabled for this scenario, sorry.
            // Try again later!!
            continue;
			}

         const Building_Descr & building = *tribe.get_building_descr(id);
         char buffer[128];
         snprintf
            (buffer, sizeof(buffer),
             _("Enhance to %s").c_str(), building.descname().c_str());
			new UI::IDButton<Building_Window, Building_Descr::Index>
				(m_capsbuttons,
				 x, 0, 34, 34,
				 4,
				 building.get_buildicon(),
				 &Building_Window::act_enhance, this, id, // Button id = building id
				 buffer);
         x += 34;
		}
	}

	if (m_capscache & (1 << Building::PCap_Bulldoze)) {
		new UI::Button<Building_Window>
			(m_capsbuttons,
			 x, 0, 34, 34,
			 4,
			 g_gr->get_picture(PicMod_Game, pic_bulldoze),
			 &Building_Window::act_bulldoze, this,
			 _("Destroy"));
		x += 34;
	}

	if (m_building->descr().m_recursive_workarea_info.size()) {
		m_toggle_workarea = new UI::Button<Building_Window>
			(m_capsbuttons,
			 x, 0, 34, 34,
			 4,
			 g_gr->get_picture(PicMod_Game,  "pics/workarea3cumulative.png"),
			 &Building_Window::toggle_workarea, this,
			 _("Show workarea"));
		x += 34;
	}

	if (m_player->get_display_flag(Interactive_Base::dfDebug)) {
		new UI::Button<Building_Window>
			(m_capsbuttons,
			 x, 0, 34, 34,
			 4,
			 g_gr->get_picture(PicMod_Game,  pic_debug),
			 &Building_Window::act_debug, this,
			 _("Debug"));
		x += 34;
	}

   if (x == 0) {
      // no capsbutton is in this window
      // resize us, so that we do not take space
      m_capsbuttons->set_inner_size(0, 0);
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
void Building_Window::act_enhance(const Building_Descr::Index id)
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


void Building_Window::toggle_workarea() {
	Map & map =
		static_cast<Interactive_Player *>(get_parent())->egbase().map();
	Overlay_Manager & overlay_manager = map.overlay_manager();
	if (m_workarea_job_id.isNull()) {
		m_workarea_job_id = overlay_manager.get_a_job_id();
		HollowArea<> hollow_area(Area<>(m_building->get_position(), 0), 0);
		const Workarea_Info & workarea_info =
			m_building->descr().m_recursive_workarea_info;
		Workarea_Info::const_iterator it = workarea_info.begin();
		for
			(Workarea_Info::size_type i =
				 std::min(workarea_info.size(), NUMBER_OF_WORKAREA_PICS);
			 i;
			 ++it)
		{
			--i;
			hollow_area.radius = it->first;
			MapHollowRegion<> mr(map, hollow_area);
			do overlay_manager.register_overlay
				(mr.location(),
				 workarea_cumulative_picid[i],
				 0,
				 Point::invalid(),
				 m_workarea_job_id);
			while (mr.advance(map));
			hollow_area.hole_radius = hollow_area.radius;
		}
		m_toggle_workarea->set_tooltip(_("Hide workarea").c_str());
	} else {
		overlay_manager.remove_overlay(m_workarea_job_id);
		m_workarea_job_id = Overlay_Manager::Job_Id::Null();
		m_toggle_workarea->set_tooltip(_("Show workarea").c_str());
	}
}


/*
==============================================================================

ConstructionSite UI IMPLEMENTATION

==============================================================================
*/

class ConstructionSite_Window : public Building_Window {
public:
	ConstructionSite_Window(Interactive_Player* parent, ConstructionSite* cs, UI::Window** registry);
	virtual ~ConstructionSite_Window();

	ConstructionSite* get_constructionsize() {return (ConstructionSite*)get_building();}

	virtual void think();

private:
	UI::Progress_Bar * m_progress;
};


/*
===============
ConstructionSite_Window::ConstructionSite_Window

Create the window and its panels
===============
*/
ConstructionSite_Window::ConstructionSite_Window(Interactive_Player* parent, ConstructionSite* cs,
                                                 UI::Window** registry)
	: Building_Window(parent, cs, registry)
{
	UI::Box* box = new UI::Box(this, 0, 0, UI::Box::Vertical);

	// Add the progress bar
	m_progress = new UI::Progress_Bar(box, 0, 0, UI::Progress_Bar::DefaultWidth, UI::Progress_Bar::DefaultHeight,
							UI::Progress_Bar::Horizontal);
	m_progress->set_total(1 << 16);
	box->add(m_progress, UI::Box::AlignCenter);

	box->add_space(8);

	// Add the wares queue
	for (uint i = 0; i < cs->get_nrwaresqueues(); i++)
	{
		WaresQueueDisplay* wqd = new WaresQueueDisplay(box, 0, 0, get_w(),
					cs->get_waresqueue(i), parent->get_game());

		box->add(wqd, UI::Box::AlignLeft);
	}

	box->add_space(8);

	// Add the caps button
	box->add(create_capsbuttons(box), UI::Box::AlignCenter);

	fit_inner(box);
	move_inside_parent();
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
UI::Window *ConstructionSite::create_options_window(Interactive_Player *plr, UI::Window **registry)
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
	Warehouse_Window(Interactive_Player *parent, Warehouse *wh, UI::Window **registry);
	virtual ~Warehouse_Window();

	Warehouse* get_warehouse() {return (Warehouse*)get_building();}

	virtual void think();

private:
	void clicked_help      ();
	void clicked_switchpage();
	void clicked_goto      ();

private:
	WaresDisplay       * m_waresdisplay;
   Interactive_Player*  m_parent;
   int                  m_curpage;
};

/*
===============
Warehouse_Window::Warehouse_Window

Open the window, create the window buttons and add to the registry.
===============
*/
Warehouse_Window::Warehouse_Window(Interactive_Player *parent, Warehouse *wh, UI::Window **registry)
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


	new UI::Button<Warehouse_Window>
		(this,
		 posx, posy, button_w, 25,
		 4,
		 g_gr->get_picture(PicMod_Game, "pics/menu_help.png"),
		 &Warehouse_Window::clicked_help, this);

   posx += button_w + spacing;

   new UI::Button<Warehouse_Window>
		(this,
		 posx, posy, button_w * 2 + spacing, 25,
		 4,
		 g_gr->get_picture(PicMod_Game, "pics/warehousewindow_switchpage.png"),
		 &Warehouse_Window::clicked_switchpage, this);

   posx += button_w*2 + 2*spacing;

	new UI::Button<Warehouse_Window>
		(this,
		 posx, posy, button_w, 25,
		 4,
		 g_gr->get_picture(PicMod_Game, "pics/menu_goto.png"),
		 &Warehouse_Window::clicked_goto, this);

   posx += button_w + spacing;
   posy += 25 + spacing;

	// Add caps buttons
   posx = 0;
   UI::Panel* caps = create_capsbuttons(this);
	caps->set_pos(Point(spacing, posy));
   if (caps->get_h())
      posy += caps->get_h() + spacing;

   set_inner_size(get_inner_w(), posy);
   move_inside_parent();
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

void Warehouse_Window::clicked_help() {
         log("TODO: Implement help!\n");
}


void Warehouse_Window::clicked_goto() {
         m_parent->move_view_to(get_warehouse()->get_position());
}


/*
 * Switch to the next page, that is, show
 * wares -> workers -> soldier
 */
void Warehouse_Window::clicked_switchpage() {
   if (m_curpage == 0) {
      // Showing wares, should show workers
      m_waresdisplay->remove_all_warelists();
      m_waresdisplay->add_warelist(&get_warehouse()->get_workers(), WaresDisplay::WORKER);
      m_curpage = 1;
	} else if (m_curpage == 1) {
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
UI::Window *Warehouse::create_options_window(Interactive_Player *plr, UI::Window **registry)
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
class ProductionSite_Window_ListWorkerWindow : public UI::Window {
   public:
      ProductionSite_Window_ListWorkerWindow(Interactive_Player*, ProductionSite* ps);
      virtual ~ProductionSite_Window_ListWorkerWindow();

      virtual void think();

   private:
      void update();
      void fill_list();

      Coords          m_ps_location;
      ProductionSite* m_ps;
      Interactive_Player* m_parent;
	UI::Listselect<Worker*> * m_ls;
      UI::Textarea* m_type, *m_experience, *m_becomes;
};

/*
 * Constructor
 */
ProductionSite_Window_ListWorkerWindow::ProductionSite_Window_ListWorkerWindow(Interactive_Player* parent, ProductionSite* ps)
:
UI::Window(parent, 0, 0, 320, 125, _("Worker Listing").c_str())
{
   m_ps=ps;
   m_ps_location=ps->get_position();
   m_parent=parent;

   // Caption
   UI::Textarea* tt=new UI::Textarea(this, 0, 0, _("Worker Listing"), Align_Left);
	tt->set_pos(Point((get_inner_w() - tt->get_w()) / 2, 5));

   int spacing=5;
   int offsx=spacing;
   int offsy=30;
   int posx=offsx;
   int posy=offsy;

   // listselect
   m_ls=new UI::Listselect<Worker*>(this, posx, posy, get_inner_w()/2-spacing, get_inner_h()-spacing-offsy);

   // the descriptive areas
   // Type
   posx=get_inner_w()/2+spacing;
   new UI::Textarea(this, posx, posy, 150, 20, _("Type: "), Align_CenterLeft);
   m_type=new UI::Textarea(this, posx+80, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // Experience
   new UI::Textarea(this, posx, posy, 150, 20, _("Experience: "), Align_CenterLeft);
   m_experience=new UI::Textarea(this, posx+80, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   // is working to become
   new UI::Textarea(this, posx, posy, 70, 20, _("Trying to become: "), Align_CenterLeft);
   posy+=20;
   m_becomes=new UI::Textarea(this, posx+25, posy, 200, 20, "---", Align_CenterLeft);
   posy+=20+spacing;

   center_to_parent();
   move_to_top();
}

/*
 * desctructor
 */
ProductionSite_Window_ListWorkerWindow::~ProductionSite_Window_ListWorkerWindow() {
}

/*
 * think()
 */
void ProductionSite_Window_ListWorkerWindow::think() {
	const BaseImmovable * const base_immovable =
		m_parent->egbase().map()[m_ps_location].get_immovable();
	if
		(not dynamic_cast<const Building *>(base_immovable)
		 or
		 dynamic_cast<const ConstructionSite *>(base_immovable))
	{
      // The Productionsite has been removed. Die quickly.
      die();
      return;
	}

   fill_list();
   UI::Window::think();
}

/*
 * fill list()
 */
void ProductionSite_Window_ListWorkerWindow::fill_list() {
	const uint m_last_select = m_ls->selection_index();
   m_ls->clear();
   std::vector<Worker*>* workers=m_ps->get_workers();

   uint i;
   for (i=0; i<workers->size(); i++) {
		Worker* worker = (*workers)[i];
		m_ls->add(worker->descname().c_str(), worker, worker->get_menu_pic());
	}
	if (m_ls->size() > m_last_select) m_ls->select(m_last_select);
	else if (m_ls->size()) m_ls->select(m_ls->size() - 1);

   update();
}

/**
 * \brief Update worker info subwindow, following user selection
 */
void ProductionSite_Window_ListWorkerWindow::update()
{
	if (m_ls->has_selection()) {
		const Worker& worker = *m_ls->get_selected();

		m_type->set_text(worker.descname());

		if
			(worker.get_current_experience() != -1
			 and
			 worker.get_needed_experience () != -1)
		{
			// Fill upgrade status
			char buffer[7];
			sprintf(buffer, "%i/%i", worker.get_current_experience(),
							worker.get_needed_experience());
			m_experience->set_text(buffer);

			// Get the descriptive name of the ongoing upgrade
			uint index = worker.get_tribe()->get_safe_worker_index(
							worker.get_becomes());
			const Worker_Descr *descr = worker.get_tribe()->get_worker_descr(
														index);
			m_becomes->set_text(descr->descname());

		} else {
			// Worker is not upgradeable
			m_experience->set_text("---");
			m_becomes->set_text("---");
		}
	}
}

struct PriorityButtonInfo {
	UI::Basic_Button* button;
	int picture_enabled;
	int picture_disabled;

	PriorityButtonInfo() {}

	PriorityButtonInfo
		(UI::Basic_Button* btn, int pic_enabled, int pic_disabled)
		: button(btn), picture_enabled(pic_enabled),
		  picture_disabled(pic_disabled)
	{}
};

struct PriorityButtonHelper : std::map<int, PriorityButtonInfo> {
	PriorityButtonHelper(ProductionSite * ps, int ware_type, int ware_index);

	void button_clicked (int priority);
	void update_buttons ();

private:
	ProductionSite * m_ps;
	int m_ware_type;
	int m_ware_index;
};

class ProductionSite_Window : public Building_Window {
public:
	ProductionSite_Window(Interactive_Player* parent, ProductionSite* ps, UI::Window** registry);
	virtual ~ProductionSite_Window();

	inline ProductionSite* get_productionsite() {return (ProductionSite*)get_building();}

	virtual void think();

private:
   Interactive_Player* m_parent;
   UI::Window** m_reg;
	std::list<PriorityButtonHelper> m_priority_helpers;
public:
   void list_worker_clicked();
protected:
	UI::Box* create_production_box(UI::Panel* ptr, ProductionSite* ps);

	void create_ware_queue_panel(UI::Box* box, ProductionSite * ps, WaresQueue * const wq);

	UI::Basic_Button* create_priority_button
		(UI::Box* box, PriorityButtonHelper & helper,
		 int priority, int x, int y, int w, int h,
		 const char * picture1, const char * picture2,
		 const std::string & tooltip);
};


PriorityButtonHelper::PriorityButtonHelper
(ProductionSite * ps, int ware_type, int ware_index)
	: m_ps(ps),
	  m_ware_type(ware_type),
	  m_ware_index(ware_index)
{}

void PriorityButtonHelper::button_clicked (int priority) {
	m_ps->set_priority (m_ware_type, m_ware_index, priority);
	update_buttons();
}

void PriorityButtonHelper::update_buttons () {
	const int priority = m_ps->get_priority(m_ware_type, m_ware_index, false);
	for (iterator it = begin(); it != end(); ++it) {
		bool enable = it->first != priority;
		it->second.button->set_enabled(enable);
		it->second.button->set_pic
			(enable
				? it->second.picture_enabled
				: it->second.picture_disabled);
	}
}

/*
===============
ProductionSite_Window::ProductionSite_Window

Create the window and its panels, add it to the registry.
===============
*/
ProductionSite_Window::ProductionSite_Window(Interactive_Player* parent, ProductionSite* ps, UI::Window** registry)
	: Building_Window(parent, ps, registry)
{
   m_parent=parent;
   m_reg=registry;

   UI::Box* prod_box = 0;
   if (ps->get_building_type() == Building::PRODUCTIONSITE)
   {
       prod_box = create_production_box (this, ps);
      fit_inner(prod_box);
      move_inside_parent();
	}
}

UI::Basic_Button * ProductionSite_Window::create_priority_button
(UI::Box* box, PriorityButtonHelper & helper,
 int priority, int x, int, int w, int h,
 const char * picture1, const char * picture2,
 const std::string & button_tooltip)
{
	int pic_enabled = g_gr->get_resized_picture (
		 g_gr->get_picture(PicMod_Game,  picture1),
		 w, h, Graphic::ResizeMode_Clip);
	int pic_disabled = g_gr->get_resized_picture (
		 g_gr->get_picture(PicMod_Game,  picture2),
		 w, h, Graphic::ResizeMode_Clip);
	UI::IDButton<PriorityButtonHelper, int> * button =
	 new UI::IDButton<PriorityButtonHelper, int>
	  (box,
	   x, 0, w, h,
	   4,
	   pic_enabled,
	   &PriorityButtonHelper::button_clicked, &helper, priority,
	   button_tooltip, true, true);
	helper[priority] = PriorityButtonInfo(button, pic_enabled, pic_disabled);
	return button;
}

void ProductionSite_Window::create_ware_queue_panel(UI::Box* box, ProductionSite * ps, WaresQueue * const wq)
{
	const int priority_buttons_width = WaresQueueDisplay::Height / 3;
	UI::Box* hbox = new UI::Box (box, 0, 0, UI::Box::Horizontal);
	WaresQueueDisplay* wqd = new WaresQueueDisplay(hbox, 0, 0,
												   get_w() - priority_buttons_width,
												   wq, m_parent->get_game());

	hbox->add(wqd, UI::Box::AlignTop);

	if (wq->get_ware() >= 0) {
		m_priority_helpers.push_back (PriorityButtonHelper(ps, Request::WARE, wq->get_ware()));
		PriorityButtonHelper & helper = m_priority_helpers.back();

		UI::Box* vbox = new UI::Box (hbox, 0, 0, UI::Box::Vertical);
		// Add priority buttons
		vbox->add
			(create_priority_button (vbox, helper, HIGH_PRIORITY, 0, 0,
									 priority_buttons_width,
									 priority_buttons_width,
									 pic_priority_high,
									 pic_priority_high_on,
									 _("Highest priority")),
			 UI::Box::AlignTop);
		vbox->add
			(create_priority_button (vbox, helper, DEFAULT_PRIORITY, 0, 0,
									 priority_buttons_width,
									 priority_buttons_width,
									 pic_priority_normal,
									 pic_priority_normal_on,
									 _("Normal priority")),
			 UI::Box::AlignTop);
		vbox->add
			(create_priority_button (vbox, helper, LOW_PRIORITY, 0, 0,
									 priority_buttons_width,
									 priority_buttons_width,
									 pic_priority_low,
									 pic_priority_low_on,
									 _("Lowest priority")),
			 UI::Box::AlignTop);

		hbox->add(vbox, UI::Box::AlignCenter);
		helper.update_buttons();
	}

	box->add(hbox, UI::Box::AlignLeft);
}

UI::Box*
ProductionSite_Window::create_production_box (UI::Panel* parent, ProductionSite* ps)
{
	UI::Box* box = new UI::Box (parent, 0, 0, UI::Box::Vertical);

	// Add the wares queue
	std::vector<WaresQueue*>* warequeues=ps->get_warequeues();
	for (uint i = 0; i < warequeues->size(); i++) {
		create_ware_queue_panel (box, ps, (*warequeues)[i]);
	}

	box->add_space(8);

	// Add caps buttons
	box->add(create_capsbuttons(box), UI::Box::AlignLeft);

	// Add list worker button
	box->add
		(new UI::Button<ProductionSite_Window>
		 (box,
		  0, 0, 32, 32,
		  4,
		  g_gr->get_picture(PicMod_Game,  pic_list_worker),
		  &ProductionSite_Window::list_worker_clicked, this,
		  _("Show worker listing")),
		 UI::Box::AlignLeft);

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
void ProductionSite_Window::list_worker_clicked() {
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
UI::Window* ProductionSite::create_options_window(Interactive_Player* plr, UI::Window** registry)
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
	MilitarySite_Window(Interactive_Player* parent, MilitarySite* ps, UI::Window** registry);
	virtual ~MilitarySite_Window();

	inline MilitarySite* get_militarysite() {return (MilitarySite*)get_building();}

	virtual void think();
private:
   void update();
	void drop_button_clicked ();
	void soldier_capacity_up () {act_change_soldier_capacity (1);}
	void soldier_capacity_down() {act_change_soldier_capacity(-1);}

   Coords          m_ms_location;
   Interactive_Player* m_parent;
   UI::Window** m_reg;
   UI::Table<Soldier &> * m_table;
	UI::Textarea         * m_capacity;
};


/*
===============
MilitarySite_Window::MilitarySite_Window

Create the window and its panels, add it to the registry.
===============
*/
MilitarySite_Window::MilitarySite_Window(Interactive_Player* parent, MilitarySite* ps, UI::Window** registry)
	: Building_Window(parent, ps, registry)
{
   m_parent=parent;
   m_reg=registry;
   m_ms_location=ps->get_position();

	UI::Box* box = new UI::Box(this, 0, 0, UI::Box::Vertical);

   // Soldiers view
	m_table=new UI::Table<Soldier &>(box, 0, 0, 360, 200);
	m_table->add_column(_("Name"),  100);
	m_table->add_column(_("HP"),     40);
	m_table->add_column(_("AT"),     40);
	m_table->add_column(_("DE"),     40);
	m_table->add_column(_("EV"),     40);
	m_table->add_column(_("Level"), 100); // enough space for scrollbar

   box->add(m_table, UI::Box::AlignCenter);

	// Add drop soldier button
	box->add_space(8);
	box->add
		(new UI::Button<MilitarySite_Window>
		 (box,
		  0, 0, 360, 32,
		  4,
		  g_gr->get_picture(PicMod_Game, pic_drop_soldier),
		  &MilitarySite_Window::drop_button_clicked, this),
		 UI::Box::AlignLeft);
	box->add_space(8);

	UI::Panel* pan = new UI::Panel(box, 0, 34, Width + 134, 34);

	// Add the caps button
	create_capsbuttons(pan);

		new UI::Textarea (pan, 104, 11, _("Capacity"), Align_Left);
	// Capacity buttons
	new UI::Button<MilitarySite_Window>
		(pan,
		 174, 4, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_Game, pic_down_train),
		 &MilitarySite_Window::soldier_capacity_down, this);
	new UI::Button<MilitarySite_Window>
		(pan,
		 222, 4, 24, 24,
		 4,
		 g_gr->get_picture(PicMod_Game, pic_up_train),
		 &MilitarySite_Window::soldier_capacity_up, this);
	m_capacity =new UI::Textarea (pan, 204, 11, "XX", Align_Center);

	box->add(pan, UI::Box::AlignLeft);


   fit_inner(box);
   move_inside_parent();
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

	const BaseImmovable * const base_immovable =
		m_parent->egbase().map()[m_ms_location].get_immovable();
	if
		(not dynamic_cast<const Building *>(base_immovable)
		 or
		 dynamic_cast<const ConstructionSite *>(base_immovable))
	{
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
FIXME What if a soldier has been removed and another added? This needs review.
soldiers
=============
*/
void MilitarySite_Window::update() {
	const std::vector<Soldier *> & soldiers = get_militarysite()->get_soldiers();

   char buf[200];
	if (soldiers.size() < m_table->size()) m_table->clear();

	for (uint i = 0; i < soldiers.size(); ++i) {
		Soldier & s = *soldiers[i] ;
		UI::Table<Soldier &>::Entry_Record * er = m_table->find(s);
		if (not er)                          er = &m_table->add(s);
		const uint hl = s.get_hp_level     (), mhl = s.get_max_hp_level     ();
		const uint al = s.get_attack_level (), mal = s.get_max_attack_level ();
		const uint dl = s.get_defense_level(), mdl = s.get_max_defense_level();
		const uint el = s.get_evade_level  (), mel = s.get_max_evade_level  ();
		er->set_string(0, s.name().c_str());
		sprintf(buf, "%i / %i", hl, mhl);
		er->set_string(1, buf);
		sprintf(buf, "%i / %i", al, mal);
		er->set_string(2, buf);
		sprintf(buf, "%i / %i", dl, mdl);
		er->set_string(3, buf);
		sprintf(buf, "%i / %i", el, mel);
		er->set_string(4, buf);
		sprintf(buf, "%i / %i", hl + al + dl + el, mhl + mel + mal + mdl);
		er->set_string(5, buf);
	}
   m_table->sort();

	std::string str;
	sprintf (buf, "%2d", ((MilitarySite *)get_building())->get_capacity());
	str = (const char *) buf;
	m_capacity->set_text (str);
}

void MilitarySite_Window::drop_button_clicked() {
	assert(*m_reg== this);
	if //  FIXME should be assert
		(m_table->selection_index()
		 <
		 get_militarysite()->get_soldiers().size())
		act_drop_soldier(m_table->get_selected().get_serial());
}

/*
===============
MilitarySite::create_options_window

Create the production site information window.
===============
*/
UI::Window* MilitarySite::create_options_window(Interactive_Player* plr, UI::Window** registry)
{
	return new MilitarySite_Window(plr, this, registry);
}

/*
====================
	TrainingSite_Options_Window Implementation
====================
*/
class TrainingSite_Options_Window : public UI::Window {
public:
	TrainingSite_Options_Window(Interactive_Player* parent, TrainingSite* ps);
	virtual ~TrainingSite_Options_Window();

	inline TrainingSite* get_trainingsite() {return m_trainingsite;}

	void think();
private:
	void heros_clicked        () {get_trainingsite ()->switch_heros ();}
	void up_hp_clicked        () {act_change_priority (atrHP,       1);}
	void up_attack_clicked    () {act_change_priority (atrAttack,   1);}
	void up_defense_clicked   () {act_change_priority (atrDefense,  1);}
	void up_evade_clicked     () {act_change_priority (atrEvade,    1);}
	void down_hp_clicked      () {act_change_priority (atrHP,      -1);}
	void down_attack_clicked  () {act_change_priority (atrAttack,  -1);}
	void down_defense_clicked () {act_change_priority (atrDefense, -1);}
	void down_evade_clicked   () {act_change_priority (atrEvade,   -1);}

	void act_change_priority (int atr, int how);

	void update();

	Coords               m_ms_location;
	Interactive_Player* m_parent;
	UI::Window *       * m_reg;
	UI::Textarea       * m_style_train;
	UI::Textarea       * m_hp_pri;
	UI::Textarea       * m_attack_pri;
	UI::Textarea       * m_defense_pri;
	UI::Textarea       * m_evade_pri;
	TrainingSite       * m_trainingsite;
};

TrainingSite_Options_Window::TrainingSite_Options_Window(Interactive_Player* parent, TrainingSite* ps)
	: UI::Window(parent, 0, 0, 320, 125, _("Training Options").c_str()) {

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
	move_inside_parent();

	// TODO: Put the capacity buttons here.

	// Add switch training mode button
	new UI::Button<TrainingSite_Options_Window>
			(this,
			 _cb, _bs, 105, _bs,
			 4,
			 0, // FIXME icon?
			 &TrainingSite_Options_Window::heros_clicked, this);

	new UI::Textarea(this, _cn - 15, _bs + 2, _("Training mode : "), Align_Left);
	m_style_train = new UI::Textarea (this, _cb + 4, _bs+2, _("Balanced"), Align_Left);


	m_hp_pri      = new UI::Textarea (this, _cb + 3 * _bs / 2, 3 + (3 + _bs) * 2, "XX", Align_Center);
	m_attack_pri  = new UI::Textarea (this, _cb + 3 * _bs / 2, 3 + (3 + _bs) * 3, "XX", Align_Center);
	m_defense_pri = new UI::Textarea (this, _cb + 3 * _bs / 2, 3 + (3 + _bs) * 4, "XX", Align_Center);
	m_evade_pri   = new UI::Textarea (this, _cb + 3 * _bs / 2, 3 + (3 + _bs) * 5, "XX", Align_Center);

	m_hp_pri->set_visible(false);
	m_attack_pri->set_visible(false);
	m_defense_pri->set_visible(false);
	m_evade_pri->set_visible(false);

	// Add priority buttons for every attribute
	const TrainingSite_Descr & ts_descr = ps->descr();
	if (ts_descr.get_train_hp()) {
		// HP buttons
		new UI::Button<TrainingSite_Options_Window>
			(this,
			 _cb, 2 * (_bs + 2), _bs, _bs,
			 4,
			 g_gr->get_picture(PicMod_Game, pic_down_train),
			 &TrainingSite_Options_Window::down_hp_clicked, this);
		new UI::Button<TrainingSite_Options_Window>
			(this,
			 _cb + 2 * _bs, 2 * (_bs + 2), _bs, _bs,
			 4,
			 g_gr->get_picture(PicMod_Game, pic_up_train),
			 &TrainingSite_Options_Window::up_hp_clicked, this);
		new UI::Textarea (this, _cn, (3+_bs)*2, _("Hit Points"), Align_Left);
		m_hp_pri->set_visible(true);
	}
	if (ts_descr.get_train_attack()) {
		// Attack buttons
		new UI::Button<TrainingSite_Options_Window>
			(this,
			 _cb, 3 * (_bs + 2), _bs, _bs,
			 4,
			 g_gr->get_picture(PicMod_Game, pic_down_train),
			 &TrainingSite_Options_Window::down_attack_clicked, this);
		new UI::Button<TrainingSite_Options_Window>
			(this,
			 _cb + 2 * _bs, 3 * (_bs + 2), _bs, _bs,
			 4,
			 g_gr->get_picture(PicMod_Game, pic_up_train),
			 &TrainingSite_Options_Window::up_attack_clicked, this);
		new UI::Textarea (this, _cn, (3+_bs)*3, _("Attack"), Align_Left);
		m_attack_pri->set_visible(true);
	}
	if (ts_descr.get_train_defense()) {
		// Defense buttons
		new UI::Button<TrainingSite_Options_Window>
			(this,
			 _cb, 4 * (_bs + 2), _bs, _bs,
			 4,
			 g_gr->get_picture(PicMod_Game, pic_down_train),
			 &TrainingSite_Options_Window::down_defense_clicked, this);
		new UI::Button<TrainingSite_Options_Window>
			(this,
			 _cb + 2 * _bs, 4 * (_bs + 2), _bs, _bs,
			 4,
			 g_gr->get_picture(PicMod_Game, pic_up_train),
			 &TrainingSite_Options_Window::up_defense_clicked, this);
		new UI::Textarea (this, _cn, (3+_bs)*4, _("Defense"), Align_Left);
		m_defense_pri->set_visible(true);
	}
	if (ts_descr.get_train_evade()) {
		// Evade buttons
		new UI::Button<TrainingSite_Options_Window>
			(this,
			 _cb, 5 * (_bs + 2), _bs, _bs,
			 4,
			 g_gr->get_picture(PicMod_Game, pic_down_train),
			 &TrainingSite_Options_Window::down_evade_clicked, this);
		new UI::Button<TrainingSite_Options_Window>
			(this,
			 _cb + 2 * _bs,
			 5 * (_bs + 2), _bs, _bs,
			 4,
			 g_gr->get_picture(PicMod_Game, pic_up_train),
			 &TrainingSite_Options_Window::up_evade_clicked, this);
		new UI::Textarea (this, _cn, (3+_bs)*5, _("Evade"), Align_Left);
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
	//Building_Window::think();

	const BaseImmovable * const base_immovable =
		m_parent->egbase().map()[m_ms_location].get_immovable();
	if
		(not dynamic_cast<const Building *>(base_immovable)
		 or
		 dynamic_cast<const ConstructionSite *>(base_immovable))
	{
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

struct TrainingSite_Window : public ProductionSite_Window {
	TrainingSite_Window(Interactive_Player* parent, TrainingSite* ps, UI::Window** registry);
	virtual ~TrainingSite_Window();

	inline TrainingSite* get_trainingsite() {return (TrainingSite*)get_building();}

	virtual void think();
	void options_button_clicked ();
	void drop_button_clicked ();
	void soldier_capacity_up () {act_change_soldier_capacity (1);}
	void soldier_capacity_down() {act_change_soldier_capacity(-1);}
   UI::Box* create_military_box (UI::Panel*);
private:
	void update();

   void add_tab(const char* picname, UI::Panel* panel);
//   void add_button(UI::Box* box, const char* picname, void (FieldActionWindow::*fn)());

   Coords              m_ms_location;
   Interactive_Player* m_parent;
   UI::Window**          m_reg;
   UI::Table<Soldier &> * m_table;
   UI::Textarea*         m_capacity;

   UI::Tab_Panel*         m_tabpanel;

};


/*
===============
TrainingSite_Window::TrainingSite_Window

Create the window and its panels, add it to the registry.
===============
*/
TrainingSite_Window::TrainingSite_Window(Interactive_Player* parent, TrainingSite* ms, UI::Window** registry)
	: ProductionSite_Window(parent, ms, registry)
{
   m_parent = parent;
   m_reg = registry;
   m_ms_location = ms->get_position ();

   m_tabpanel = new UI::Tab_Panel(this, 0, 0, 1);
   m_tabpanel->set_snapparent(true);

	// Training Box (wares and buttons related to them)
   UI::Box* prod_box = create_production_box (m_tabpanel, ms);
   prod_box->resize();
   add_tab(pic_tab_training, prod_box);

	// Military Box (Soldiers and buttons related to them)
	// Training Box (wares and buttons related to them)
   UI::Box* train_box = create_military_box (m_tabpanel);
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

UI::Box* TrainingSite_Window::create_military_box (UI::Panel* panel)
{
   UI::Box* sold_box = new UI::Box (panel, 0, 0, UI::Box::Vertical);

      // Soldiers view
	m_table = new UI::Table<Soldier &>(sold_box, 0, 0, 360, 200);
	m_table->add_column(_("Name"),  100);
	m_table->add_column(_("HP"),     40);
	m_table->add_column(_("AT"),     40);
	m_table->add_column(_("DE"),     40);
	m_table->add_column(_("EV"),     40);
	m_table->add_column(_("Level"), 100); // enough space for scrollbar
   sold_box->add (m_table, Align_Left);

      // Add drop soldier button
   sold_box->add
		(new UI::Button<TrainingSite_Window>
		 (sold_box,
		  0, 0, 360, 32,
		  4,
		  g_gr->get_picture(PicMod_Game, pic_drop_soldier),
		  &TrainingSite_Window::drop_button_clicked, this),
		 Align_Left);

      // Add TrainingSite Options and Capacity  buttons
   UI::Box* box = new UI::Box (sold_box, 0, 0, UI::Box::Horizontal);
   box->add
		(new UI::Button<TrainingSite_Window>
		 (box,
		  32, 0, 32, 32,
		  4,
		  g_gr->get_picture(PicMod_Game, pic_train_options),
		  &TrainingSite_Window::options_button_clicked, this),
		 Align_Top);

   box->add (new UI::Textarea (box, 0, 11, _("Capacity"), Align_Left), Align_Left);
      // Capacity buttons
	box->add
		(new UI::Button<TrainingSite_Window>
		 (box,
		  70, 4, 24, 24,
		  4,
		  g_gr->get_picture(PicMod_Game, pic_down_train),
		  &TrainingSite_Window::soldier_capacity_down, this),
		 Align_Top);

   m_capacity = new UI::Textarea (box, 0, 11, _("xx"), Align_Center);
   box->add (m_capacity, Align_Top);

	box->add
		(new UI::Button<TrainingSite_Window>
		 (box,
		  118, 4, 24, 24,
		  4,
		  g_gr->get_picture(PicMod_Game, pic_up_train),
		  &TrainingSite_Window::soldier_capacity_up, this),
		 Align_Top);
   sold_box->add (box, Align_Left);

   return sold_box;
}

/*
===============
TrainingSite_Window::add_tab

Convenience function: Adds a new tab to the main tab panel
===============
*/
void TrainingSite_Window::add_tab(const char* picname, UI::Panel* panel)
{
	m_tabpanel->add(g_gr->get_picture(PicMod_Game,   picname), panel);
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

	const BaseImmovable * const base_immovable =
		m_parent->egbase().map()[m_ms_location].get_immovable();
	if
		(not dynamic_cast<const Building *>(base_immovable)
		 or
		 dynamic_cast<const ConstructionSite *>(base_immovable))
	{
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
FIXME What if a soldier have been removed and another added? This needs review.
=============
*/
void TrainingSite_Window::update() {
	const std::vector<Soldier*> & soldiers = get_trainingsite()->get_soldiers();

	char buffer[200];
	if (soldiers.size() != m_table->size()) m_table->clear();

	for (uint i = 0; i < soldiers.size(); ++i) {
		Soldier & s = *soldiers[i] ;
		UI::Table<Soldier &>::Entry_Record * er = m_table->find(s);
		if (not er)                           er = &m_table->add(s);
		const uint hl = s.get_hp_level     (), mhl = s.get_max_hp_level     ();
		const uint al = s.get_attack_level (), mal = s.get_max_attack_level ();
		const uint dl = s.get_defense_level(), mdl = s.get_max_defense_level();
		const uint el = s.get_evade_level  (), mel = s.get_max_evade_level  ();
		er->set_string(0, s.name().c_str());
		snprintf(buffer, sizeof(buffer), "%i / %i", hl, mhl);
		er->set_string(1, buffer);
		snprintf(buffer, sizeof(buffer), "%i / %i", al, mal);
		er->set_string(2, buffer);
		snprintf(buffer, sizeof(buffer), "%i / %i", dl, mdl);
		er->set_string(3, buffer);
		snprintf(buffer, sizeof(buffer), "%i / %i", el, mel);
		er->set_string(4, buffer);
		snprintf
			(buffer, sizeof(buffer),
			 "%i / %i", hl + al + dl + el, mhl + mel + mal + mdl);
		er->set_string(5, buffer);
	}
	m_table->sort();

	snprintf
		(buffer, sizeof(buffer),
		 "%2d",
		 dynamic_cast<const TrainingSite &>(*get_building()).get_capacity());
	m_capacity->set_text (buffer);
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
	if
		(m_table->selection_index()
		 <
		 get_trainingsite()->get_soldiers().size())
		act_drop_soldier(m_table->get_selected().get_serial());
}

/*
===============
TrainingSite::create_options_window

Create the training site information window.
===============
*/
UI::Window* TrainingSite::create_options_window(Interactive_Player* plr, UI::Window** registry)
{
	return new TrainingSite_Window(plr, this, registry);
}
