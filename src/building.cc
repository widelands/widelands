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

#include "widelands.h"
#include "profile.h"
#include "tribedata.h"
#include "tribe.h"
#include "game.h"
#include "player.h"
#include "transport.h"

#include "interactive_base.h"
#include "building_int.h"


#define BUILDING_LEAVE_INTERVAL		1000
#define CARRIER_SPAWN_INTERVAL		2500
#define CONSTRUCTIONSITE_STEP_TIME	30000


/*
==============================================================================

Basic building

==============================================================================
*/

/*
===============
Building_Descr::Building_Descr

Initialize with sane defaults
===============
*/
Building_Descr::Building_Descr(Tribe_Descr *tribe, const char *name)
{
	m_tribe = tribe;
	snprintf(m_name, sizeof(m_name), "%s", name);
	strcpy(m_descname, m_name);
	m_buildable = true;
	m_buildicon = 0;
	m_buildicon_fname = 0;
	m_size = BaseImmovable::SMALL;
	m_mine = false;
}


/*
===============
Building_Descr::~Building_Descr

Cleanup
===============
*/
Building_Descr::~Building_Descr(void)
{
	if (m_buildicon_fname)
		free(m_buildicon_fname);
}

/*
===============
Building_Descr::create

Create a building of this type. Does not perform any sanity checks.
===============
*/
Building *Building_Descr::create(Editor_Game_Base *g, Player *owner, Coords pos, bool construct)
{
	assert(owner);

	Building *b = construct ? create_constructionsite() : create_object();
	b->set_owner(owner);
	b->m_position = pos;
	b->init(g);

	return b;
}

/*
===============
Building_Descr::parse

Parse the basic building settings from the given profile and directory
===============
*/
void Building_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Section *global = prof->get_safe_section("global");
	Section* s;
	const char *string;
	char buf[256];
	char fname[256];

	snprintf(m_descname, sizeof(m_descname), "%s", global->get_safe_string("descname"));

	string = global->get_safe_string("size");
	if (!strcasecmp(string, "small")) {
		m_size = BaseImmovable::SMALL;
	} else if (!strcasecmp(string, "medium")) {
		m_size = BaseImmovable::MEDIUM;
	} else if (!strcasecmp(string, "big")) {
		m_size = BaseImmovable::BIG;
	} else if (!strcasecmp(string, "mine")) {
		m_size = BaseImmovable::SMALL;
		m_mine = true;
	} else
		throw wexception("Section [global], unknown size '%s'. Valid values are small, medium, big, mine",
		                 string);

	// Parse build options
	m_buildable = global->get_bool("buildable", true);

	if (m_buildable)
		{
		// Get build icon
		snprintf(buf, sizeof(buf), "%s_build.bmp", m_name);
		string = global->get_string("buildicon", buf);

		snprintf(fname, sizeof(fname), "%s/%s", directory, string);

		m_buildicon_fname = strdup(fname);

		// Get costs
		s = prof->get_safe_section("buildcost");

		Section::Value* val;

		while((val = s->get_next_val(0)))
			m_buildcost.push_back(CostItem(val->get_name(), val->get_int()));
		}


	// Parse basic animation data
	s = prof->get_section("idle");
	if (!s)
		throw wexception("Missing idle animation");
	m_idle = g_anim.get(directory, s, 0, encdata);
}


/*
===============
Building_Descr::load_graphics

Called whenever building graphics need to be loaded.
===============
*/
void Building_Descr::load_graphics()
{
	if (m_buildicon_fname)
		m_buildicon = g_gr->get_picture(PicMod_Game, m_buildicon_fname, RGBColor(0,0,255));
}

/*
===============
Building_Descr::create_constructionsite

Create a construction site for this type of building
===============
*/
Building* Building_Descr::create_constructionsite()
{
	Building_Descr* descr = m_tribe->get_building_descr(m_tribe->get_building_index("constructionsite"));
	if (!descr)
		throw wexception("Tribe %s has no constructionsite", m_tribe->get_name());

	ConstructionSite* csite = (ConstructionSite*)descr->create_object();

	csite->set_building(this);

	return csite;
}


/*
==============================

Implementation

==============================
*/

Building::Building(Building_Descr *descr)
	: PlayerImmovable(descr)
{
	m_flag = 0;
	m_optionswindow = 0;
}

Building::~Building()
{
	if (m_optionswindow)
		hide_options();
}

/*
===============
Building::get_type
Building::get_size
Building::get_passable
Building::get_base_flag
===============
*/
int Building::get_type()
{
	return BUILDING;
}

int Building::get_size()
{
	return get_descr()->get_size();
}

bool Building::get_passable()
{
	return false;
}

Flag *Building::get_base_flag()
{
	return m_flag;
}


/*
===============
Building::get_playercaps

Return a bitfield of commands the owning player can issue for this building.
The bits are (1 << PCap_XXX).
By default, all buildable buildings can be bulldozed.
===============
*/
uint Building::get_playercaps()
{
	uint caps = 0;

	if (get_descr()->get_buildable())
		caps |= 1 << PCap_Bulldoze;

	return caps;
}


/*
===============
Building::start_animation

Start the given animation
===============
*/
void Building::start_animation(Editor_Game_Base *g, uint anim)
{
	m_anim = anim;
	m_animstart = g->get_gametime();
}

/*
===============
Building::init

Common building initialization code. You must call this from derived class' init.
===============
*/
void Building::init(Editor_Game_Base* g)
{
	PlayerImmovable::init(g);

	// Set the building onto the map
	Map *map = g->get_map();
	Coords neighb;

	set_position(g, m_position);

	if (get_size() == BIG) {
		map->get_ln(m_position, &neighb);
		set_position(g, neighb);

		map->get_tln(m_position, &neighb);
		set_position(g, neighb);

		map->get_trn(m_position, &neighb);
		set_position(g, neighb);
	}

	// Make sure the flag is there
	BaseImmovable *imm;
	Flag *flag;

	map->get_brn(m_position, &neighb);
	imm = map->get_immovable(neighb);

	if (imm && imm->get_type() == FLAG)
		flag = (Flag *)imm;
	else
		flag = Flag::create(g, get_owner(), neighb);

	m_flag = flag;
	m_flag->attach_building(g, this);

	// Start the animation
	start_animation(g, get_descr()->get_idle_anim());

	m_leave_time = g->get_gametime();
}

/*
===============
Building::cleanup

Cleanup the building
===============
*/
void Building::cleanup(Editor_Game_Base *g)
{
	// Remove from flag
	m_flag->detach_building(g);

	// Unset the building
   unset_position(g, m_position);

	if (get_size() == BIG) {
		Map *map = g->get_map();
		Coords neighb;

		map->get_ln(m_position, &neighb);
		unset_position(g, neighb);

		map->get_tln(m_position, &neighb);
		unset_position(g, neighb);

		map->get_trn(m_position, &neighb);
		unset_position(g, neighb);
	}

	PlayerImmovable::cleanup(g);
}


/*
===============
Building::burn_on_destroy [virtual]

Return true if a fire should be created when the building is destroyed.
By default, burn always.
===============
*/
bool Building::burn_on_destroy()
{
	return true;
}


/*
===============
Building::destroy

Remove the building from the world now, and create a fire in its place if
applicable.
===============
*/
void Building::destroy(Editor_Game_Base* g)
{
	Coords pos = m_position;
	bool fire = burn_on_destroy();

	PlayerImmovable::destroy(g);

	// We are deleted. Only use stack variables beyond this point
	if (fire)
		g->create_immovable(pos, "fire");
}


