/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "logic/map_objects/tribes/dismantlesite.h"

#include "base/i18n.h"
#include "base/wexception.h"
#include "economy/wares_queue.h"
#include "graphic/rendertarget.h"
#include "graphic/style_manager.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "sound/note_sound.h"
#include "sound/sound_handler.h"

namespace Widelands {

constexpr Duration DismantleSite::kDismantlesiteStepTime;

/**
 * The contents of 'table' are documented in
 * /data/tribes/buildings/partially_finished/dismantlesite/init.lua
 */

DismantleSiteDescr::DismantleSiteDescr(const std::string& init_descname,
                                       const LuaTable& table,
                                       Descriptions& descriptions)
   : BuildingDescr(init_descname, MapObjectType::DISMANTLESITE, table, descriptions),
     creation_fx_(
        SoundHandler::register_fx(SoundType::kAmbient, "sound/create_construction_site")) {
}

Building& DismantleSiteDescr::create_object() const {
	return *new DismantleSite(*this);
}

FxId DismantleSiteDescr::creation_fx() const {
	return creation_fx_;
}

/*
==============================

IMPLEMENTATION

==============================
*/

DismantleSite::DismantleSite(const DismantleSiteDescr& gdescr)
   : PartiallyFinishedBuilding(gdescr), next_dropout_index_(0) {
}

DismantleSite::DismantleSite(const DismantleSiteDescr& gdescr,
                             EditorGameBase& egbase,
                             const Coords& c,
                             Player* plr,
                             bool loading,
                             const FormerBuildings& former_buildings,
                             const std::map<DescriptionIndex, Quantity>& preserved_wares)
   : PartiallyFinishedBuilding(gdescr), preserved_wares_(preserved_wares), next_dropout_index_(0) {
	position_ = c;
	set_owner(plr);

	assert(!former_buildings.empty());
	for (const auto& pair : former_buildings) {
		old_buildings_.push_back(pair);
	}
	const BuildingDescr* cur_descr = owner().tribe().get_building_descr(old_buildings_.back().first);
	set_building(*cur_descr);

	if (loading) {
		PartiallyFinishedBuilding::init(egbase);
	} else {
		init(egbase);
	}
}

void DismantleSite::cleanup(EditorGameBase& egbase) {
	PartiallyFinishedBuilding::cleanup(egbase);

	if (was_immovable_ && work_completed_ >= work_steps_) {
		// Put the old immovable in place again
		for (const auto& pair : old_buildings_) {
			// 'false' means that this was built on top of an immovable, so we reinstate that immovable
			if (!pair.second) {
				egbase.create_immovable(position_, pair.first, get_owner());
				break;
			}
		}
	}
}

/*
===============
Print completion percentage.
===============
*/
void DismantleSite::update_statistics_string(std::string* s) {
	unsigned int percent = (get_built_per64k() * 100) >> 16;
	*s = StyleManager::color_tag(format(_("%u%% dismantled"), percent),
	                             g_style_manager->building_statistics_style().construction_color());
}

/*
===============
Initialize the construction site by starting orders
===============
*/
bool DismantleSite::init(EditorGameBase& egbase) {
	Notifications::publish(
	   NoteSound(SoundType::kAmbient, descr().creation_fx(), position_, kFxMaximumPriority, true));

	PartiallyFinishedBuilding::init(egbase);

	for (const auto& pair : preserved_wares_) {
		WaresQueue* q = new WaresQueue(*this, pair.first, pair.second);
		q->set_filled(pair.second);
		dropout_wares_.push_back(q);
	}
	for (const auto& ware : count_returned_wares(this)) {
		WaresQueue* wq = new WaresQueue(*this, ware.first, ware.second);
		wq->set_filled(ware.second);
		consume_wares_.push_back(wq);
		work_steps_ += ware.second;
	}
	return true;
}

/*
===============
Count which wares you get back if you dismantle the given building
===============
*/
const Buildcost DismantleSite::count_returned_wares(Building* building) {
	Buildcost result;
	DescriptionIndex first_idx = INVALID_INDEX;
	for (const auto& pair : building->get_former_buildings()) {
		// 'true' means that this is an enhanced building
		if (pair.second) {
			first_idx = pair.first;
			break;
		}
	}
	assert(first_idx != INVALID_INDEX);
	for (const auto& pair : building->get_former_buildings()) {
		if (!pair.second) {
			continue;
		}
		const BuildingDescr* former_descr = building->owner().tribe().get_building_descr(pair.first);
		const Buildcost& return_wares = pair.first != first_idx ?
                                         former_descr->enhancement_returns_on_dismantle() :
                                         former_descr->returns_on_dismantle();

		for (const auto& ware : return_wares) {
			// TODO(GunChleoc): Once we have trading, we might want to return all wares again.
			if (building->owner().tribe().has_ware(ware.first)) {
				result[ware.first] += ware.second;
			}
		}
	}
	return result;
}

/*
===============
Construction sites only burn if some of the work has been completed.
===============
*/
bool DismantleSite::burn_on_destroy() {
	return work_completed_ < work_steps_;
}

/*
===============
Called by our builder to get instructions.
===============
*/
bool DismantleSite::get_building_work(Game& game, Worker& worker, bool /*success*/) {
	if (&worker != builder_.get(game)) {
		// Not our construction worker; e.g. a miner leaving a mine
		// that is supposed to be enhanced. Make him return to a warehouse
		worker.pop_task(game);
		worker.start_task_leavebuilding(game, true);
		return true;
	}

	// Drop out preserved wares round-robin
	if (const size_t nr_dropout_queues = dropout_wares_.size()) {
		bool first_round = true;
		for (size_t i = next_dropout_index_; i != next_dropout_index_ || first_round;
		     i = (i + 1) % nr_dropout_queues, first_round = false) {
			WaresQueue& q = *dropout_wares_[i];
			if (q.get_filled()) {
				q.set_filled(q.get_filled() - 1);
				q.set_max_size(q.get_max_size() - 1);
				const WareDescr& wd = *owner().tribe().get_ware_descr(q.get_index());
				WareInstance& ware = *new WareInstance(q.get_index(), &wd);
				ware.init(game);
				worker.start_task_dropoff(game, ware);
				next_dropout_index_ = (i + 1) % nr_dropout_queues;
				return true;
			}
		}
	}

	if (!work_steps_) {
		// Happens for building without buildcost. Complete the building immediately.
		schedule_destroy(game);
	}

	// Check if one step has completed
	if (game.get_gametime() >= work_steptime_ && working_) {
		++work_completed_;

		for (WaresQueue* wq : consume_wares_) {
			if (!wq->get_filled()) {
				continue;
			}

			wq->set_filled(wq->get_filled() - 1);
			wq->set_max_size(wq->get_max_size() - 1);

			// Update statistics
			get_owner()->ware_produced(wq->get_index());

			const WareDescr& wd = *owner().tribe().get_ware_descr(wq->get_index());
			WareInstance& ware = *new WareInstance(wq->get_index(), &wd);
			ware.init(game);
			worker.start_task_dropoff(game, ware);

			working_ = false;
			return true;
		}
	}

	if (work_completed_ >= work_steps_) {
		schedule_destroy(game);

		worker.pop_task(game);
		// No more building, so move to the flag
		worker.start_task_move(
		   game, WALK_SE, worker.descr().get_right_walk_anims(false, &worker), true);
		worker.set_location(nullptr);
	} else if (!working_) {
		work_steptime_ = game.get_gametime() + kDismantlesiteStepTime;
		worker.start_task_idle(
		   game, worker.descr().get_animation("work", &worker), kDismantlesiteStepTime.get());

		working_ = true;
	}
	return true;
}

/*
===============
Draw it.
===============
*/
void DismantleSite::draw(const Time& gametime,
                         const InfoToDraw info_to_draw,
                         const Vector2f& point_on_dst,
                         const Widelands::Coords& coords,
                         float scale,
                         RenderTarget* dst) {
	const Time tanim((gametime - animstart_).get());
	const RGBColor& player_color = get_owner()->get_playercolor();

	if (was_immovable_) {
		if (info_to_draw & InfoToDraw::kShowBuildings) {
			dst->blit_animation(
			   point_on_dst, coords, scale, was_immovable_->main_animation(), tanim, &player_color);
		} else {
			dst->blit_animation(point_on_dst, coords, scale, was_immovable_->main_animation(), tanim,
			                    nullptr, kBuildingSilhouetteOpacity);
		}
	} else {
		// Draw the construction site marker
		if (info_to_draw & InfoToDraw::kShowBuildings) {
			dst->blit_animation(
			   point_on_dst, Widelands::Coords::null(), scale, anim_, tanim, &player_color);
		} else {
			dst->blit_animation(point_on_dst, Widelands::Coords::null(), scale, anim_, tanim, nullptr,
			                    kBuildingSilhouetteOpacity);
		}
	}

	// Blit bottom part of the animation according to dismantle progress
	if (info_to_draw & InfoToDraw::kShowBuildings) {
		dst->blit_animation(point_on_dst, coords, scale, building_->get_unoccupied_animation(), tanim,
		                    &player_color, 1.f, 100 - ((get_built_per64k() * 100) >> 16));
	} else {
		dst->blit_animation(point_on_dst, coords, scale, building_->get_unoccupied_animation(), tanim,
		                    nullptr, kBuildingSilhouetteOpacity,
		                    100 - ((get_built_per64k() * 100) >> 16));
	}

	// Draw help strings
	draw_info(info_to_draw, point_on_dst, scale, dst);
}
}  // namespace Widelands
