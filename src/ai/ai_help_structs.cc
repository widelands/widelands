/*
 * Copyright (C) 2009-2017 by the Widelands Development Team
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

#include "ai/ai_help_structs.h"

#include "base/macros.h"
#include "base/time_string.h"
#include "logic/ai_dna_handler.h"
#include "logic/map.h"
#include "logic/player.h"

namespace Widelands {

// couple of constants for calculation of road interconnections
constexpr int kRoadNotFound = -1000;
constexpr int kShortcutWithinSameEconomy = 1000;
constexpr int kRoadToDifferentEconomy = 10000;
constexpr int kUpperDefaultMutationLimit = 150;
constexpr int kLowerDefaultMutationLimit = 75;
constexpr int16_t kPrefNumberProbability = (kAITrainingMode) ? 5 : 100;

// CheckStepRoadAI
CheckStepRoadAI::CheckStepRoadAI(Player* const pl, uint8_t const mc, bool const oe)
   : player(pl), movecaps(mc), open_end(oe) {
}

bool CheckStepRoadAI::allowed(
   Map& map, FCoords start, FCoords end, int32_t, CheckStep::StepId const id) const {
	uint8_t endcaps = player->get_buildcaps(end);

	// we should not cross fields with road or flags (or any other immovable)
	if ((map.get_immovable(start)) && !(id == CheckStep::stepFirst)) {
		return false;
	}

	// Calculate cost and passability
	if (!(endcaps & movecaps))
		return false;

	// Check for blocking immovables
	if (BaseImmovable const* const imm = map.get_immovable(end))
		if (imm->get_size() >= BaseImmovable::SMALL) {
			if (id != CheckStep::stepLast && !open_end)
				return false;

			if (dynamic_cast<Flag const*>(imm))
				return true;

			if (!dynamic_cast<Road const*>(imm) || !(endcaps & BUILDCAPS_FLAG))
				return false;
		}

	return true;
}

bool CheckStepRoadAI::reachable_dest(const Map& map, const FCoords& dest) const {
	NodeCaps const caps = dest.field->nodecaps();

	if (!(caps & movecaps)) {
		if (!((movecaps & MOVECAPS_SWIM) && (caps & MOVECAPS_WALK)))
			return false;

		if (!map.can_reach_by_water(dest))
			return false;
	}

	return true;
}

// We are looking for fields we can walk on
// and owned by hostile player.
FindNodeEnemy::FindNodeEnemy(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeEnemy::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && fc.field->get_owned_by() != 0 &&
	       player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// We are looking for buildings owned by hostile player
// (sometimes there is a enemy's teritorry without buildings, and
// this confuses the AI)
FindNodeEnemiesBuilding::FindNodeEnemiesBuilding(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeEnemiesBuilding::accept(const Map&, const FCoords& fc) const {
	return (fc.field->get_immovable()) && fc.field->get_owned_by() != 0 &&
	       player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// When looking for unowned terrain to acquire, we are actually
// only interested in fields we can walk on.
// Fields should either be completely unowned or owned by an opposing player
FindEnemyNodeWalkable::FindEnemyNodeWalkable(Player* p, Game& g) : player(p), game(g) {
}

bool FindEnemyNodeWalkable::accept(const Map&, const FCoords& fc) const {
	return ((fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() > 0) &&
	        player->is_hostile(*game.get_player(fc.field->get_owned_by())));
}

// Sometimes we need to know how many nodes our allies owns
FindNodeAllyOwned::FindNodeAllyOwned(Player* p, Game& g, PlayerNumber n)
   : player(p), game(g), player_number(n) {
}

bool FindNodeAllyOwned::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() != 0) &&
	       (fc.field->get_owned_by() != player_number) &&
	       !player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// When looking for unowned terrain to acquire, we must
// pay speciall attention to fields where mines can be built.
// Fields should be completely unowned
FindNodeUnownedMineable::FindNodeUnownedMineable(Player* p, Game& g, int32_t t)
   : player(p), game(g), ore_type(t) {
}

bool FindNodeUnownedMineable::accept(const Map&, const FCoords& fc) const {
	if (ore_type == INVALID_INDEX) {
		return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_owned_by() == neutral());
	}
	return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_owned_by() == neutral()) &&
	       fc.field->get_resources() == ore_type;
}

FindNodeUnownedBuildable::FindNodeUnownedBuildable(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeUnownedBuildable::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & BUILDCAPS_SIZEMASK) && (fc.field->get_owned_by() == neutral());
}

// Unowned but walkable fields nearby
FindNodeUnownedWalkable::FindNodeUnownedWalkable(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeUnownedWalkable::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() == neutral());
}

// Looking only for mines-capable fields nearby
// of specific type
FindNodeMineable::FindNodeMineable(Game& g, DescriptionIndex r) : game(g), res(r) {
}

bool FindNodeMineable::accept(const Map&, const FCoords& fc) const {

	return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_resources() == res);
}

// Fishers and fishbreeders must be built near water
FindNodeWater::FindNodeWater(const World& world) : world_(world) {
}

bool FindNodeWater::accept(const Map& map, const FCoords& coord) const {
	return (world_.terrain_descr(coord.field->terrain_d()).get_is() &
	        TerrainDescription::Is::kWater) ||
	       (world_.terrain_descr(map.get_neighbour(coord, WALK_W).field->terrain_r()).get_is() &
	        TerrainDescription::Is::kWater) ||
	       (world_.terrain_descr(map.get_neighbour(coord, WALK_NW).field->terrain_r()).get_is() &
	        TerrainDescription::Is::kWater);
}

bool FindNodeOpenWater::accept(const Map& /* map */, const FCoords& coord) const {
	return !(coord.field->nodecaps() & MOVECAPS_WALK) && (coord.field->nodecaps() & MOVECAPS_SWIM);
}