/*
===============
Building::get_ui_anim [virtual]

Return the animation ID that is used for the building in UI items
(the building UI, messages, etc..)
===============
*/
uint Building::get_ui_anim()
{
	return get_descr()->get_idle_anim();
}


/*
===============
Building::get_census_string [virtual]

Return the overlay string that is displayed on the map view when
enabled by the player.

Default is the descriptive name of the building, but e.g. construction
sites may want to override this.
===============
*/
std::string Building::get_census_string()
{
	return get_descname();
}


/*
===============
Building::get_statistics_string [virtual]

Return the overlay string that is displayed on the map view when enabled
by the player.

By default, there is no such string. Production buildings will want to
override this with a percentage indicating how well the building works, etc.
===============
*/
std::string Building::get_statistics_string()
{
	return "";
}


/*
===============
Building::get_building_work [virtual]

This function is called by workers in the buildingwork task.
Give the worker w a new task.
success is true if the previous task was finished successfully (without a
signal).
Return false if there's nothing to be done.
===============
*/
bool Building::get_building_work(Game* g, Worker* w, bool success)
{
	throw wexception("MO(%u): get_building_work() for unknown worker %u", get_serial(), w->get_serial());
}


/*
===============
Building::leave_check_and_wait

Return true if the given worker can leave the building immediately.
Otherwise, return false. The worker's wakeup_leave_building() will be called as
soon as the worker can leave the building.
===============
*/
bool Building::leave_check_and_wait(Game* g, Worker* w)
{
	Map_Object* allow = m_leave_allow.get(g);

	molog("Building::leave_check_and_wait\n");

	if (w == allow) {
		m_leave_allow = 0;
		return true;
	}

	// Check time and queue
	uint time = g->get_gametime();

	if (!m_leave_queue.size())
	{
		if ((int)(time - m_leave_time) >= 0) {
			molog("Building::leave_check_and_wait: Leave now\n");
			m_leave_time = time + BUILDING_LEAVE_INTERVAL;
			return true;
		}

		schedule_act(g, m_leave_time - time);
	}

	molog("Building::leave_check_and_wait: Put on queue\n");

	m_leave_queue.push_back(w);
	return false;
}


/*
===============
Building::act

Advance the leave queue.
===============
*/
void Building::act(Game *g, uint data)
{
	uint time = g->get_gametime();

	if ((int)(time - m_leave_time) >= 0)
	{
		bool wakeup = false;

		// Wake up one worker
		while(m_leave_queue.size())
		{
			Worker* w = (Worker*)m_leave_queue[0].get(g);

			m_leave_queue.erase(m_leave_queue.begin());

			if (!w)
				continue;

			m_leave_allow = w;

			if (w->wakeup_leave_building(g, this)) {
				m_leave_time = time + BUILDING_LEAVE_INTERVAL;
				wakeup = true;
				break;
			}
		}

		if (m_leave_queue.size())
			schedule_act(g, m_leave_time - time);

		if (!wakeup)
			m_leave_time = time; // make sure leave_time doesn't get too far behind
	}

	PlayerImmovable::act(g, data);
}


/*
===============
Building::fetch_from_flag [virtual]

This function is called by our base flag to indicate that some item on the
flag wants to move into this building.
Return true if we can service that request (even if it is delayed), or false
otherwise.
===============
*/
bool Building::fetch_from_flag(Game* g)
{
	molog("TODO: Implement Building::fetch_from_flag\n");

	return false;
}


/*
===============
Building::draw

Draw the building.
===============
*/
void Building::draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos)
{
	if (coords != m_position)
		return; // draw big buildings only once

	dst->drawanim(pos.x, pos.y, m_anim, game->get_gametime() - m_animstart, get_owner()->get_playercolor());

	// door animation?

	// Overlay strings (draw when enabled)
	draw_help(game, dst, coords, pos);
}


/*
===============
Building::draw_help

Draw overlay help strings when enabled.
===============
*/
void Building::draw_help(Editor_Game_Base* game, RenderTarget* dst, FCoords coords, Point pos)
{
	uint dpyflags = game->get_iabase()->get_display_flags();

	if (dpyflags & Interactive_Base::dfShowCensus)
	{
		std::string txt = get_census_string();

		g_font->draw_string(dst, pos.x, pos.y - 45, txt.c_str(), Align_Center);
	}

	if (dpyflags & Interactive_Base::dfShowStatistics)
	{
		std::string txt = get_statistics_string();

		g_font->draw_string(dst, pos.x, pos.y - 35, txt.c_str(), Align_Center);
	}
}


/*
==============================================================================

ConstructionSite BUILDING

==============================================================================
*/

/*
===============
ConstructionSite_Descr::ConstructionSite_Descr
===============
*/
ConstructionSite_Descr::ConstructionSite_Descr(Tribe_Descr* tribe, const char* name)
	: Building_Descr(tribe, name)
{
}


/*
===============
ConstructionSite_Descr::parse

Parse tribe-specific construction site data, such as graphics, worker type,
etc...
===============
*/
void ConstructionSite_Descr::parse(const char* directory, Profile* prof, const EncodeData* encdata)
{
	Building_Descr::parse(directory, prof, encdata);

	// TODO
}


/*
===============
ConstructionSite_Descr::create_object

Allocate a ConstructionSite
===============
*/
Building* ConstructionSite_Descr::create_object()
{
	return new ConstructionSite(this);
}


/*
==============================

IMPLEMENTATION

==============================
*/


/*
===============
ConstructionSite::ConstructionSite

Initialize with default values
===============
*/
ConstructionSite::ConstructionSite(ConstructionSite_Descr* descr)
	: Building(descr)
{
	m_building = 0;

	m_builder = 0;
	m_builder_request = 0;

	m_fetchfromflag = 0;

	m_working = false;
	m_work_steptime = 0;
	m_work_completed = 0;
	m_work_steps = 0;
}


/*
===============
ConstructionSite::~ConstructionSite
===============
*/
ConstructionSite::~ConstructionSite()
{
}


/*
===============
ConstructionSite::get_size

Override: construction size is always the same size as the building
===============
*/
int ConstructionSite::get_size()
{
	return m_building->get_size();
}


/*
===============
ConstructionSite::get_playercaps

Override: Even though construction sites cannot be built themselves, you can
bulldoze them.
===============
*/
uint ConstructionSite::get_playercaps()
{
	uint caps = Building::get_playercaps();

	caps |= 1 << PCap_Bulldoze;

	return caps;
}


/*
===============
ConstructionSite::get_ui_anim

Return the animation for the building that is in construction, as this
should be more useful to the player.
===============
*/
uint ConstructionSite::get_ui_anim()
{
	return get_building()->get_idle_anim();
}


/*
===============
ConstructionSite::get_census_string

Print the name of the building we build.
===============
*/
std::string ConstructionSite::get_census_string()
{
	return get_building()->get_descname();
}


/*
===============
ConstructionSite::get_statistics_string

Print completion percentage.
===============
*/
std::string ConstructionSite::get_statistics_string()
{
	char buf[40];

	snprintf(buf, sizeof(buf), "%u%% built", (get_built_per64k() * 100) >> 16);

	return std::string(buf);
}


