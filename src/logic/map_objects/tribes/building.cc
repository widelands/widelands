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

#include "logic/map_objects/tribes/building.h"

#include <cstdio>
#include <cstring>
#include <memory>
#include <sstream>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "base/macros.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/request.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/tribes.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"
#include "sound/sound_handler.h"
#include "wui/interactive_player.h"

namespace Widelands {

static const int32_t BUILDING_LEAVE_INTERVAL = 1000;

BuildingDescr::BuildingDescr(const std::string& init_descname,
                             const MapObjectType init_type,
                             const LuaTable& table,
                             const EditorGameBase& egbase)
   : MapObjectDescr(init_type, table.get_string("name"), init_descname, table),
     egbase_(egbase),
     buildable_(false),
     size_(BaseImmovable::SMALL),
     mine_(false),
     port_(false),
     enhancement_(INVALID_INDEX),
     enhanced_from_(INVALID_INDEX),
     enhanced_building_(false),
     hints_(table.get_table("aihints")),
     vision_range_(0) {
	if (!is_animation_known("idle")) {
		throw GameDataError("Building %s has no idle animation", table.get_string("name").c_str());
	}
	if (icon_filename().empty()) {
		throw GameDataError("Building %s needs a menu icon", table.get_string("name").c_str());
	}

	i18n::Textdomain td("tribes");

	// Partially finished buildings get their sizes from their associated building
	if (type() != MapObjectType::CONSTRUCTIONSITE && type() != MapObjectType::DISMANTLESITE) {
		try {
			const std::string size = table.get_string("size");
			if (boost::iequals(size, "small")) {
				size_ = BaseImmovable::SMALL;
			} else if (boost::iequals(size, "medium")) {
				size_ = BaseImmovable::MEDIUM;
			} else if (boost::iequals(size, "big")) {
				size_ = BaseImmovable::BIG;
			} else if (boost::iequals(size, "mine")) {
				size_ = BaseImmovable::SMALL;
				mine_ = true;
			} else if (boost::iequals(size, "port")) {
				size_ = BaseImmovable::BIG;
				port_ = true;
			} else {
				throw GameDataError("expected %s but found \"%s\"",
				                    "{\"small\"|\"medium\"|\"big\"|\"port\"|\"mine\"}", size.c_str());
			}
		} catch (const WException& e) {
			throw GameDataError("size: %s", e.what());
		}
	}

	// Parse build options
	destructible_ = table.has_key("destructible") ? table.get_bool("destructible") : true;

	if (table.has_key("enhancement")) {
		const std::string enh = table.get_string("enhancement");

		if (enh == name()) {
			throw wexception("enhancement to same type");
		}
		DescriptionIndex const en_i = egbase_.tribes().building_index(enh);
		if (egbase_.tribes().building_exists(en_i)) {
			enhancement_ = en_i;

			//  Merge the enhancements workarea info into this building's
			//  workarea info.
			const BuildingDescr* tmp_enhancement = egbase_.tribes().get_building_descr(en_i);
			for (auto area : tmp_enhancement->workarea_info_) {
				std::set<std::string>& strs = workarea_info_[area.first];
				for (std::string str : area.second)
					strs.insert(str);
			}
		} else {
			throw wexception(
			   "\"%s\" has not been defined as a building type (wrong declaration order?)",
			   enh.c_str());
		}
	}

	if (table.has_key("buildcost")) {
		buildable_ = true;
		try {
			buildcost_ = Buildcost(table.get_table("buildcost"), egbase_.tribes());
			return_dismantle_ = Buildcost(table.get_table("return_on_dismantle"), egbase_.tribes());
		} catch (const WException& e) {
			throw wexception(
			   "A buildable building must define \"buildcost\" and \"return_on_dismantle\": %s",
			   e.what());
		}
	}
	if (table.has_key("enhancement_cost")) {
		enhanced_building_ = true;
		try {
			enhance_cost_ = Buildcost(table.get_table("enhancement_cost"), egbase_.tribes());
			return_enhanced_ =
			   Buildcost(table.get_table("return_on_dismantle_on_enhanced"), egbase_.tribes());
		} catch (const WException& e) {
			throw wexception("An enhanced building must define \"enhancement_cost\""
			                 "and \"return_on_dismantle_on_enhanced\": %s",
			                 e.what());
		}
	}

	helptext_script_ = table.get_string("helptext_script");

	needs_seafaring_ = table.has_key("needs_seafaring") ? table.get_bool("needs_seafaring") : false;

	if (table.has_key("vision_range")) {
		vision_range_ = table.get_int("vision_range");
	}
}

Building& BuildingDescr::create(EditorGameBase& egbase,
                                Player& owner,
                                Coords const pos,
                                bool const construct,
                                bool loading,
                                Building::FormerBuildings const former_buildings) const {
	Building& b = construct ? create_constructionsite() : create_object();
	b.position_ = pos;
	b.set_owner(&owner);
	for (DescriptionIndex idx : former_buildings) {
		b.old_buildings_.push_back(idx);
	}
	if (loading) {
		b.Building::init(egbase);
		return b;
	}
	b.init(egbase);
	return b;
}

int32_t BuildingDescr::suitability(const Map&, const FCoords& fc) const {
	return size_ <= (fc.field->nodecaps() & Widelands::BUILDCAPS_SIZEMASK);
}

/**
 * Normal buildings don't conquer anything, so this returns 0 by default.
 *
 * \return the radius of the conquered area.
 */
uint32_t BuildingDescr::get_conquers() const {
	return 0;
}

/**
 * \return the radius (in number of fields) of the area seen by this
 * building.
 */
uint32_t BuildingDescr::vision_range() const {
	return vision_range_ ? vision_range_ : get_conquers() + 4;
}

/*
===============
Create a construction site for this type of building
===============
*/
Building& BuildingDescr::create_constructionsite() const {
	BuildingDescr const* const descr =
	   egbase_.tribes().get_building_descr(egbase_.tribes().safe_building_index("constructionsite"));
	ConstructionSite& csite = dynamic_cast<ConstructionSite&>(descr->create_object());
	csite.set_building(*this);

	return csite;
}

/*
==============================

Implementation

==============================
*/

Building::Building(const BuildingDescr& building_descr)
   : PlayerImmovable(building_descr),
     flag_(nullptr),
     anim_(0),
     animstart_(0),
     leave_time_(0),
     defeating_player_(0),
     seeing_(false) {
}

void Building::load_finish(EditorGameBase& egbase) {
	auto should_be_deleted = [&egbase, this](const OPtr<Worker>& optr) {
		Worker& worker = *optr.get(egbase);
		OPtr<PlayerImmovable> const worker_location = worker.get_location();
		if (worker_location.serial() != serial() &&
		    worker_location.serial() != base_flag().serial()) {
			log("WARNING: worker %u is in the leave queue of building %u with "
			    "base flag %u but is neither inside the building nor at the "
			    "flag!\n",
			    worker.serial(), serial(), base_flag().serial());
			return true;
		}

		Bob::State const* const state = worker.get_state(Worker::taskLeavebuilding);
		if (!state) {
			log("WARNING: worker %u is in the leave queue of building %u but "
			    "does not have a leavebuilding task! Removing from queue.\n",
			    worker.serial(), serial());
			return true;
		}

		if (state->objvar1 != this) {
			log("WARNING: worker %u is in the leave queue of building %u but its "
			    "leavebuilding task is for map object %u! Removing from queue.\n",
			    worker.serial(), serial(), state->objvar1.serial());
			return true;
		}
		return false;
	};

	leave_queue_.erase(std::remove_if(leave_queue_.begin(), leave_queue_.end(), should_be_deleted),
	                   leave_queue_.end());
}

int32_t Building::get_size() const {
	return descr().get_size();
}

bool Building::get_passable() const {
	return false;
}

Flag& Building::base_flag() {
	return *flag_;
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
	const BuildingDescr& tmp_descr = descr();
	if (tmp_descr.is_destructible()) {
		caps |= PCap_Bulldoze;
		if (tmp_descr.is_buildable() || tmp_descr.is_enhanced())
			caps |= PCap_Dismantle;
	}
	if (tmp_descr.enhancement() != INVALID_INDEX)
		caps |= PCap_Enhancable;
	return caps;
}

void Building::start_animation(EditorGameBase& egbase, uint32_t const anim) {
	anim_ = anim;
	animstart_ = egbase.get_gametime();
}

/*
===============
Common building initialization code. You must call this from
derived class' init.
===============
*/
void Building::init(EditorGameBase& egbase) {
	PlayerImmovable::init(egbase);

	// Set the building onto the map
	Map& map = egbase.map();
	Coords neighb;

	set_position(egbase, position_);

	if (get_size() == BIG) {
		map.get_ln(position_, &neighb);
		set_position(egbase, neighb);

		map.get_tln(position_, &neighb);
		set_position(egbase, neighb);

		map.get_trn(position_, &neighb);
		set_position(egbase, neighb);
	}

	// Make sure the flag is there

	map.get_brn(position_, &neighb);
	{
		Flag* flag = dynamic_cast<Flag*>(map.get_immovable(neighb));
		if (!flag)
			flag = new Flag(egbase, owner(), neighb);
		flag_ = flag;
		flag->attach_building(egbase, *this);
	}

	// Start the animation
	if (descr().is_animation_known("unoccupied"))
		start_animation(egbase, descr().get_animation("unoccupied"));
	else
		start_animation(egbase, descr().get_animation("idle"));

	leave_time_ = egbase.get_gametime();
}

void Building::cleanup(EditorGameBase& egbase) {
	if (defeating_player_) {
		Player& defeating_player = egbase.player(defeating_player_);
		if (descr().get_conquers()) {
			owner().count_msite_lost();
			defeating_player.count_msite_defeated();
		} else {
			owner().count_civil_bld_lost();
			defeating_player.count_civil_bld_defeated();
		}
	}

	// Remove from flag
	flag_->detach_building(egbase);

	// Unset the building
	unset_position(egbase, position_);

	if (get_size() == BIG) {
		Map& map = egbase.map();
		Coords neighb;

		map.get_ln(position_, &neighb);
		unset_position(egbase, neighb);

		map.get_tln(position_, &neighb);
		unset_position(egbase, neighb);

		map.get_trn(position_, &neighb);
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
bool Building::burn_on_destroy() {
	return true;
}

/**
 * Return all positions on the map that we occupy
 */
BaseImmovable::PositionList Building::get_positions(const EditorGameBase& egbase) const {
	PositionList rv;

	rv.push_back(position_);
	if (get_size() == BIG) {
		Map& map = egbase.map();
		Coords neighb;

		map.get_ln(position_, &neighb);
		rv.push_back(neighb);

		map.get_tln(position_, &neighb);
		rv.push_back(neighb);

		map.get_trn(position_, &neighb);
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
void Building::destroy(EditorGameBase& egbase) {
	Notifications::publish(NoteBuildingWindow(serial(), NoteBuildingWindow::Action::kClose));
	const bool fire = burn_on_destroy();
	const Coords pos = position_;
	PlayerImmovable::destroy(egbase);
	// We are deleted. Only use stack variables beyond this point
	if (fire)
		egbase.create_immovable(pos, "destroyed_building", MapObjectDescr::OwnerType::kTribe);
}

std::string Building::info_string(const InfoStringFormat& format) {
	std::string result = "";
	switch (format) {
	case InfoStringFormat::kCensus:
		if (upcast(ConstructionSite const, constructionsite, this)) {
			result = constructionsite->building().descname();
		} else {
			result = descr().descname();
		}
		break;
	case InfoStringFormat::kStatistics:
		result = update_and_get_statistics_string();
		break;
	case InfoStringFormat::kTooltip:
		if (upcast(ProductionSite const, productionsite, this)) {
			result = productionsite->production_result();
		}
	}
	return result;
}

WaresQueue& Building::waresqueue(DescriptionIndex const wi) {
	throw wexception("%s (%u) has no WaresQueue for %u", descr().name().c_str(), serial(), wi);
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
bool Building::get_building_work(Game&, Worker& worker, bool) {
	throw wexception("MO(%u): get_building_work() for unknown worker %u", serial(), worker.serial());
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
bool Building::leave_check_and_wait(Game& game, Worker& w) {
	if (&w == leave_allow_.get(game)) {
		leave_allow_ = nullptr;
		return true;
	}

	// Check time and queue
	uint32_t const time = game.get_gametime();

	if (leave_queue_.empty()) {
		if (leave_time_ <= time) {
			leave_time_ = time + BUILDING_LEAVE_INTERVAL;
			return true;
		}

		schedule_act(game, leave_time_ - time);
	}

	leave_queue_.push_back(&w);
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
void Building::leave_skip(Game&, Worker& w) {
	LeaveQueue::iterator const it = std::find(leave_queue_.begin(), leave_queue_.end(), &w);

	if (it != leave_queue_.end())
		leave_queue_.erase(it);
}

/*
===============
Advance the leave queue.
===============
*/
void Building::act(Game& game, uint32_t const data) {
	uint32_t const time = game.get_gametime();

	if (leave_time_ <= time) {
		bool wakeup = false;

		// Wake up one worker
		while (!leave_queue_.empty()) {
			upcast(Worker, worker, leave_queue_[0].get(game));

			leave_queue_.erase(leave_queue_.begin());

			if (worker) {
				leave_allow_ = worker;

				if (worker->wakeup_leave_building(game, *this)) {
					leave_time_ = time + BUILDING_LEAVE_INTERVAL;
					wakeup = true;
					break;
				}
			}
		}

		if (!leave_queue_.empty())
			schedule_act(game, leave_time_ - time);

		if (!wakeup)
			leave_time_ = time;  // make sure leave_time doesn't get too far behind
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
bool Building::fetch_from_flag(Game&) {
	molog("TODO(unknown): Implement Building::fetch_from_flag\n");

	return false;
}


void Building::draw(uint32_t gametime,
                    const DrawText draw_text,
                    const Vector2f& point_on_dst,
                    const float scale,
                    RenderTarget* dst) {
	dst->blit_animation(
	   point_on_dst, scale, anim_, gametime - animstart_, get_owner()->get_playercolor());

	//  door animation?

	//  overlay strings (draw when enabled)
	draw_info(draw_text, point_on_dst, scale, dst);
}

/*
===============
Draw overlay help strings when enabled.
===============
*/
void Building::draw_info(const DrawText draw_text,
                         const Vector2f& point_on_dst,
								 const float scale,
                         RenderTarget* dst) {
	const std::string statistics_string =
	   (draw_text & DrawText::kStatistics) ? info_string(InfoStringFormat::kStatistics) : "";
	do_draw_info(draw_text, info_string(InfoStringFormat::kCensus), statistics_string, point_on_dst,
	             scale, dst);
}

int32_t
Building::get_priority(WareWorker type, DescriptionIndex const ware_index, bool adjust) const {
	int32_t priority = DEFAULT_PRIORITY;
	if (type == wwWARE) {
		// if priority is defined for specific ware,
		// combine base priority and ware priority
		std::map<DescriptionIndex, int32_t>::const_iterator it = ware_priorities_.find(ware_index);
		if (it != ware_priorities_.end())
			priority = adjust ? (priority * it->second / DEFAULT_PRIORITY) : it->second;
	}

	return priority;
}

/**
* Collect priorities assigned to wares of this building
* priorities are identified by ware type and index
 */
void Building::collect_priorities(std::map<int32_t, std::map<DescriptionIndex, int32_t>>& p) const {
	if (ware_priorities_.empty())
		return;
	std::map<DescriptionIndex, int32_t>& ware_priorities = p[wwWARE];
	std::map<DescriptionIndex, int32_t>::const_iterator it;
	for (it = ware_priorities_.begin(); it != ware_priorities_.end(); ++it) {
		if (it->second == DEFAULT_PRIORITY)
			continue;
		ware_priorities[it->first] = it->second;
	}
}

/**
* Set base priority for this building (applies for all wares)
 */
void Building::set_priority(int32_t const type,
                            DescriptionIndex const ware_index,
                            int32_t const new_priority) {
	if (type == wwWARE) {
		ware_priorities_[ware_index] = new_priority;
	}
}

void Building::log_general_info(const EditorGameBase& egbase) {
	PlayerImmovable::log_general_info(egbase);

	molog("position: (%i, %i)\n", position_.x, position_.y);
	molog("flag: %p\n", flag_);
	molog("* position: (%i, %i)\n", flag_->get_position().x, flag_->get_position().y);

	molog("anim: %s\n", descr().get_animation_name(anim_).c_str());
	molog("animstart: %i\n", animstart_);

	molog("leave_time: %i\n", leave_time_);

	molog("leave_queue.size(): %lu\n", static_cast<long unsigned int>(leave_queue_.size()));
	molog("leave_allow.get(): %p\n", leave_allow_.get(egbase));
}

void Building::add_worker(Worker& worker) {
	if (!get_workers().size()) {
		if (owner().tribe().safe_worker_index(worker.descr().name()) != owner().tribe().builder()) {
			set_seeing(true);
		}
	}
	PlayerImmovable::add_worker(worker);
	Notifications::publish(
	   NoteBuildingWindow(serial(), NoteBuildingWindow::Action::kWorkersChanged));
}

void Building::remove_worker(Worker& worker) {
	PlayerImmovable::remove_worker(worker);
	if (!get_workers().size())
		set_seeing(false);
	Notifications::publish(
	   NoteBuildingWindow(serial(), NoteBuildingWindow::Action::kWorkersChanged));
}

/**
 * Change whether this building sees its vision range based on workers
 * inside the building.
 *
 * \note Warehouses always see their surroundings; this is handled separately.
 */
void Building::set_seeing(bool see) {
	if (see == seeing_)
		return;

	Player& player = owner();
	Map& map = player.egbase().map();

	if (see)
		player.see_area(Area<FCoords>(map.get_fcoords(get_position()), descr().vision_range()));
	else
		player.unsee_area(Area<FCoords>(map.get_fcoords(get_position()), descr().vision_range()));

	seeing_ = see;
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
 * \param link_to_building_lifetime if true, the message will be deleted when this
 *   building is removed from the game. Default is true
 * \param throttle_time if non-zero, the minimum time delay in milliseconds
 *   between messages of this type (see \p msgsender) within the
 *   given \p throttle_radius
 */
void Building::send_message(Game& game,
                            const Message::Type msgtype,
                            const std::string& title,
                            const std::string& icon_filename,
                            const std::string& heading,
                            const std::string& description,
                            bool link_to_building_lifetime,
                            uint32_t throttle_time,
                            uint32_t throttle_radius) {
	// TODO(sirver): add support into the font renderer to get to representative
	// animations of buildings so that the messages can still be displayed, even
	// after reload.
	const std::string& img = descr().representative_image_filename();
	std::string rt_description;
	rt_description.reserve(strlen("<rt image=") + img.size() + 1 +
	                       strlen("<p font-size=14 font-face=serif>") + description.size() +
	                       strlen("</p></rt>"));
	rt_description = "<rt image=";
	rt_description += img;
	{
		std::string::iterator it = rt_description.end() - 1;
		for (; it != rt_description.begin() && *it != '?'; --it) {
		}
		for (; *it == '?'; --it)
			*it = '0';
	}
	rt_description = (boost::format("%s><p font-face=serif font-size=14>%s</p></rt>") %
	                  rt_description % description)
	                    .str();

	Message* msg =
	   new Message(msgtype, game.get_gametime(), title, icon_filename, heading, rt_description,
	               get_position(), (link_to_building_lifetime ? serial_ : 0));

	if (throttle_time)
		owner().add_message_with_timeout(game, *msg, throttle_time, throttle_radius);
	else
		owner().add_message(game, *msg);
}
}