// FindNodeWithFlagOrRoad
bool FindNodeWithFlagOrRoad::accept(const Map&, FCoords fc) const {
	if (upcast(PlayerImmovable const, pimm, fc.field->get_immovable()))
		return (dynamic_cast<Flag const*>(pimm) ||
		        (dynamic_cast<Road const*>(pimm) && (fc.field->nodecaps() & BUILDCAPS_FLAG)));
	return false;
}

NearFlag::NearFlag(const Flag& f, int32_t const c, int32_t const d)
   : flag(&f), cost(c), distance(d) {
}

EventTimeQueue::EventTimeQueue() {
}

void EventTimeQueue::push(const uint32_t production_time) {
	queue.push(production_time);
}

uint32_t EventTimeQueue::count(const uint32_t current_time) {
	strip_old(current_time);
	return queue.size();
}

void EventTimeQueue::strip_old(const uint32_t current_time) {
	while (!queue.empty() && queue.front() < current_time - duration_) {
		queue.pop();
	}
}

BuildableField::BuildableField(const Widelands::FCoords& fc)
   : coords(fc),
     field_info_expiration(20000),
     preferred(false),
     enemy_nearby(0),
     enemy_accessible_(false),
     enemy_wh_nearby(false),
     unowned_land_nearby(0),
     near_border(false),
     unowned_mines_spots_nearby(0),
     unowned_iron_mines_nearby(false),
     trees_nearby(0),
     // explanation of starting values
     // this is done to save some work for AI (CPU utilization)
     // base rules are:
     // count of rocks can only decrease, so  amount of rocks
     // is recalculated only when previous count is positive
     // count of water fields are stable, so if the current count is
     // non-negative, water is not recaldulated
     rocks_nearby(1),
     water_nearby(-1),
     open_water_nearby(-1),
     distant_water(0),
     fish_nearby(-1),
     critters_nearby(-1),
     ground_water(1),
     space_consumers_nearby(0),
     rangers_nearby(0),
     area_military_capacity(0),
     military_loneliness(1000),
     military_in_constr_nearby(0),
     own_military_presence(0),
     enemy_military_presence(0),
     ally_military_presence(0),
     military_stationed(0),
     unconnected_nearby(false),
     military_unstationed(0),
     own_non_military_nearby(0),
     is_portspace(Widelands::ExtendedBool::kUnset),
     port_nearby(false),
     portspace_nearby(Widelands::ExtendedBool::kUnset),
     max_buildcap_nearby(0),
     last_resources_check_time(0),
     military_score_(0),
     inland(false),
     local_soldier_capacity(0),
     is_militarysite(false) {
}

int32_t BuildableField::own_military_sites_nearby_() {
	return military_stationed + military_unstationed;
}

MineableField::MineableField(const Widelands::FCoords& fc)
   : coords(fc),
     field_info_expiration(20000),
     preferred(false),
     mines_nearby(0),
     same_mine_fields_nearby(0) {
}