/*
===============
ConstructionSite::get_built_per64k

Return the completion "percentage", where 2^16 = completely built,
0 = nothing built.
===============
*/
uint ConstructionSite::get_built_per64k()
{
	uint time = get_owner()->get_game()->get_gametime();
	uint thisstep = m_working ? (CONSTRUCTIONSITE_STEP_TIME - m_work_steptime + time) : 0;
	uint total;

	thisstep = (thisstep << 16) / CONSTRUCTIONSITE_STEP_TIME;
	total = (thisstep + (m_work_completed << 16)) / m_work_steps;

	assert(thisstep >= 0 && thisstep <= (1 << 16));
	assert(total >= 0 && total <= (1 << 16));

	return total;
}


/*
===============
ConstructionSite::set_building

Set the type of building we're going to build
===============
*/
void ConstructionSite::set_building(Building_Descr* descr)
{
	assert(!m_building);

	m_building = descr;
}


/*
===============
ConstructionSite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void ConstructionSite::set_economy(Economy* e)
{
	Economy* old = get_economy();
	uint i;

	if (old) {
		for(i = 0; i < m_wares.size(); i++)
			m_wares[i]->remove_from_economy(old);
	}

	Building::set_economy(e);
	if (m_builder_request)
		m_builder_request->set_economy(e);

	if (e) {
		for(i = 0; i < m_wares.size(); i++)
			m_wares[i]->add_to_economy(e);
	}
}


/*
===============
ConstructionSite::init

Initialize the construction site by starting orders
===============
*/
void ConstructionSite::init(Editor_Game_Base* g)
{
	Building::init(g);

	if (g->is_game()) {
		uint i;

		// TODO: figure out whether planing is necessary

		// Initialize the wares queues
		const Building_Descr::BuildCost* bc = m_building->get_buildcost();

		m_wares.resize(bc->size());

		for(i = 0; i < bc->size(); i++) {
			WaresQueue* wq = new WaresQueue(this);

			m_wares[i] = wq;

			wq->set_callback(&ConstructionSite::wares_queue_callback, this);
			wq->set_consume_interval(CONSTRUCTIONSITE_STEP_TIME);
			wq->init((Game*)g, g->get_safe_ware_id((*bc)[i].name.c_str()), (*bc)[i].amount);

			m_work_steps += (*bc)[i].amount;
		}

		request_builder((Game*)g);
	}
}


/*
===============
ConstructionSite::cleanup

Release worker and material (if any is left).
If construction was finished successfully, place the building at our position.
===============
*/
void ConstructionSite::cleanup(Editor_Game_Base* g)
{
	// Release worker
	if (m_builder_request) {
		delete m_builder_request;
		m_builder_request = 0;
	}

	// Cleanup the wares queues
	for(uint i = 0; i < m_wares.size(); i++) {
		m_wares[i]->cleanup((Game*)g);
		delete m_wares[i];
	}
	m_wares.clear();

	Building::cleanup(g);

	if (m_work_completed >= m_work_steps)
	{
		// Put the real building in place
		Building* bld = m_building->create(g, get_owner(), m_position, false);

		// Walk the builder home safely
		m_builder->reset_tasks((Game*)g);
		m_builder->set_location(bld);
		m_builder->start_task_gowarehouse((Game*)g);
	}
}


/*
===============
ConstructionSite::burn_on_destroy

Construction sites only burn if some of the work has been completed.
===============
*/
bool ConstructionSite::burn_on_destroy()
{
	if (m_work_completed >= m_work_steps)
		return false; // completed, so don't burn

	if (m_work_completed)
		return true;

	return false;
}


/*
===============
ConstructionSite::request_builder

Issue a request for the builder.
===============
*/
void ConstructionSite::request_builder(Game* g)
{
	assert(!m_builder && !m_builder_request);

	m_builder_request = new Request(this, g->get_safe_ware_id("builder"),
	                                &ConstructionSite::request_builder_callback, this);
}


/*
===============
ConstructionSite::request_builder_callback [static]

Called by transfer code when the builder has arrived on site.
===============
*/
void ConstructionSite::request_builder_callback(Game* g, Request* rq, int ware, Worker* w, void* data)
{
	assert(w);

	ConstructionSite* cs = (ConstructionSite*)data;

	cs->m_builder = w;

	delete rq;
	cs->m_builder_request = 0;

	w->start_task_buildingwork(g);
}


/*
===============
ConstructionSite::fetch_from_flag

Remember the item on the flag. The worker will be sent from get_building_work().
===============
*/
bool ConstructionSite::fetch_from_flag(Game* g)
{
	m_fetchfromflag++;

	if (m_builder)
		m_builder->update_task_buildingwork(g);

	return true;
}


/*
===============
ConstructionSite::get_building_work

Called by our builder to get instructions.
===============
*/
bool ConstructionSite::get_building_work(Game* g, Worker* w, bool success)
{
	assert(w == m_builder);

	// Check if one step has completed
	if (m_working) {
		if ((int)(g->get_gametime() - m_work_steptime) < 0) {
			w->start_task_idle(g, w->get_idle_anim(), m_work_steptime - g->get_gametime());
			return true;
		} else {
			molog("ConstructionSite::check_work: step %i completed\n", m_work_completed);

			m_work_completed++;
			if (m_work_completed >= m_work_steps)
				schedule_destroy(g);

			m_working = false;
		}
	}

	// Fetch items from flag
	if (m_fetchfromflag) {
		m_fetchfromflag--;
		w->start_task_fetchfromflag(g);
		return true;
	}

	// Check if we've got wares to consume
	if (m_work_completed < m_work_steps)
	{
		for(uint i = 0; i < m_wares.size(); i++) {
			WaresQueue* wq = m_wares[i];

			if (!wq->get_filled())
				continue;

			molog("ConstructionSite::check_work: wq has %i/%i, begin work\n",
						wq->get_filled(), wq->get_size());

			wq->set_filled(wq->get_filled() - 1);
			wq->set_size(wq->get_size() - 1);
			wq->update(g);

			m_working = true;
			m_work_steptime = g->get_gametime() + CONSTRUCTIONSITE_STEP_TIME;

			w->start_task_idle(g, w->get_idle_anim(), CONSTRUCTIONSITE_STEP_TIME);
			return true;
		}
	}

	return false; // sorry, got no work for you
}


/*
===============
ConstructionSite::wares_queue_callback [static]

Called by WaresQueue code when an item has arrived
===============
*/
void ConstructionSite::wares_queue_callback(Game* g, WaresQueue* wq, int ware, void* data)
{
	ConstructionSite* cs = (ConstructionSite*)data;

	if (!cs->m_working && cs->m_builder)
		cs->m_builder->update_task_buildingwork(g);
}


/*
===============
ConstructionSite::draw

Draw the construction site.
===============
*/
void ConstructionSite::draw(Editor_Game_Base* g, RenderTarget* dst, FCoords coords, Point pos)
{
	uint tanim = g->get_gametime() - m_animstart;

	if (coords != m_position)
		return; // draw big buildings only once

	// Draw the construction site marker
	dst->drawanim(pos.x, pos.y, m_anim, tanim, get_owner()->get_playercolor());

	// Draw the partially finished building
	int totaltime;
	int completedtime;
	int w, h;
	int lines;
	uint anim;

	totaltime = CONSTRUCTIONSITE_STEP_TIME * m_work_steps;
	completedtime = CONSTRUCTIONSITE_STEP_TIME * m_work_completed;

	if (m_working)
		completedtime += CONSTRUCTIONSITE_STEP_TIME + g->get_gametime() - m_work_steptime;

	anim = get_building()->get_idle_anim();
	g_gr->get_animation_size(anim, tanim, &w, &h);

	lines = completedtime * h / totaltime;

	dst->drawanimrect(pos.x, pos.y, anim, tanim, get_owner()->get_playercolor(), 0, h-lines, w, lines);

	// Draw help strings
	draw_help(g, dst, coords, pos);
}


