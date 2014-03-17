/*
 * Copyright (C) 2002-2004, 2006-2011 by the Widelands Development Team
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "logic/building.h"

#include <cstdio>
#include <sstream>

#include <boost/foreach.hpp>

#include "economy/flag.h"
#include "economy/request.h"
#include "graphic/font.h"
#include "graphic/font_handler.h"
#include "graphic/font_handler1.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/constructionsite.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/player.h"
#include "logic/productionsite.h"
#include "logic/tribe.h"
#include "logic/worker.h"
#include "profile/profile.h"
#include "sound/sound_handler.h"
#include "text_layout.h"
#include "upcast.h"
#include "wexception.h"
#include "wui/interactive_player.h"

namespace Widelands {

static const int32_t BUILDING_LEAVE_INTERVAL = 1000;


Building_Descr::Building_Descr
	(char const * const _name, char const * const _descname,
	 const std::string & directory, Profile & prof, Section & global_s,
	 const Tribe_Descr & _descr)
	:
	Map_Object_Descr(_name, _descname),
	m_tribe         (_descr),
	m_buildable     (true),
	m_buildicon     (nullptr),
	m_size          (BaseImmovable::SMALL),
	m_mine          (false),
	m_port          (false),
	m_hints         (prof.get_section("aihints")),
	m_global        (false),
	m_vision_range  (0)
{
	try {
		char const * const string = global_s.get_safe_string("size");
		if      (!strcasecmp(string, "small"))
			m_size = BaseImmovable::SMALL;
		else if (!strcasecmp(string, "medium"))
			m_size = BaseImmovable::MEDIUM;
		else if (!strcasecmp(string, "big"))
			m_size = BaseImmovable::BIG;
		else if (!strcasecmp(string, "mine")) {
			m_size = BaseImmovable::SMALL;
			m_mine = true;
		} else if (!strcasecmp(string, "port")) {
			m_size = BaseImmovable::BIG;
			m_port = true;
		} else
			throw game_data_error
				("expected %s but found \"%s\"",
				 "{\"small\"|\"medium\"|\"big\"|\"port\"|\"mine\"}", string);
	} catch (const _wexception & e) {
		throw game_data_error("size: %s", e.what());
	}

	m_helptext_script = directory + "/help.lua";
	if (not g_fs->FileExists(m_helptext_script))
		m_helptext_script = "";

	// Parse build options
	m_buildable = global_s.get_bool("buildable", true);
	m_destructible = global_s.get_bool("destructible", true);
	while
		(Section::Value const * const v = global_s.get_next_val("enhancement"))
		try {
			std::string const target_name = v->get_string();
			if (target_name == name())
				throw wexception("enhancement to same type");
			if (target_name == "constructionsite")
				throw wexception("enhancement to special type constructionsite");
			if (Building_Index const en_i = tribe().building_index(target_name)) {
				if (enhancements().count(en_i))
					throw wexception("this has already been declared");
				m_enhancements.insert(en_i);

				//  Merge the enhancements workarea info into this building's
				//  workarea info.
				const Building_Descr & enhancement =
					*tribe().get_building_descr(en_i);
				container_iterate_const
					(Workarea_Info, enhancement.m_workarea_info, j)
				{
					std::set<std::string> & r = m_workarea_info[j.current->first];
					container_iterate_const
						(std::set<std::string>, j.current->second, i)
						r.insert(*i.current);
				}
			} else
				throw wexception
					("\"%s\" has not been defined as a building type (wrong declaration order?)",
					 target_name.c_str());
		} catch (const _wexception & e) {
			throw wexception("\"enhancements=%s\": %s", v->get_string(), e.what());
		}
	m_enhanced_building = global_s.get_bool("enhanced_building", false);
	m_global = directory.find("global/") < directory.size();
	if (m_buildable || m_enhanced_building) {
		//  get build icon
		m_buildicon_fname  = directory;
		m_buildicon_fname += "/menu.png";

		//  build animation
		if (Section * const build_s = prof.get_section("build")) {
			if (build_s->get_int("fps", -1) != -1)
				throw wexception("fps defined for build animation!");
			if (!is_animation_known("build"))
				add_animation("build", g_gr->animations().load(directory, *build_s));
		}

		// Get costs
		if (m_buildable) {
			Section & buildcost_s = prof.get_safe_section("buildcost");
			m_buildcost.parse(m_tribe, buildcost_s);
			Section & returnsect_s = prof.get_safe_section("return_on_dismantle");
			m_return_dismantle.parse(m_tribe, returnsect_s);
		}

		if (m_enhanced_building) {
			Section & en_buildcost_s = prof.get_safe_section("enhancement_cost");
			m_enhance_cost.parse(m_tribe, en_buildcost_s);
			Section & en_returnsect_s = prof.get_safe_section("return_on_dismantle_on_enhanced");
			m_return_enhanced.parse(m_tribe, en_returnsect_s);
		}
	} else if (m_global) {
		//  get build icon for global buildings (for statistics window)
		m_buildicon_fname  = directory;
		m_buildicon_fname += "/menu.png";
	}

	{ //  parse basic animation data
		Section & idle_s = prof.get_safe_section("idle");
		if (!is_animation_known("idle"))
			add_animation("idle", g_gr->animations().load(directory, idle_s));
		if (Section * unoccupied = prof.get_section("unoccupied"))
			if (!is_animation_known("unoccupied"))
				add_animation("unoccupied", g_gr->animations().load(directory, *unoccupied));
		if (Section * empty = prof.get_section("empty"))
			if (!is_animation_known("empty"))
				add_animation("empty", g_gr->animations().load(directory, *empty));
	}

	m_vision_range = global_s.get_int("vision_range");
}


Building & Building_Descr::create
	(Editor_Game_Base     &       egbase,
	 Player               &       owner,
	 Coords                 const pos,
	 bool                   const construct,
	 bool                         loading,
	 Building::FormerBuildings const former_buildings)
	const
{
	Building & b = construct ? create_constructionsite() : create_object();
	b.m_position = pos;
	b.set_owner(&owner);
	BOOST_FOREACH(Building_Index idx, former_buildings) {
		b.m_old_buildings.push_back(idx);
	}
	if (loading) {
		b.Building::init(egbase);
		return b;
	}
	b.init(egbase);
	return b;
}


int32_t Building_Descr::suitability(const Map &, FCoords const fc) const {
	return m_size <= (fc.field->nodecaps() & Widelands::BUILDCAPS_SIZEMASK);
}

/**
 * Normal buildings don't conquer anything, so this returns 0 by default.
 *
 * \return the radius of the conquered area.
 */
