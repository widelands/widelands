/*
 * Copyright (C) 2002-2017 by the Widelands Development Team
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

#include "logic/map_objects/tribes/constructionsite.h"

#include <cstdio>

#include <boost/format.hpp>

#include "base/i18n.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/wares_queue.h"
#include "graphic/animation.h"
#include "graphic/graphic.h"
#include "graphic/rendertarget.h"
#include "graphic/text_constants.h"
#include "logic/editor_game_base.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/map_objects/tribes/worker.h"
#include "ui_basic/window.h"
#include "wui/interactive_gamebase.h"

namespace Widelands {

/**
  * The contents of 'table' are documented in
  * /data/tribes/buildings/partially_finished/constructionsite/init.lua
  */
ConstructionSiteDescr::ConstructionSiteDescr(const std::string& init_descname,
                                             const LuaTable& table,
                                             const EditorGameBase& egbase)
   : BuildingDescr(init_descname, MapObjectType::CONSTRUCTIONSITE, table, egbase) {
	add_attribute(MapObject::CONSTRUCTIONSITE);
}

Building& ConstructionSiteDescr::create_object() const {
	return *new ConstructionSite(*this);
}

/*
==============================

IMPLEMENTATION

==============================
*/

ConstructionSite::ConstructionSite(const ConstructionSiteDescr& cs_descr)
   : PartiallyFinishedBuilding(cs_descr), fetchfromflag_(0), builder_idle_(false) {
}

void ConstructionSite::update_statistics_string(std::string* s) {
	unsigned int percent = (get_built_per64k() * 100) >> 16;
	*s = (boost::format("<font color=%s>%s</font>") % UI_FONT_CLR_DARK.hex_value() %
	      (boost::format(_("%i%% built")) % percent))
	        .str();
}

/*
=======
Access to the wares queues by id
=======
*/
InputQueue& ConstructionSite::inputqueue(DescriptionIndex const wi, WareWorker const type) {
	// There are no worker queues here
	// Hopefully, our construction sites are safe enough not to kill workers
	if (type != wwWARE) {
		throw wexception("%s (%u) (building %s) has no WorkersQueues", descr().name().c_str(),
		                 serial(), building_->name().c_str());
	}
	for (WaresQueue* ware : wares_) {
		if (ware->get_index() == wi) {
			return *ware;
		}
	}
	throw wexception("%s (%u) (building %s) has no WaresQueue for %u", descr().name().c_str(),
	                 serial(), building_->name().c_str(), wi);
}

/*
===============
Set the type of building we're going to build
===============
*/
void ConstructionSite::set_building(const BuildingDescr& building_descr) {
	PartiallyFinishedBuilding::set_building(building_descr);

	info_.becomes = &building_descr;
}

/*
===============
Initialize the construction site by starting orders
===============
*/
void ConstructionSite::init(EditorGameBase& egbase) {
	PartiallyFinishedBuilding::init(egbase);

	const std::map<DescriptionIndex, uint8_t>* buildcost;
	if (!old_buildings_.empty()) {
		// Enhancement
		DescriptionIndex was_index = old_buildings_.back();
		const BuildingDescr* was_descr = owner().tribe().get_building_descr(was_index);
		info_.was = was_descr;
		buildcost = &building_->enhancement_cost();
	} else {
		buildcost = &building_->buildcost();
	}

	//  TODO(unknown): figure out whether planing is necessary

	//  initialize the wares queues
	size_t const buildcost_size = buildcost->size();
	wares_.resize(buildcost_size);
	std::map<DescriptionIndex, uint8_t>::const_iterator it = buildcost->begin();

	for (size_t i = 0; i < buildcost_size; ++i, ++it) {
		WaresQueue& wq = *(wares_[i] = new WaresQueue(*this, it->first, it->second));

		wq.set_callback(ConstructionSite::wares_queue_callback, this);
		wq.set_consume_interval(CONSTRUCTIONSITE_STEP_TIME);

		work_steps_ += it->second;
	}
}