/*
==============================================================================

WarehouseSupply IMPLEMENTATION

==============================================================================
*/

/*
WarehouseSupply is the implementation of Supply that is used by Warehouses.
It also manages the list of wares in the warehouse.
*/
class WarehouseSupply : public Supply {
public:
	WarehouseSupply(Warehouse* wh);
	virtual ~WarehouseSupply();

	void set_economy(Economy* e);

	const WareList &get_wares() const { return m_wares; }
	int stock(int id) const { return m_wares.stock(id); }
	void add_wares(int id, int count);
	void remove_wares(int id, int count);

public: // Supply implementation
	virtual PlayerImmovable* get_position(Game* g);
	virtual int get_amount(Game* g, int ware);
	virtual bool is_active(Game* g);

	virtual WareInstance* launch_item(Game* g, int ware);
	virtual Worker* launch_worker(Game* g, int ware);

private:
	Economy*		m_economy;
	WareList		m_wares;
	Warehouse*	m_warehouse;
};


/*
===============
WarehouseSupply::WarehouseSupply

Initialize the supply
===============
*/
WarehouseSupply::WarehouseSupply(Warehouse* wh)
{
	m_warehouse = wh;
	m_economy = 0;
}


/*
===============
WarehouseSupply::~WarehouseSupply

Destroy the supply.
===============
*/
WarehouseSupply::~WarehouseSupply()
{
	if (m_economy) {
		log("WarehouseSupply::~WarehouseSupply: Warehouse %u still belongs to an economy",
				m_warehouse->get_serial());
		set_economy(0);
	}

	// We're removed from the Economy. Therefore, the wares can simply
	// be cleared out. The global inventory will be okay.
	m_wares.clear();
}


/*
===============
WarehouseSupply::set_economy

Add and remove our wares and the Supply to the economies as necessary.
===============
*/
void WarehouseSupply::set_economy(Economy* e)
{
	if (e == m_economy)
		return;

	if (m_economy) {
		for(int i = 0; i < m_wares.get_nrwareids(); i++) {
			if (m_wares.stock(i)) {
				m_economy->remove_wares(i, m_wares.stock(i));
				m_economy->remove_supply(i, this);
			}
		}
	}

	m_economy = e;

	if (m_economy) {
		for(int i = 0; i < m_wares.get_nrwareids(); i++) {
			if (m_wares.stock(i)) {
				m_economy->add_wares(i, m_wares.stock(i));
				m_economy->add_supply(i, this);
			}
		}
	}
}


/*
===============
WarehouseSupply::add_wares

Add wares and update the economy.
===============
*/
void WarehouseSupply::add_wares(int id, int count)
{
	if (!count)
		return;

	if (!m_wares.stock(id))
		m_economy->add_supply(id, this);

	m_economy->add_wares(id, count);
	m_wares.add(id, count);
}


/*
===============
WarehouseSupply::remove_wares

Remove wares and update the economy.
===============
*/
void WarehouseSupply::remove_wares(int id, int count)
{
	if (!count)
		return;

	m_wares.remove(id, count);
	m_economy->remove_wares(id, count);

	if (!m_wares.stock(id))
		m_economy->remove_supply(id, this);
}


/*
===============
WarehouseSupply::get_position

Return the position of the Supply, i.e. the owning Warehouse.
===============
*/
PlayerImmovable* WarehouseSupply::get_position(Game* g)
{
	return m_warehouse;
}


/*
===============
WarehouseSupply::get_amount

Return our stock of the given ware.
===============
*/
int WarehouseSupply::get_amount(Game* g, int ware)
{
	return m_wares.stock(ware);
}


/*
===============
WarehouseSupply::is_active

Warehouse supplies are never active.
===============
*/
bool WarehouseSupply::is_active(Game* g)
{
	return false;
}


/*
===============
WarehouseSupply::launch_item

Launch a ware as item.
===============
*/
WareInstance* WarehouseSupply::launch_item(Game* g, int ware)
{
	assert(m_wares.stock(ware));

	return m_warehouse->launch_item(g, ware);
}


/*
===============
WarehouseSupply::launch_worker

Launch a ware as worker.
===============
*/
Worker* WarehouseSupply::launch_worker(Game* g, int ware)
{
	assert(m_wares.stock(ware));

	return m_warehouse->launch_worker(g, ware);
}


/*
==============================================================================

Warehouse building

==============================================================================
*/

/*
===============
Warehouse_Descr::Warehouse_Descr

Initialize with sane defaults
===============
*/
Warehouse_Descr::Warehouse_Descr(Tribe_Descr *tribe, const char *name)
	: Building_Descr(tribe, name)
{
	m_subtype = Subtype_Normal;
	m_conquers = 0;
}

/*
===============
Warehouse_Descr::parse

Parse the additional warehouse settings from the given profile and directory
===============
*/
void Warehouse_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	add_attribute(Map_Object::WAREHOUSE);

	Building_Descr::parse(directory, prof, encdata);

	Section *global = prof->get_safe_section("global");
	const char *string;

	string = global->get_safe_string("subtype");
	if (!strcasecmp(string, "HQ")) {
		m_subtype = Subtype_HQ;
	} else if (!strcasecmp(string, "port")) {
		m_subtype = Subtype_Port;
	} else if (!strcasecmp(string, "none")) {
		//
	} else
		throw wexception("Unsupported warehouse subtype '%s'. Possible values: none, HQ, port", string);

	if (m_subtype == Subtype_HQ)
		m_conquers = global->get_int("conquers");
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
Warehouse::Warehouse

Initialize a warehouse (zero contents, etc...)
===============
*/
Warehouse::Warehouse(Warehouse_Descr *descr)
	: Building(descr)
{
	m_supply = new WarehouseSupply(this);
	m_next_carrier_spawn = 0;
}


/*
===============
Warehouse::Warehouse

Cleanup
===============
*/
Warehouse::~Warehouse()
{
	delete m_supply;
}


/*
===============
Warehouse::init

Conquer the land around the HQ on init.
===============
*/
void Warehouse::init(Editor_Game_Base* gg)
{
   Building::init(gg);

   if (get_descr()->get_subtype() == Warehouse_Descr::Subtype_HQ)
      gg->conquer_area(get_owner()->get_player_number(), m_position, get_descr());

	for(int i = 0; i < gg->get_nrwares(); i++) {
		Request* req = new Request(this, i, &Warehouse::idle_request_cb, this);

		req->set_idle(true);

		m_requests.push_back(req);
	}

	if (gg->is_game()) {
      Game* g=static_cast<Game*>(gg);

		m_next_carrier_spawn = schedule_act(g, CARRIER_SPAWN_INTERVAL);
   }
}


/*
===============
Warehouse::cleanup

Destroy the warehouse.
===============
*/
void Warehouse::cleanup(Editor_Game_Base *g)
{
	while(m_requests.size()) {
		Request* req = m_requests[m_requests.size()-1];

		m_requests.pop_back();

		delete req;
	}

   // TODO: un-conquer the area?
	Building::cleanup(g);
}


