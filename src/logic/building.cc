/*
 * Copyright (C) 2002-2004, 2006-2009 by the Widelands Development Team
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

#include "constructionsite.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "font_handler.h"
#include "game.h"
#include "game_data_error.h"
#include "wui/interactive_gamebase.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "map.h"
#include "player.h"
#include "productionsite.h"
#include "profile/profile.h"
#include "graphic/rendertarget.h"
#include "sound/sound_handler.h"
#include "tribe.h"
#include "upcast.h"
#include "wexception.h"
#include "worker.h"

#include <cstdio>
#include <sstream>

namespace Widelands {

static const int32_t BUILDING_LEAVE_INTERVAL = 1000;


Building_Descr::Building_Descr
	(char const * const _name, char const * const _descname,
	 std::string const & directory, Profile & prof, Section & global_s,
	 Tribe_Descr const & _descr, EncodeData const * const encdata)
	:
	Map_Object_Descr(_name, _descname),
	m_tribe         (_descr),
	m_buildable     (true),
	m_buildicon     (g_gr->get_no_picture()),
	m_size          (BaseImmovable::SMALL),
	m_mine          (false),
	m_hints         (prof.get_section("aihints")),
#ifdef WRITE_GAME_DATA_AS_HTML
	m_global        (false),
#endif
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
		} else
			throw game_data_error
				(_("expected %s but found \"%s\""),
				 "{\"small\"|\"medium\"|\"big\"}", string);
	} catch (_wexception const & e) {
		throw game_data_error("size: %s", e.what());
	}

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
				Building_Descr const & enhancement =
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
					("\"%s\" has not beed defined as a building type (wrong "
					 "declaration order?)",
					 target_name.c_str());
		} catch (_wexception const & e) {
			throw wexception
				("\"enhancements=%s\": %s", v->get_string(), e.what());
		}
	m_enhanced_building = global_s.get_bool("enhanced_building", false);
	if (m_buildable || m_enhanced_building) {
		//  get build icon
		m_buildicon_fname  = directory;
		m_buildicon_fname += "/menu.png";

		//  build animation
		if (Section * const build_s = prof.get_section("build")) {
			if (build_s->get_int("fps", -1) != -1)
				throw wexception("fps defined for build animation!");
			if (!is_animation_known("build"))
				add_animation
					("build",
					 g_anim.get(directory.c_str(), *build_s, 0, encdata));
		}

		// Get costs
		Section & buildcost_s = prof.get_safe_section("buildcost");
		while (Section::Value const * const val = buildcost_s.get_next_val())
			try {
				if (Ware_Index const idx = m_tribe.ware_index(val->get_name())) {
					if (m_buildcost.count(idx))
						throw wexception
							("a buildcost item of this ware type has already been "
							 "defined");
					int32_t const value = val->get_int();
					if (value < 1 or 255 < value)
						throw wexception("count is out of range 1 .. 255");
					m_buildcost.insert(std::pair<Ware_Index, uint8_t>(idx, value));
				} else
					throw wexception
						("tribe does not define a ware type with this name");
			} catch (_wexception const & e) {
				throw wexception
					("[buildcost] \"%s=%s\": %s",
					 val->get_name(), val->get_string(), e.what());
			}
	}

	{ //  parse basic animation data
		Section & idle_s = prof.get_safe_section("idle");
		if (!is_animation_known("idle"))
			add_animation
				("idle",
				 g_anim.get(directory.c_str(), idle_s, 0, encdata));
	}

	while (Section::Value const * const v = global_s.get_next_val("soundfx"))
		g_sound_handler.load_fx(directory, v->get_string());

	m_vision_range = global_s.get_int("vision_range");
#ifdef WRITE_GAME_DATA_AS_HTML
	m_global = directory.find("global/") < directory.size();
#endif
}


/*
===============
Create a building of this type. Does not perform any sanity checks.

if old != 0 this is an enhancing
===============
*/
Building & Building_Descr::create
	(Editor_Game_Base     &       egbase,
	 Player               &       owner,
	 Coords                 const pos,
	 bool                   const construct,
	 uint32_t       const * const ware_counts,
	 uint32_t       const * const worker_counts,
	 Soldier_Counts const * const soldier_counts,
	 Building_Descr const * const old)
	const
{
	Building & b = construct ? create_constructionsite(old) : create_object();
	b.m_position = pos;
	b.set_owner(&owner);
	b.prefill
		(ref_cast<Game, Editor_Game_Base>(egbase),
		 ware_counts, worker_counts, soldier_counts);
	b.init(egbase);
	b.postfill
		(ref_cast<Game, Editor_Game_Base>(egbase),
		 ware_counts, worker_counts, soldier_counts);

	return b;
}


