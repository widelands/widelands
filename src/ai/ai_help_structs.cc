/*
 * Copyright (C) 2009-2022 by the Widelands Development Team
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

#include "ai/ai_help_structs.h"

#include <algorithm>
#include <cstdlib>

#include "base/log.h"
#include "base/macros.h"
#include "base/time_string.h"
#include "logic/ai_dna_handler.h"
#include "logic/map.h"
#include "logic/player.h"

namespace AI {

constexpr int kNormalMutation = 500;  // Almost no mutation
constexpr int kTrainingMutation = 200;

constexpr int kMilitaryNumbersPos = 0;
constexpr int kNeuronsPos = 1;
constexpr int kFNeuronsPos = 2;

// CheckStepRoadAI
CheckStepRoadAI::CheckStepRoadAI(Widelands::Player* const pl, uint8_t const mc, bool const oe)
   : player(pl), movecaps(mc), open_end(oe) {
}

bool CheckStepRoadAI::allowed(const Widelands::Map& map,
                              Widelands::FCoords start,
                              Widelands::FCoords end,
                              int32_t /* direction */,
                              Widelands::CheckStep::StepId const id) const {
	const uint8_t endcaps = player->get_buildcaps(end);

	// we should not cross fields with road or flags (or any other immovable)
	if (((map.get_immovable(start)) != nullptr) && !(id == Widelands::CheckStep::stepFirst)) {
		return false;
	}

	// Calculate cost and passability
	if ((endcaps & movecaps) == 0) {
		return false;
	}

	// Check for blocking immovables
	if (Widelands::BaseImmovable const* const imm = map.get_immovable(end)) {
		if (imm->get_size() >= Widelands::BaseImmovable::SMALL) {
			if (id != Widelands::CheckStep::stepLast && !open_end) {
				return false;
			}
			if (imm->descr().type() == Widelands::MapObjectType::FLAG) {
				return true;
			}
			if (imm->descr().type() != Widelands::MapObjectType::ROAD ||
			    ((endcaps & Widelands::BUILDCAPS_FLAG) == 0)) {
				return false;
			}
		}
	}
	return true;
}

bool CheckStepRoadAI::reachable_dest(const Widelands::Map& map,
                                     const Widelands::FCoords& dest) const {
	Widelands::NodeCaps const caps = dest.field->nodecaps();

	if ((caps & movecaps) == 0) {
		if (!(((movecaps & Widelands::MOVECAPS_SWIM) != 0) &&
		      ((caps & Widelands::MOVECAPS_WALK) != 0))) {
			return false;
		}
		if (!map.can_reach_by_water(dest)) {
			return false;
		}
	}

	return true;
}

// CheckStepOwnTerritory
CheckStepOwnTerritory::CheckStepOwnTerritory(Widelands::Player* const pl,
                                             uint8_t const mc,
                                             bool const oe)
   : player(pl), movecaps(mc), open_end(oe) {
}

// Defines when movement is allowed:
// 1. startfield is walkable (or it is the first step)
// And endfield either:
// 2a. is walkable
// 2b. has our PlayerImmovable (building or flag)
bool CheckStepOwnTerritory::allowed(const Widelands::Map& map,
                                    Widelands::FCoords start,
                                    Widelands::FCoords end,
                                    int32_t /* direction */,
                                    Widelands::CheckStep::StepId const id) const {
	const uint8_t endcaps = player->get_buildcaps(end);
	const uint8_t startcaps = player->get_buildcaps(start);

	// We should not cross fields with road or flags (or any other immovable)
	// Or rather we can step on it, but not go on from such field
	if (((map.get_immovable(start)) != nullptr) && !(id == Widelands::CheckStep::stepFirst)) {
		return false;
	}

	// Start field must be walkable
	if ((startcaps & movecaps) == 0) {
		return false;
	}

	// Endfield can not be water
	if ((endcaps & Widelands::MOVECAPS_SWIM) != 0) {
		return false;
	}

	return true;
}

// We accept either walkable territory or field with own immovable
bool CheckStepOwnTerritory::reachable_dest(const Widelands::Map& map,
                                           const Widelands::FCoords& dest) const {
	const uint8_t endcaps = player->get_buildcaps(dest);
	if (Widelands::BaseImmovable const* const imm = map.get_immovable(dest)) {
		return imm->descr().type() >= Widelands::MapObjectType::FLAG;
	}
	return (endcaps & Widelands::MOVECAPS_WALK) != 0;
}

// We are looking for fields we can walk on
// and owned by hostile player.
FindNodeEnemy::FindNodeEnemy(Widelands::Player* p, Widelands::Game& g) : player(p), game(g) {
}