/*
===============
Warehouse::act

Act regularly to create carriers. According to intelligence, this is some
highly advanced technology. Not only do the settlers have no problems with
birth control, they don't even need anybody to procreate. They must have
built-in DNA samples in those warehouses. And what the hell are they doing,
killing useless tribesmen! The Borg? Or just like Soilent Green?
Or maybe I should just stop writing comments that late at night ;-)
===============
*/
void Warehouse::act(Game *g, uint data)
{
	if (g->get_gametime() - m_next_carrier_spawn >= 0)
	{
		int id = g->get_safe_ware_id("carrier");
		int stock = m_supply->stock(id);
		int tdelta = CARRIER_SPAWN_INTERVAL;

		if (stock < 100) {
			tdelta -= 4*(100 - stock);
			create_wares(id, 1);
		} else if (stock > 100) {
			tdelta -= 4*(stock - 100);
			if (tdelta < 10)
				tdelta = 10;
			destroy_wares(id, 1);
		}

		m_next_carrier_spawn = schedule_act(g, tdelta);
	}

	Building::act(g, data);
}


/*
===============
Warehouse::set_economy

Transfer our registration to the new economy.
===============
*/
void Warehouse::set_economy(Economy *e)
{
	Economy *old = get_economy();

	if (old == e)
		return;

	if (old)
		old->remove_warehouse(this);

	m_supply->set_economy(e);
	Building::set_economy(e);

	for(uint i = 0; i < m_requests.size(); i++)
		m_requests[i]->set_economy(e);

	if (e)
		e->add_warehouse(this);
}


/*
===============
Warehouse::get_wares
===============
*/
const WareList& Warehouse::get_wares() const
{
	return m_supply->get_wares();
}


/*
===============
Warehouse::create_wares

Magically create wares in this warehouse. Updates the economy accordingly.
===============
*/
void Warehouse::create_wares(int id, int count)
{
	assert(get_economy());

	m_supply->add_wares(id, count);
}


/*
===============
Warehouse::destroy_wares

Magically destroy wares.
===============
*/
void Warehouse::destroy_wares(int id, int count)
{
	assert(get_economy());

	m_supply->remove_wares(id, count);
}


/*
===============
Warehouse::fetch_from_flag

Launch a carrier to fetch an item from our flag.
===============
*/
bool Warehouse::fetch_from_flag(Game* g)
{
	int carrierid;
	Worker* worker;

	carrierid = g->get_ware_id("carrier");

	if (!m_supply->stock(carrierid)) // yep, let's cheat
		create_wares(carrierid, 1);

	worker = launch_worker(g, carrierid);
	worker->start_task_fetchfromflag(g);

	return true;
}


/*
===============
Warehouse::launch_worker

Start a worker of a given type. The worker will be assigned a job by the caller.
===============
*/
Worker *Warehouse::launch_worker(Game *g, int ware)
{
	assert(m_supply->stock(ware));

	Ware_Descr *waredescr;
	Worker_Descr *workerdescr;
	Worker *worker;

	waredescr = g->get_ware_description(ware);
	assert(waredescr->is_worker());

	workerdescr = ((Worker_Ware_Descr*)waredescr)->get_worker(get_owner()->get_tribe());

	worker = workerdescr->create(g, get_owner(), this, m_position);

	m_supply->remove_wares(ware, 1);

	return worker;
}


/*
===============
Warehouse::incorporate_worker

This is the opposite of launch_worker: destroy the worker and add the
appropriate ware to our warelist
===============
*/
void Warehouse::incorporate_worker(Game *g, Worker *w)
{
	assert(w->get_owner() == get_owner());

	int ware = w->get_ware_id();
	WareInstance* item = w->fetch_carried_item(g); // rescue an item

	w->remove(g);

	m_supply->add_wares(ware, 1);

	if (item)
		incorporate_item(g, item);
}


/*
===============
Warehouse::launch_item

Create an instance of a ware, and make sure it gets carried out of the warehouse.
===============
*/
WareInstance* Warehouse::launch_item(Game* g, int ware)
{
	WareInstance* item;

	// Create the item
	item = new WareInstance(ware);
	item->init(g);

	m_supply->remove_wares(ware, 1);

	do_launch_item(g, item);

	return item;
}


/*
===============
Warehouse::do_launch_item

Get a carrier to actually move this item out of the warehouse.
===============
*/
void Warehouse::do_launch_item(Game* g, WareInstance* item)
{
	int carrierid;
	Ware_Descr* waredescr;
	Worker_Descr* workerdescr;
	Worker* worker;

	// Create a carrier
	carrierid = g->get_ware_id("carrier");
	waredescr = g->get_ware_description(carrierid);
	workerdescr = ((Worker_Ware_Descr*)waredescr)->get_worker(get_owner()->get_tribe());

	worker = workerdescr->create(g, get_owner(), this, m_position);

	// Yup, this is cheating.
	if (m_supply->stock(carrierid))
		m_supply->remove_wares(carrierid, 1);

	// Setup the carrier
	worker->start_task_dropoff(g, item);
}


/*
===============
Warehouse::incorporate_item

Swallow the item, adding it to out inventory.
===============
*/
void Warehouse::incorporate_item(Game* g, WareInstance* item)
{
	int ware = item->get_ware();

	item->destroy(g);

	m_supply->add_wares(ware, 1);
}


/*
===============
Warehouse::idle_request_cb [static]

Called when a transfer for one of the idle Requests completes.
===============
*/
void Warehouse::idle_request_cb(Game* g, Request* rq, int ware, Worker* w, void* data)
{
	Warehouse* wh = (Warehouse*)data;

	if (w)
		wh->incorporate_worker(g, w);
	else
		wh->m_supply->add_wares(ware, 1);
}


/*
===============
Warehouse_Descr::create_object
===============
*/
Building *Warehouse_Descr::create_object()
{
	return new Warehouse(this);
}


/*
==============================================================================

class ProductionProgram

==============================================================================
*/

struct ProductionAction {
	enum Type {
		actSleep,		// iparam1 = sleep time in milliseconds
		actWorker,		// sparam1 = worker program to run
      actConsume,    // sparam1 = consume this ware, has to be an input
      actAnimate,    // sparam1 = activate this animation until timeout
      actProduce,    // sparem1 = ware to produce. the worker carriers it out of the house
      actCheck,      // sparam1 = ist DAs da?
   };

	Type			type;
	int			iparam1;
	int			iparam2;
	std::string	sparam1;
};

/*
class ProductionProgram
-----------------------
Holds a series of actions to perform for production.
*/
class ProductionProgram {
public:
	ProductionProgram(std::string name);

	std::string get_name() const { return m_name; }
	int get_size() const { return m_actions.size(); }
	const ProductionAction* get_action(int idx) const {
		assert(idx >= 0 && (uint)idx < m_actions.size());
		return &m_actions[idx];
	}

	void parse(std::string directory, Profile* prof, std::string name, ProductionSite_Descr* building, const EncodeData* encdata);

private:
	std::string							m_name;
	std::vector<ProductionAction>	m_actions;
};


/*
===============
ProductionProgram::ProductionProgram
===============
*/
ProductionProgram::ProductionProgram(std::string name)
{
	m_name = name;
}