EconomyObserver::EconomyObserver(Widelands::Economy& e) : economy(e) {
	dismantle_grace_time = std::numeric_limits<int32_t>::max();
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

Widelands::AiModeBuildings BuildingObserver::aimode_limit_status() {
	if (total_count() > cnt_limit_by_aimode) {
		return Widelands::AiModeBuildings::kLimitExceeded;
	} else if (total_count() == cnt_limit_by_aimode) {
		return Widelands::AiModeBuildings::kOnLimit;
	} else {
		return Widelands::AiModeBuildings::kAnotherAllowed;
	}
}
bool BuildingObserver::buildable(Widelands::Player& p) {
	return is(BuildingAttribute::kBuildable) && p.is_building_type_allowed(id);
}

// Computer player does not get notification messages about enemy militarysites
// and warehouses, so following is collected based on observation
// It is conventient to have some information preserved, like nearby minefields,
// when it was attacked, whether it is warehouse and so on
// Also AI test more such targets when considering attack and calculated score is
// is stored in the observer
EnemySiteObserver::EnemySiteObserver()
   : is_warehouse(false),
     attack_soldiers_strength(0),
     defenders_strength(0),
     stationed_soldiers(0),
     last_time_seen(0),
     last_tested(0),
     score(0),
     mines_nearby(Widelands::ExtendedBool::kUnset),
     last_time_attacked(0),
     attack_counter(0) {
}

// as all mines have 3 levels, AI does not know total count of mines per mined material
// so this observer will be used for this
MineTypesObserver::MineTypesObserver()
   : in_construction(0), finished(0), is_critical(false), unoccupied(0) {
}

ExpansionType::ExpansionType() {
	type = ExpansionMode::kResources;
}

void ExpansionType::set_expantion_type(const ExpansionMode etype) {
	type = etype;
}

ManagementData::ManagementData() {
	score = 1;
	primary_parent = 255;
	next_neuron_id = 0;
	performance_change = 0;
	AiDnaHandler ai_dna_handler();
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
	for (uint8_t i = 0; i <= kNeuronMaxPosition; i += 1) {
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
FNeuron::FNeuron(uint32_t c, uint16_t i) {
	core = c;
	id = i;
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
	return core.test(bool1 * 16 + bool2 * 8 + bool3 * 4 + bool4 * 2 + bool5);
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
	int16_t new_value = bottom_limit + std::rand() % (upper_limit - bottom_limit + 1);

	if (!aggressive && ((old_value > 0 && new_value < 0) || (old_value < 0 && new_value > 0))) {
		new_value = 0;
	}

	new_value = Neuron::clip_weight_to_range(new_value);
	return static_cast<int8_t>(new_value);
}

// Used to score performance of AI
// Should be disabled for "production"
void ManagementData::review(const uint32_t gametime,
                            PlayerNumber pn,
                            const uint32_t land,
                            const uint32_t max_e_land,
                            const uint32_t old_land,
                            const uint16_t attackers,
                            const int16_t trained_soldiers,
                            const int16_t latest_attackers,
                            const uint16_t conq_ws,
                            const uint16_t m_strength,
                            const uint32_t existing_ps) {

	const int16_t main_bonus =
	   ((static_cast<int32_t>(land - old_land) > 0 && land > max_e_land * 5 / 6 && attackers > 0 &&
	     trained_soldiers > 0 && latest_attackers > 0) ?
	       kBonus :
	       0);

	const int16_t land_delta_bonus = static_cast<int16_t>(land - old_land) * kLandDeltaMultiplier;

	const uint32_t ps_sites_bonus =
	   kPSitesRatioMultiplier * existing_ps * existing_ps * existing_ps / 1000 / 1000;

	score = land / kCurrentLandDivider + land_delta_bonus + main_bonus +
	        attackers * kAttackersMultiplier + ((attackers > 0) ? kAttackBonus : -kAttackBonus) +
	        trained_soldiers * kTrainedSoldiersScore + kConqueredWhBonus * conq_ws +
	        m_strength * kStrengthMultiplier + ps_sites_bonus - 500 * kPSitesRatioMultiplier;

	log(" %2d %s: reviewing AI mngm. data, sc: %5d Pr.p: %d (l:%4d/%s/%4d, "
	    "at:%4d(%3d),ts:%4d/%2d,cWH:%2d,str:%2d/%4d,ps:%4d/%4d)\n",
	    pn, gamestring_with_leading_zeros(gametime), score, primary_parent,
	    land / kCurrentLandDivider, (main_bonus) ? "*" : " ", land_delta_bonus,
	    attackers * kAttackersMultiplier, latest_attackers, trained_soldiers * kTrainedSoldiersScore,
	    trained_soldiers, conq_ws, m_strength, m_strength * kStrengthMultiplier, existing_ps,
	    ps_sites_bonus);

	if (score < -10000 || score > 30000) {
		log("%2d %s: reviewing AI mngm. data, score too extreme: %4d\n", pn,
		    gamestring_with_leading_zeros(gametime), score);
	}
	assert(score > -10000 && score < 100000);
}

// Here we generate new AI DNA (no mutation yet) and push them into persistent data
// this can cause inconsistency between local and persistent
void ManagementData::new_dna_for_persistent(const uint8_t pn, const Widelands::AiType type) {

	ai_type = type;

	log("%2d: DNA initialization... \n", pn);

	primary_parent = std::rand() % 4;
	const uint8_t parent2 = std::rand() % 4;

	std::vector<int16_t> AI_military_numbers_P1(kMagicNumbersSize);
	std::vector<int8_t> input_weights_P1(kNeuronPoolSize);
	std::vector<int8_t> input_func_P1(kNeuronPoolSize);
	std::vector<uint32_t> f_neurons_P1(kFNeuronPoolSize);
	ai_dna_handler.fetch_dna(
	   AI_military_numbers_P1, input_weights_P1, input_func_P1, f_neurons_P1, primary_parent + 1);

	std::vector<int16_t> AI_military_numbers_P2(kMagicNumbersSize);
	std::vector<int8_t> input_weights_P2(kNeuronPoolSize);
	std::vector<int8_t> input_func_P2(kNeuronPoolSize);
	std::vector<uint32_t> f_neurons_P2(kFNeuronPoolSize);
	ai_dna_handler.fetch_dna(
	   AI_military_numbers_P2, input_weights_P2, input_func_P2, f_neurons_P2, parent2 + 1);

	log("    ... Primary parent: %d, secondary parent: %d\n", primary_parent, parent2);

	// First setting of military numbers, they go directly to persistent data
	for (uint16_t i = 0; i < kMagicNumbersSize; i += 1) {
		// Child inherits DNA with probability 1/kSecondParentProbability from main parent
		uint8_t dna_donor = ((std::rand() % kSecondParentProbability) > 0) ? 0 : 1;
		if (i == kMutationRatePosition) {  // Overwriting
			dna_donor = 0;
		}

		switch (dna_donor) {
		case 0:
			set_military_number_at(i, AI_military_numbers_P1[i]);
			break;
		case 1:
			set_military_number_at(i, AI_military_numbers_P2[i]);
			break;
		default:
			log("Invalid dna_donor for military numbers: %d?\n", dna_donor);
			NEVER_HERE();
		}
	}

	pd->neuron_weights.clear();
	pd->neuron_functs.clear();
	pd->f_neurons.clear();

	for (uint16_t i = 0; i < kNeuronPoolSize; i += 1) {
		const uint8_t dna_donor = ((std::rand() % kSecondParentProbability) > 0) ? 0 : 1;

		switch (dna_donor) {
		case 0:
			pd->neuron_weights.push_back(input_weights_P1[i]);
			pd->neuron_functs.push_back(input_func_P1[i]);
			break;
		case 1:
			pd->neuron_weights.push_back(input_weights_P2[i]);
			pd->neuron_functs.push_back(input_func_P2[i]);
			break;
		default:
			log("Invalid dna_donor for neurons:  %d?\n", dna_donor);
			NEVER_HERE();
		}
	}

	for (uint16_t i = 0; i < kFNeuronPoolSize; i += 1) {
		const uint8_t dna_donor = (std::rand() % kSecondParentProbability > 0) ? 0 : 1;
		switch (dna_donor) {
		case 0:
			pd->f_neurons.push_back(f_neurons_P1[i]);
			break;
		case 1:
			pd->f_neurons.push_back(f_neurons_P2[i]);
			break;
		default:
			log("Invalid dna_donor for f-neurons:  %d?\n", dna_donor);
			NEVER_HERE();
		}
	}

	pd->magic_numbers_size = kMagicNumbersSize;
	pd->neuron_pool_size = kNeuronPoolSize;
	pd->f_neuron_pool_size = kFNeuronPoolSize;
}
// Decides if mutation takes place and how intensive it will be
MutatingIntensity ManagementData::do_mutate(const uint8_t is_preferred,
                                            const int16_t mutation_probability) {
	if (is_preferred > 0) {
		return MutatingIntensity::kAgressive;
	}
	if (std::rand() % mutation_probability == 0) {
		return MutatingIntensity::kNormal;
	}
	return MutatingIntensity::kNo;
}

// Mutating, but all done on persistent data
void ManagementData::mutate(const uint8_t pn) {

	int16_t probability =
	   shift_weight_value(get_military_number_at(kMutationRatePosition), false) + 101;
	if (probability > kUpperDefaultMutationLimit) {
		probability = kUpperDefaultMutationLimit;
	}
	if (probability < kLowerDefaultMutationLimit) {
		probability = kLowerDefaultMutationLimit;
	}

	set_military_number_at(kMutationRatePosition, probability - 101);

	// decreasing probability (or rather increasing probability of mutation) if weaker player
	if (ai_type == Widelands::AiType::kWeak) {
		probability /= 2;
		log("%2d: Weak mode, increasing mutation probability to 1 / %d\n", pn, probability);
	} else if (ai_type == Widelands::AiType::kVeryWeak) {
		probability /= 4;
		log("%2d: Very weak mode, increasing mutation probability to 1 / %d\n", pn, probability);
	}

	// Widlcard for ai trainingmode
	if (kAITrainingMode && std::rand() % 8 == 0) {
		probability /= 3;
	}

	assert(probability > 0 && probability <= 201);

	log("%2d: mutating DNA with probability 1 / %3d:\n", pn, probability);

	if (probability < 201) {

		// Modifying pool of Military numbers
		{
			// Preferred numbers are ones that will be mutated agressively in full range
			// [-kWeightRange, kWeightRange]
			std::set<int32_t> preferred_numbers = {std::rand() % kMagicNumbersSize *
			                                       kPrefNumberProbability};

			for (uint16_t i = 0; i < kMagicNumbersSize; i += 1) {
				if (i == kMutationRatePosition) {  // mutated above
					continue;
				}

				const MutatingIntensity mutating_intensity =
				   do_mutate(preferred_numbers.count(i) > 0, probability);

				if (mutating_intensity != MutatingIntensity::kNo) {
					const int16_t old_value = get_military_number_at(i);
					const int16_t new_value = shift_weight_value(
					   get_military_number_at(i), mutating_intensity == MutatingIntensity::kAgressive);
					set_military_number_at(i, new_value);
					log("      Magic number %3d: value changed: %4d -> %4d  %s\n", i, old_value,
					    new_value,
					    (mutating_intensity == MutatingIntensity::kAgressive) ? "aggressive" : "");
				}
			}
		}

		// Modifying pool of neurons
		{
			// Neurons to be mutated more agressively
			std::set<int32_t> preferred_neurons = {std::rand() % kNeuronPoolSize *
			                                       kPrefNumberProbability};
			for (auto& item : neuron_pool) {

				const MutatingIntensity mutating_intensity =
				   do_mutate(preferred_neurons.count(item.get_id()) > 0, probability);

				if (mutating_intensity != MutatingIntensity::kNo) {
					const int16_t old_value = item.get_weight();
					if (std::rand() % 4 == 0) {
						assert(!neuron_curves.empty());
						item.set_type(std::rand() % neuron_curves.size());
						pd->neuron_functs[item.get_id()] = item.get_type();
					} else {
						int16_t new_value = shift_weight_value(
						   item.get_weight(), mutating_intensity == MutatingIntensity::kAgressive);
						item.set_weight(new_value);
						pd->neuron_weights[item.get_id()] = item.get_weight();
					}
					log("      Neuron %2d: weight: %4d -> %4d, new curve: %d   %s\n", item.get_id(),
					    old_value, item.get_weight(), item.get_type(),
					    (mutating_intensity == MutatingIntensity::kAgressive) ? "aggressive" : "");

					item.recalculate();
				}
			}
		}

		// Modifying pool of f-neurons
		{
			// FNeurons to be mutated more agressively
			std::set<int32_t> preferred_f_neurons = {std::rand() % kFNeuronPoolSize *
			                                         kPrefNumberProbability};
			for (auto& item : f_neuron_pool) {
				uint8_t changed_bits = 0;
				// is this a preferred neuron
				if (preferred_f_neurons.count(item.get_id()) > 0) {
					for (uint8_t i = 0; i < kFNeuronBitSize; i += 1) {
						if (std::rand() % 5 == 0) {
							item.flip_bit(i);
							changed_bits += 1;
						}
					}
				} else {  // normal mutation
					for (uint8_t i = 0; i < kFNeuronBitSize; i += 1) {
						if (std::rand() % (probability * 3) == 0) {
							item.flip_bit(i);
							changed_bits += 1;
						}
					}
				}

				if (changed_bits) {
					pd->f_neurons[item.get_id()] = item.get_int();
					log("      F-Neuron %2d: new value: %13ul, changed bits: %2d   %s\n", item.get_id(),
					    item.get_int(), changed_bits,
					    (preferred_f_neurons.count(item.get_id()) > 0) ? "aggressive" : "");
				}
			}
		}
	}

	test_consistency();
}

// Now we copy persistent to local
void ManagementData::copy_persistent_to_local() {

	assert(pd->neuron_weights.size() == kNeuronPoolSize);
	assert(pd->neuron_functs.size() == kNeuronPoolSize);
	neuron_pool.clear();
	for (uint32_t i = 0; i < kNeuronPoolSize; i = i + 1) {
		neuron_pool.push_back(Neuron(pd->neuron_weights[i], pd->neuron_functs[i], i));
	}

	assert(pd->f_neurons.size() == kFNeuronPoolSize);
	f_neuron_pool.clear();
	for (uint32_t i = 0; i < kFNeuronPoolSize; i = i + 1) {
		f_neuron_pool.push_back(FNeuron(pd->f_neurons[i], i));
	}

	pd->magic_numbers_size = kMagicNumbersSize;
	pd->neuron_pool_size = kNeuronPoolSize;
	pd->f_neuron_pool_size = kFNeuronPoolSize;

	test_consistency();
	log("    ... DNA initialized\n");
}

void ManagementData::test_consistency(bool itemized) {

	assert(pd->neuron_weights.size() == pd->neuron_pool_size);
	assert(pd->neuron_functs.size() == pd->neuron_pool_size);
	assert(neuron_pool.size() == pd->neuron_pool_size);
	assert(neuron_pool.size() == kNeuronPoolSize);

	assert(pd->magic_numbers_size == kMagicNumbersSize);
	assert(pd->magic_numbers.size() == kMagicNumbersSize);

	assert(pd->f_neurons.size() == pd->f_neuron_pool_size);
	assert(f_neuron_pool.size() == pd->f_neuron_pool_size);
	assert(f_neuron_pool.size() == kFNeuronPoolSize);

	if (itemized) {
		// comparing contents of neuron and fneuron pools
		for (uint16_t i = 0; i < kNeuronPoolSize; i += 1) {
			assert(pd->neuron_weights[i] == neuron_pool[i].get_weight());
			assert(pd->neuron_functs[i] == neuron_pool[i].get_type());
			assert(neuron_pool[i].get_id() == i);
		}
		for (uint16_t i = 0; i < kFNeuronPoolSize; i += 1) {
			assert(pd->f_neurons[i] == f_neuron_pool[i].get_int());
			assert(f_neuron_pool[i].get_id() == i);
		}
	}

	return;
}

void ManagementData::dump_data(const PlayerNumber pn) {
	ai_dna_handler.dump_output(pd, pn);
}

// Querying military number at a possition
int16_t ManagementData::get_military_number_at(uint8_t pos) {
	assert(pos < kMagicNumbersSize);
	return pd->magic_numbers[pos];
}

// Setting military number (persistent numbers are used also for local use)
void ManagementData::set_military_number_at(const uint8_t pos, int16_t value) {
	assert(pos < kMagicNumbersSize);

	while (pos >= pd->magic_numbers.size()) {
		pd->magic_numbers.push_back(0);
	}

	value = Neuron::clip_weight_to_range(value);
	pd->magic_numbers[pos] = value;
}

uint16_t MineTypesObserver::total_count() const {
	return in_construction + finished;
}

// this is used to count militarysites by their size
MilitarySiteSizeObserver::MilitarySiteSizeObserver() : in_construction(0), finished(0) {
}

// this represents a scheduler task
SchedulerTask::SchedulerTask(const uint32_t time,
                             const Widelands::SchedulerTaskId t,
                             const uint8_t p,
                             const char* d)
   : due_time(time), id(t), priority(p), descr(d) {
}

bool SchedulerTask::operator<(const SchedulerTask& other) const {
	return priority > other.priority;
}

// List of blocked fields with block time, with some accompanying functions
void BlockedFields::add(Widelands::Coords coords, uint32_t till) {
	const uint32_t hash = coords.hash();
	if (blocked_fields_.count(hash) == 0) {
		blocked_fields_.insert(std::pair<uint32_t, uint32_t>(hash, till));
	} else if (blocked_fields_[hash] < till) {
		blocked_fields_[hash] = till;
	}
	// The third possibility is that a field has been already blocked for longer time than 'till'
}

uint32_t BlockedFields::count() {
	return blocked_fields_.size();
}

void BlockedFields::remove_expired(uint32_t gametime) {
	std::vector<uint32_t> fields_to_remove;
	for (auto field : blocked_fields_) {
		if (field.second < gametime) {
			fields_to_remove.push_back(field.first);
		}
	}
	while (!fields_to_remove.empty()) {
		blocked_fields_.erase(fields_to_remove.back());
		fields_to_remove.pop_back();
	}
}

bool BlockedFields::is_blocked(Coords coords) {
	return (blocked_fields_.count(coords.hash()) != 0);
}

FlagsForRoads::Candidate::Candidate(uint32_t coords, int32_t distance, bool economy)
   : coords_hash(coords), air_distance(distance), different_economy(economy) {
	new_road_possible = false;
	accessed_via_roads = false;
	// Values are only very rough, and are dependant on the map size
	new_road_length = 2 * Widelands::kMapDimensions.at(Widelands::kMapDimensions.size() - 1);
	current_roads_distance = 2 * (Widelands::kMapDimensions.size() - 1);  // must be big enough
	reduction_score = -air_distance;  // allows reasonable ordering from the start
}

bool FlagsForRoads::Candidate::operator<(const Candidate& other) const {
	if (reduction_score == other.reduction_score) {
		return coords_hash < other.coords_hash;
	} else {
		return reduction_score > other.reduction_score;
	}
}

bool FlagsForRoads::Candidate::operator==(const Candidate& other) const {
	return coords_hash == other.coords_hash;
}

void FlagsForRoads::Candidate::calculate_score() {
	if (!new_road_possible) {
		reduction_score = kRoadNotFound - air_distance;  // to have at least some ordering preserved
	} else if (different_economy) {
		reduction_score = kRoadToDifferentEconomy - air_distance - 2 * new_road_length;
	} else if (!accessed_via_roads) {
		if (air_distance + 6 > new_road_length) {
			reduction_score = kShortcutWithinSameEconomy - air_distance - 2 * new_road_length;
		} else {
			reduction_score = kRoadNotFound;
		}
	} else {
		reduction_score = current_roads_distance - 2 * new_road_length;
	}
}

void FlagsForRoads::print() {  // this is for debugging and development purposes
	for (auto& candidate_flag : queue) {
		log("   %starget: %3dx%3d, saving: %5d (%3d), air distance: %3d, new road: %6d, score: %5d "
		    "%s\n",
		    (candidate_flag.reduction_score >= min_reduction && candidate_flag.new_road_possible) ?
		       "+" :
		       " ",
		    Coords::unhash(candidate_flag.coords_hash).x,
		    Coords::unhash(candidate_flag.coords_hash).y,
		    candidate_flag.current_roads_distance - candidate_flag.new_road_length, min_reduction,
		    candidate_flag.air_distance, candidate_flag.new_road_length,
		    candidate_flag.reduction_score,
		    (candidate_flag.new_road_possible) ? ", new road possible" : " ");
	}
}

// Queue is ordered but some target flags are only estimations so we take such a candidate_flag
// first
bool FlagsForRoads::get_best_uncalculated(uint32_t* winner) {
	for (auto& candidate_flag : queue) {
		if (!candidate_flag.new_road_possible) {
			*winner = candidate_flag.coords_hash;
			return true;
		}
	}
	return false;
}

// Road from starting flag to this flag can be built
void FlagsForRoads::road_possible(Widelands::Coords coords, uint32_t distance) {
	// std::set does not allow updating
	Candidate new_candidate_flag = Candidate(0, 0, false);
	for (auto candidate_flag : queue) {
		if (candidate_flag.coords_hash == coords.hash()) {
			new_candidate_flag = candidate_flag;
			assert(new_candidate_flag.coords_hash == candidate_flag.coords_hash);
			queue.erase(candidate_flag);
			break;
		}
	}

	new_candidate_flag.new_road_length = distance;
	new_candidate_flag.new_road_possible = true;
	new_candidate_flag.calculate_score();
	queue.insert(new_candidate_flag);
}

// Remove the flag from candidates as interconnecting road is not possible
void FlagsForRoads::road_impossible(Widelands::Coords coords) {
	const uint32_t hash = coords.hash();
	for (auto candidate_flag : queue) {
		if (candidate_flag.coords_hash == hash) {
			queue.erase(candidate_flag);
			return;
		}
	}
}

// Updating walking distance over existing roads
// Queue does not allow modifying its members so we erase and then eventually insert modified member
void FlagsForRoads::set_road_distance(Widelands::Coords coords, int32_t distance) {
	const uint32_t hash = coords.hash();
	Candidate new_candidate_flag = Candidate(0, 0, false);
	bool replacing = false;
	for (auto candidate_flag : queue) {
		if (candidate_flag.coords_hash == hash) {
			assert(!candidate_flag.different_economy);
			if (distance < candidate_flag.current_roads_distance) {
				new_candidate_flag = candidate_flag;
				queue.erase(candidate_flag);
				replacing = true;
				break;
			}
			break;
		}
	}
	if (replacing) {
		new_candidate_flag.current_roads_distance = distance;
		new_candidate_flag.accessed_via_roads = true;
		new_candidate_flag.calculate_score();
		queue.insert(new_candidate_flag);
	}
}

bool FlagsForRoads::get_winner(uint32_t* winner_hash) {
	// If AI can ask for 2nd position, but there is only one viable candidate
	// we return the first one of course
	bool has_winner = false;
	for (auto candidate_flag : queue) {
		if (candidate_flag.reduction_score < min_reduction || !candidate_flag.new_road_possible) {
			continue;
		}
		assert(candidate_flag.air_distance > 0);
		assert(candidate_flag.reduction_score >= min_reduction);
		assert(candidate_flag.new_road_possible);
		*winner_hash = candidate_flag.coords_hash;
		has_winner = true;

		if (std::rand() % 3 > 0) {
			// with probability of 2/3 we accept this flag
			return true;
		}
	}

	if (has_winner) {
		return true;
	}
	return false;
}

PlayersStrengths::PlayersStrengths() : update_time(0) {
}

// Default constructor
PlayersStrengths::PlayerStat::PlayerStat()
   : team_number(0),
     is_enemy(false),
     players_power(0),
     old_players_power(0),
     old60_players_power(0),
     players_casualities(0) {
}

// Constructor to be used
PlayersStrengths::PlayerStat::PlayerStat(Widelands::TeamNumber tc,
                                         bool e,
                                         uint32_t pp,
                                         uint32_t op,
                                         uint32_t o60p,
                                         uint32_t cs,
                                         uint32_t land,
                                         uint32_t oland,
                                         uint32_t o60l)
   : team_number(tc),
     is_enemy(e),
     players_power(pp),
     old_players_power(op),
     old60_players_power(o60p),
     players_casualities(cs),
     players_land(land),
     old_players_land(oland),
     old60_players_land(o60l) {
	last_time_seen = kNever;
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
		bool enemy = false;
		if (pn == opn) {
			;
		} else if (pltn == 0 || mytn == 0) {
			enemy = true;
		} else if (pltn != mytn) {
			enemy = true;
		}
		this_player_number = pn;
		all_stats.insert(
		   std::make_pair(opn, PlayerStat(pltn, enemy, pp, op, o60p, cs, land, oland, o60l)));
	} else {
		all_stats[opn].players_power = pp;
		all_stats[opn].old_players_power = op;
		all_stats[opn].old60_players_power = o60p;
		all_stats[opn].players_casualities = cs;
		all_stats[opn].players_land = land;
		all_stats[opn].old_players_land = oland;
		all_stats[opn].old60_players_land = oland;
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
bool PlayersStrengths::any_enemy_seen_lately(const uint32_t gametime) {
	for (auto& item : all_stats) {
		if (item.second.is_enemy && player_seen_lately(item.first, gametime)) {
			return true;
		}
	}
	return false;
}

// Returns count of nearby enemies
uint8_t PlayersStrengths::enemies_seen_lately_count(const uint32_t gametime) {
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (item.second.is_enemy && player_seen_lately(item.first, gametime)) {
			count += 1;
		}
	}
	return count;
}

// When we see enemy, we use this to store the time
void PlayersStrengths::set_last_time_seen(const uint32_t seentime, Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) == 0) {
		return;
	}
	all_stats[pn].last_time_seen = seentime;
}

bool PlayersStrengths::get_is_enemy(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) == 0) {
		// Should happen only rarely so we print a warning here
		log("%d: WARNING: player has no statistics yet\n", this_player_number);
		return false;
	}
	return all_stats[pn].is_enemy;
}