bool FindNodeEnemy::accept(const Widelands::EditorGameBase& /* egbase */,
                           const Widelands::FCoords& fc) const {
	return ((fc.field->nodecaps() & Widelands::MOVECAPS_WALK) != 0) &&
	       fc.field->get_owned_by() != 0 &&
	       player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// We are looking for buildings owned by hostile player
// (sometimes there is a enemy's teritorry without buildings, and
// this confuses the AI)
FindNodeEnemiesBuilding::FindNodeEnemiesBuilding(Widelands::Player* p, Widelands::Game& g)
   : player(p), game(g) {
}

bool FindNodeEnemiesBuilding::accept(const Widelands::EditorGameBase& /* egbase */,
                                     const Widelands::FCoords& fc) const {
	return ((fc.field->get_immovable()) != nullptr) && fc.field->get_owned_by() != 0 &&
	       player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// When looking for unowned terrain to acquire, we are actually
// only interested in fields we can walk on.
// Fields should either be completely unowned or owned by an opposing player
FindEnemyNodeWalkable::FindEnemyNodeWalkable(Widelands::Player* p, Widelands::Game& g)
   : player(p), game(g) {
}

bool FindEnemyNodeWalkable::accept(const Widelands::EditorGameBase& /* egbase */,
                                   const Widelands::FCoords& fc) const {
	return (((fc.field->nodecaps() & Widelands::MOVECAPS_WALK) != 0) &&
	        (fc.field->get_owned_by() > 0) &&
	        player->is_hostile(*game.get_player(fc.field->get_owned_by())));
}

// Sometimes we need to know how many nodes our allies owns
FindNodeAllyOwned::FindNodeAllyOwned(Widelands::Player* p,
                                     Widelands::Game& g,
                                     Widelands::PlayerNumber n)
   : player(p), game(g), player_number(n) {
}

bool FindNodeAllyOwned::accept(const Widelands::EditorGameBase& /* egbase */,
                               const Widelands::FCoords& fc) const {
	return ((fc.field->nodecaps() & Widelands::MOVECAPS_WALK) != 0) &&
	       (fc.field->get_owned_by() != 0) && (fc.field->get_owned_by() != player_number) &&
	       !player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// When looking for unowned terrain to acquire, we must
// pay speciall attention to fields where mines can be built.
// Fields should be completely unowned
FindNodeUnownedMineable::FindNodeUnownedMineable(Widelands::Player* p,
                                                 Widelands::Game& g,
                                                 int32_t t)
   : player(p), game(g), ore_type(t) {
}

bool FindNodeUnownedMineable::accept(const Widelands::EditorGameBase& /* egbase */,
                                     const Widelands::FCoords& fc) const {
	if (ore_type == Widelands::INVALID_INDEX) {
		return ((fc.field->nodecaps() & Widelands::BUILDCAPS_MINE) != 0) &&
		       (fc.field->get_owned_by() == Widelands::neutral());
	}
	return ((fc.field->nodecaps() & Widelands::BUILDCAPS_MINE) != 0) &&
	       (fc.field->get_owned_by() == Widelands::neutral()) &&
	       fc.field->get_resources() == ore_type;
}

FindNodeUnownedBuildable::FindNodeUnownedBuildable(Widelands::Player* p, Widelands::Game& g)
   : player(p), game(g) {
}

bool FindNodeUnownedBuildable::accept(const Widelands::EditorGameBase& /* egbase */,
                                      const Widelands::FCoords& fc) const {
	return (((fc.field->nodecaps() & Widelands::BUILDCAPS_SIZEMASK) != 0) ||
	        ((fc.field->nodecaps() & Widelands::BUILDCAPS_MINE) != 0)) &&
	       (fc.field->get_owned_by() == Widelands::neutral());
}

// Unowned but walkable fields nearby
FindNodeUnownedWalkable::FindNodeUnownedWalkable(Widelands::Player* p, Widelands::Game& g)
   : player(p), game(g) {
}

bool FindNodeUnownedWalkable::accept(const Widelands::EditorGameBase& /* egbase */,
                                     const Widelands::FCoords& fc) const {
	return ((fc.field->nodecaps() & Widelands::MOVECAPS_WALK) != 0) &&
	       (fc.field->get_owned_by() == Widelands::neutral());
}

// Looking only for mines-capable fields nearby
// of specific type
FindNodeMineable::FindNodeMineable(Widelands::Game& g, Widelands::DescriptionIndex r)
   : game(g), res(r) {
}

bool FindNodeMineable::accept(const Widelands::EditorGameBase& /* egbase */,
                              const Widelands::FCoords& fc) const {

	return ((fc.field->nodecaps() & Widelands::BUILDCAPS_MINE) != 0) &&
	       (fc.field->get_resources() == res);
}

// Fishers and fishbreeders must be built near water
FindNodeWater::FindNodeWater(const Widelands::Descriptions& descriptions)
   : descriptions_(descriptions) {
}

bool FindNodeWater::accept(const Widelands::EditorGameBase& egbase,
                           const Widelands::FCoords& coord) const {
	return ((descriptions_.get_terrain_descr(coord.field->terrain_d())->get_is() &
	         Widelands::TerrainDescription::Is::kWater) != 0) ||
	       ((descriptions_
	            .get_terrain_descr(
	               egbase.map().get_neighbour(coord, Widelands::WALK_W).field->terrain_r())
	            ->get_is() &
	         Widelands::TerrainDescription::Is::kWater) != 0) ||
	       ((descriptions_
	            .get_terrain_descr(
	               egbase.map().get_neighbour(coord, Widelands::WALK_NW).field->terrain_r())
	            ->get_is() &
	         Widelands::TerrainDescription::Is::kWater) != 0);
}

bool FindNodeOpenWater::accept(const Widelands::EditorGameBase& /* egbase */,
                               const Widelands::FCoords& coord) const {
	return ((coord.field->nodecaps() & Widelands::MOVECAPS_WALK) == 0) &&
	       ((coord.field->nodecaps() & Widelands::MOVECAPS_SWIM) != 0);
}

// FindNodeWithFlagOrRoad
bool FindNodeWithFlagOrRoad::accept(const Widelands::EditorGameBase& /* egbase */,
                                    Widelands::FCoords fc) const {
	if (upcast(Widelands::PlayerImmovable const, pimm, fc.field->get_immovable())) {
		return ((dynamic_cast<Widelands::Flag const*>(pimm) != nullptr) ||
		        ((dynamic_cast<Widelands::Road const*>(pimm) != nullptr) &&
		         ((fc.field->nodecaps() & Widelands::BUILDCAPS_FLAG) != 0)));
	}
	return false;
}

NearFlag::NearFlag(const Widelands::Flag* f, int32_t const c) : flag(f), current_road_distance(c) {
	to_be_checked = true;
}

NearFlag::NearFlag() {
	flag = nullptr;
	current_road_distance = 0;
	to_be_checked = true;
}

void EventTimeQueue::push(const Time& production_time, const uint32_t additional_id) {
	queue.push_front(std::make_pair(production_time, additional_id));
}

// Return count of entries in log (deque), if id is provided, it counts corresponding
// members. id here can be index of building, f.e. it count how many soldiers were
// trained in particular type of training site
uint32_t EventTimeQueue::count(const Time& current_time, const uint32_t additional_id) {
	strip_old(current_time);
	if (additional_id == std::numeric_limits<uint32_t>::max()) {
		return queue.size();
	}
	uint32_t cnt = 0;
	for (const auto& item : queue) {
		if (item.second == additional_id) {
			++cnt;
		}
	}
	return cnt;
}

void EventTimeQueue::strip_old(const Time& current_time) {
	while (!queue.empty() && queue.back().first + duration_ < current_time) {
		queue.pop_back();
	}
}

BuildableField::BuildableField(const Widelands::FCoords& fc)
   : coords(fc),
     field_info_expiration(20000),
     preferred(false),
     enemy_nearby(false),
     enemy_accessible_(false),
     invalidated(false),
     enemy_wh_nearby(false),
     unowned_land_nearby(0),
     enemy_owned_land_nearby(0U),
     unowned_buildable_spots_nearby(0U),
     unowned_portspace_vicinity_nearby(0U),
     nearest_buildable_spot_nearby(0U),
     near_border(false),
     unowned_mines_spots_nearby(0),
     unowned_iron_mines_nearby(0u),
     // explanation of starting values
     // this is done to save some work for AI (CPU utilization)
     // base rules are:
     // count of water fields are stable, so if the current count is
     // non-negative, water is not recalculated
     water_nearby(-1),
     open_water_nearby(-1),
     distant_water(0),
     fish_nearby(-1),
     critters_nearby(-1),
     ground_water(1),
     space_consumers_nearby(0),
     rangers_nearby(0),
     area_military_capacity(0),
     future_area_military_capacity(0),
     military_loneliness(1000),
     future_military_loneliness(1000),
     military_in_constr_nearby(0),
     own_military_presence(0),
     enemy_military_presence(0),
     enemy_military_sites(0),
     ally_military_presence(0),
     military_stationed(0),
     average_flag_dist_to_wh(kWhFarButReachable),
     military_unstationed(0),
     own_non_military_nearby(0),
     defense_msite_allowed(false),
     is_portspace(ExtendedBool::kUnset),
     port_nearby(false),
     portspace_nearby(ExtendedBool::kUnset),
     max_buildcap_nearby(0),
     last_resources_check_time(0),
     // the higher the most important the side/field is
     military_score_(0),
     inland(false),
     local_soldier_capacity(0),
     is_militarysite(false) {
}

MineableField::MineableField(const Widelands::FCoords& fc)
   : coords(fc),
     field_info_expiration(20000),
     preferred(false),
     mines_nearby(0),
     same_mine_fields_nearby(0) {
}

EconomyObserver::EconomyObserver(Widelands::Economy& e)
   : economy(e), fields_block_last_time(Time(0)) {
}

int32_t BuildingObserver::total_count() const {
	return cnt_built + cnt_under_construction;
}

bool BuildingObserver::is(BuildingAttribute attribute) const {
	return is_what.count(attribute) == 1;
}

void BuildingObserver::set_is(const BuildingAttribute attribute) {
	is_what.insert(attribute);
}

void BuildingObserver::unset_is(const BuildingAttribute attribute) {
	is_what.erase(attribute);
	assert(!is(attribute));
}

AiModeBuildings BuildingObserver::aimode_limit_status() const {
	if (total_count() > cnt_limit_by_aimode) {
		return AiModeBuildings::kLimitExceeded;
	}
	if (total_count() == cnt_limit_by_aimode) {
		return AiModeBuildings::kOnLimit;
	}
	return AiModeBuildings::kAnotherAllowed;
}
bool BuildingObserver::buildable(const Widelands::Player& p) const {
	return is(BuildingAttribute::kBuildable) && p.is_building_type_allowed(id) &&
	       p.tribe().has_building(id);
}

// as all mines have 3 levels, AI does not know total count of mines per mined material
// so this observer will be used for this
MineTypesObserver::MineTypesObserver()
   : in_construction(0), finished(0), is_critical(false), unoccupied(0) {
}

// Reset counter for all field types
void MineFieldsObserver::zero() {
	for (auto& material : stat) {
		material.second = 0;
	}
}

// Increase counter by one for specific ore/minefield type
void MineFieldsObserver::add(const Widelands::DescriptionIndex idx) {
	++stat[idx];
}

// Add ore into critical_ores
void MineFieldsObserver::add_critical_ore(const Widelands::DescriptionIndex idx) {
	critical_ores.insert(idx);
}

// Does the player has at least one mineable field with positive amount for each critical ore?
bool MineFieldsObserver::has_critical_ore_fields() {
	for (auto ore : critical_ores) {
		if (get(ore) == 0) {
			return false;
		}
	}
	return true;
}

// Returns count of fields with desired ore
uint16_t MineFieldsObserver::get(const Widelands::DescriptionIndex idx) {
	if (stat.count(idx) == 0) {
		return 0;
	}
	return stat[idx];
}

// Count of types of mineable fields, up to 4 currently
uint8_t MineFieldsObserver::count_types() {
	uint16_t count = 0;
	for (auto material : stat) {
		if (material.second > 0) {
			++count;
		}
	}
	return count;
}

ExpansionType::ExpansionType() {
	type = ExpansionMode::kResources;
}

void ExpansionType::set_expantion_type(const ExpansionMode etype) {
	type = etype;
}

// Initialization of neuron. Neuron is defined by curve (type) and weight [-kWeightRange,
// kWeightRange]
// third argument is just id
Neuron::Neuron(int8_t w, uint8_t f, uint16_t i) : weight(w), type(f), id(i) {
	assert(type < neuron_curves.size());
	assert(weight >= -kNeuronWeightLimit && weight <= kNeuronWeightLimit);
	recalculate();
}

// Weight, or rather value in range [-kWeightRange, kWeightRange]. Automatically adjusts the weight
// to the range in case of
// overflow.
void Neuron::set_weight(int8_t w) {
	weight = Neuron::clip_weight_to_range(w);
}

// Neuron stores calculated values in an array of size 21.
// This has to be recalculated when the weight or curve type change
void Neuron::recalculate() {
	assert(neuron_curves.size() > type);
	for (uint8_t i = 0; i <= kNeuronMaxPosition; ++i) {
		results[i] = weight * neuron_curves[type][i] / kNeuronWeightLimit;
	}
}

// The Following two functions return Neuron values on position
int8_t Neuron::get_result(const size_t pos) {
	assert(pos <= kNeuronMaxPosition);
	return results[pos];
}

// get value corresponding to input in range 0-20, if you are out of range
// the input will be cropped
int8_t Neuron::get_result_safe(int32_t pos, const bool absolute) {
	// pos has to be normalized into range 0 - 20(kNeuronMaxPosition)
	pos = std::max(0, std::min(static_cast<int>(kNeuronMaxPosition), pos));

	assert(pos <= static_cast<int32_t>(kNeuronMaxPosition));
	assert(pos >= 0);
	assert(results[pos] >= -kNeuronWeightLimit && results[pos] <= kNeuronWeightLimit);

	if (absolute) {
		return std::abs(results[pos]);
	}
	return results[pos];
}

// Setting the type of curve
void Neuron::set_type(uint8_t new_type) {
	assert(new_type < neuron_curves.size());
	type = new_type;
}

// FNeuron is basically a single uint32_t integer, and the AI can query every bit of that uint32_t
FNeuron::FNeuron(uint32_t c, uint16_t i) : core(c), id(i) {
}

// Returning a result depending on combinations of 5 bools
// Bools are completely anonymous, but can present any yes/no inputs, e.g. imagine the AI that is
// to figure out if it should attack from a militarysite. The inputs can be:
// bool1 - are we stronger than the enemy?
// bool2 - do we have a basic economy established?
// bool3 - do we have local predominance?
// bool4 - has our strength grown during the last 60 minutes?
// bool5 - are there mines in the vicinity?
// These five bools can create 32 combinations = yes/no answers.
// In fact this can be perceived as a complicated if..then structure, but one that can
// adjust automatically as a part of training.
// Or rather it is a 5-dimensional table with 2 columns in every dimension :)
// In fact this concept if very demanding for training so we don't use it much
bool FNeuron::get_result(
   const bool bool1, const bool bool2, const bool bool3, const bool bool4, const bool bool5) {
	return core.test(static_cast<int>(bool1) * 16 + static_cast<int>(bool2) * 8 +
	                 static_cast<int>(bool3) * 4 + static_cast<int>(bool4) * 2 +
	                 static_cast<int>(bool5));
}

// Returning bool on a position
bool FNeuron::get_position(const uint8_t pos) {
	assert(pos < kFNeuronBitSize);
	return core.test(pos);
}

// Returning numerical value of FNeuron. Used for saving and priting into log
uint32_t FNeuron::get_int() {
	return core.to_ulong();
}

// This is basically a mutation of FNeuron
void FNeuron::flip_bit(const uint8_t pos) {
	assert(pos < kFNeuronBitSize);
	core.flip(pos);
}

// Shifting the value in range -kWeightRange to kWeightRange, if zero_align is true, it is now
// allowed to shift
// from negative to positive and vice versa, 0 must be used.
int8_t ManagementData::shift_weight_value(const int8_t old_value, const bool aggressive) {

	int16_t halfVArRange = 50;
	if (aggressive) {
		halfVArRange = 200;
	}

	const int16_t upper_limit = std::min<int16_t>(old_value + halfVArRange, kNeuronWeightLimit);
	const int16_t bottom_limit = std::max<int16_t>(old_value - halfVArRange, -kNeuronWeightLimit);
	int16_t new_value = bottom_limit + RNG::static_rand(upper_limit - bottom_limit + 1);

	if (!aggressive && ((old_value > 0 && new_value < 0) || (old_value < 0 && new_value > 0))) {
		new_value = 0;
	}

	new_value = Neuron::clip_weight_to_range(new_value);
	return static_cast<int8_t>(new_value);
}

// Used to score performance of AI
// Should be disabled for "production"
void ManagementData::review(const Time& gametime,
                            Widelands::PlayerNumber pn,
                            const uint32_t land,
                            const uint32_t max_e_land,
                            const uint32_t old_land,
                            const uint16_t attackers,
                            const int16_t trained_soldiers,
                            const uint16_t strength,
                            const uint32_t existing_ps,
                            const Time& first_iron_mine_time,
                            const uint16_t ships_count,
                            const uint16_t finished_mine_types) {

	// bonuses (something or nothing)
	const uint16_t iron_mine_bonus = (first_iron_mine_time < Time(2 * 60 * 60 * 1000)) ? 1000 : 0;
	const uint16_t attack_bonus = (attackers > 0) ? kAttackBonus : 0;
	const uint16_t training_bonus = (trained_soldiers > 0) ? 1000 : 0;
	// For having at least one mine of each type (iron, coal, ...), so up to 1000 points
	const uint16_t finished_mine_type_bonus = finished_mine_types * 250;
	// Are this player best one in the game? Probably does not make sense on irregular maps
	const uint16_t best_player_bonus = (land > max_e_land) ? 100 : 0;

	// scores (numbers dependant on performance)
	// points for the size of territory
	const uint16_t land_score = land / kCurrentLandDivider;
	// points for the territory growth within last 60 minutes
	const int16_t territory_growth_bonus = (land - old_land) * kLandDeltaMultiplier;
	// score for what get_player_power() returns
	const uint16_t strength_score = std::min<uint16_t>(strength, 200) * kStrengthMultiplier;
	// score for soldiers that ever attacked (can repeat of course)
	const uint16_t attackers_score = std::min<uint16_t>(attackers, 200) * kAttackersBonus;
	const uint32_t ps_sites_score = kPSitesRatioMultiplier * std::pow(existing_ps, 3) / 1000 / 1000;
	// On most maps AI will not build ships of course
	const uint32_t ships_score = kShipBonus * ships_count;

	score = territory_growth_bonus + iron_mine_bonus + attack_bonus + training_bonus + land_score +
	        strength_score + ps_sites_score + attackers_score + ships_score +
	        finished_mine_type_bonus + best_player_bonus;

	verb_log_dbg_time(gametime,
	                  "AIPARSE %2d reviewing sc: %5d Pr.p: %d (Bonuses:Te:%s I:%s Tr:%s, "
	                  "Scores:Land:%5d+%4d Str:%4d PS:%4d, Att:%4d, Sh:%d, FinMns:%d\n",
	                  pn, score, primary_parent, (best_player_bonus) ? "Y" : "N",
	                  (iron_mine_bonus) ? "Y" : "N", (training_bonus) ? "Y" : "N", land_score,
	                  territory_growth_bonus, strength_score, ps_sites_score,
	                  attack_bonus + attackers_score, ships_count, finished_mine_types);

	if (score < -10000 || score > 30000) {
		verb_log_dbg_time(gametime, "%2d %s: reviewing AI mngm. data, score too extreme: %4d\n", pn,
		                  gamestring_with_leading_zeros(gametime.get()), score);
	}
	assert(score > -10000 && score < 100000);
}

// Here we generate new AI DNA (no mutation yet) and push them into persistent data
// this can cause inconsistency between local and persistent
void ManagementData::new_dna_for_persistent(const uint8_t pn, const AiType type) {

	ai_type = type;

	verb_log_dbg("%2d: DNA initialization... \n", pn);

	primary_parent = RNG::static_rand(4);
	const uint8_t parent2 = RNG::static_rand(4);

	std::vector<int16_t> AI_military_numbers_P1(
	   Widelands::Player::AiPersistentState::kMagicNumbersSize);
	std::vector<int8_t> input_weights_P1(Widelands::Player::AiPersistentState::kNeuronPoolSize);
	std::vector<int8_t> input_func_P1(Widelands::Player::AiPersistentState::kNeuronPoolSize);
	std::vector<uint32_t> f_neurons_P1(Widelands::Player::AiPersistentState::kFNeuronPoolSize);
	ai_dna_handler.fetch_dna(
	   AI_military_numbers_P1, input_weights_P1, input_func_P1, f_neurons_P1, primary_parent + 1);

	std::vector<int16_t> AI_military_numbers_P2(
	   Widelands::Player::AiPersistentState::kMagicNumbersSize);
	std::vector<int8_t> input_weights_P2(Widelands::Player::AiPersistentState::kNeuronPoolSize);
	std::vector<int8_t> input_func_P2(Widelands::Player::AiPersistentState::kNeuronPoolSize);
	std::vector<uint32_t> f_neurons_P2(Widelands::Player::AiPersistentState::kFNeuronPoolSize);
	ai_dna_handler.fetch_dna(
	   AI_military_numbers_P2, input_weights_P2, input_func_P2, f_neurons_P2, parent2 + 1);

	verb_log_dbg("    ... Primary parent: %d, secondary parent: %d\n", primary_parent, parent2);

	// First setting of military numbers, they go directly to persistent data
	for (uint16_t i = 0; i < Widelands::Player::AiPersistentState::kMagicNumbersSize; ++i) {
		// Child inherits DNA with probability 1/kSecondParentProbability from main parent
		DnaParent dna_donor = (RNG::static_rand(kSecondParentProbability) > 0) ?
                               DnaParent::kPrimary :
                               DnaParent::kSecondary;

		switch (dna_donor) {
		case DnaParent::kPrimary:
			set_military_number_at(i, AI_military_numbers_P1[i]);
			break;
		case DnaParent::kSecondary:
			set_military_number_at(i, AI_military_numbers_P2[i]);
			break;
		}
	}

	persistent_data->neuron_weights.clear();
	persistent_data->neuron_functs.clear();
	persistent_data->f_neurons.clear();

	for (uint16_t i = 0; i < Widelands::Player::AiPersistentState::kNeuronPoolSize; ++i) {
		const DnaParent dna_donor = (RNG::static_rand(kSecondParentProbability) > 0) ?
                                     DnaParent::kPrimary :
                                     DnaParent::kSecondary;

		switch (dna_donor) {
		case DnaParent::kPrimary:
			persistent_data->neuron_weights.push_back(input_weights_P1[i]);
			persistent_data->neuron_functs.push_back(input_func_P1[i]);
			break;
		case DnaParent::kSecondary:
			persistent_data->neuron_weights.push_back(input_weights_P2[i]);
			persistent_data->neuron_functs.push_back(input_func_P2[i]);
			break;
		}
	}

	for (uint16_t i = 0; i < Widelands::Player::AiPersistentState::kFNeuronPoolSize; ++i) {
		const DnaParent dna_donor = (RNG::static_rand(kSecondParentProbability) > 0) ?
                                     DnaParent::kPrimary :
                                     DnaParent::kSecondary;
		switch (dna_donor) {
		case DnaParent::kPrimary:
			persistent_data->f_neurons.push_back(f_neurons_P1[i]);
			break;
		case DnaParent::kSecondary:
			persistent_data->f_neurons.push_back(f_neurons_P2[i]);
			break;
		}
	}

	assert(persistent_data->magic_numbers.size() ==
	       Widelands::Player::AiPersistentState::kMagicNumbersSize);
}
// Decides if mutation takes place and how intensive it will be
MutatingIntensity ManagementData::do_mutate(const bool is_preferred,
                                            const int16_t mutation_probability) {
	if (is_preferred) {
		return MutatingIntensity::kAgressive;
	}
	if (RNG::static_rand(mutation_probability) == 0) {
		return MutatingIntensity::kNormal;
	}
	return MutatingIntensity::kNo;
}

// Mutating, but all done on persistent data
void ManagementData::mutate(const uint8_t pn) {

	// Below numbers are used to dictate intensity of mutation
	// Probability that a value will be mutated = 1 / probability
	// (lesser number means higher probability and higher mutation)
	int16_t mutation_intensity = (ai_training_mode_) ? kTrainingMutation : kNormalMutation;
	// Some of mutation will be agressive - over full range of values, the number below
	// say how many (aproximately) they will be
	uint16_t preferred_numbers_count = 0;
	// This is used to store status whether wild card was or was not used
	bool wild_card = false;

	// decreasing probability (or rather increasing probability of mutation) if weaker player
	// The reason is to make them play worse
	if (ai_type == AiType::kWeak) {
		mutation_intensity /= 15;
		preferred_numbers_count = 25;
	} else if (ai_type == AiType::kVeryWeak) {
		mutation_intensity /= 40;
		preferred_numbers_count = 50;
	}

	// Wildcard for ai trainingmode. This means that one of 8 AI players will mutate more agressively
	// sometimes such more agrassively mutated AI player surprises and have better results than
	// the rest
	// (number 8 is just arbitrary number)
	if (ai_training_mode_ && RNG::static_rand(8) == 0 && ai_type == AiType::kNormal) {
		mutation_intensity /= 3;
		preferred_numbers_count = 5;
		wild_card = true;
	}

	verb_log_dbg("AIPARSE %2d  mutating_probability 1 / %3d preffered numbers target %d %s\n", pn,
	             mutation_intensity, preferred_numbers_count, (wild_card) ? ", wild card" : "");

	// This statistics is not used in the game, but is printed and perhaps evaluated by a human
	// Helps to understand how aggressive the mutation was in each of category:
	// military numbers, neurons, f-neurons. So its length is 3
	uint16_t mutation_stat[3] = {};

	// Modifying pool of Military numbers
	{
		// Preferred numbers are ones that will be mutated agressively in full range
		// [-kWeightRange, kWeightRange]
		std::set<int32_t> preferred_numbers;
		for (int i = 0; i < preferred_numbers_count; i++) {
			preferred_numbers.insert(RNG::static_rand(pref_number_probability));
		}

		for (uint16_t i = 0; i < Widelands::Player::AiPersistentState::kMagicNumbersSize; ++i) {

			const MutatingIntensity mutating_intensity =
			   do_mutate(preferred_numbers.count(i) > 0, mutation_intensity);

			if (mutating_intensity != MutatingIntensity::kNo) {
				const int16_t old_value = get_military_number_at(i);
				const int16_t new_value = shift_weight_value(
				   get_military_number_at(i), mutating_intensity == MutatingIntensity::kAgressive);
				set_military_number_at(i, new_value);
				++mutation_stat[kMilitaryNumbersPos];
				verb_log_dbg("      Magic number %3d: value changed: %4d -> %4d  %s\n", i, old_value,
				             new_value,
				             (mutating_intensity == MutatingIntensity::kAgressive) ? "aggressive" : "");
			}
		}
	}

	// Modifying pool of neurons
	{
		// Neurons to be mutated more agressively
		std::set<int32_t> preferred_neurons;
		for (int i = 0; i < preferred_numbers_count; i++) {
			preferred_neurons.insert(RNG::static_rand(pref_number_probability));
		}
		for (auto& item : neuron_pool) {

			const MutatingIntensity mutating_intensity =
			   do_mutate(preferred_neurons.count(item.get_id()) > 0, mutation_intensity);

			if (mutating_intensity != MutatingIntensity::kNo) {
				const int16_t old_value = item.get_weight();
				if (RNG::static_rand(4) == 0) {
					assert(!neuron_curves.empty());
					item.set_type(RNG::static_rand(neuron_curves.size()));
					persistent_data->neuron_functs[item.get_id()] = item.get_type();
				} else {
					int16_t new_value = shift_weight_value(
					   item.get_weight(), mutating_intensity == MutatingIntensity::kAgressive);
					item.set_weight(new_value);
					persistent_data->neuron_weights[item.get_id()] = item.get_weight();
				}
				++mutation_stat[kNeuronsPos];
				verb_log_dbg("      Neuron %2d: weight: %4d -> %4d, new curve: %d   %s\n",
				             item.get_id(), old_value, item.get_weight(), item.get_type(),
				             (mutating_intensity == MutatingIntensity::kAgressive) ? "aggressive" : "");

				item.recalculate();
			}
		}
	}

	// Modifying pool of f-neurons
	{
		// FNeurons to be mutated more agressively
		std::set<int32_t> preferred_f_neurons;
		// preferred_numbers_count is multiplied by 3 because FNeuron store more than
		// one value
		for (int i = 0; i < 3 * preferred_numbers_count; i++) {
			preferred_f_neurons.insert(RNG::static_rand(pref_number_probability));
		}

		for (auto& item : f_neuron_pool) {
			uint8_t changed_bits = 0;
			// is this a preferred neuron
			if (preferred_f_neurons.count(item.get_id()) > 0) {
				for (uint8_t i = 0; i < kFNeuronBitSize; ++i) {
					if (RNG::static_rand(5) == 0) {
						item.flip_bit(i);
						++changed_bits;
					}
				}
			} else {  // normal mutation
				for (uint8_t i = 0; i < kFNeuronBitSize; ++i) {
					if (RNG::static_rand(mutation_intensity * 3) == 0) {
						item.flip_bit(i);
						++changed_bits;
					}
				}
			}

			if (changed_bits != 0u) {  // -> the f-neuron was changed
				++mutation_stat[kFNeuronsPos];
				persistent_data->f_neurons[item.get_id()] = item.get_int();
				verb_log_dbg("      F-Neuron %2d: new value: %13ul, changed bits: %2d   %s\n",
				             item.get_id(), item.get_int(), changed_bits,
				             (preferred_f_neurons.count(item.get_id()) > 0) ? "aggressive" : "");
			}
		}
	}

	verb_log_dbg("AIPARSE %2d mutation_statistics %d %d %d\n", pn,
	             mutation_stat[kMilitaryNumbersPos], mutation_stat[kNeuronsPos],
	             mutation_stat[kFNeuronsPos]);

	test_consistency();
}

// Now we copy persistent to local
void ManagementData::copy_persistent_to_local() {

	assert(persistent_data->neuron_weights.size() ==
	       Widelands::Player::AiPersistentState::kNeuronPoolSize);
	assert(persistent_data->neuron_functs.size() ==
	       Widelands::Player::AiPersistentState::kNeuronPoolSize);
	neuron_pool.clear();
	for (uint32_t i = 0; i < Widelands::Player::AiPersistentState::kNeuronPoolSize; ++i) {
		neuron_pool.emplace_back(
		   persistent_data->neuron_weights[i], persistent_data->neuron_functs[i], i);
	}

	assert(persistent_data->f_neurons.size() ==
	       Widelands::Player::AiPersistentState::kFNeuronPoolSize);
	f_neuron_pool.clear();
	for (uint32_t i = 0; i < Widelands::Player::AiPersistentState::kFNeuronPoolSize; ++i) {
		f_neuron_pool.emplace_back(persistent_data->f_neurons[i], i);
	}

	assert(persistent_data->magic_numbers.size() ==
	       Widelands::Player::AiPersistentState::kMagicNumbersSize);

	test_consistency();
	verb_log_dbg("    ... DNA initialized\n");
}

void ManagementData::test_consistency(bool itemized) {
	assert(persistent_data->magic_numbers.size() ==
	       Widelands::Player::AiPersistentState::kMagicNumbersSize);
	assert(persistent_data->neuron_weights.size() ==
	       Widelands::Player::AiPersistentState::kNeuronPoolSize);
	assert(persistent_data->neuron_functs.size() ==
	       Widelands::Player::AiPersistentState::kNeuronPoolSize);
	assert(neuron_pool.size() == Widelands::Player::AiPersistentState::kNeuronPoolSize);
	assert(f_neuron_pool.size() == Widelands::Player::AiPersistentState::kFNeuronPoolSize);

	if (itemized) {
		// comparing contents of neuron and fneuron pools
		for (uint16_t i = 0; i < Widelands::Player::AiPersistentState::kNeuronPoolSize; ++i) {
			assert(persistent_data->neuron_weights[i] == neuron_pool[i].get_weight());
			assert(persistent_data->neuron_functs[i] == neuron_pool[i].get_type());
			assert(neuron_pool[i].get_id() == i);
		}
		for (uint16_t i = 0; i < Widelands::Player::AiPersistentState::kFNeuronPoolSize; ++i) {
			assert(persistent_data->f_neurons[i] == f_neuron_pool[i].get_int());
			assert(f_neuron_pool[i].get_id() == i);
		}
	}
}

void ManagementData::dump_data(const Widelands::PlayerNumber pn) {
	ai_dna_handler.dump_output(persistent_data, pn);
}

// Querying military number at a possition
int16_t ManagementData::get_military_number_at(uint8_t pos) const {
	assert(pos < Widelands::Player::AiPersistentState::kMagicNumbersSize);
	return persistent_data->magic_numbers.at(pos);
}

// Setting military number (persistent numbers are used also for local use)
void ManagementData::set_military_number_at(const uint8_t pos, int16_t value) const {
	assert(pos < Widelands::Player::AiPersistentState::kMagicNumbersSize);
	assert(persistent_data->magic_numbers.size() ==
	       Widelands::Player::AiPersistentState::kMagicNumbersSize);
	persistent_data->magic_numbers.at(pos) = Neuron::clip_weight_to_range(value);
}

uint16_t MineTypesObserver::total_count() const {
	return in_construction + finished;
}

// this is used to count militarysites by their size
MilitarySiteSizeObserver::MilitarySiteSizeObserver() : in_construction(0), finished(0) {
}

// this represents a scheduler task
SchedulerTask::SchedulerTask(const Time& time,
                             const SchedulerTaskId t,
                             const uint8_t p,
                             const char* d)
   : due_time(time), id(t), priority(p), descr(d) {
	call_count = 0;
	total_exec_time_ms = 0;
	max_exec_time_ms = 0;
}

bool SchedulerTask::operator<(const SchedulerTask& other) const {
	return priority > other.priority;
}

// List of blocked fields with block time, with some accompanying functions
void BlockedFields::add(Widelands::Coords coords, const Time& till) {
	const uint32_t hash = coords.hash();
	if (blocked_fields_.count(hash) == 0) {
		blocked_fields_.insert(std::make_pair(hash, till));
	} else if (blocked_fields_[hash] < till) {
		blocked_fields_[hash] = till;
	}
	// The third possibility is that a field has been already blocked for longer time than 'till'
}

uint32_t BlockedFields::count() {
	return blocked_fields_.size();
}

void BlockedFields::remove_expired(const Time& gametime) {
	std::vector<uint32_t> fields_to_remove;
	for (const auto& field : blocked_fields_) {
		if (field.second < gametime) {
			fields_to_remove.push_back(field.first);
		}
	}
	while (!fields_to_remove.empty()) {
		blocked_fields_.erase(fields_to_remove.back());
		fields_to_remove.pop_back();
	}
}

bool BlockedFields::is_blocked(Widelands::Coords coords) {
	return (blocked_fields_.count(coords.hash()) != 0);
}

PlayersStrengths::PlayersStrengths() : update_time(0) {
}

// Constructor to be used
PlayersStrengths::PlayerStat::PlayerStat(Widelands::TeamNumber tc,
                                         uint32_t pp,
                                         uint32_t op,
                                         uint32_t o60p,
                                         uint32_t cs,
                                         uint32_t land,
                                         uint32_t oland,
                                         uint32_t o60l)
   : team_number(tc),
     players_power(pp),
     old_players_power(op),
     old60_players_power(o60p),
     players_casualities(cs),

     players_land(land),
     old_players_land(oland),
     old60_players_land(o60l) {
}

// Inserting/updating data
// We keep information for
// - player strength / power
// - player casualties
// - player land
// We store actual values, but for some of them we store also
// - old = 15 mins ago
// - old60 = 60 mins ago
// e.g. players_power / old_players_power / old60_players_power
// we recieve also player and team numbers to figure out if we are enemies, or in the team
void PlayersStrengths::add(Widelands::PlayerNumber pn,
                           Widelands::PlayerNumber opn,
                           Widelands::TeamNumber mytn,
                           Widelands::TeamNumber pltn,
                           uint32_t pp,
                           uint32_t op,
                           uint32_t o60p,
                           uint32_t cs,
                           uint32_t land,
                           uint32_t oland,
                           uint32_t o60l) {
	if (all_stats.count(opn) == 0) {
		this_player_number = pn;
		this_player_team = mytn;
		all_stats.insert(std::make_pair(opn, PlayerStat(pltn, pp, op, o60p, cs, land, oland, o60l)));
	} else {
		all_stats[opn].players_power = pp;
		all_stats[opn].old_players_power = op;
		all_stats[opn].old60_players_power = o60p;
		all_stats[opn].players_casualities = cs;
		all_stats[opn].players_land = land;
		all_stats[opn].old_players_land = oland;
		all_stats[opn].old60_players_land = oland;
		assert(this_player_number == pn);
		if (this_player_team != mytn) {
			verb_log_dbg("%2d: Team changed %d -> %d\n", pn, this_player_team, mytn);
			this_player_team = mytn;
		}
		if (all_stats[opn].team_number != pltn) {
			verb_log_dbg("%2d: Team changed for player %d: %d -> %d\n", pn, opn,
			             all_stats[opn].team_number, pltn);
			all_stats[opn].team_number = pltn;
		}
	}
}

// Very tiny possibility that player that has a statistics info here
// does not exist anymore
void PlayersStrengths::remove_stat(const Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		verb_log_dbg("%d: AI: Erasing statistics for player %d\n", this_player_number, pn);
		all_stats.erase(pn);
	}
}

// After statistics for team members are updated, this calculation is needed
void PlayersStrengths::recalculate_team_power() {
	team_powers.clear();
	for (auto& item : all_stats) {
		if (item.second.team_number > 0) {  // is a member of a team
			if (team_powers.count(item.second.team_number) > 0) {
				team_powers[item.second.team_number] += item.second.players_power;
			} else {
				team_powers[item.second.team_number] = item.second.players_power;
			}
		}
	}
}

// This just goes over information about all enemies and where they were seen the last time
bool PlayersStrengths::any_enemy_seen_lately(const Time& gametime) {
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first) && player_seen_lately(item.first, gametime)) {
			return true;
		}
	}
	return false;
}