/*
===============
ProductionProgram::parse

Parse a program. The building is parsed completly. hopefully
===============
*/
void ProductionProgram::parse(std::string directory, Profile* prof, std::string name, ProductionSite_Descr* building, const EncodeData* encdata)
{
	Section* sprogram = prof->get_safe_section(name.c_str());

	for(uint idx = 0; ; ++idx) {
		char buf[32];
		const char* string;
		std::vector<std::string> cmd;

		snprintf(buf, sizeof(buf), "%i", idx);
		string = sprogram->get_string(buf, 0);
		if (!string)
			break;

		split_string(string, &cmd, " \t\r\n");
		if (!cmd.size())
			continue;

		ProductionAction act;

		if (cmd[0] == "sleep")
		{
			char* endp;

			if (cmd.size() != 2)
				throw wexception("Line %i: Usage: sleep <time in ms>", idx);

			act.type = ProductionAction::actSleep;
			act.iparam1 = strtol(cmd[1].c_str(), &endp, 0);

			if (endp && *endp)
				throw wexception("Line %i: bad integer '%s'", idx, cmd[1].c_str());
		} 
      else if (cmd[0] == "consume") {
         if(cmd.size() != 2)
            throw wexception("Line %i: Usage: consume <ware>", idx);


         Section* s=prof->get_safe_section("inputs");
         if(!s->get_string(cmd[1].c_str(), 0))
            throw wexception("Line %i: Ware %s is not in [inputs]\n", idx, cmd[1].c_str());

         act.type = ProductionAction::actConsume;
         act.sparam1 = cmd[1];
      }  else if (cmd[0] == "check") {
         if(cmd.size() != 2)
            throw wexception("Line %i: Usage: checking <ware>", idx);


         Section* s=prof->get_safe_section("inputs");
         if(!s->get_string(cmd[1].c_str(), 0))
            throw wexception("Line %i: Ware %s is not in [inputs]\n", idx, cmd[1].c_str());

         act.type = ProductionAction::actCheck;
         act.sparam1 = cmd[1];
      } else if (cmd[0] == "produce") {
         if(cmd.size() != 2)
            throw wexception("Line %i: Usage: produce <ware>", idx);

         if(!building->is_output(cmd[1]))
            throw wexception("Line %i: Ware %s is not in [outputs]\n", idx, cmd[1].c_str());

         act.type = ProductionAction::actProduce;
         act.sparam1 = cmd[1];
      } else if (cmd[0] == "worker") {
			if (cmd.size() != 2)
				throw wexception("Line %i: Usage: worker <program name>", idx);

			act.type = ProductionAction::actWorker;
			act.sparam1 = cmd[1];
      } else if (cmd[0] == "animation") {
         char* endp;

         if (cmd.size() != 3)
            throw wexception("Usage: animation <name> <time>");

         act.type = ProductionAction::actAnimate;

         // dynamically allocate animations here
         Section* s = prof->get_safe_section(cmd[1].c_str());
         act.iparam1 = g_anim.get(directory.c_str(), s, 0, encdata);
         
         if (cmd[1] == "idle")
            throw wexception("Idle animation is default, no calling senseful!");

         act.iparam2 = strtol(cmd[2].c_str(), &endp, 0);
         if (endp && *endp)
            throw wexception("Bad duration '%s'", cmd[2].c_str());

         if (act.iparam2 <= 0)
            throw wexception("animation duration must be positive");
			
      }
		else
			throw wexception("Line %i: unknown command '%s'", idx, cmd[0].c_str());

		m_actions.push_back(act);
	}
}


/*
==============================================================================

ProductionSite BUILDING

==============================================================================
*/

ProductionSite_Descr::ProductionSite_Descr(Tribe_Descr* tribe, const char* name)
	: Building_Descr(tribe, name)
{
}

ProductionSite_Descr::~ProductionSite_Descr()
{
	while(m_programs.size()) {
		delete m_programs.begin()->second;
		m_programs.erase(m_programs.begin());
	}
}


/*
===============
ProductionSite_Descr::parse

Parse the additional information necessary for production buildings
===============
*/
void ProductionSite_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Section* sglobal = prof->get_section("global");
	const char* string;

	Building_Descr::parse(directory, prof, encdata);

   // Get inputs and outputs
   while(sglobal->get_next_string("output", &string))
      m_output.insert(string);

   Section* s=prof->get_section("inputs");
   if(s) {
      // This house obviously requests wares and works on them

      Section::Value* val;
      while((val=s->get_next_val(0))) {

         int idx=get_tribe()->get_ware_index(val->get_name());
         if(idx==-1)
            throw wexception("Error in [inputs], ware %s is unknown!", val->get_name());


         Item_Ware_Descr* ware= get_tribe()->get_ware_descr(idx);

         Input input(ware,val->get_int());
         m_inputs.push_back(input);
      }
   }

   if(is_only_production_site()) {
      // Are we only a production site? 
      // If not, we might not have a worker
      m_worker_name = sglobal->get_safe_string("worker");
   } else {
      m_worker_name = sglobal->get_string("worker", "");
   }

	// Get programs
	while(sglobal->get_next_string("program", &string)) {
		ProductionProgram* program = 0;

		try
		{
			program = new ProductionProgram(string);
			program->parse(directory, prof, string, this, encdata); 
			m_programs[program->get_name()] = program;
		}
		catch(std::exception& e)
		{
			if (program)
				delete program;

			throw wexception("Error in program %s: %s", string, e.what());
		}
	}
}


/*
===============
ProductionSite_Descr::get_program

Get the program of the given name.
===============
*/
const ProductionProgram* ProductionSite_Descr::get_program(std::string name) const
{
	ProgramMap::const_iterator it = m_programs.find(name);

	if (it == m_programs.end())
		throw wexception("%s has no program '%s'", get_name(), name.c_str());

	return it->second;
}

/*
===============
ProductionSite_Descr::create_object

Create a new building of this type
===============
*/
Building* ProductionSite_Descr::create_object()
{
	return new ProductionSite(this);
}


/*
==============================

IMPLEMENTATION

==============================
*/

/*
===============
ProductionSite::ProductionSite
===============
*/
ProductionSite::ProductionSite(ProductionSite_Descr* descr)
	: Building(descr)
{
	m_worker = 0;
	m_worker_request = 0;

	m_fetchfromflag = 0;

	m_program = 0;
	m_program_ip = 0;
	m_program_phase = 0;
	m_program_timer = false;
	m_program_time = 0;
}


/*
===============
ProductionSite::~ProductionSite
===============
*/
ProductionSite::~ProductionSite()
{
}


/*
===============
ProductionSite::get_statistics_string

Display whether we're occupied.
===============
*/
std::string ProductionSite::get_statistics_string()
{
	if (!m_worker)
		return "(not occupied)";

	return "%%%%"; // TODO: implement percentage counter
}


/*
===============
ProductionSite::init

Initialize the production site.
===============
*/
void ProductionSite::init(Editor_Game_Base *g)
{
	Building::init(g);

   if (g->is_game()) {
		// Request worker
      if(!m_worker)
         request_worker((Game*)g);

      // Init input ware queues
      const std::vector<Input>* inputs=((ProductionSite_Descr*)get_descr())->get_inputs();

      for(uint i = 0; i < inputs->size(); i++) {
         WaresQueue* wq = new WaresQueue(this);

         m_input_queues.push_back(wq);
         //         wq->set_callback(&ConstructionSite::wares_queue_callback, this);
         wq->init((Game*)g, g->get_safe_ware_id((*inputs)[i].get_ware()->get_name()), (*inputs)[i].get_max());
      }
   }
}