uint32_t Building_Descr::get_conquers() const
{
	return 0;
}


/**
 * \return the radius (in number of fields) of the area seen by this
 * building.
 */
uint32_t Building_Descr::vision_range() const
{
	return m_vision_range ? m_vision_range : get_conquers() + 4;
}


/*
===============
Called whenever building graphics need to be loaded.
===============
*/
void Building_Descr::load_graphics()
{
	if (m_buildicon_fname.size())
		m_buildicon = g_gr->images().get(m_buildicon_fname);
}

/*
===============
Create a construction site for this type of building
===============
*/
Building & Building_Descr::create_constructionsite() const
{
	Building_Descr const * const descr =
		m_tribe.get_building_descr
			(m_tribe.safe_building_index("constructionsite"));
	ConstructionSite & csite =
		ref_cast<ConstructionSite, Map_Object>(descr->create_object());
	csite.set_building(*this);

	return csite;
}


/*
==============================

Implementation

==============================
*/

Building::Building(const Building_Descr & building_descr) :
PlayerImmovable(building_descr),
m_optionswindow(nullptr),
m_flag         (nullptr),
m_anim(0),
m_animstart(0),
m_leave_time(0),
m_defeating_player(0),
m_seeing(false)
{}

Building::~Building()
{
	if (m_optionswindow)
		hide_options();
}

void Building::load_finish(Editor_Game_Base & egbase) {
	Leave_Queue & queue = m_leave_queue;
	for (wl_range<Leave_Queue> i(queue); i;)
	{
		Worker & worker = *i->get(egbase);
		{
			OPtr<PlayerImmovable> const worker_location = worker.get_location();
			if
				(worker_location.serial() !=             serial() and
				 worker_location.serial() != base_flag().serial())
				log
					("WARNING: worker %u is in the leave queue of building %u with "
					 "base flag %u but is neither inside the building nor at the "
					 "flag!\n",
					 worker.serial(), serial(), base_flag().serial());
		}
		Bob::State const * const state =
			worker.get_state(Worker::taskLeavebuilding);
		if (not state)
			log
				("WARNING: worker %u is in the leave queue of building %u but "
				 "does not have a leavebuilding task! Removing from queue.\n",
				 worker.serial(), serial());
		else if (state->objvar1 != this)
			log
				("WARNING: worker %u is in the leave queue of building %u but its "
				 "leavebuilding task is for map object %u! Removing from queue.\n",
				 worker.serial(), serial(), state->objvar1.serial());
		else {
			++i;
			continue;
		}
		i = wl_erase(queue, i.current);
	}
}