// Returns count of nearby enemies
uint8_t PlayersStrengths::enemies_seen_lately_count(const Time& gametime) {
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first) && player_seen_lately(item.first, gametime)) {
			++count;
		}
	}
	return count;
}

// When we see enemy, we use this to store the time
void PlayersStrengths::set_last_time_seen(const Time& seentime, Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) == 0) {
		return;
	}
	all_stats[pn].last_time_seen = seentime;
}

bool PlayersStrengths::get_is_enemy(Widelands::PlayerNumber other_player_number) {
	// So this is me
	if (other_player_number == this_player_number) {
		return false;
	}
	// If we do not belong to any team, all others are our enemies
	if (this_player_team == 0) {
		return true;
	}
	if (all_stats.count(other_player_number) == 0) {
		// Should happen only rarely so we print a warning here
		verb_log_warn("AI %d: player has no statistics yet for player %d\n", this_player_number,
		              other_player_number);
		return false;
	}
	// finally we compare my team number of the other player team number
	return all_stats[other_player_number].team_number != this_player_team;
}

// Was the player seen less then 2 minutes ago
bool PlayersStrengths::player_seen_lately(Widelands::PlayerNumber pn, const Time& gametime) {
	if (all_stats.count(pn) == 0) {
		// Should happen only rarely so we print a warning here
		verb_log_warn("AI %d: player has no statistics yet\n", this_player_number);
		return false;
	}
	if (all_stats[pn].last_time_seen.is_invalid()) {
		return false;
	}
	return all_stats[pn].last_time_seen + Duration(2U * 60U * 1000U) > gametime;
}

