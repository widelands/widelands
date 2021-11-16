/*
 * Copyright (C) 2002-2021 by the Widelands Development Team
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

#include <memory>

#include "base/log.h"
#include "base/macros.h"
#include "base/multithreading.h"
#include "base/string.h"
#include "base/wexception.h"
#include "economy/flag.h"
#include "economy/input_queue.h"
#include "economy/request.h"
#include "graphic/rendertarget.h"
#include "graphic/text_layout.h"
#include "io/filesystem/filesystem.h"
#include "io/filesystem/layered_filesystem.h"
#include "logic/game.h"
#include "logic/game_data_error.h"
#include "logic/map.h"
#include "logic/map_objects/descriptions.h"
#include "logic/map_objects/immovable.h"
#include "logic/map_objects/tribes/constructionsite.h"
#include "logic/map_objects/tribes/productionsite.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/player.h"

namespace Widelands {

static const Duration kBuildingLeaveInterval = Duration(1000);
/**
 * The contents of 'table' are documented in doc/sphinx/source/lua_tribes_buildings.rst.org
 */
BuildingDescr::BuildingDescr(const std::string& init_descname,
                             const MapObjectType init_type,
                             const LuaTable& table,
                             Descriptions& descriptions)
   : MapObjectDescr(init_type, table.get_string("name"), init_descname, table),
     descriptions_(descriptions),
     buildable_(table.has_key("buildcost")),
     can_be_dismantled_(table.has_key("return_on_dismantle")),
     destructible_(table.has_key("destructible") ? table.get_bool("destructible") : true),
     size_(BaseImmovable::SMALL),
     mine_(false),
     port_(false),
     enhancement_(INVALID_INDEX),
     enhanced_from_(INVALID_INDEX),
     enhanced_building_(false),
     hints_(table.get_table("aihints"), name()),
     vision_range_(0) {
	if (!is_animation_known("idle")) {
		throw GameDataError("Building %s has no idle animation", name().c_str());
	}
	if (icon_filename().empty()) {
		throw GameDataError("Building %s needs a menu icon", name().c_str());
	}

	// Partially finished buildings get their sizes from their associated building
	if (type() != MapObjectType::CONSTRUCTIONSITE && type() != MapObjectType::DISMANTLESITE) {
		try {
			const std::string size = table.get_string("size");
			if (iequals(size, "small")) {
				size_ = BaseImmovable::SMALL;
			} else if (iequals(size, "medium")) {
				size_ = BaseImmovable::MEDIUM;
			} else if (iequals(size, "big")) {
				size_ = BaseImmovable::BIG;
			} else if (iequals(size, "mine")) {
				size_ = BaseImmovable::SMALL;
				mine_ = true;
			} else if (iequals(size, "port")) {
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
	if (table.has_key("built_over_immovable")) {
		// Throws an error if no matching immovable exists
		built_over_immovable_ = get_attribute_id(table.get_string("built_over_immovable"), false);
	} else {
		built_over_immovable_ = INVALID_INDEX;
	}

	// Parse build options
	if (table.has_key("enhancement")) {
		// TODO(GunChleoc): Compatibility code, remove "if" section after v1.0.
		// The "else" branch with get_table is the current code.
		if (table.get_datatype("enhancement") == LuaTable::DataType::kString) {
			const std::string enh = table.get_string("enhancement");
			log_warn("Deprecated enhancement code found in building '%s'", name().c_str());

			if (enh == name()) {
				throw wexception("enhancement to same type");
			}
			DescriptionIndex const en_i = descriptions.load_building(enh);
			if (descriptions.building_exists(en_i)) {
				enhancement_ = en_i;

				//  Merge the enhancements workarea info into this building's
				//  workarea info.
				const BuildingDescr* tmp_enhancement = descriptions.get_building_descr(en_i);
				for (const auto& area : tmp_enhancement->workarea_info_) {
					std::set<std::string>& strs = workarea_info_[area.first];
					for (const std::string& str : area.second) {
						strs.insert(str);
					}
				}
			} else {
				throw GameDataError(
				   "\"%s\" has not been defined as a building type (wrong declaration order?)",
				   enh.c_str());
			}
		} else {
			std::unique_ptr<LuaTable> enhancement_table = table.get_table("enhancement");
			set_enhancement(descriptions, *enhancement_table);
		}
	}

	// We define a building as buildable if it has a "buildcost" table.
	// A buildable building must also define "return_on_dismantle".
	// However, we support "return_on_dismantle" without "buildable", because this is used by custom
	// scenario buildings.
	if (table.has_key("return_on_dismantle")) {
		returns_on_dismantle_ = Buildcost(table.get_table("return_on_dismantle"), descriptions);
	}
	if (table.has_key("buildcost")) {
		if (!table.has_key("return_on_dismantle")) {
			throw wexception(
			   "The building '%s' has a \"buildcost\" but no \"return_on_dismantle\"", name().c_str());
		}
		buildcost_ = Buildcost(table.get_table("buildcost"), descriptions);
	}

	if (table.has_key("enhancement_cost")) {
		// TODO(GunChleoc): Compatibility code, remove after v1.0
		log_warn("Deprecated enhancement_cost code found in building '%s'", name().c_str());
		if (!table.has_key("return_on_dismantle_on_enhanced")) {
			throw GameDataError("The enhanced building '%s' has an \"enhancement_cost\" but no "
			                    "\"return_on_dismantle_on_enhanced\"",
			                    name().c_str());
		}
		set_enhancement_cost(
		   Buildcost(table.get_table("enhancement_cost"), descriptions),
		   Buildcost(table.get_table("return_on_dismantle_on_enhanced"), descriptions));
	}

	needs_seafaring_ = false;
	needs_waterways_ = false;
	if (table.has_key("map_check")) {
		for (const std::string& map_check :
		     table.get_table("map_check")->array_entries<std::string>()) {
			if (map_check == "seafaring") {
				needs_seafaring_ = true;
			} else if (map_check == "waterways") {
				needs_waterways_ = true;
			} else {
				throw GameDataError(
				   "Unexpected map_check item '%s' in building description", map_check.c_str());
			}
		}
	}

	if (table.has_key("vision_range")) {
		vision_range_ = table.get_int("vision_range");
	}
}

void BuildingDescr::set_enhancement(Descriptions& descriptions, LuaTable& enhancement_table) {
	const std::string enhancement_name(enhancement_table.get_string("name"));
	if (enhancement_name == name()) {
		throw GameDataError("Building '%s' has enhancement to same type", name().c_str());
	}
	DescriptionIndex const enhancement_idx = descriptions.load_building(enhancement_name);
	if (!descriptions.building_exists(enhancement_idx)) {
		throw GameDataError(
		   "'%s' has enhancement to unknown building '%s'", name().c_str(), enhancement_name.c_str());
	}

	enhancement_ = enhancement_idx;
	BuildingDescr* enhanced_building = descriptions.get_mutable_building_descr(enhancement_idx);
	enhanced_building->set_enhancement_cost(
	   Buildcost(enhancement_table.get_table("enhancement_cost"), descriptions),
	   Buildcost(enhancement_table.get_table("enhancement_return_on_dismantle"), descriptions));

	// Merge the enhancements workarea info into this building's workarea info
	for (const auto& area : enhanced_building->workarea_info_) {
		std::set<std::string>& strs = workarea_info_[area.first];
		for (const std::string& str : area.second) {
			strs.insert(str);
		}
	}
}

void BuildingDescr::set_owning_tribe(const std::string& t) {
	if (!owning_tribe_.empty()) {
		throw GameDataError(
		   "Building %s already belongs to tribe %s; cannot belong to tribe %s as well",
		   name().c_str(), owning_tribe_.c_str(), t.c_str());
	}
	owning_tribe_ = t;
}

Building& BuildingDescr::create(EditorGameBase& egbase,
                                Player* owner,
                                Coords const pos,
                                bool const construct,
                                bool loading,
                                const FormerBuildings& former_buildings) const {
	DescriptionIndex immovable = INVALID_INDEX;
	if (built_over_immovable_ != INVALID_INDEX && !loading) {
		bool immovable_previously_found = false;
		for (const auto& pair : former_buildings) {
			// 'false' means we're building on top of an immovable
			if (!pair.second) {
				const MapObjectDescr* d = egbase.descriptions().get_immovable_descr(pair.first);
				if (d->has_attribute(built_over_immovable_)) {
					immovable_previously_found = true;
					break;
				}
			}
		}
		if (!immovable_previously_found) {
			// Must be done first, because the immovable will be gone the moment the building is placed
			const FCoords f = egbase.map().get_fcoords(pos);
			if (f.field->get_immovable() &&
			    f.field->get_immovable()->has_attribute(built_over_immovable_)) {
				upcast(const ImmovableDescr, imm, &f.field->get_immovable()->descr());
				assert(imm);
				immovable = egbase.descriptions().immovable_index(imm->name());
			} else {
				throw wexception(
				   "Attempting to build %s at %dx%d – no immovable with required attribute %i found",
				   name().c_str(), pos.x, pos.y, built_over_immovable_);
			}
		}
	}

	Building& b = construct ? create_constructionsite() : create_object();
	b.position_ = pos;
	b.set_owner(owner);
	if (immovable != INVALID_INDEX) {
		// Remember that we're building on top of an immovable so we can put it back if the building
		// gets removed
		b.old_buildings_.push_back(std::make_pair(immovable, false));
	}
	for (const auto& pair : former_buildings) {
		b.old_buildings_.push_back(pair);
	}
	if (loading) {
		b.Building::init(egbase);
		return b;
	}
	b.init(egbase);
	return b;
}

bool BuildingDescr::suitability(const Map&, const FCoords& fc) const {
	return (mine_ ? fc.field->nodecaps() & Widelands::BUILDCAPS_MINE :
                   size_ <= ((built_over_immovable_ == INVALID_INDEX ? fc.field->nodecaps() :
                                                                       fc.field->maxcaps()) &
	                          Widelands::BUILDCAPS_SIZEMASK)) &&
	       (built_over_immovable_ == INVALID_INDEX ||
	        (fc.field->get_immovable() &&
	         fc.field->get_immovable()->has_attribute(built_over_immovable_)));
}

const AI::BuildingHints& BuildingDescr::hints() const {
	return hints_;
}

void BuildingDescr::set_hints_trainingsites_max_percent(int percent) {
	hints_.set_trainingsites_max_percent(percent);
}

uint32_t BuildingDescr::get_unoccupied_animation() const {
	return get_animation(is_animation_known("unoccupied") ? "unoccupied" : "idle", nullptr);
}

bool BuildingDescr::is_useful_on_map(bool seafaring_allowed, bool waterways_allowed) const {
	if (needs_seafaring_ && needs_waterways_) {
		return seafaring_allowed || waterways_allowed;
	}
	if (needs_seafaring_) {
		return seafaring_allowed;
	}
	if (needs_waterways_) {
		return waterways_allowed;
	}
	return true;
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
	   descriptions_.get_building_descr(descriptions_.safe_building_index("constructionsite"));
	ConstructionSite& csite = dynamic_cast<ConstructionSite&>(descr->create_object());
	csite.set_building(*this);

	return csite;
}

void BuildingDescr::set_enhancement_cost(const Buildcost& enhance_cost,
                                         const Buildcost& return_enhanced) {
	enhanced_building_ = true;
	if (!return_enhanced.empty()) {
		can_be_dismantled_ = true;
	}
	enhancement_cost_ = enhance_cost;
	enhancement_returns_on_dismantle_ = return_enhanced;
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
     seeing_(false),
     was_immovable_(nullptr),
     attack_target_(nullptr),
     soldier_control_(nullptr),
     mute_messages_(false),
     is_destruction_blocked_(false) {
}

void Building::load_finish(EditorGameBase& egbase) {
	auto should_be_deleted = [&egbase, this](const OPtr<Worker>& optr) {
		Worker& worker = *optr.get(egbase);
		OPtr<PlayerImmovable> const worker_location = worker.get_location();
		if (worker_location.serial() != serial() &&
		    worker_location.serial() != base_flag().serial()) {
			log_warn("worker %u is in the leave queue of building %u with "
			         "base flag %u but is neither inside the building nor at the "
			         "flag!\n",
			         worker.serial(), serial(), base_flag().serial());
			return true;
		}

		Bob::State const* const state = worker.get_state(Worker::taskLeavebuilding);
		if (!state) {
			log_warn("worker %u is in the leave queue of building %u but "
			         "does not have a leavebuilding task! Removing from queue.\n",
			         worker.serial(), serial());
			return true;
		}

		if (state->objvar1 != this) {
			log_warn("worker %u is in the leave queue of building %u but its "
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
	if (tmp_descr.is_destructible() && !is_destruction_blocked()) {
		caps |= PCap_Bulldoze;
		if (tmp_descr.can_be_dismantled()) {
			caps |= PCap_Dismantle;
		}
	}
	if (tmp_descr.enhancement() != INVALID_INDEX) {
		caps |= PCap_Enhancable;
	}
	return caps;
}

void Building::start_animation(const EditorGameBase& egbase, uint32_t const anim) {
	anim_ = anim;
	animstart_ = egbase.get_gametime();
}

/*
===============
Common building initialization code. You must call this from
derived class' init.
===============
*/
bool Building::init(EditorGameBase& egbase) {
	MutexLock m(MutexLock::ID::kObjects);

	PlayerImmovable::init(egbase);

	// Set the building onto the map
	const Map& map = egbase.map();
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
		if (!flag) {
			flag = new Flag(egbase, get_owner(), neighb);
		}
		flag_ = flag;
		flag->attach_building(egbase, *this);
	}

	for (const auto& pair : old_buildings_) {
		// 'false' means we're building on top of an immovable
		if (!pair.second) {
			assert(!was_immovable_);
			was_immovable_ = egbase.descriptions().get_immovable_descr(pair.first);
			assert(was_immovable_);
			break;
		}
	}

	// Start the animation
	start_animation(egbase, descr().get_unoccupied_animation());

	if (descr().type() == MapObjectType::WAREHOUSE) {
		set_seeing(true);
	}

	leave_time_ = egbase.get_gametime();
	return true;
}

void Building::cleanup(EditorGameBase& egbase) {
	set_seeing(false);

	if (defeating_player_) {
		Player* defeating_player = egbase.get_player(defeating_player_);
		if (descr().get_conquers()) {
			get_owner()->count_msite_lost();
			defeating_player->count_msite_defeated();
		} else {
			get_owner()->count_civil_bld_lost();
			defeating_player->count_civil_bld_defeated();
		}
	}

	// Remove from flag
	flag_->detach_building(egbase);

	// Unset the building
	unset_position(egbase, position_);

	if (get_size() == BIG) {
		const Map& map = egbase.map();
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
		const Map& map = egbase.map();
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
	const bool fire = burn_on_destroy();
	const Coords pos = position_;
	Player* building_owner = get_owner();
	const BuildingDescr* building_descr = &descr();
	PlayerImmovable::destroy(egbase);
	// We are deleted. Only use stack variables beyond this point
	if (fire) {
		egbase.create_immovable_with_name(pos, "destroyed_building", building_owner, building_descr);
	}
}

std::string Building::info_string(const InfoStringFormat& format) {
	std::string result;
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
		break;
	}
	return result;
}

InputQueue& Building::inputqueue(DescriptionIndex const wi, WareWorker const, const Request*) {
	throw wexception("%s (%u) has no InputQueue for %u", descr().name().c_str(), serial(), wi);
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
	const Time& time = game.get_gametime();

	if (leave_queue_.empty()) {
		if (leave_time_ <= time) {
			leave_time_ = time + kBuildingLeaveInterval;
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

	if (it != leave_queue_.end()) {
		leave_queue_.erase(it);
	}
}

/*
===============
Advance the leave queue.
===============
*/
void Building::act(Game& game, uint32_t const data) {
	const Time& time = game.get_gametime();

	if (leave_time_ <= time) {
		bool wakeup = false;

		// Wake up one worker
		while (!leave_queue_.empty()) {
			upcast(Worker, worker, leave_queue_[0].get(game));

			leave_queue_.erase(leave_queue_.begin());

			if (worker) {
				leave_allow_ = worker;

				if (worker->wakeup_leave_building(game, *this)) {
					leave_time_ = time + kBuildingLeaveInterval;
					wakeup = true;
					break;
				}
			}
		}

		if (!leave_queue_.empty()) {
			schedule_act(game, leave_time_ - time);
		}
		if (!wakeup) {
			leave_time_ = time;  // make sure leave_time doesn't get too far behind
		}
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
bool Building::fetch_from_flag(Game& game) {
	molog(game.get_gametime(), "TODO(unknown): Implement Building::fetch_from_flag\n");

	return false;
}

void Building::draw(const Time& gametime,
                    const InfoToDraw info_to_draw,
                    const Vector2f& point_on_dst,
                    const Widelands::Coords& coords,
                    const float scale,
                    RenderTarget* dst) {
	const Time t((gametime - animstart_).get());

	if (was_immovable_) {
		if (info_to_draw & InfoToDraw::kShowBuildings) {
			dst->blit_animation(point_on_dst, coords, scale, was_immovable_->main_animation(), t,
			                    &get_owner()->get_playercolor());
		} else {
			dst->blit_animation(point_on_dst, coords, scale, was_immovable_->main_animation(), t,
			                    nullptr, kBuildingSilhouetteOpacity);
		}
	}

	if (info_to_draw & InfoToDraw::kShowBuildings) {
		dst->blit_animation(point_on_dst, coords, scale, anim_, t, &get_owner()->get_playercolor());
	} else {
		dst->blit_animation(
		   point_on_dst, coords, scale, anim_, t, nullptr, kBuildingSilhouetteOpacity);
	}

	//  door animation?

	//  overlay strings (draw when enabled)
	draw_info(info_to_draw, point_on_dst, scale, dst);
}

/*
===============
Draw overlay help strings when enabled.
===============
*/
void Building::draw_info(const InfoToDraw info_to_draw,
                         const Vector2f& point_on_dst,
                         const float scale,
                         RenderTarget* dst) {
	const std::string statistics_string =
	   (info_to_draw & InfoToDraw::kStatistics) ? info_string(InfoStringFormat::kStatistics) : "";
	do_draw_info(info_to_draw, info_string(InfoStringFormat::kCensus), statistics_string,
	             point_on_dst, scale, dst);
}

const WarePriority& Building::get_priority(const WareWorker type,
                                           const DescriptionIndex ware_index) const {
	if (type == wwWARE) {
		const auto it = ware_priorities_.find(ware_index);
		if (it != ware_priorities_.end()) {
			return it->second;
		}
	}

	return WarePriority::kNormal;
}

/**
 * Set base priority for this building (applies for all wares)
 */
void Building::set_priority(const WareWorker type,
                            const DescriptionIndex ware_index,
                            const WarePriority& new_priority) {
	if (type == wwWARE) {
		// WarePriority is not default-constructible, so no [] access :(
		if (ware_priorities_.count(ware_index)) {
			ware_priorities_.at(ware_index) = new_priority;
		} else {
			ware_priorities_.emplace(ware_index, new_priority);
		}
	}
}

void Building::log_general_info(const EditorGameBase& egbase) const {
	PlayerImmovable::log_general_info(egbase);

	molog(egbase.get_gametime(), "position: (%i, %i)\n", position_.x, position_.y);
	FORMAT_WARNINGS_OFF
	molog(egbase.get_gametime(), "flag: %p\n", flag_);
	FORMAT_WARNINGS_ON
	molog(egbase.get_gametime(), "* position: (%i, %i)\n", flag_->get_position().x,
	      flag_->get_position().y);

	molog(egbase.get_gametime(), "anim: %s\n", descr().get_animation_name(anim_).c_str());
	molog(egbase.get_gametime(), "animstart: %i\n", animstart_.get());

	molog(egbase.get_gametime(), "leave_time: %i\n", leave_time_.get());

	molog(egbase.get_gametime(), "leave_queue.size(): %" PRIuS "\n", leave_queue_.size());
	FORMAT_WARNINGS_OFF
	molog(egbase.get_gametime(), "leave_allow.get(): %p\n", leave_allow_.get(egbase));
	FORMAT_WARNINGS_ON
}

void Building::add_worker(Worker& worker) {
	MutexLock m(MutexLock::ID::kObjects);

	// Builders should make partially finished building see, but not finished buildings.
	// So we prevent builders from seeing here and override this in PartiallyFinishedBuilding.
	if (owner().tribe().safe_worker_index(worker.descr().name()) != owner().tribe().builder()) {
		set_seeing(true);
	}
	PlayerImmovable::add_worker(worker);
	Notifications::publish(NoteBuilding(serial(), NoteBuilding::Action::kWorkersChanged));
}

void Building::remove_worker(Worker& worker) {
	MutexLock m(MutexLock::ID::kObjects);

	PlayerImmovable::remove_worker(worker);
	if (get_workers().empty() && descr().type() != MapObjectType::WAREHOUSE) {
		set_seeing(false);
	}
	Notifications::publish(NoteBuilding(serial(), NoteBuilding::Action::kWorkersChanged));
}

void Building::notify_worker_evicted(Game& game, Worker&) {
	// If the building was working, we do not tell it to cancel – it'll notice by itself soon –
	// but we already change the animation so it won't look strange
	start_animation(game, descr().get_unoccupied_animation());
	worker_evicted_ = game.get_gametime();
}

void Building::set_attack_target(AttackTarget* new_attack_target) {
	assert(attack_target_ == nullptr);
	attack_target_ = new_attack_target;
}

void Building::set_soldier_control(SoldierControl* new_soldier_control) {
	assert(soldier_control_ == nullptr);
	soldier_control_ = new_soldier_control;
}

/**
 * Change whether this building sees its vision range based on workers
 * inside the building.
 *
 * \note Warehouses always see their surroundings; this is handled separately.
 */
void Building::set_seeing(bool see) {
	if (see == seeing_) {
		return;
	}

	Player* player = get_owner();
	const Map& map = player->egbase().map();

	if (see) {
		player->see_area(Area<FCoords>(map.get_fcoords(get_position()), descr().vision_range()));
	} else {
		player->unsee_area(Area<FCoords>(map.get_fcoords(get_position()), descr().vision_range()));
	}

	seeing_ = see;
}

/**
 * Send a message about this building to the owning player.
 *
 * It will have the building's coordinates, and display a picture of the
 * building in its description.
 *
 * \param msgtype a computer-readable description of why the message was sent
 * \param title short title to be displayed in message listings
 * \param icon_filename the filename to be used for the icon in message listings
 * \param heading long title to be displayed within the message
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
                            const Duration& throttle_time,
                            uint32_t throttle_radius) {
	if (mute_messages() ||
	    owner().is_muted(game.descriptions().safe_building_index(descr().name()))) {
		return;
	}

	const std::string rt_description =
	   as_mapobject_message(descr().name(), descr().representative_image()->width(), description,
	                        &owner().get_playercolor());

	std::unique_ptr<Message> msg(new Message(msgtype, game.get_gametime(), title, icon_filename,
	                                         heading, rt_description, get_position(),
	                                         (link_to_building_lifetime ? serial_ : 0)));

	if (throttle_time.get() > 0) {
		get_owner()->add_message_with_timeout(game, std::move(msg), throttle_time, throttle_radius);
	} else {
		get_owner()->add_message(game, std::move(msg));
	}
}
}  // namespace Widelands