// Was the player seen less then 2 minutes ago
bool PlayersStrengths::player_seen_lately(Widelands::PlayerNumber pn, const uint32_t gametime) {
	if (all_stats.count(pn) == 0) {
		// Should happen only rarely so we print a warning here
		log("%d: WARNING: player has no statistics yet\n", this_player_number);
		return false;
	}
	if (all_stats[pn].last_time_seen == kNever) {
		return false;
	}
	if (all_stats[pn].last_time_seen + (2U * 60U * 1000U) > gametime) {
		return true;
	}
	return false;
}

// This is the strength of a player
uint32_t PlayersStrengths::get_player_power(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_power;
	};
	return 0;
}

// This is the land size owned by player
uint32_t PlayersStrengths::get_player_land(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_land;
	};
	return 0;
}

// Calculates the strength of the enemies seen within the last 2 minutes
uint32_t PlayersStrengths::get_visible_enemies_power(const uint32_t gametime) {
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
		log(" %d: Players statistics are still empty\n", pn);
		return 0;
	}
	return all_stats[pn].old_players_land;
}

uint32_t PlayersStrengths::get_old60_player_land(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) == 0) {
		log(" %d: Players statistics are still empty\n", pn);
		return 0;
	}
	return all_stats[pn].old60_players_land;
}