// This is the strength of a player
uint32_t PlayersStrengths::get_player_power(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_power;
	}
	return 0;
}

// This is the land size owned by player
uint32_t PlayersStrengths::get_player_land(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_land;
	}
	return 0;
}

// Calculates the strength of the enemies seen within the last 2 minutes
uint32_t PlayersStrengths::get_visible_enemies_power(const Time& gametime) {
	uint32_t pw = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first) && player_seen_lately(item.first, gametime)) {
			pw += item.second.players_power;
		}
	}
	return pw;
}

uint32_t PlayersStrengths::get_enemies_average_power() {
	uint32_t sum = 0;
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			sum += item.second.players_power;
			++count;
		}
	}
	if (count > 0) {
		return sum / count;
	}
	return 0;
}

uint32_t PlayersStrengths::get_enemies_average_land() {
	uint32_t sum = 0;
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			sum += item.second.players_land;
			++count;
		}
	}
	if (count > 0) {
		return sum / count;
	}
	return 0;
}

// Strength of stronger player
uint32_t PlayersStrengths::get_enemies_max_power() {
	uint32_t power = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			power = std::max<uint32_t>(power, item.second.players_power);
		}
	}
	return power;
}

uint32_t PlayersStrengths::get_enemies_max_land() {
	uint32_t land = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			land = std::max<uint32_t>(land, item.second.players_land);
		}
	}
	return land;
}