int32_t Building_Descr::suitability(Map const &, FCoords const fc) const {
	return m_size <= (fc.field->get_caps() & Widelands::BUILDCAPS_SIZEMASK);
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
uint32_t Building_Descr::vision_range() const throw ()
{
	if (m_vision_range > 0)
		return m_vision_range;
	else
		return get_conquers() + 4;
}


/*
===============
Called whenever building graphics need to be loaded.
===============
*/
void Building_Descr::load_graphics()
{
	if (m_buildicon_fname.size())
		m_buildicon = g_gr->get_picture(PicMod_Game, m_buildicon_fname.c_str());
}

/*
===============
Create a construction site for this type of building

if old != 0 this is an enhancement from an older building
===============
*/
Building & Building_Descr::create_constructionsite
	(Building_Descr const * const old) const
{
	if
		(Building_Descr const * const descr =
		 	m_tribe.get_building_descr
		 		(m_tribe.building_index("constructionsite")))
	{

		ConstructionSite & csite =
			ref_cast<ConstructionSite, Map_Object>(descr->create_object());
		csite.set_building(*this);
		if (old)
			csite.set_previous_building(old);

		return csite;
	} else
		throw wexception
			("Tribe %s has no constructionsite", m_tribe.name().c_str());
}


/*
==============================

Implementation

==============================
*/

Building::Building(const Building_Descr & building_descr) :
PlayerImmovable(building_descr),
m_optionswindow(0),
m_flag         (0),
m_defeating_player(0),
m_priority (DEFAULT_PRIORITY)
{}

Building::~Building()
{
	if (m_optionswindow)
		hide_options();
}

int32_t Building::get_type() const throw () {return BUILDING;}

int32_t Building::get_size() const throw () {return descr().get_size();}

bool Building::get_passable() const throw () {return false;}

Flag & Building::base_flag()
{
	return *m_flag;
}


/**
 * \return a bitfield of commands the owning player can issue for this building.
 *
 * The bits are (1 << PCap_XXX).
 * By default, all buildings can be bulldozed. If a building should not be
 * destructible, "destructible=no" must be added to buildings conf.
 */
uint32_t Building::get_playercaps() const throw () {
	uint32_t caps = 0;
	if (descr().destructible())
		caps                                |= 1 << PCap_Bulldoze;
	if (descr().enhancements().size())
		caps |= 1 << PCap_Enhancable;
	return caps;
}


std::string const & Building::name() const throw () {return descr().name();}


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
				new Flag
					(ref_cast<Game, Editor_Game_Base>(egbase), owner(), neighb);
		m_flag = flag;
		flag->attach_building(egbase, *this);
	}

	// Start the animation
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
	Tribe_Descr const & t = tribe();
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
std::string Building::info_string(std::string const & format) {
	std::ostringstream result;
	container_iterate_const(std::string, format, i)
		if (*i.current == '%') {
			if (++i.current == i.end) { //  unterminated format sequence
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
			}
		} else
			result << *i.current;
	return result.str();
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


void Building::prefill
	(Game &, uint32_t const *, uint32_t const *, Soldier_Counts const *)
{}
void Building::postfill
	(Game &, uint32_t const *, uint32_t const *, Soldier_Counts const *)
{}


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
		m_leave_allow = 0;
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
		std::find(m_leave_queue.begin(), m_leave_queue.end(), Object_Ptr(&w));

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

	if (static_cast<int32_t>(time - m_leave_time) >= 0)
	{
		bool wakeup = false;

		// Wake up one worker
		while (m_leave_queue.size()) {
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

		if (m_leave_queue.size())
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
	(Editor_Game_Base const &       game,
	 RenderTarget           &       dst,
	 FCoords                  const coords,
	 Point                    const pos)
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
	(Editor_Game_Base const &       game,
	 RenderTarget           &       dst,
	 FCoords,
	 Point                    const pos)
{
	Interactive_GameBase const & igbase =
		ref_cast<Interactive_GameBase const, Interactive_Base const>
			(*game.get_ibase());
	uint32_t const dpyflags = igbase.get_display_flags();

	if (dpyflags & Interactive_Base::dfShowCensus) {
		//  TODO make more here
		UI::g_fh->draw_string
			(dst,
			 UI_FONT_SMALL,
			 UI_FONT_SMALL_CLR,
			 pos - Point(0, 45),
			 info_string(igbase.building_census_format()),
			 UI::Align_Center);
	}

	if (dpyflags & Interactive_Base::dfShowStatistics) {
		UI::g_fh->draw_string
			(dst,
			 UI_FONT_SMALL,
			 UI_FONT_SMALL_CLR,
			 pos - Point(0, 35),
			 info_string(igbase.building_statistics_format()),
			 UI::Align_Center);
	}
}

/**
* Get priority of a requested ware.
* Currently always returns base priority - to be extended later
 */
int32_t Building::get_priority
	(int32_t const type, Ware_Index const ware_index, bool const adjust) const
{
	int32_t priority = m_priority;

	if (type == Request::WARE) {
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
	std::map<Ware_Index, int32_t> & ware_priorities = p[Request::WARE];
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
void Building::set_priority(int32_t const new_priority) {
	m_priority = new_priority;
}

void Building::set_priority
	(int32_t    const type,
	 Ware_Index const ware_index,
	 int32_t    const new_priority)
{
	if (type == Request::WARE) {
		m_ware_priorities[ware_index] = new_priority;
	}
}


void Building::log_general_info(Editor_Game_Base const & egbase) {
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
		//  The first worker will enter the building so it should start seeing.
		Player & player = owner();
		Map    & map    = player.egbase().map();
		player.see_area
			(Area<FCoords>(map.get_fcoords(get_position()), vision_range()));
	}
	PlayerImmovable::add_worker(worker);
}


void Building::remove_worker(Worker & worker) {
	PlayerImmovable::remove_worker(worker);
	if (not get_workers().size()) {
		//  The last worker has left the building so it should stop seeing.
		Player & player = owner();
		Map    & map    = player.egbase().map();
		player.unsee_area
			(Area<FCoords>(map.get_fcoords(get_position()), vision_range()));
	}
}

}