/*
===============
ProductionSite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void ProductionSite::set_economy(Economy* e)
{
   Economy* old = get_economy();
	uint i;

	if (old) {
		for(i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->remove_from_economy(old);
	}

	Building::set_economy(e);
	if (m_worker_request)
		m_worker_request->set_economy(e);

	if (e) {
		for(i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->add_to_economy(e);
	}
}

/*
===============
ProductionSite::cleanup

Cleanup after a production site is removed
===============
*/
void ProductionSite::cleanup(Editor_Game_Base *g)
{
	// Release worker
	if (m_worker_request) {
		delete m_worker_request;
		m_worker_request = 0;
	}

	if (m_worker) {
		Worker* w = m_worker;

		m_worker = 0;
		w->set_location(0);
	}


   // Cleanup the wares queues
	for(uint i = 0; i < m_input_queues.size(); i++) {
		m_input_queues[i]->cleanup((Game*)g);
		delete m_input_queues[i];
	}
	m_input_queues.clear();


	Building::cleanup(g);
}


/*
===============
ProductionSite::remove_worker

Intercept remove_worker() calls to unassign our worker, if necessary.
===============
*/
void ProductionSite::remove_worker(Worker *w)
{
	if (m_worker == w) {
		m_worker = 0;

		request_worker((Game*)get_owner()->get_game());
	}

	Building::remove_worker(w);
}


/*
===============
ProductionSite::request_worker

Issue the worker request
===============
*/
void ProductionSite::request_worker(Game* g)
{
	assert(!m_worker);
	assert(!m_worker_request);
   if(!get_descr()->get_worker_name().size()) return; // no worker for this house. it's not a productionsite only

	int wareid = g->get_safe_ware_id(get_descr()->get_worker_name().c_str());

	m_worker_request = new Request(this, wareid, &ProductionSite::request_worker_callback, this);
}


/*
===============
ProductionSite::request_worker_callback [static]

Called when our worker arrives.
===============
*/
void ProductionSite::request_worker_callback(Game* g, Request* rq, int ware, Worker* w, void* data)
{
	ProductionSite* psite = (ProductionSite*)data;

	assert(w);
	assert(w->get_location(g) == psite);
	assert(rq == psite->m_worker_request);

	psite->m_worker = w;

	delete rq;
	psite->m_worker_request = 0;

	w->start_task_buildingwork(g);
}


/*
===============
ProductionSite::act

Advance the program state if applicable.
===============
*/
void ProductionSite::act(Game *g, uint data)
{
	if (m_program_timer && (int)(g->get_gametime() - m_program_time) >= 0) {
		const ProductionAction* action = m_program->get_action(m_program_ip);

		m_program_timer = false;
		m_program_needs_restart=false;

      molog("PSITE: program %s#%i\n", m_program->get_name().c_str(), m_program_ip);

      if(m_anim!=get_descr()->get_idle_anim()) {
         // Restart idle animation, which is the default
         start_animation(g, get_descr()->get_idle_anim());
      }


      switch(action->type) {
         case ProductionAction::actSleep:
            molog("  Sleep(%i)\n", action->iparam1);

            program_step();
            m_program_timer = true;
            m_program_time = schedule_act(g, action->iparam1);
            break;

         case ProductionAction::actAnimate:
            molog("  Animate(%i,%i)\n", action->iparam1, action->iparam2);

            start_animation(g, action->iparam1);

            program_step();
            m_program_timer = true;
            m_program_time = schedule_act(g, action->iparam2);
            break;

         case ProductionAction::actWorker:
            molog("  Worker(%s)\n", action->sparam1.c_str());

            m_worker->update_task_buildingwork(g);
            break;

         case ProductionAction::actConsume:
            molog("  Consume(%s)\n", action->sparam1.c_str());
            for(uint i=0; i<get_descr()->get_inputs()->size(); i++) {
               if(!strcmp((*get_descr()->get_inputs())[i].get_ware()->get_name(), action->sparam1.c_str())) {
                  WaresQueue* wq=m_input_queues[i];
                  if(wq->get_filled())
						{
                     wq->set_filled(wq->get_filled()-1);
							wq->update(g);
						}
                  else
						{
							molog("   Consume failed, program restart\n");
							program_restart();
						}
                  break;
               }
            }
            molog("  Consume done!\n");
            program_step();
            m_program_timer=true;
            m_program_time=schedule_act(g, 10);
            break;

         case ProductionAction::actCheck:
            molog("  Checking(%s)\n", action->sparam1.c_str());

            for(uint i=0; i<get_descr()->get_inputs()->size(); i++) {
               if(!strcmp((*get_descr()->get_inputs())[i].get_ware()->get_name(), action->sparam1.c_str())) {
                  WaresQueue* wq=m_input_queues[i];
                  if(wq->get_filled())
                  {
                     // okay, do nothing
                     molog("    okay\n");
                  }
                  else
                  {
                     molog("   Checking failed, program restart\n");
                     program_restart();
                  }
                  break;
               }
            }

				molog("  Check done!\n");

            program_step();
            m_program_timer = true;
            m_program_time = schedule_act(g, 10);
            break;

         case ProductionAction::actProduce:
            {
            molog("  Produce(%s)\n", action->sparam1.c_str());

            int wareid = g->get_safe_ware_id(action->sparam1.c_str());

            WareInstance* item = new WareInstance(wareid);
            item->init(g);
            m_worker->set_carried_item(g,item);

				// get the worker to drop the item off
				// get_building_work() will advance the program
            m_worker->update_task_buildingwork(g);
            }
            break;
      }
   }

   Building::act(g, data);
}


/*
===============
ProductionSite::fetch_from_flag

Remember that we need to fetch an item from the flag.
===============
*/
bool ProductionSite::fetch_from_flag(Game* g)
{
	m_fetchfromflag++;

	if (m_worker)
		m_worker->update_task_buildingwork(g);

	return true;
}


/*
===============
ProductionSite::get_building_work

There's currently nothing to do for the worker.
Note: we assume that the worker is inside the building when this is called.
===============
*/
bool ProductionSite::get_building_work(Game* g, Worker* w, bool success)
{
	assert(w == m_worker);

	// Default actions first
	WareInstance* item = w->fetch_carried_item(g);

	if (item) {
		if (!get_descr()->is_output(item->get_ware_descr()->get_name()))
			molog("PSITE: WARNING: carried item %s is not an output item\n",
					item->get_ware_descr()->get_name());

		molog("ProductionSite::get_building_work: start dropoff\n");

		w->start_task_dropoff(g, item);
		return true;
	}

	if (m_fetchfromflag) {
		m_fetchfromflag--;
		w->start_task_fetchfromflag(g);
		return true;
	}

	// Start program if we haven't already done so
	if (!m_program)
	{
		m_program = get_descr()->get_program("work");
		m_program_ip = 0;
		m_program_phase = 0;
		m_program_timer = true;
		m_program_needs_restart=false;
      m_program_time = schedule_act(g, 10);
	}
	else
	{
		const ProductionAction* action = m_program->get_action(m_program_ip);

		if (action->type == ProductionAction::actWorker) {
			if (m_program_phase == 0)
			{
				w->start_task_program(g, action->sparam1);
				m_program_phase++;
				return true;
			}
			else
			{
				program_step();
				m_program_timer = true;
				m_program_time = schedule_act(g, 10);
			}
		} else if (action->type == ProductionAction::actProduce) {
			// Worker returned home after dropping item
			program_step();
			m_program_timer = true;
			m_program_time = schedule_act(g, 10);
		}
	}

	return false;
}