uint32_t PlayersStrengths::get_old_player_power(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].old_players_power;
	}
	return 0;
}

uint32_t PlayersStrengths::get_old60_player_power(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].old60_players_power;
	}
	return 0;
}

uint32_t PlayersStrengths::get_old_player_land(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) == 0) {
		verb_log_dbg(" AI %d: Players statistics are still empty\n", pn);
		return 0;
	}
	return all_stats[pn].old_players_land;
}

uint32_t PlayersStrengths::get_old60_player_land(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) == 0) {
		verb_log_dbg(" AI %d: Players statistics are still empty\n", pn);
		return 0;
	}
	return all_stats[pn].old60_players_land;
}

uint32_t PlayersStrengths::get_old_visible_enemies_power(const Time& gametime) {
	uint32_t pw = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first) && player_seen_lately(item.first, gametime)) {
			pw += item.second.old_players_power;
		}
	}
	return pw;
}

// This is strength of player plus third of strength of other members of his team
uint32_t PlayersStrengths::get_modified_player_power(Widelands::PlayerNumber pn) {
	uint32_t result = 0;
	Widelands::TeamNumber team = 0;
	if (all_stats.count(pn) > 0) {
		result = all_stats[pn].players_power;
		team = all_stats[pn].team_number;
	}
	if (team > 0 && team_powers.count(team) > 0) {
		result = result + (team_powers[team] - result) / 3;
	}
	return result;
}