/*
===============
Release worker and material (if any is left).
If construction was finished successfully, place the building at our position.
===============
*/
void ConstructionSite::cleanup(EditorGameBase& egbase) {
	PartiallyFinishedBuilding::cleanup(egbase);

	if (work_steps_ <= work_completed_) {
		// Put the real building in place
		DescriptionIndex becomes_idx = owner().tribe().building_index(building_->name());
		old_buildings_.push_back(becomes_idx);
		Building& b = building_->create(egbase, owner(), position_, false, false, old_buildings_);
		if (Worker* const builder = builder_.get(egbase)) {
			builder->reset_tasks(dynamic_cast<Game&>(egbase));
			builder->set_location(&b);
		}
		// Open the new building window if needed
		if (optionswindow_) {
			Vector2i window_position = optionswindow_->get_pos();
			hide_options();
			InteractiveGameBase& igbase = dynamic_cast<InteractiveGameBase&>(*egbase.get_ibase());
			b.show_options(igbase, false, window_position);
		}
	}
}

/*
===============
Construction sites only burn if some of the work has been completed.
===============
*/
bool ConstructionSite::burn_on_destroy() {
	if (work_completed_ >= work_steps_)
		return false;  // completed, so don't burn

	return work_completed_ || !old_buildings_.empty();
}

/*
===============
Remember the ware on the flag. The worker will be sent from get_building_work().
===============
*/
bool ConstructionSite::fetch_from_flag(Game& game) {
	++fetchfromflag_;

	if (Worker* const builder = builder_.get(game))
		builder->update_task_buildingwork(game);

	return true;
}

/*
===============
Called by our builder to get instructions.
===============
*/
bool ConstructionSite::get_building_work(Game& game, Worker& worker, bool) {
	if (&worker != builder_.get(game)) {
		// Not our construction worker; e.g. a miner leaving a mine
		// that is supposed to be enhanced. Make him return to a warehouse
		worker.pop_task(game);
		worker.start_task_leavebuilding(game, true);
		return true;
	}

	if (!work_steps_)           //  Happens for building without buildcost.
		schedule_destroy(game);  //  Complete the building immediately.

	// Check if one step has completed
	if (working_) {
		if (static_cast<int32_t>(game.get_gametime() - work_steptime_) < 0) {
			worker.start_task_idle(
			   game, worker.descr().get_animation("work"), work_steptime_ - game.get_gametime());
			builder_idle_ = false;
			return true;
		} else {
			// TODO(fweber): cause "construction sounds" to be played -
			// perhaps dependent on kind of construction?

			++work_completed_;
			if (work_completed_ >= work_steps_)
				schedule_destroy(game);

			working_ = false;
		}
	}

	// Fetch wares from flag
	if (fetchfromflag_) {
		--fetchfromflag_;
		builder_idle_ = false;
		worker.start_task_fetchfromflag(game);
		return true;
	}

	// Drop all the wares that are too much out to the flag.
	for (WaresQueue* iqueue : wares_) {
		WaresQueue* queue = iqueue;
		if (queue->get_filled() > queue->get_max_fill()) {
			queue->set_filled(queue->get_filled() - 1);
			const WareDescr& wd = *owner().tribe().get_ware_descr(queue->get_index());
			WareInstance& ware = *new WareInstance(queue->get_index(), &wd);
			ware.init(game);
			worker.start_task_dropoff(game, ware);
			return true;
		}
	}

	// Check if we've got wares to consume
	if (work_completed_ < work_steps_) {
		for (uint32_t i = 0; i < wares_.size(); ++i) {
			WaresQueue& wq = *wares_[i];

			if (!wq.get_filled())
				continue;

			wq.set_filled(wq.get_filled() - 1);
			wq.set_max_size(wq.get_max_size() - 1);

			// Update consumption statistic
			owner().ware_consumed(wq.get_index(), 1);

			working_ = true;
			work_steptime_ = game.get_gametime() + CONSTRUCTIONSITE_STEP_TIME;

			worker.start_task_idle(
			   game, worker.descr().get_animation("work"), CONSTRUCTIONSITE_STEP_TIME);
			builder_idle_ = false;
			return true;
		}
	}
	// The only work we have got for you, is to run around to look cute ;)
	if (!builder_idle_) {
		worker.set_animation(game, worker.descr().get_animation("idle"));
		builder_idle_ = true;
	}
	worker.schedule_act(game, 2000);
	return true;
}

