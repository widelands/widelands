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

#include "building.h"
#include "constructionsite.h"
#include "editor_game_base.h"
#include "error.h"
#include "filesystem.h"
#include "font_handler.h"
#include "game.h"
#include "graphic.h"
#include "interactive_base.h"
#include "map.h"
#include "militarysite.h"
#include "player.h"
#include "productionsite.h"
#include "profile.h"
#include "rendertarget.h"
#include "transport.h"
#include "tribe.h"
#include "warehouse.h"
#include "wexception.h"
#include "worker.h"
#include "constants.h"

static const int BUILDING_LEAVE_INTERVAL = 1000;


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
Building_Descr::Building_Descr(Tribe_Descr* tribe, const char* name)
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
	free(m_buildicon_fname);
}

/*
===============
Building_Descr::create

Create a building of this type. Does not perform any sanity checks.
===============
*/
Building* Building_Descr::create(Editor_Game_Base* g, Player* owner,
	Coords pos, bool construct)
{
	assert(owner);

	Building* b = construct ? create_constructionsite() : create_object();
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
void Building_Descr::parse(const char* directory, Profile* prof,
	const EncodeData* encdata)
{
	Section* global = prof->get_safe_section("global");
	Section* s;
	const char* string;
	char buf[256];
	char fname[256];

	snprintf(m_descname, sizeof(m_descname), "%s",
		global->get_safe_string("descname"));

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
		throw wexception(
			"Section [global], unknown size '%s'. "
			"Valid values are small, medium, big, mine",
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
		m_buildicon =
			g_gr->get_picture(PicMod_Game, m_buildicon_fname, RGBColor(0,0,255));
}

/*
===============
Building_Descr::create_constructionsite

Create a construction site for this type of building
===============
*/
Building* Building_Descr::create_constructionsite()
{
	Building_Descr* descr =
		m_tribe->get_building_descr(
			m_tribe->get_building_index("constructionsite"));
	if (!descr)
		throw wexception("Tribe %s has no constructionsite", m_tribe->get_name());

	ConstructionSite* csite = (ConstructionSite*)descr->create_object();

	csite->set_building(this);

	return csite;
}


/*
===============
Building_Descr::create_from_dir

Open the appropriate configuration file and check if a building description
is there.

May return 0.
===============
*/
Building_Descr* Building_Descr::create_from_dir(Tribe_Descr* tribe,
	const char* directory, const EncodeData* encdata)
{
	const char* name;

	// name = last element of path
	const char* slash = strrchr(directory, '/');
	const char* backslash = strrchr(directory, '\\');

	if (backslash && (!slash || backslash > slash))
		slash = backslash;

	if (slash)
		name = slash+1;
	else
		name = directory;

	// Open the config file
	Building_Descr* descr = 0;
	char fname[256];

	snprintf(fname, sizeof(fname), "%s/conf", directory);

	if (!g_fs->FileExists(fname))
		return 0;

	try
	{
		Profile prof(fname, "global"); // section-less file
		Section* s = prof.get_safe_section("global");
		const char* type = s->get_safe_string("type");

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
		delete descr;
		throw wexception("Error reading building %s: %s", name, e.what());
	}
	catch(...) {
		delete descr;
		throw;
	}

	return descr;
}


/*
==============================

Implementation

==============================
*/

Building::Building(Building_Descr* descr)
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

Flag* Building::get_base_flag()
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
void Building::start_animation(Editor_Game_Base* g, uint anim)
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
	Map* map = g->get_map();
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
	BaseImmovable* imm;
	Flag* flag;

	map->get_brn(m_position, &neighb);
	imm = map->get_immovable(neighb);

	if (imm && imm->get_type() == FLAG)
		flag = (Flag*)imm;
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
void Building::cleanup(Editor_Game_Base* g)
{
	// Remove from flag
	m_flag->detach_building(g);

	// Unset the building
	unset_position(g, m_position);

	if (get_size() == BIG) {
		Map* map = g->get_map();
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
	throw wexception("MO(%u): get_building_work() for unknown worker %u",
		get_serial(), w->get_serial());
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
void Building::act(Game* g, uint data)
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
void Building::draw(Editor_Game_Base* game, RenderTarget* dst, FCoords coords,
	Point pos)
{
	if (coords != m_position)
		return; // draw big buildings only once

	dst->drawanim(pos.x, pos.y, m_anim, game->get_gametime() - m_animstart,
		get_owner()->get_playercolor());

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
void Building::draw_help(Editor_Game_Base* game, RenderTarget* dst,
	FCoords coords, Point pos)
{
	uint dpyflags = game->get_iabase()->get_display_flags();

	if (dpyflags & Interactive_Base::dfShowCensus)
	{
		std::string txt = get_census_string();

      // TODO: Make more here
		g_fh->draw_string(dst, UI_FONT_SMALL, UI_FONT_SMALL_CLR, pos.x, pos.y - 45, txt.c_str(), Align_Center);
	}

	if (dpyflags & Interactive_Base::dfShowStatistics)
	{
		std::string txt = get_statistics_string();

		g_fh->draw_string(dst, UI_FONT_SMALL, UI_FONT_SMALL_CLR, pos.x, pos.y - 35, txt.c_str(), Align_Center);
	}
}