// Are the player in the same team
bool PlayersStrengths::players_in_same_team(Widelands::PlayerNumber pl1,
                                            Widelands::PlayerNumber pl2) {
	assert(all_stats.count(pl1) > 0);
	assert(all_stats.count(pl2) > 0);
	return pl1 != pl2 && all_stats.at(pl1).team_number > 0 &&
	       all_stats.at(pl1).team_number == all_stats.at(pl2).team_number;
}

bool PlayersStrengths::strong_enough(Widelands::PlayerNumber pl) {
	if (all_stats.count(pl) == 0) {
		return false;
	}
	uint32_t my_strength = all_stats[pl].players_power;
	uint32_t strongest_opponent_strength = 0;
	for (const auto& item : all_stats) {
		if (!players_in_same_team(item.first, pl) && pl != item.first) {
			if (get_modified_player_power(item.first) > strongest_opponent_strength) {
				strongest_opponent_strength = get_modified_player_power(item.first);
			}
		}
	}
	return my_strength > strongest_opponent_strength + 50;
}

// Update_time is used to prevent too frequent updates of statistics
void PlayersStrengths::set_update_time(const Time& gametime) {
	update_time = gametime;
}

const Time& PlayersStrengths::get_update_time() {
	return update_time;
}

FlagWarehouseDistances::FlagInfo::FlagInfo(const Time& gametime,
                                           const uint16_t dist,
                                           const uint32_t wh) {
	expiry_time = gametime + kFlagDistanceExpirationPeriod;
	soft_expiry_time = gametime + kFlagDistanceExpirationPeriod / 2;
	distance = dist;
	nearest_warehouse = wh;
	new_road_prohibited_till = Time(0);
}
FlagWarehouseDistances::FlagInfo::FlagInfo() {
	expiry_time = Time(0);
	distance = 1000;
	new_road_prohibited_till = Time(0);
}