/*
===============
Called by InputQueue code when an ware has arrived
===============
*/
void ConstructionSite::wares_queue_callback(
   Game& game, InputQueue*, DescriptionIndex, Worker*, void* const data) {
	ConstructionSite& cs = *static_cast<ConstructionSite*>(data);

	if (!cs.working_)
		if (Worker* const builder = cs.builder_.get(game))
			builder->update_task_buildingwork(game);
}

/*
===============
Draw the construction site.
===============
*/
void ConstructionSite::draw(uint32_t gametime,
                            TextToDraw draw_text,
                            const Vector2f& point_on_dst,
                            float scale,
                            RenderTarget* dst) {
	uint32_t tanim = gametime - animstart_;
	// Draw the construction site marker
	const RGBColor& player_color = get_owner()->get_playercolor();
	dst->blit_animation(point_on_dst, scale, anim_, tanim, player_color);

	// Draw the partially finished building

	static_assert(
	   0 <= CONSTRUCTIONSITE_STEP_TIME, "assert(0 <= CONSTRUCTIONSITE_STEP_TIME) failed.");
	info_.totaltime = CONSTRUCTIONSITE_STEP_TIME * work_steps_;
	info_.completedtime = CONSTRUCTIONSITE_STEP_TIME * work_completed_;

	if (working_) {
		assert(work_steptime_ <= info_.completedtime + CONSTRUCTIONSITE_STEP_TIME + gametime);
		info_.completedtime += CONSTRUCTIONSITE_STEP_TIME + gametime - work_steptime_;
	}

	uint32_t anim_idx;
	try {
		anim_idx = building().get_animation("build");
	} catch (MapObjectDescr::AnimationNonexistent&) {
		try {
			anim_idx = building().get_animation("unoccupied");
		} catch (MapObjectDescr::AnimationNonexistent) {
			anim_idx = building().get_animation("idle");
		}
	}
	const Animation& anim = g_gr->animations().get_animation(anim_idx);
	const size_t nr_frames = anim.nr_frames();
	const uint32_t cur_frame =
	   info_.totaltime ? info_.completedtime * nr_frames / info_.totaltime : 0;
	tanim = cur_frame * FRAME_LENGTH;

	if (cur_frame) {  //  not the first pic
		// Draw the complete prev pic , so we won't run into trouble if images have different sizes
		dst->blit_animation(point_on_dst, scale, anim_idx, tanim - FRAME_LENGTH, player_color);
	} else if (!old_buildings_.empty()) {
		DescriptionIndex prev_idx = old_buildings_.back();
		const BuildingDescr* prev_building = owner().tribe().get_building_descr(prev_idx);
		//  Is the first picture but there was another building here before,
		//  get its most fitting picture and draw it instead.
		const uint32_t prev_building_anim_idx = prev_building->get_animation(
		   prev_building->is_animation_known("unoccupied") ? "unoccupied" : "idle");
		dst->blit_animation(
		   point_on_dst, scale, prev_building_anim_idx, tanim - FRAME_LENGTH, player_color);
	}
	// Now blit a segment of the current construction phase from the bottom.
	int percent = 100 * info_.completedtime * nr_frames;
	if (info_.totaltime) {
		percent /= info_.totaltime;
	}
	percent -= 100 * cur_frame;
	dst->blit_animation(point_on_dst, scale, anim_idx, tanim, player_color, percent);

	// Draw help strings
	draw_info(draw_text, point_on_dst, scale, dst);
}
}