int32_t Building::get_type() const {return BUILDING;}

int32_t Building::get_size() const {return descr().get_size();}

bool Building::get_passable() const {return false;}

Flag & Building::base_flag()
{
	return *m_flag;
}


/**
 * \return a bitfield of commands the owning player can issue for this building.
 *
 * The bits are PCap_XXX.
 * By default, all buildings can be bulldozed. If a building should not be
 * destructible, "destructible=no" must be added to buildings conf.
 */
uint32_t Building::get_playercaps() const {
	uint32_t caps = 0;
	const Building_Descr & d = descr();
	if (d.is_destructible()) {
		caps |= PCap_Bulldoze;
		if (d.is_buildable() or d.is_enhanced())
			caps |= PCap_Dismantle;
	}
	if (d.enhancements().size())
		caps |= PCap_Enhancable;
	return caps;
}


const std::string & Building::name() const {return descr().name();}


void Building::start_animation(Editor_Game_Base & egbase, uint32_t const anim)
{
	m_anim = anim;
	m_animstart = egbase.get_gametime();
}

/*
===============
Common building initialization code. You must call this from
derived class' init.
===============
*/
void Building::init(Editor_Game_Base & egbase)
{
	PlayerImmovable::init(egbase);

	// Set the building onto the map
	Map & map = egbase.map();
	Coords neighb;

	set_position(egbase, m_position);

	if (get_size() == BIG) {
		map.get_ln(m_position, &neighb);
		set_position(egbase, neighb);

		map.get_tln(m_position, &neighb);
		set_position(egbase, neighb);

		map.get_trn(m_position, &neighb);
		set_position(egbase, neighb);
	}

	// Make sure the flag is there


	map.get_brn(m_position, &neighb);
	{
		Flag * flag = dynamic_cast<Flag *>(map.get_immovable(neighb));
		if (not flag)
			flag =
				new Flag (egbase, owner(), neighb);
		m_flag = flag;
		flag->attach_building(egbase, *this);
	}

	// Start the animation
	if (descr().is_animation_known("unoccupied"))
		start_animation(egbase, descr().get_animation("unoccupied"));
	else
		start_animation(egbase, descr().get_animation("idle"));

	m_leave_time = egbase.get_gametime();
}


void Building::cleanup(Editor_Game_Base & egbase)
{
	if (m_defeating_player) {
		Player & defeating_player = egbase.player(m_defeating_player);
		if (descr().get_conquers()) {
			owner()         .count_msite_lost        ();
			defeating_player.count_msite_defeated    ();
		} else {
			owner()         .count_civil_bld_lost    ();
			defeating_player.count_civil_bld_defeated();
		}
	}

	// Remove from flag
	m_flag->detach_building(egbase);

	// Unset the building
	unset_position(egbase, m_position);

	if (get_size() == BIG) {
		Map & map = egbase.map();
		Coords neighb;

		map.get_ln(m_position, &neighb);
		unset_position(egbase, neighb);

		map.get_tln(m_position, &neighb);
		unset_position(egbase, neighb);

		map.get_trn(m_position, &neighb);
		unset_position(egbase, neighb);
	}

	PlayerImmovable::cleanup(egbase);

	BOOST_FOREACH(boost::signals2::connection& c, options_window_connections)
		c.disconnect();
}


/*
===============
Building::burn_on_destroy [virtual]

Return true if a "fire" should be created when the building is destroyed.
By default, burn always.
===============
*/
bool Building::burn_on_destroy()
{
	return true;
}


/**
 * Return all positions on the map that we occupy
 */