// We are updating the distance info, but not all the time.
// Always if after soft expiration period, but
// if below expiration period only when the new value is lower than current one
// In both cases new expiration times are calculated
bool FlagWarehouseDistances::FlagInfo::update(const Time& gametime,
                                              const uint16_t new_distance,
                                              const uint32_t nearest_wh) {
	const Time new_expiry_time = gametime + kFlagDistanceExpirationPeriod;

	if (gametime > soft_expiry_time) {
		distance = new_distance;
		expiry_time = new_expiry_time;
		soft_expiry_time = gametime + kFlagDistanceExpirationPeriod / 2;
		nearest_warehouse = nearest_wh;
		return true;
	}
	if (new_distance < distance || (new_distance == distance && expiry_time < new_expiry_time)) {
		distance = new_distance;
		expiry_time = new_expiry_time;
		nearest_warehouse = nearest_wh;
		return true;
	}
	return false;
}

uint16_t FlagWarehouseDistances::FlagInfo::get(const Time& gametime, uint32_t* nw) const {
	if (nw != nullptr) {  // cannot set if this is just nullptr
		*nw = nearest_warehouse;
	}
	if (gametime <= expiry_time) {
		return distance;
	}
	return kWhNotReachable;
}

void FlagWarehouseDistances::FlagInfo::set_road_built(const Time& gametime) {
	// Prohibiting for next 60 seconds
	new_road_prohibited_till = gametime + Duration(60 * 1000);
}