/*
===============
ProductionSite::program_step

Advance the program to the next step, but does not schedule anything.
===============
*/
void ProductionSite::program_step()
{
	if(m_program_needs_restart) return;

   m_program_ip = (m_program_ip + 1) % m_program->get_size();
	m_program_phase = 0;
}

/*
===============
ProductionSite::program_restart()

The program needs to restart (maybe because of failure). all further 
schedules are ignored
===============
*/
void ProductionSite::program_restart() {
   m_program_ip=0;
   m_program_phase=0;
   m_program_timer=false;
   m_program_time=0;
   m_program_needs_restart=true;
}


/*
==============================================================================

MilitarySite CLASSES

==============================================================================
*/

/*
=============================

class MilitarySite_Descr

=============================
*/
MilitarySite_Descr::MilitarySite_Descr(Tribe_Descr* tribe, const char* name)
	: ProductionSite_Descr(tribe, name)
{
   m_conquer_radius=0;
   m_num_soldiers=0;
   m_num_medics=0;
   m_heal_per_second=0;
   m_heal_incr_per_medic=0;
}

MilitarySite_Descr::~MilitarySite_Descr()
{
}

/*
===============
MilitarySite_Descr::parse

Parse the additional information necessary for miltary buildings
===============
*/
void MilitarySite_Descr::parse(const char *directory, Profile *prof, const EncodeData *encdata)
{
	Section* sglobal = prof->get_section("global");

	Building_Descr::parse(directory, prof, encdata);
   ProductionSite_Descr::parse(directory,prof,encdata);

   m_conquer_radius=sglobal->get_safe_int("conquers");
   m_num_soldiers=sglobal->get_safe_int("max_soldiers");
   m_num_medics=sglobal->get_safe_int("max_medics");
   m_heal_per_second=sglobal->get_safe_int("heal_per_second");
   m_heal_incr_per_medic=sglobal->get_safe_int("heal_increase_per_medic");
}

/*
===============
MilitarySite_Descr::create_object

Create a new building of this type
===============
*/
Building* MilitarySite_Descr::create_object()
{
	return new MilitarySite(this);
}


/*
=============================

class MilitarySite

=============================
*/

/*
===============
MilitarySite::MilitarySite
===============
*/
MilitarySite::MilitarySite(MilitarySite_Descr* descr)
	: ProductionSite(descr)
{
	m_soldier = 0;
	m_soldier_request = 0;
}


/*
===============
MilitarySite::~MilitarySite
===============
*/
MilitarySite::~MilitarySite()
{
}


/*
===============
MilitarySite::get_statistics_string

Display number of soldiers.
===============
*/
std::string MilitarySite::get_statistics_string()
{
	return "(soldiers)";
}


/*
===============
MilitarySite::init

Initialize the military site.
===============
*/
void MilitarySite::init(Editor_Game_Base *g)
{
   ProductionSite::init(g);

   if (g->is_game()) {
   // Request soldier
      if(!m_soldier)
         request_soldier((Game*)g);

   }
}

/*
===============
MilitarySite::set_economy

Change the economy for the wares queues.
Note that the workers are dealt with in the PlayerImmovable code.
===============
*/
void MilitarySite::set_economy(Economy* e)
{
/*   Economy* old = get_economy();
	uint i;

	if (old) {
		for(i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->remove_from_economy(old);
	}

	Building::set_economy(e);

	if (e) {
		for(i = 0; i < m_input_queues.size(); i++)
			m_input_queues[i]->add_to_economy(e);
	}
   */
   // TODO: SoldiersQueue migration
   ProductionSite::set_economy(e);
	if (m_soldier_request)
		m_soldier_request->set_economy(e);
}

/*
===============
MilitarySite::cleanup

Cleanup after a military site is removed
===============
*/
void MilitarySite::cleanup(Editor_Game_Base *g)
{
	// Release worker
	if (m_soldier_request) {
		delete m_soldier_request;
		m_soldier_request = 0;
	}

	if (m_soldier) {
		Worker* w = m_soldier;

		m_soldier = 0;
		w->set_location(0);
	}

   // unconquer land
   g->unconquer_area(get_owner()->get_player_number(), get_position());

   ProductionSite::cleanup(g);
}


/*
===============
MilitarySite::remove_worker

Intercept remove_worker() calls to unassign our worker, if necessary.
===============

void MilitarySite::remove_worker(Worker *w)
{
	if (m_worker == w) {
		m_worker = 0;

		request_worker((Game*)get_owner()->get_game());
	}

	Building::remove_worker(w);
}
*/

/*
===============
MilitarySite::request_soldier

Issue the soldier request
===============
*/
void MilitarySite::request_soldier(Game* g)
{
	assert(!m_soldier);
	assert(!m_soldier_request);

	int wareid = g->get_safe_ware_id("lumberjack");

	m_soldier_request = new Request(this, wareid, &MilitarySite::request_soldier_callback, this);
}


/*
===============
MilitarySite::request_soldier_callback [static]

Called when our soldier arrives.
===============
*/
void MilitarySite::request_soldier_callback(Game* g, Request* rq, int ware, Worker* w, void* data)
{
	MilitarySite* psite = (MilitarySite*)data;

	assert(w);
	assert(w->get_location(g) == psite);
      
   g->conquer_area(psite->get_owner()->get_player_number(), psite->get_position(), psite->get_descr());
}


/*
===============
MilitarySite::act

Advance the program state if applicable.
===============
*/
void MilitarySite::act(Game *g, uint data)
{
   // TODO: do all kinds of stuff, but if you do nothing, let ProductionSite::act() 
   // handle all this. Also note, that some ProductionSite commands rely, that ProductionSite::act()
   // is not called for a certain period (like cmdAnimation). This should be reworked.
   // Maybe a new queueing system like MilitaryAct could be introduced.
   ProductionSite::act(g,data);
}

/*
==============================================================================

Building_Descr Factory

==============================================================================
*/

/*
===============
Building_Descr::create_from_dir

Open the appropriate configuration file and check if a building description
is there.

May return 0.
===============
*/
Building_Descr *Building_Descr::create_from_dir(Tribe_Descr *tribe, const char *directory,
		                                          const EncodeData *encdata)
{
	const char *name;

	// name = last element of path
	const char *slash = strrchr(directory, '/');
	const char *backslash = strrchr(directory, '\\');

	if (backslash && (!slash || backslash > slash))
		slash = backslash;

	if (slash)
		name = slash+1;
	else
		name = directory;

	// Open the config file
	Building_Descr *descr = 0;
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/conf", directory);

	if (!g_fs->FileExists(fname))
		return 0;

	try
	{
		Profile prof(fname, "global"); // section-less file
		Section *s = prof.get_safe_section("global");
		const char *type = s->get_safe_string("type");

		if (!strcasecmp(type, "warehouse"))
			descr = new Warehouse_Descr(tribe, name);
		else if (!strcasecmp(type, "production"))
			descr = new ProductionSite_Descr(tribe, name);
		else if (!strcasecmp(type, "construction"))
			descr = new ConstructionSite_Descr(tribe, name);
		else if (!strcasecmp(type, "military"))
			descr = new MilitarySite_Descr(tribe, name);
		else
			throw wexception("Unknown building type '%s'", type);
		
		descr->parse(directory, &prof, encdata);
	}
	catch(std::exception &e) {
		if (descr)
			delete descr;
		throw wexception("Error reading building %s: %s", name, e.what());
	}
	catch(...) {
		if (descr)
			delete descr;
		throw;
	}

	return descr;
}