uint32_t PlayersStrengths::get_old_visible_enemies_power(const uint32_t gametime) {
	uint32_t pw = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first) && player_seen_lately(item.first, gametime)) {
			pw += item.second.old_players_power;
		}
	}
	return pw;
}

// This is casualities of player
uint32_t PlayersStrengths::get_player_casualities(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_casualities;
	}
	return 0;
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
	if (pl1 == pl2) {
		return false;
	} else if (all_stats[pl1].team_number > 0 &&
	           all_stats[pl1].team_number == all_stats[pl2].team_number) {
		// team number 0 = no team
		return true;
	} else {
		return false;
	}
}

bool PlayersStrengths::strong_enough(Widelands::PlayerNumber pl) {
	if (all_stats.count(pl) == 0) {
		return false;
	}
	uint32_t my_strength = all_stats[pl].players_power;
	uint32_t strongest_opponent_strength = 0;
	for (auto item : all_stats) {
		if (!players_in_same_team(item.first, pl) && pl != item.first) {
			if (get_modified_player_power(item.first) > strongest_opponent_strength) {
				strongest_opponent_strength = get_modified_player_power(item.first);
			}
		}
	}
	return my_strength > strongest_opponent_strength + 50;
}

// Update_time is used to prevent too frequent updates of statistics
void PlayersStrengths::set_update_time(const uint32_t gametime) {
	update_time = gametime;
}

uint32_t PlayersStrengths::get_update_time() {
	return update_time;
}

ProductionSiteObserver::ProductionSiteObserver()
   : no_resources_since(kNever), upgrade_pending(false), dismantle_pending_since(kNever) {
}

}  // namespace Widelands