bool FlagWarehouseDistances::FlagInfo::is_road_prohibited(const Time& gametime) const {
	return new_road_prohibited_till > gametime;
}

bool FlagWarehouseDistances::set_distance(const uint32_t flag_coords,
                                          const uint16_t distance,
                                          const Time& gametime,
                                          uint32_t const nearest_warehouse) {
	if (flags_map.count(flag_coords) == 0) {
		flags_map[flag_coords] =
		   FlagWarehouseDistances::FlagInfo(gametime, distance, nearest_warehouse);
		return true;
	}
	return flags_map[flag_coords].update(gametime, distance, nearest_warehouse);
}

uint16_t FlagWarehouseDistances::count() const {
	return flags_map.size();
}

int16_t FlagWarehouseDistances::get_wh_distance(const uint32_t flag_coords,
                                                const Time& gametime,
                                                uint32_t* nw) {
	if (flags_map.count(flag_coords) == 0) {
		if (nw != nullptr) {
			*nw = 0;
		}
		return kWhNotReachable;
	}
	return flags_map[flag_coords].get(gametime, nw);
}

void FlagWarehouseDistances::set_road_built(const uint32_t coords_hash, const Time& gametime) {
	if (flags_map.count(coords_hash) == 1) {
		flags_map[coords_hash].set_road_built(gametime);
	}
}

bool FlagWarehouseDistances::is_road_prohibited(const uint32_t coords_hash, const Time& gametime) {
	if (flags_map.count(coords_hash) == 1) {
		return flags_map[coords_hash].is_road_prohibited(gametime);
	}
	return false;
}

bool FlagWarehouseDistances::remove_old_flag(const Time& gametime) {
	for (std::map<uint32_t, FlagWarehouseDistances::FlagInfo>::iterator it = flags_map.begin();
	     it != flags_map.end(); it++) {
		if (it->second.expiry_time + kOldFlagRemoveTime < gametime) {
			it = flags_map.erase(it);
			return true;
		}
	}
	return false;
}

// Returns pointer to winning road, provided that the treshold is exceeded
FlagCandidates::Candidate* FlagCandidates::get_winner(const int16_t threshold) {
	if (flags_.empty()) {
		return nullptr;
	}
	sort();
	if (flags_[0].score() < threshold) {
		return nullptr;
	}
	if (!flags_[0].is_buildable()) {
		return nullptr;
	}
	return &flags_[0];
}

FlagCandidates::Candidate::Candidate(const uint32_t c_hash,
                                     bool different_eco,
                                     uint16_t start_f_dist,
                                     uint16_t act_dist_to_wh,
                                     uint16_t air_dst) {
	coords_hash = c_hash;
	different_economy = different_eco;
	start_flag_dist_to_wh = start_f_dist;
	flag_to_flag_road_distance = 0;
	possible_road_distance = kWhFarButReachable;
	cand_flag_distance_to_wh = act_dist_to_wh;
	air_distance = air_dst;
}

int16_t FlagCandidates::Candidate::score() const {
	return static_cast<int>(different_economy) * 2000 +
	       (start_flag_dist_to_wh - cand_flag_distance_to_wh) +
	       (flag_to_flag_road_distance - 2 * possible_road_distance);
}

bool FlagCandidates::set_road_possible(const uint32_t coords_hash, const uint16_t steps) {
	for (auto& item : flags_) {
		if (item.coords_hash == coords_hash) {
			item.possible_road_distance = steps;
			return true;
		}
	}
	return false;
}

bool FlagCandidates::set_cur_road_distance(const uint32_t coords, uint16_t dist) {
	for (auto& item : flags_) {
		if (item.coords_hash == coords) {
			item.flag_to_flag_road_distance = dist;
			return true;
		}
	}
	return false;
}
void FlagCandidates::sort() {
	std::sort(flags_.begin(), flags_.end());
}

void FlagCandidates::sort_by_air_distance() {
	std::sort(flags_.begin(), flags_.end(),
	          [](const FlagCandidates::Candidate& lf, const FlagCandidates::Candidate& rf) {
		          return lf.air_distance < rf.air_distance;
	          });
}

void FlagCandidates::add_flag(const uint32_t coords,
                              const bool different_economy,
                              const uint16_t act_dist_to_wh,
                              const uint16_t air_distance) {
	flags_.emplace_back(
	   coords, different_economy, start_flag_dist_to_wh, act_dist_to_wh, air_distance);
}

bool FlagCandidates::has_candidate(const uint32_t coords_hash) const {
	for (const auto& item : flags_) {
		if (item.coords_hash == coords_hash) {
			return true;
		}
	}
	return false;
}

}  // namespace AI