BaseImmovable::PositionList Building::get_positions
	(const Editor_Game_Base & egbase) const
{
	PositionList rv;

	rv.push_back(m_position);
	if (get_size() == BIG) {
		Map & map = egbase.map();
		Coords neighb;

		map.get_ln(m_position, &neighb);
		rv.push_back(neighb);

		map.get_tln(m_position, &neighb);
		rv.push_back(neighb);

		map.get_trn(m_position, &neighb);
		rv.push_back(neighb);
	}
	return rv;
}


/*
===============
Remove the building from the world now, and create a fire in its place if
applicable.
===============
*/
void Building::destroy(Editor_Game_Base & egbase)
{
	const bool fire           = burn_on_destroy();
	const Coords pos          = m_position;
	const Tribe_Descr & t = tribe();
	PlayerImmovable::destroy(egbase);
	// We are deleted. Only use stack variables beyond this point
	if (fire)
		egbase.create_immovable(pos, "destroyed_building", &t);
}


/*
===============
Building::get_ui_anim [virtual]

Return the animation ID that is used for the building in UI items
(the building UI, messages, etc..)
===============
*/
uint32_t Building::get_ui_anim() const {return descr().get_ui_anim();}


#define FORMAT(key, value) case key: result << value; break
std::string Building::info_string(const std::string & format) {
	std::ostringstream result;
	container_iterate_const(std::string, format, i)
		if (*i.current == '%') {
			if (i.advance().empty()) { //  unterminated format sequence
				result << '%';
				break;
			}
			switch (*i.current) {
			FORMAT('%', '%');
			FORMAT('i', serial());
			FORMAT('t', get_statistics_string());
			FORMAT
				('s',
				 (descr().get_ismine()                  ? _("mine")   :
				  get_size  () == BaseImmovable::SMALL  ? _("small")  :
				  get_size  () == BaseImmovable::MEDIUM ? _("medium") : _("big")));
			FORMAT
				('S',
				 (descr().get_ismine()                  ? _("Mine")   :
				  get_size  () == BaseImmovable::SMALL  ? _("Small")  :
				  get_size  () == BaseImmovable::MEDIUM ? _("Medium") : _("Big")));
			FORMAT('x', get_position().x);
			FORMAT('y', get_position().y);
			FORMAT
				('c', '(' << get_position().x << ", " << get_position().y << ')');
			FORMAT('A', descname());
			FORMAT('a', name());
			case 'N':
				if (upcast(ConstructionSite const, constructionsite, this))
					result << constructionsite->building().descname();
				else
					result << descname();
				break;
			case 'n':
				if (upcast(ConstructionSite const, constructionsite, this))
					result << constructionsite->building().name();
				else
					result << name();
				break;
			case 'r':
				if (upcast(ProductionSite const, productionsite, this))
					result << productionsite->result_string();
				break;
			default: //  invalid format sequence
				result << '%' << *i.current;
				break;
			}
		} else
			result << *i.current;
	const std::string result_str = result.str();
	return result_str.empty() ? result_str : as_uifont(result_str);
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


WaresQueue & Building::waresqueue(Ware_Index const wi) {
	throw wexception
		("%s (%u) has no WaresQueue for %u",
		 name().c_str(), serial(), wi.value());
}

/*
===============
This function is called by workers in the buildingwork task.
Give the worker w a new task.
success is true if the previous task was finished successfully (without a
signal).
Return false if there's nothing to be done.
===============
*/
bool Building::get_building_work(Game &, Worker & worker, bool)
{
	throw wexception
		("MO(%u): get_building_work() for unknown worker %u",
		 serial(), worker.serial());
}


/**
 * Maintains the building leave queue. This ensures that workers don't leave
 * a building (in particular a military building or warehouse) all at once.
 * This is mostly for aesthetic purpose.
 *
 * \return \c true if the given worker can leave the building immediately.
 * Otherwise, the worker will be added to the buildings leave queue, and
 * \ref Worker::wakeup_leave_building() will be called as soon as the worker
 * can leave the building.
 *
 * \see Worker::start_task_leavebuilding(), leave_skip()
 */
bool Building::leave_check_and_wait(Game & game, Worker & w)
{
	if (&w == m_leave_allow.get(game)) {
		m_leave_allow = nullptr;
		return true;
	}

	// Check time and queue
	uint32_t const time = game.get_gametime();

	if (m_leave_queue.empty()) {
		if (m_leave_time <= time) {
			m_leave_time = time + BUILDING_LEAVE_INTERVAL;
			return true;
		}

		schedule_act(game, m_leave_time - time);
	}

	m_leave_queue.push_back(&w);
	return false;
}


/**
 * Indicate that the given worker wants to leave the building leave queue.
 * This function must be called when a worker aborts the waiting task for
 * some reason (e.g. the worker is carrying a ware, and the ware's transfer
 * has been cancelled).
 *
 * \see Building::leave_check_and_wait()
 */
void Building::leave_skip(Game &, Worker & w)
{
	Leave_Queue::iterator const it =
		std::find(m_leave_queue.begin(), m_leave_queue.end(), &w);

	if (it != m_leave_queue.end())
		m_leave_queue.erase(it);
}


/*
===============
Advance the leave queue.
===============
*/
void Building::act(Game & game, uint32_t const data)
{
	uint32_t const time = game.get_gametime();

	if (m_leave_time <= time) {
		bool wakeup = false;

		// Wake up one worker
		while (!m_leave_queue.empty()) {
			upcast(Worker, worker, m_leave_queue[0].get(game));

			m_leave_queue.erase(m_leave_queue.begin());

			if (worker) {
				m_leave_allow = worker;

				if (worker->wakeup_leave_building(game, *this)) {
					m_leave_time = time + BUILDING_LEAVE_INTERVAL;
					wakeup = true;
					break;
				}
			}
		}

		if (!m_leave_queue.empty())
			schedule_act(game, m_leave_time - time);

		if (!wakeup)
			m_leave_time = time; // make sure leave_time doesn't get too far behind
	}

	PlayerImmovable::act(game, data);
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
bool Building::fetch_from_flag(Game &)
{
	molog("TODO: Implement Building::fetch_from_flag\n");

	return false;
}


/*
===============
Draw the building.
===============
*/
void Building::draw
	(const Editor_Game_Base& game, RenderTarget& dst, const FCoords& coords, const Point& pos)
{
	if (coords == m_position) { // draw big buildings only once
		dst.drawanim
			(pos, m_anim, game.get_gametime() - m_animstart, get_owner());

		//  door animation?

		//  overlay strings (draw when enabled)
		draw_help(game, dst, coords, pos);
	}
}


/*
===============
Draw overlay help strings when enabled.
===============
*/
void Building::draw_help
	(const Editor_Game_Base& game, RenderTarget& dst, const FCoords&, const Point& pos)
{
	const Interactive_GameBase & igbase =
		ref_cast<Interactive_GameBase const, Interactive_Base const>
			(*game.get_ibase());
	uint32_t const dpyflags = igbase.get_display_flags();

	if (dpyflags & Interactive_Base::dfShowCensus) {
		const std::string info = info_string(igbase.building_census_format());
		if (!info.empty()) {
			dst.blit(pos - Point(0, 48), UI::g_fh1->render(info), CM_Normal, UI::Align_Center);
		}
	}

	if (dpyflags & Interactive_Base::dfShowStatistics) {
		if (upcast(Interactive_Player const, iplayer, &igbase))
			if
				(!iplayer->player().see_all() &&
				 iplayer->player().is_hostile(*get_owner()))
				return;
		const std::string info = info_string(igbase.building_statistics_format());
		if (!info.empty()) {
			dst.blit(pos - Point(0, 35), UI::g_fh1->render(info), CM_Normal, UI::Align_Center);
		}
	}
}

int32_t Building::get_priority
	(WareWorker type, Ware_Index const ware_index, bool adjust) const
{
	int32_t priority = DEFAULT_PRIORITY;
	if (type == wwWARE) {
		// if priority is defined for specific ware,
		// combine base priority and ware priority
		std::map<Ware_Index, int32_t>::const_iterator it =
			m_ware_priorities.find(ware_index);
		if (it != m_ware_priorities.end())
			priority = adjust
				? (priority * it->second / DEFAULT_PRIORITY)
				: it->second;
	}

	return priority;
}

/**
* Collect priorities assigned to wares of this building
* priorities are identified by ware type and index
 */
void Building::collect_priorities
	(std::map<int32_t, std::map<Ware_Index, int32_t> > & p) const
{
	if (m_ware_priorities.empty())
		return;
	std::map<Ware_Index, int32_t> & ware_priorities = p[wwWARE];
	std::map<Ware_Index, int32_t>::const_iterator it;
	for (it = m_ware_priorities.begin(); it != m_ware_priorities.end(); ++it) {
		if (it->second == DEFAULT_PRIORITY)
			continue;
		ware_priorities[it->first] = it->second;
	}
}

/**
* Set base priority for this building (applies for all wares)
 */
void Building::set_priority
	(int32_t    const type,
	 Ware_Index const ware_index,
	 int32_t    const new_priority)
{
	if (type == wwWARE) {
		m_ware_priorities[ware_index] = new_priority;
	}
}


void Building::log_general_info(const Editor_Game_Base & egbase) {
	PlayerImmovable::log_general_info(egbase);

	molog("m_position: (%i, %i)\n", m_position.x, m_position.y);
	molog("m_flag: %p\n", m_flag);
	molog
		("* position: (%i, %i)\n",
		 m_flag->get_position().x, m_flag->get_position().y);

	molog("m_anim: %s\n", descr().get_animation_name(m_anim).c_str());
	molog("m_animstart: %i\n", m_animstart);

	molog("m_leave_time: %i\n", m_leave_time);

	molog
		("m_leave_queue.size(): %lu\n",
		 static_cast<long unsigned int>(m_leave_queue.size()));
	molog("m_leave_allow.get(): %p\n", m_leave_allow.get(egbase));
}


void Building::add_worker(Worker & worker) {
	if (not get_workers().size()) {
		if (worker.descr().name() != "builder")
			set_seeing(true);
	}
	PlayerImmovable::add_worker(worker);
	workers_changed();
}


void Building::remove_worker(Worker & worker) {
	PlayerImmovable::remove_worker(worker);
	if (not get_workers().size())
		set_seeing(false);
	workers_changed();
}

/**
 * Change whether this building sees its vision range based on workers
 * inside the building.
 *
 * \note Warehouses always see their surroundings; this is handled separately.
 */
void Building::set_seeing(bool see)
{
	if (see == m_seeing)
		return;

	Player & player = owner();
	Map    & map    = player.egbase().map();

	if (see)
		player.see_area
			(Area<FCoords>(map.get_fcoords(get_position()), vision_range()));
	else
		player.unsee_area
			(Area<FCoords>(map.get_fcoords(get_position()), vision_range()));

	m_seeing = see;
}

/**
 * Send a message about this building to the owning player.
 *
 * It will have the building's coordinates, and display a picture of the
 * building in its description.
 *
 * \param msgsender a computer-readable description of why the message was sent
 * \param title user-visible title of the message
 * \param description user-visible message body, will be placed in an
 *   appropriate rich-text paragraph
 * \param link_to_building_lifetime if true, the message will expire when this
 *   building is removed from the game. Default is true
 * \param throttle_time if non-zero, the minimum time delay in milliseconds
 *   between messages of this type (see \p msgsender) within the
 *   given \p throttle_radius
 */
void Building::send_message
	(Game & game,
	 const std::string & msgsender,
	 const std::string & title,
	 const std::string & description,
	 bool link_to_building_lifetime,
	 uint32_t throttle_time,
	 uint32_t throttle_radius)
{
	const std::string& img = g_gr->animations().get_animation
		(descr().get_ui_anim()).representative_image(owner().get_playercolor()).hash();
	std::string rt_description;
	rt_description.reserve
		(strlen("<rt image=") + img.size() + 1 +
		 strlen("<p font-size=14 font-face=DejaVuSerif></p>") +
		 description.size() +
		 strlen("</rt>"));
	rt_description  = "<rt image=";
	rt_description += img;
	{
		std::string::iterator it = rt_description.end() - 1;
		for (; it != rt_description.begin() and *it != '?'; --it) {}
		for (;                                  *it == '?'; --it)
			*it = '0';
	}
	rt_description += "><p font-size=14 font-face=DejaVuSerif>";
	rt_description += description;
	rt_description += "</p></rt>";

	Message * msg = new Message
		(msgsender, game.get_gametime(), 60 * 60 * 1000,
		 title, rt_description, get_position(), (link_to_building_lifetime ? m_serial : 0));

	if (throttle_time)
		owner().add_message_with_timeout
			(game, *msg, throttle_time, throttle_radius);
	else
		owner().add_message(game, *msg);
}

}
