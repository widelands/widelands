/*
 * Copyright (C) 2009-2010 by the Widelands Development Team
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
#include "logic/map.h"
#include "logic/map.h"
#include "logic/player.h"

// couple of constants for calculation of road interconnections
constexpr int kRoadNotFound = -1000;
constexpr int kShortcutWithinSameEconomy = 1000;
constexpr int kRoadToDifferentEconomy = 10000;



namespace Widelands {

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
FindNodeUnowned::FindNodeUnowned(Player* p, Game& g, bool oe)
   : player(p), game(g), only_enemies(oe) {
}

bool FindNodeUnowned::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) &&
	       ((fc.field->get_owned_by() == 0) ||
	        player->is_hostile(*game.get_player(fc.field->get_owned_by()))) &&
	       (!only_enemies || (fc.field->get_owned_by() != 0));
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
FindNodeUnownedMineable::FindNodeUnownedMineable(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeUnownedMineable::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_owned_by() == 0);
}

FindNodeUnownedBuildable::FindNodeUnownedBuildable(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeUnownedBuildable::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & BUILDCAPS_SIZEMASK) && (fc.field->get_owned_by() == 0);
}

// Unowned but walkable fields nearby
FindNodeUnownedWalkable::FindNodeUnownedWalkable(Player* p, Game& g) : player(p), game(g) {
}

bool FindNodeUnownedWalkable::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() == 0);
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

BuildableField::BuildableField(const Widelands::FCoords& fc)
	: coords(fc),
	  field_info_expiration(20000),
	  preferred(false),
	  enemy_nearby(0),
	  enemy_accessible_(false),
	  unowned_land_nearby(0),
	  near_border(false),
	  unowned_mines_spots_nearby(0),
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
	return is_buildable && p.is_building_type_allowed(id);
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
     last_time_attackable(std::numeric_limits<uint32_t>::max()),
     last_tested(0),
     score(0),
     mines_nearby(Widelands::ExtendedBool::kUnset),
     no_attack_counter(0) {
}

// as all mines have 3 levels, AI does not know total count of mines per mined material
// so this observer will be used for this
MineTypesObserver::MineTypesObserver() : in_construction(0), finished(0) {
}

ManagementData::ManagementData() {
		scores[0] = 1;
		scores[1] = 1;
		scores[2] = 1;
		review_count = 0;
		last_mutate_time = 0;
		next_neuron_id = 0;
		performance_change = 0;
		mutation_multiplicator = 1;
}

Neuron::Neuron(int8_t w, uint8_t f, uint16_t i) : 
	weight(w),type(f), id(i) {
	assert(type < neuron_curves.size());
	assert(weight >= -100 && weight <= 100);
	lowest_pos = std::numeric_limits<uint8_t>::max();
	highest_pos = std::numeric_limits<uint8_t>::min();
	recalculate();
}


FNeuron::FNeuron(uint32_t c){
	core = c;
}

bool FNeuron::get_result(const bool bool1, const bool bool2, const bool bool3, const bool bool4, const bool bool5){
	//printf ("returning %lu (pos: %2d, integer representation %8u)\n",
		//core.test(bool1 * 16 + bool2 * 8 + bool3 * 4 + bool4 * 2 + bool5),
		//bool1 * 16 + bool2 * 8 + bool3 * 4 + bool4 * 2 + bool5,
		//core.to_ulong()
		//);
	return core.test(bool1 * 16 + bool2 * 8 + bool3 * 4 + bool4 * 2 + bool5);
}

uint32_t FNeuron::get_int(){
	return core.to_ulong();
}

void FNeuron::flip_bit(const uint8_t pos){
	assert (pos < f_neuron_bit_size);
	core.flip(pos);
}

void Neuron::set_weight(int8_t w) {
	if (w > 100) {
		weight = 100;
	}else if (w < -100) {
		weight = -100;
	} else {
		weight = w;
	}
}


void Neuron::recalculate() {
	//printf (" type: %d\n", type);
	assert (neuron_curves.size() > type);
	for (int8_t i = 0; i <= 20; i+=1) {
		results[i] = weight * neuron_curves[type][i] / 100;
		//printf (" neuron output: %3d\n", results[i]);
	}
}

int8_t Neuron::get_result(const uint8_t pos){
	assert(pos <= 20);
	return results[pos];
}

int8_t Neuron::get_result_safe(int32_t pos, const bool absolute){
	if (pos > highest_pos) {highest_pos = pos;};
	if (pos < lowest_pos) {lowest_pos = pos;};
	if (pos < 0) {
		pos = 0;
	}
	if (pos > 20) {
		pos = 20;
	}
	assert(pos <= 20);
	assert (results[pos] >= -100 && results[pos] <=100);
	if (absolute) {
		return std::abs(results[pos]);
	}	
	return results[pos];
}

void Neuron::set_type(uint8_t new_type) {
	type = new_type;
}

// this randomly sets new values into neurons and AI magic numbers
void ManagementData::mutate(const uint32_t gametime) {

	int16_t probability = -1;

	probability = get_military_number_at(13) / 8 + 15;
	
	probability *= mutation_multiplicator;

	assert(probability > 0);
	
	printf ("    ... mutating , time since last mutation: %6d, probability: 1 / %d (multiplicator :%d)\n",
	(gametime - last_mutate_time) / 1000 / 60,
	probability,
	mutation_multiplicator);
	last_mutate_time = gametime;

	for (uint16_t i = 0; i < magic_numbers_size; i += 1){
	   	if (std::rand() % probability == 0) {
			// poor man's gausian distribution probability
			int16_t new_value = ((-100 + std::rand() % 200) * 3 -100 + std::rand() % 200) / 4;
			assert (new_value >= -100 && new_value <=100);
			set_military_number_at(i,new_value);
			printf ("      Magic number %d: new value: %4d\n", i, new_value);
		} 
	}

	// Modifying pool of neurons	
	for (auto& item : neuron_pool){
		if (std::rand() % probability == 0) {
			item.set_weight(((-100 + std::rand() % 200) * 3 -100 + std::rand() % 200) / 4);
			pd->neuron_weights[item.get_id()] = item.get_weight();
			assert(neuron_curves.size() > 0);
			item.set_type(std::rand() % neuron_curves.size());
			pd->neuron_functs[item.get_id()] = item.get_type();
			printf ("      Neuron %2d: new weight: %4d, new curve: %d\n", item.get_id(), item.get_weight(), item.get_type());
			item.recalculate();
		} 
	}

	// Modifying pool of f-neurons
	uint16_t pos = 0;	
	for (auto& item : f_neuron_pool){
		bool fneuron_changed = false;
		for (uint8_t i = 0; i < f_neuron_bit_size; i += 1) {
			if (std::rand() % probability == 0) {
				item.flip_bit(i);
				fneuron_changed = true;
			}
		}

		if (fneuron_changed) {
			pd->f_neurons[pos] = item.get_int();
			printf ("      F-Neuron %2d: new value: %ul\n", pos, item.get_int());
		}
		pos += 1; 
	}

	test_consistency();
	dump_data();
}


void ManagementData::review(const uint32_t gametime, PlayerNumber pn, const uint16_t mines,
 const uint32_t strength, const uint32_t strength_delta, const uint32_t land, const uint32_t land_delta, const bool is_attacker) {
	assert(!pd->magic_numbers.empty());
	scores[0] = scores[1];
	scores[1] = scores[2];
	scores[2] = land / 3 + static_cast<int32_t>(land_delta) / 6 + static_cast<int32_t>(strength_delta) - 100 + 100 * is_attacker;
	assert (scores[2] > -10000 && scores[2] < 100000);
	
	printf (" %d %s: reviewing AI mngm. data, score: %4d ->%4d ->%4d (land:%3d, delta: %3d, strength: %3d, delta:%3d, mines: %3d, attacker: %s)\n",
	pn, gamestring_with_leading_zeros(gametime), scores[0], scores[1], scores[2],
	land, land_delta, strength, strength_delta, mines, (is_attacker)?"YES":"NO");


	performance_change = (scores[0] != 0) ? scores[2] * 100 / scores[0] : 0;

	//If under 25 seconds we re-initialize
	if (gametime < 25 * 1000){
		printf (" %d - reinitializing DNA\n", pn);
		initialize(pn, true);
		//if (pn == 2) { 
			mutate(gametime);
		//}
		
		dump_data();
	} else {
		printf ("   still using mutate from %d minutes ago (performance: %3d):\n",
		(gametime - last_mutate_time) / 1000 / 60,
		performance_change); 
		dump_data();
	}
	
	review_count += 1;	
}

void ManagementData::initialize( const uint8_t pn, const bool reinitializing) {
	printf (" ... initialize starts %s\n", reinitializing?" * reinitializing *":"");


    //AutoSCore_EForests_   150
    const std::vector<int16_t> AI_initial_military_numbers_A =
      {  83 ,   43 ,  -38 ,  -31 ,   28 ,  -61 ,  -53 ,   40 ,   -9 ,  -10 ,  //AutoContent_01_EForests_
         -9 ,  -94 ,   33 ,   34 ,   40 ,   36 ,  -29 ,  -58 ,   22 ,    7 ,  //AutoContent_02_EForests_
          4 ,   73 ,  -14 ,    0 ,  -53 ,  -25 ,  -62 ,    0 ,  -97 ,  -32 ,  //AutoContent_03_EForests_
         58 ,   58 ,   35 ,   25 ,    6 ,  -61 ,  -75 ,   91 ,   19 ,   10 ,  //AutoContent_04_EForests_
         23 ,   -7 ,  -39 ,  -28 ,  -46 ,   49 ,   43 ,  -29 ,   -5 ,   43 ,  //AutoContent_05_EForests_
          0 ,   10 ,  -42 ,   14 ,  -72 ,   14 ,   38 ,  -63 ,   33 ,  -27 ,  //AutoContent_06_EForests_
         64 ,   63 ,    0 ,   51 ,  -55 ,   50 ,  -22 ,  -25 ,   27 ,    0  //AutoContent_07_EForests_
       }
		;
	
	assert(magic_numbers_size == AI_initial_military_numbers_A.size());
	
	const std::vector<int8_t> input_weights_A =
      {   38 ,  -64 ,   34 ,   45 ,   10 ,   -5 ,   15 ,  -24 ,   42 ,  -81 ,  //AutoContent_08_EForests_
        -15 ,  -75 ,  -58 ,   -8 ,   35 ,   30 ,   54 ,   55 ,   46 ,   60 ,  //AutoContent_09_EForests_
        -55 ,  -48 ,  -41 ,  -48 ,   57 ,  -51 ,   -1 ,   49 ,   27 ,   37 ,  //AutoContent_10_EForests_
         39 ,  -13 ,   -2 ,   33 ,  -82 ,   14 ,    6 ,  -39 ,   86 ,  -37 ,  //AutoContent_11_EForests_
        -40 ,   47 ,  -14 ,   66 ,   72 ,   19 ,  -39 ,   65 ,   59 ,  -50 ,  //AutoContent_12_EForests_
         18 ,   -9 ,    5 ,  -60 ,   64 ,  -48 ,    7 ,   56 ,  -20 ,  -43 ,  //AutoContent_13_EForests_
        -46 ,  -58 ,   78 ,    0 ,   29 ,   36 ,  -69 ,   -9 ,   49 ,  -23 ,  //AutoContent_14_EForests_
        -35 ,   29 ,   35 ,   62 ,  -11 ,    0 ,  -41 ,   53 ,  -74 ,   19  //AutoContent_15_EForests_
	}
			;
	const std::vector<int8_t> input_func_A =
      {    2 ,    0 ,    2 ,    2 ,    1 ,    1 ,    2 ,    0 ,    1 ,    2 ,  //AutoContent_16_EForests_
          1 ,    0 ,    2 ,    0 ,    1 ,    1 ,    2 ,    1 ,    1 ,    1 ,  //AutoContent_17_EForests_
          0 ,    1 ,    1 ,    0 ,    1 ,    1 ,    0 ,    2 ,    0 ,    1 ,  //AutoContent_18_EForests_
          2 ,    0 ,    0 ,    0 ,    1 ,    0 ,    2 ,    1 ,    1 ,    1 ,  //AutoContent_19_EForests_
          0 ,    2 ,    1 ,    0 ,    0 ,    2 ,    0 ,    0 ,    1 ,    1 ,  //AutoContent_20_EForests_
          2 ,    2 ,    0 ,    2 ,    0 ,    1 ,    2 ,    2 ,    1 ,    0 ,  //AutoContent_21_EForests_
          1 ,    2 ,    0 ,    0 ,    2 ,    0 ,    1 ,    1 ,    0 ,    2 ,  //AutoContent_22_EForests_
          2 ,    0 ,    0 ,    0 ,    2 ,    0 ,    1 ,    0 ,    2 ,    2  //AutoContent_23_EForests_
	}
		;
	assert(neuron_pool_size == input_func_A.size());
	assert(neuron_pool_size == input_weights_A.size());

	const std::vector<uint32_t> f_neurons_A =
      {  1927329170 ,  2903144221 ,  3830649595 ,  1609782967 ,  3384226757 ,  2586070652 ,  1988230621 ,  3233361425 ,  2585204590 ,  1647213901 ,  //AutoContent_24_EForests_
        1075462706 ,  518327319 ,  656300088 ,  635354186 ,  2363339227 ,  166498779 ,  2579251968 ,  811611789 ,  4004202195 ,  461130814 ,  //AutoContent_25_EForests_
        444156012 ,  3780854709 ,  4281866505 ,  603130245 ,  1605571141 ,  65919862 ,  3121446808 ,  1824641649 ,  3356724554 ,  1493183879 ,  //AutoContent_26_EForests_
        2320058671 ,  535108562 ,  3041810896 ,  2847341666 ,  1314896363 ,  185651419 ,  2994344224 ,  1946795469 ,  3054327523 ,  173370710  //AutoContent_27_EForests_
	 };
	assert(f_neuron_pool_size == f_neurons_A.size());

		
    //AutoSCore_CratEr___   150
	const std::vector<int16_t> AI_initial_military_numbers_B =
      {  83 ,   43 ,  -38 ,   60 ,   28 ,  -61 ,  -53 ,   40 ,   33 ,  -10 ,  //AutoContent_01_CratEr___
         -9 ,  -94 ,   33 ,   34 ,   40 ,   36 ,  -29 ,  -58 ,   22 ,    7 ,  //AutoContent_02_CratEr___
          4 ,   73 ,  -14 ,    0 ,  -53 ,  -25 ,  -62 ,    0 ,  -97 ,  -32 ,  //AutoContent_03_CratEr___
         58 ,   58 ,   55 ,   25 ,    6 ,  -61 ,  -75 ,   91 ,   19 ,   10 ,  //AutoContent_04_CratEr___
         23 ,   -7 ,  -39 ,  -28 ,  -46 ,   49 ,   43 ,  -29 ,   -5 ,   43 ,  //AutoContent_05_CratEr___
          0 ,   10 ,  -42 ,   14 ,  -31 ,   14 ,   38 ,  -63 ,   33 ,  -27 ,  //AutoContent_06_CratEr___
         64 ,   63 ,    0 ,   51 ,  -56 ,  100 ,  -22 ,  -25 ,   27 ,    0  //AutoContent_07_CratEr___
		}
		;
	assert(magic_numbers_size == AI_initial_military_numbers_B.size());
		
	const std::vector<int8_t> input_weights_B =
      {   38 ,  -64 ,   34 ,   45 ,   10 ,   -5 ,   15 ,  -24 ,   42 ,  -81 ,  //AutoContent_08_CratEr___
        -15 ,  -75 ,  -58 ,   -8 ,   35 ,   30 ,  -49 ,   55 ,   46 ,   60 ,  //AutoContent_09_CratEr___
        -55 ,  -48 ,  -41 ,  -48 ,   57 ,  -51 ,   -1 ,   49 ,   27 ,   37 ,  //AutoContent_10_CratEr___
         39 ,  -13 ,   -2 ,   33 ,  -82 ,   14 ,    6 ,  -39 ,   86 ,  -37 ,  //AutoContent_11_CratEr___
        -40 ,   47 ,  -14 ,   66 ,   72 ,   19 ,  -39 ,   65 ,   59 ,  -50 ,  //AutoContent_12_CratEr___
         18 ,   -9 ,    5 ,  -60 ,   64 ,  -48 ,    7 ,   56 ,  -20 ,  -43 ,  //AutoContent_13_CratEr___
        -46 ,  -58 ,   78 ,    0 ,   29 ,   36 ,  -69 ,   -9 ,    0 ,  -23 ,  //AutoContent_14_CratEr___
        -35 ,   29 ,   35 ,    0 ,  -11 ,    0 ,    0 ,   53 ,  -74 ,   19  //AutoContent_15_CratEr___
}
	      ;
	
	const std::vector<int8_t> input_func_B = 
      {    2 ,    0 ,    2 ,    2 ,    1 ,    1 ,    2 ,    0 ,    1 ,    2 ,  //AutoContent_16_CratEr___
          1 ,    0 ,    2 ,    0 ,    1 ,    1 ,    0 ,    1 ,    1 ,    1 ,  //AutoContent_17_CratEr___
          0 ,    1 ,    1 ,    0 ,    1 ,    1 ,    0 ,    2 ,    0 ,    1 ,  //AutoContent_18_CratEr___
          2 ,    0 ,    0 ,    0 ,    1 ,    0 ,    2 ,    1 ,    1 ,    1 ,  //AutoContent_19_CratEr___
          0 ,    2 ,    1 ,    0 ,    0 ,    2 ,    0 ,    0 ,    1 ,    1 ,  //AutoContent_20_CratEr___
          2 ,    2 ,    0 ,    2 ,    0 ,    1 ,    2 ,    2 ,    1 ,    0 ,  //AutoContent_21_CratEr___
          1 ,    2 ,    0 ,    0 ,    2 ,    0 ,    1 ,    1 ,    0 ,    2 ,  //AutoContent_22_CratEr___
          2 ,    0 ,    0 ,    0 ,    2 ,    0 ,    0 ,    0 ,    2 ,    2  //AutoContent_23_CratEr___
}
		;
		assert(neuron_pool_size == input_func_B.size());
		assert(neuron_pool_size == input_weights_B.size());

      
	const std::vector<uint32_t> f_neurons_B =
      {  1927345554 ,  2903127837 ,  3863155451 ,  1609785015 ,  3384226757 ,  3139767924 ,  1988230620 ,  1084853767 ,  2585220974 ,  1647209805 ,  //AutoContent_24_CratEr___
        1075462706 ,  518380565 ,  656300592 ,  4056874090 ,  3432821215 ,  166498779 ,  2579251968 ,  2959095437 ,  4004202195 ,  461130806 ,  //AutoContent_25_CratEr___
        444160108 ,  3847963573 ,  4281866505 ,  601033093 ,  3753054789 ,  65919862 ,  3121446808 ,  1824641649 ,  3356724554 ,  1493183879 ,  //AutoContent_26_CratEr___
        2320058671 ,  535108562 ,  3075365344 ,  3113679970 ,  3462377579 ,  152080585 ,  2992247072 ,  1946795469 ,  369975265 ,  2320852310  //AutoContent_27_CratEr___
	 };
	assert(f_neuron_pool_size == f_neurons_B.size());


    //AutoSCore_4Mount_   150
	const std::vector<int16_t> AI_initial_military_numbers_C =
      {  83 ,   43 ,  -38 ,  -31 ,   28 ,  -61 ,  -53 ,   40 ,   -9 ,  -10 ,  //AutoContent_01_4Mount_
         -9 ,  -94 ,   33 ,   34 ,   40 ,   36 ,  -29 ,  -58 ,   22 ,    7 ,  //AutoContent_02_4Mount_
          4 ,   73 ,  -14 ,    0 ,  -53 ,  -25 ,  -62 ,    0 ,  -97 ,  -32 ,  //AutoContent_03_4Mount_
         58 ,   58 ,   35 ,   25 ,    6 ,  -61 ,  -75 ,   91 ,   19 ,   10 ,  //AutoContent_04_4Mount_
         23 ,   -7 ,  -39 ,  -28 ,  -46 ,   49 ,   43 ,  -29 ,   -5 ,   43 ,  //AutoContent_05_4Mount_
          0 ,   10 ,  -42 ,   14 ,  -72 ,   14 ,   38 ,  -63 ,   33 ,  -27 ,  //AutoContent_06_4Mount_
         64 ,   63 ,    0 ,   51 ,  -55 ,   50 ,  -22 ,   61 ,   27 ,    0  //AutoContent_07_4Mount_
       }

		;
	
		assert(magic_numbers_size == AI_initial_military_numbers_C.size());
	
	const std::vector<int8_t> input_weights_C =
      {   38 ,  -64 ,   34 ,   45 ,   10 ,   -5 ,   15 ,  -24 ,   42 ,  -81 ,  //AutoContent_08_4Mount_
        -15 ,  -75 ,  -58 ,   -8 ,   35 ,  -29 ,   54 ,   55 ,   46 ,   60 ,  //AutoContent_09_4Mount_
        -55 ,  -48 ,  -41 ,  -48 ,   57 ,  -51 ,   -1 ,   49 ,   27 ,   37 ,  //AutoContent_10_4Mount_
         39 ,  -13 ,   -2 ,   33 ,  -82 ,   14 ,   76 ,  -39 ,   86 ,  -49 ,  //AutoContent_11_4Mount_
        -40 ,   47 ,  -14 ,   66 ,   72 ,   19 ,  -39 ,   65 ,   59 ,  -50 ,  //AutoContent_12_4Mount_
         18 ,   -9 ,    5 ,  -60 ,   64 ,  -48 ,    7 ,   56 ,  -20 ,  -43 ,  //AutoContent_13_4Mount_
        -46 ,  -58 ,   78 ,    0 ,   29 ,   36 ,  -69 ,   -9 ,   49 ,  -23 ,  //AutoContent_14_4Mount_
        -35 ,   29 ,   35 ,   62 ,  -11 ,    0 ,  -68 ,   53 ,  -74 ,   19  //AutoContent_15_4Mount_
       }
			;
	const std::vector<int8_t> input_func_C =
      {    2 ,    0 ,    2 ,    2 ,    1 ,    1 ,    2 ,    0 ,    1 ,    2 ,  //AutoContent_16_4Mount_
          1 ,    0 ,    2 ,    0 ,    1 ,    0 ,    2 ,    1 ,    1 ,    1 ,  //AutoContent_17_4Mount_
          0 ,    1 ,    1 ,    0 ,    1 ,    1 ,    0 ,    2 ,    0 ,    1 ,  //AutoContent_18_4Mount_
          2 ,    0 ,    0 ,    0 ,    1 ,    0 ,    1 ,    1 ,    1 ,    1 ,  //AutoContent_19_4Mount_
          0 ,    2 ,    1 ,    0 ,    0 ,    2 ,    0 ,    0 ,    1 ,    1 ,  //AutoContent_20_4Mount_
          2 ,    2 ,    0 ,    2 ,    0 ,    1 ,    2 ,    2 ,    1 ,    0 ,  //AutoContent_21_4Mount_
          1 ,    2 ,    0 ,    0 ,    2 ,    0 ,    1 ,    1 ,    0 ,    2 ,  //AutoContent_22_4Mount_
          2 ,    0 ,    0 ,    0 ,    2 ,    0 ,    2 ,    0 ,    2 ,    2  //AutoContent_23_4Mount_
       }
			;
	assert(neuron_pool_size == input_func_C.size());
	assert(neuron_pool_size == input_weights_C.size());
	
	const std::vector<uint32_t> f_neurons_C =
      {  1928377746 ,  2903127837 ,  3830649595 ,  1609782967 ,  3384226757 ,  2586070652 ,  1988230621 ,  3233361425 ,  3122075502 ,  1647213901 ,  //AutoContent_24_4Mount_
        1075463730 ,  518327319 ,  656300088 ,  903789642 ,  2363339227 ,  148156819 ,  2579251968 ,  811611789 ,  4004202195 ,  461130814 ,  //AutoContent_25_4Mount_
        444156012 ,  3780854709 ,  4281866505 ,  601033093 ,  1605571141 ,  65917814 ,  3121446872 ,  1824641649 ,  3356724554 ,  1493183879 ,  //AutoContent_26_4Mount_
        2320058671 ,  535108562 ,  3041810880 ,  2847341666 ,  1314894315 ,  185651419 ,  2994344224 ,  4093754829 ,  3054327523 ,  173370710  //AutoContent_27_4Mount_
	 };
	assert(f_neuron_pool_size == f_neurons_C.size());

		
    //AutoSCore_CraBso___   150
	const std::vector<int16_t> AI_initial_military_numbers_D =
      {  83 ,   43 ,  -38 ,  -31 ,   28 ,  -61 ,  -53 ,   40 ,   -9 ,  -10 ,  //AutoContent_01_CraBso___
         -9 ,  -94 ,   33 ,   34 ,   40 ,   36 ,  -29 ,  -58 ,   22 ,    7 ,  //AutoContent_02_CraBso___
          4 ,   73 ,  -14 ,    0 ,  -53 ,  -25 ,  -62 ,    0 ,  -97 ,   57 ,  //AutoContent_03_CraBso___
         58 ,   58 ,   35 ,   25 ,    6 ,  -61 ,  -75 ,   91 ,   19 ,   10 ,  //AutoContent_04_CraBso___
         23 ,   -7 ,  -39 ,  -28 ,  -46 ,   49 ,   43 ,  -29 ,   -5 ,   43 ,  //AutoContent_05_CraBso___
          0 ,   10 ,  -42 ,   14 ,  -72 ,   14 ,   38 ,  -63 ,   33 ,  -27 ,  //AutoContent_06_CraBso___
         64 ,   63 ,    0 ,   51 ,  -55 ,   50 ,  -22 ,  -25 ,   27 ,    0  //AutoContent_07_CraBso___
	}
		;
	assert(magic_numbers_size == AI_initial_military_numbers_D.size());
		
	const std::vector<int8_t> input_weights_D =
      {   38 ,  -64 ,   34 ,   45 ,   10 ,   -5 ,   15 ,  -24 ,   42 ,  -81 ,  //AutoContent_08_CraBso___
        -15 ,  -75 ,  -58 ,   -8 ,   35 ,   30 ,   54 ,   55 ,   46 ,   60 ,  //AutoContent_09_CraBso___
        -55 ,  -48 ,  -41 ,  -48 ,   57 ,  -51 ,   -1 ,   49 ,   27 ,   37 ,  //AutoContent_10_CraBso___
         39 ,  -13 ,   -2 ,   33 ,  -82 ,   14 ,   76 ,  -39 ,   86 ,  -37 ,  //AutoContent_11_CraBso___
        -40 ,   47 ,   33 ,   66 ,   72 ,   19 ,  -39 ,   65 ,   59 ,  -50 ,  //AutoContent_12_CraBso___
         18 ,   -9 ,    5 ,  -60 ,   64 ,  -48 ,    7 ,   56 ,  -20 ,  -43 ,  //AutoContent_13_CraBso___
        -46 ,  -58 ,   78 ,    0 ,   29 ,   36 ,  -69 ,   -9 ,   49 ,  -23 ,  //AutoContent_14_CraBso___
        -35 ,   29 ,   35 ,   62 ,  -11 ,    0 ,  -41 ,   53 ,  -74 ,   19  //AutoContent_15_CraBso___
	}
	      ;
	
	const std::vector<int8_t> input_func_D = 
      {    2 ,    0 ,    2 ,    2 ,    1 ,    1 ,    2 ,    0 ,    1 ,    2 ,  //AutoContent_16_CraBso___
          1 ,    0 ,    2 ,    0 ,    1 ,    1 ,    2 ,    1 ,    1 ,    1 ,  //AutoContent_17_CraBso___
          0 ,    1 ,    1 ,    0 ,    1 ,    1 ,    0 ,    2 ,    0 ,    1 ,  //AutoContent_18_CraBso___
          2 ,    0 ,    0 ,    0 ,    1 ,    0 ,    1 ,    1 ,    1 ,    1 ,  //AutoContent_19_CraBso___
          0 ,    2 ,    0 ,    0 ,    0 ,    2 ,    0 ,    0 ,    1 ,    1 ,  //AutoContent_20_CraBso___
          2 ,    2 ,    0 ,    2 ,    0 ,    1 ,    2 ,    2 ,    1 ,    0 ,  //AutoContent_21_CraBso___
          1 ,    2 ,    0 ,    0 ,    2 ,    0 ,    1 ,    1 ,    0 ,    2 ,  //AutoContent_22_CraBso___
          2 ,    0 ,    0 ,    0 ,    2 ,    0 ,    1 ,    0 ,    2 ,    2  //AutoContent_23_CraBso___
	}
		;
	assert(neuron_pool_size == input_func_D.size());
	assert(neuron_pool_size == input_weights_D.size());

	const std::vector<uint32_t> f_neurons_D =
      {  1928377746 ,  2903127837 ,  3830649595 ,  1609782967 ,  3384226757 ,  2586070652 ,  1988230621 ,  3233361425 ,  2585204590 ,  1647213901 ,  //AutoContent_24_CraBso___
        1075462706 ,  518327319 ,  656300088 ,  903789642 ,  2363339227 ,  148156819 ,  2579251968 ,  811611789 ,  4004202195 ,  461130814 ,  //AutoContent_25_CraBso___
        444156012 ,  3780854709 ,  4281866505 ,  601033093 ,  1605571141 ,  65917814 ,  3121446808 ,  1824641649 ,  3356724554 ,  1493183879 ,  //AutoContent_26_CraBso___
        2320058671 ,  535108562 ,  3041810896 ,  2847341666 ,  1314894315 ,  185651419 ,  2994344224 ,  4093754829 ,  3054327523 ,  173370710  //AutoContent_27_CraBso___
	 };
	assert(f_neuron_pool_size == f_neurons_D.size());


////  Old winners section start ############
    ////for_EForests_   2609
    //const std::vector<int16_t> AI_initial_military_numbers_E =
      //{ 83,  43, -38, -31,  28, -61, -39,  48,  -9, -10,  //_01_EForests_
        //-9, -94,  33,  34,  40,  36, -29, -58,  22,   7,  //_02_EForests_
         //4,  73, -14,   0, -53, -25, -52,   0, -97, -32,  //_03_EForests_
        //58,  58,  35,  25,   6, -61,  42,  91,  19,  10,  //_04_EForests_
        //23,  -7, -39, -28, -46,  49,  43, -29,  -5,  10,  //_05_EForests_
         //0,  10, -42,  60, -72,  14,  38,  32,  89, -27,  //_06_EForests_
        //64,  63,   0,  51, -56,   0, -22, -25,  27,   0 //_07_EForests_
       //}
                //;

        //assert(magic_numbers_size == AI_initial_military_numbers_E.size());

        //const std::vector<int8_t> input_weights_E =
      //{ 17,  64,  34,  77,  10,  -4,  15, -24,  42,  25,  //_08_EForests_
       //-15,  -3, -58,  -2,  -8,  30,   6,  55,  16,  60,  //_09_EForests_
       //-55, -48, -41, -26,  57, -51, -33,  49,  27,  37,  //_10_EForests_
       //-53,  -1,  -2,  33, -82,  14,   6, -39,  86, -37,  //_11_EForests_
       //-40,  47, -21,  66, -80,  32, -39,  65,  59, -50,  //_12_EForests_
       //-15,  26,   5, -60,  64, -48,   7,  56, -20, -43,  //_13_EForests_
       //-46, -58,  78,   0,  29,  36, -69,  -9,   0, -13,  //_14_EForests_
       //-35,  18,  25,   0, -34,   0,   0,  53, -74,   0 //_15_EForests_
        //}
                        //;
        //const std::vector<int8_t> input_func_E =
      //{  2,   0,   2,   1,   1,   2,   2,   0,   1,   1,  //_16_EForests_
         //1,   0,   2,   0,   0,   1,   2,   1,   2,   1,  //_17_EForests_
         //0,   1,   1,   1,   1,   1,   1,   2,   0,   1,  //_18_EForests_
         //2,   2,   0,   0,   1,   0,   2,   1,   1,   1,  //_19_EForests_
         //0,   2,   2,   0,   2,   0,   0,   0,   1,   1,  //_20_EForests_
         //1,   2,   0,   2,   0,   1,   2,   2,   1,   0,  //_21_EForests_
         //1,   2,   0,   0,   2,   0,   1,   1,   0,   0,  //_22_EForests_
         //2,   1,   0,   0,   2,   0,   0,   0,   2,   0 //_23_EForests_
        //}
                //;
        //assert(neuron_pool_size == input_func_E.size());
        //assert(neuron_pool_size == input_weights_E.size());

        //const std::vector<uint32_t> f_neurons_E =
      //{2058400978, 3856581951, 3998946025, 1071859335, 1136006093, 2317437552, 4005952980, 1185491537, 3675461455, 3778702677,  //_24_EForests_
       //1880769078, 518360085, 3013499064, 2447293512, 3452547547, 464031883, 2612152194, 2422224777, 2928625239, 451823670,  //_25_EForests_
       //2285460076, 3847964604, 4265218825, 601056901, 1538470853, 845496948, 3390013337, 1826171509, 3356694858, 1344298199,  //_26_EForests_
       //3927107845, 518334807, 3041794544, 3516071026, 3403594187, 195088585, 3124238592, 2754133389, 2995738339, 522743126 //_27_EForests_
         //};
        //assert(f_neuron_pool_size == f_neurons_E.size());
    ////for_CratEr___   1262
        //const std::vector<int16_t> AI_initial_military_numbers_F =
      //{ 83,  43, -38, -31,  28, -61, -39,  48,  -9, -10,  //_01_CratEr___
        //-9, -94,  33,  34,  40,  36, -29, -58,  22,   7,  //_02_CratEr___
         //4,  73, -14,   0, -53,  52, -52,   0, -97, -32,  //_03_CratEr___
        //58,  58,  35,  25,   6, -61, -75,  91,  19,  10,  //_04_CratEr___
        //23,  -7, -39, -28, -46,  49,  43, -29,  -5,  10,  //_05_CratEr___
         //0,  10, -42,  60, -72,  14,  38,  32,  89, -27,  //_06_CratEr___
       //-29,  63,   0,  51, -56,   0, -22, -25,  27,   0 //_07_CratEr___
                //}
                //;
        //assert(magic_numbers_size == AI_initial_military_numbers_F.size());

        //const std::vector<int8_t> input_weights_F =
      //{ 17,  64,  34,  77,  10,  -5,  15, -24,  42,  25,  //_08_CratEr___
       //-15,  -3, -58,  -2,  35,  30,   6,  55,  16,  60,  //_09_CratEr___
       //-55, -48, -41, -26,  57,  48, -33,  49,  27,  37,  //_10_CratEr___
       //-53,  -1,  -2,  33, -82,  14,   6, -39,  86, -37,  //_11_CratEr___
         //0,  47, -21,  66, -80,  32, -39,  65,  59, -50,  //_12_CratEr___
       //-15,  26,   5, -60,  64, -48,   7,  56, -20, -43,  //_13_CratEr___
       //-46,   0,  78,   0,  29,  36, -69,  -9,   0, -23,  //_14_CratEr___
       //-35,  18, -50,   0,  80,   0,   0,  53, -74,   0 //_15_CratEr___
//}
              //;

        //const std::vector<int8_t> input_func_F =
      //{  2,   0,   2,   1,   1,   1,   2,   0,   1,   1,  //_16_CratEr___
         //1,   0,   2,   0,   1,   1,   2,   1,   2,   1,  //_17_CratEr___
         //0,   1,   1,   1,   1,   0,   1,   2,   0,   1,  //_18_CratEr___
         //2,   2,   0,   0,   1,   0,   2,   1,   1,   1,  //_19_CratEr___
         //0,   2,   2,   0,   2,   0,   0,   0,   1,   1,  //_20_CratEr___
         //1,   2,   0,   2,   0,   1,   2,   2,   1,   0,  //_21_CratEr___
         //1,   0,   0,   0,   2,   0,   1,   1,   0,   2,  //_22_CratEr___
         //2,   1,   0,   0,   1,   0,   0,   0,   2,   0 //_23_CratEr___
//}
                //;
                //assert(neuron_pool_size == input_func_F.size());
                //assert(neuron_pool_size == input_weights_F.size());


        //const std::vector<uint32_t> f_neurons_F =
      //{982529490, 3847112543, 3998880545, 1072909959, 3283555269, 3123006068, 4001889684, 1157180017, 3658684234, 4054346837,  //_24_CratEr___
       //1679442487, 519408901, 3013499064, 2447293640, 3436818907, 464556171, 2679653258, 2422224777, 2928625239, 451823670,  //_25_CratEr___
       //2285460076, 3847964213, 3961199113, 601050757, 1538470853, 598032500, 3389751194, 1826171509, 3356695882, 2015391447,  //_26_CratEr___
       //1242712325, 518334807, 3072301242, 3516333170, 3470703051, 262509728, 3128430848, 2686467469, 3062849251, 443043158 //_27_CratEr___
         //};
    ////for_4Mount_   1894
        //const std::vector<int16_t> AI_initial_military_numbers_G =
      //{ 83,  43, -38, -31,  28, -61, -39,  48,  -9, -10,  //_01_4Mount_
        //-9, -94,  33,  34,  40,  36, -29, -58,  22,   7,  //_02_4Mount_
         //4,  73, -43,   0, -53, -25, -52,   0, -97, -32,  //_03_4Mount_
        //58,  58,  35,  25,   6, -61, -75,  91,  19,  10,  //_04_4Mount_
        //23,  -7, -39, -28, -46,  49,  43, -29,  -5,  10,  //_05_4Mount_
         //0,  10, -42,  60, -72,  14,  38,  32,  89, -27,  //_06_4Mount_
       //-29,  63,   4,  51, -56,   0, -22, -25,  27,   0 //_07_4Mount_
       //}

                //;

                //assert(magic_numbers_size == AI_initial_military_numbers_G.size());

        //const std::vector<int8_t> input_weights_G =
      //{ 17,  64,  34,  77, -53,  -5,  15, -24,  42,  25,  //_08_4Mount_
       //-15,  -3, -58,  -2,  -8,  30,   6,  55,  16,  60,  //_09_4Mount_
       //-55, -48, -41, -26,  30, -51, -33,  49,  27,  37,  //_10_4Mount_
        //39,  -1,  -2,  33, -82,  14,   6, -39,  86, -37,  //_11_4Mount_
       //-40,  47, -21,  66, -80,  19, -39,  65,  59, -50,  //_12_4Mount_
       //-15,  26,   5, -60,  64, -48, -78,  56, -20, -43,  //_13_4Mount_
       //-46,   0,  78,   0,  29,  36, -69,  -9,   0, -23,  //_14_4Mount_
       //-35,  18, -50,   0, -34,   0,   0,  53, -74,   0 //_15_4Mount_
       //}
                        //;
        //const std::vector<int8_t> input_func_G =
      //{  2,   0,   2,   1,   0,   1,   2,   0,   1,   1,  //_16_4Mount_
         //1,   0,   2,   0,   0,   1,   2,   1,   2,   1,  //_17_4Mount_
         //0,   1,   1,   1,   0,   1,   1,   2,   0,   1,  //_18_4Mount_
         //2,   2,   0,   0,   1,   0,   2,   1,   1,   1,  //_19_4Mount_
         //0,   2,   2,   0,   2,   2,   0,   0,   1,   1,  //_20_4Mount_
         //1,   2,   0,   2,   0,   1,   1,   2,   1,   0,  //_21_4Mount_
         //1,   0,   0,   0,   2,   0,   1,   1,   0,   2,  //_22_4Mount_
         //2,   1,   0,   0,   2,   0,   0,   0,   2,   0 //_23_4Mount_
       //}
                        //;
        //assert(neuron_pool_size == input_func_G.size());
        //assert(neuron_pool_size == input_weights_G.size());

        //const std::vector<uint32_t> f_neurons_G =
      //{2058400978, 2773116767, 3998880617, 1609782935, 3283555269, 2584046196, 1854275028, 1152985717, 3658700622, 3785903317,  //_24_4Mount_
       //1612333622, 451251221, 3072253116, 2447293512, 3453596123, 467701899, 2612152258, 3225971400, 2928035415, 449726518,  //_25_4Mount_
       //3359201388, 3847964605, 3994686217, 601056389, 1538462149, 572867188, 3255795608, 1764846633, 3356694858, 1478503639,  //_26_4Mount_
       //2014988557, 518372183, 3042843056, 1368587378, 3403594059, 196137161, 3123714816, 2687024525, 3054327521, 524840278 //_27_4Mount_
         //};
    ////for_CraBso___   806
        //const std::vector<int16_t> AI_initial_military_numbers_H =
      //{ 83,  43, -38, -31,  28, -61, -39,  48,  -9, -10,  //_01_CraBso___
        //-9, -94,  33,  34,  40,  36, -29, -58,  22,   7,  //_02_CraBso___
         //4,  73, -43,   0, -53,  52, -52,   0, -97, -32,  //_03_CraBso___
        //58,  58,  35,  25,   6, -61, -75,  91, -66,  10,  //_04_CraBso___
        //23,  -7, -39, -28, -46,  49,  43, -29,  -5,  10,  //_05_CraBso___
         //0,  10, -42,  60,  20,  14,  38,  32,  89, -27,  //_06_CraBso___
        //64,  63,   4,  51, -56,   0, -22, -25,  27,   0 //_07_CraBso___
        //}
                //;
        //assert(magic_numbers_size == AI_initial_military_numbers_H.size());

        //const std::vector<int8_t> input_weights_H =
      //{ 17,  64,  34,  77,  10,  -5,  15, -24,  42,  25,  //_08_CraBso___
       //-15,  -3,  61,  -2, -90,  30,   6,  55,  16,  60,  //_09_CraBso___
       //-55, -48, -41, -26,  57,  48, -33,  49,  27, -10,  //_10_CraBso___
       //-26,  -1,  -2,  33, -82,  14,   6, -39,  86, -37,  //_11_CraBso___
       //-40,  47, -21,  66, -80,  19, -39,  65,  59, -50,  //_12_CraBso___
       //-15,  26, -20, -60,  64, -48,   7,  56, -20, -43,  //_13_CraBso___
       //-46,   0, -39,   0,  29,  36, -69,  -9,   0, -13,  //_14_CraBso___
       //-35,  18, -50,   0, -34,   0,   0,  53, -74,   0 //_15_CraBso___
        //}
              //;

        //const std::vector<int8_t> input_func_H =
      //{  2,   0,   2,   1,   1,   1,   2,   0,   1,   1,  //_16_CraBso___
         //1,   0,   2,   0,   1,   1,   2,   1,   2,   1,  //_17_CraBso___
         //0,   1,   1,   1,   1,   0,   1,   2,   0,   1,  //_18_CraBso___
         //1,   2,   0,   0,   1,   0,   2,   1,   1,   1,  //_19_CraBso___
         //0,   2,   2,   0,   2,   2,   0,   0,   1,   1,  //_20_CraBso___
         //1,   2,   2,   2,   0,   1,   2,   2,   1,   0,  //_21_CraBso___
         //1,   0,   1,   0,   2,   0,   1,   1,   0,   0,  //_22_CraBso___
         //2,   1,   0,   0,   2,   0,   0,   0,   2,   0 //_23_CraBso___
        //}
                //;
        //assert(neuron_pool_size == input_func_H.size());
        //assert(neuron_pool_size == input_weights_H.size());

        //const std::vector<uint32_t> f_neurons_H =
      //{1592866519, 3848226111, 4267317037, 1056130695, 1136006094, 2938456676, 4005953492, 1185491537, 3390249803, 3778702677,  //_24_CraBso___
       //3658737924, 518360453, 3072253116, 2447293512, 3453598170, 463507595, 2477935586, 2418030473, 2928035415, 317606678,  //_25_CraBso___
       //2285460076, 3849143857, 3994685961, 601050757, 1538470853, 598016630, 3456860120, 1769023545, 3356694859, 1882221783,  //_26_CraBso___
       //3927107845, 1592076663, 3041794544, 3516071026, 4199636321, 1067816096, 3124239112, 2686459277, 3029293027, 522743126 //_27_CraBso___
         //};
        //assert(f_neuron_pool_size == f_neurons_H.size());
////  Old winners section end ############



    //Static AI   
	const std::vector<int16_t> AI_initial_military_numbers_static =
      {-65,  43, -70,  -1, -71,  12,  24,  48,  26, -39,  // from EForests_
        12,  24, -43,  34, -100, -69,  28,  36, -68,   0,  // from EForests_
                0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  // from EForests_
       52,   0,  19, -34, -16, -71,  46, -68, -76, -38,  // from EForests_
       -22,   0, -17, -62, -53,  35, -43, -29, -22,  75, //A from EForests_
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0
		}
		;
	assert(magic_numbers_size == AI_initial_military_numbers_static.size());
		
	const std::vector<int8_t> input_weights_static =
      {-15,  64, -45, -47,  48,  -5,   1,  10,   8, -24,  // from LRing
       -2, -29,   0,  71,  49, -68,  -30,  16,  67, -68,  // from LRing
         7,  -8,  58,  35, -11,  92,  -87,  -4,  38,  37,  // from LRing
        12,  -2,  52,  89,  40,  35,  -2, -27, -17,  45,  // from LRing
         0,  47, 83, -39,  -4,  -6,  3,  35, -16,  50,   // from LRing
       -75, -12,  27,  49, -42, -38, -31,   5, -20, -43,  // from LRing
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0
}
	      ;
	
	const std::vector<int8_t> input_func_static = 
      {  1,   1,   1,   1,   0,   1,   1,   1,   1,   0,  // from 4Mount_
                 0,  0,  0,  0, 0,  0,  0,  0,  0,  0,  // from 4Mount_
         2,   0,   0,   2,   1,   1,   1,   2,   1,   1,  //A from 4Mount_
         1,   1,   1,   1,   0,   1,   1,   1,   2,   1,  // from 4Mount_
         0,   2,   0,   0,   1,   1,   1,   1,   1,   0,  // from 4Mount_
         0,   2,   2,   1,   1,   1,   2,   2,   1,   0, // from 4Mount_
		0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,0

}
		;
	assert(neuron_pool_size == input_func_static.size());
	assert(neuron_pool_size == input_weights_static.size());


	const std::vector<uint32_t> f_neurons_static =
      {  28,   13,    4,    0,   10,   16,   12,    1,   28,    5,  // from CraBso___
         25,   11,    1,   23,   16,    4,   10,    7,   24,    7, //A from CraBso___
	  28,   13,    4,    0,   10,   16,   12,    1,   28,    5,  // from CraBso___
         25,   11,    1,   23,   16,    4,   10,    7,   24,    7 //A from CraBso___
	 };
	assert(f_neuron_pool_size == f_neurons_static.size());



	for (uint16_t i =  0; i < magic_numbers_size; i = i+1){
		if (std::abs(AI_initial_military_numbers_A[i]) < 45 &&
		std::abs(AI_initial_military_numbers_B[i]) < 45 &&
		std::abs(AI_initial_military_numbers_C[i]) < 45 &&
		std::abs(AI_initial_military_numbers_D[i]) < 45 ) {
			printf ("military number rebalance candidate: %2d: %4d %4d %4d %4d\n",
			i,
			AI_initial_military_numbers_A[i],
			AI_initial_military_numbers_B[i],
			AI_initial_military_numbers_C[i],
			AI_initial_military_numbers_D[i] );
		}
	}
	
	for (uint16_t i =  0; i < neuron_pool_size; i = i+1){
		uint8_t count = 0;
		count  += std::abs(input_weights_A[i]) > 80;
		count  += std::abs(input_weights_B[i]) > 80;
		count  += std::abs(input_weights_C[i]) > 80;
		count  += std::abs(input_weights_D[i]) > 80;	
		if (count >= 2 ) {
			printf ("pool weights rebalance candidate: %2d: %4d %4d %4d %4d\n",
			i,
			input_weights_A[i],
			input_weights_B[i],
			input_weights_C[i],
			input_weights_D[i] );
		}
	}

	printf (" %d: initializing AI's DNA\n", pn);

	// filling vector with zeros
	if (!reinitializing) {
		for (uint16_t i =  0; i < magic_numbers_size; i = i+1){
			pd->magic_numbers.push_back(0);
		}
	}
	assert (pd->magic_numbers.size() == magic_numbers_size);
	
	const uint8_t parent = std::rand() % 4;
	const uint8_t parent2 = std::rand() % 4;
	
	//if (pn == 2) {
		printf (" ... DNA initialization (parent: %d, secondary parent: %d)\n", parent, parent2);
	//}else {
		//printf (" ... DNA initialization\n");
	//}
	
	for (uint16_t i = 0; i < magic_numbers_size; i += 1){
		//if (pn != 2) {
			//set_military_number_at(i,AI_initial_military_numbers_static[i]);
			//continue;
		//}

		// Child inherites DNA with probability 5:1 from main parent
		const uint8_t dna_donor = (std::rand() % 6 > 0) ? parent : parent2;
		
		switch ( dna_donor ) {
			case 0 : 
				set_military_number_at(i,AI_initial_military_numbers_A[i]);
				break;
			case 1 : 
				set_military_number_at(i,AI_initial_military_numbers_B[i]);
				break;
			case 2 : 
				set_military_number_at(i,AI_initial_military_numbers_C[i]);
				break;
			case 3 : 
				set_military_number_at(i,AI_initial_military_numbers_D[i]);
				break;
			//case 4 : 
				//set_military_number_at(i,AI_initial_military_numbers_E[i]);
				//break;
			//case 5 : 
				//set_military_number_at(i,AI_initial_military_numbers_F[i]);
				//break;
			//case 6 : 
				//set_military_number_at(i,AI_initial_military_numbers_G[i]);
				//break;
			//case 7 : 
				//set_military_number_at(i,AI_initial_military_numbers_H[i]);
				//break;
			default:
				printf ("parent %d?\n", dna_donor);
				NEVER_HERE();
			}
		}

	if (reinitializing) {
		neuron_pool.clear();
		reset_neuron_id();
		pd->neuron_weights.clear();
		pd->neuron_functs.clear();
		f_neuron_pool.clear();
		pd->f_neurons.clear();
	}

	//printf (" ... initialize 2, pool size: %lu\n", neuron_pool.size());
	assert(neuron_pool.empty());
	
	for (uint16_t i = 0; i <neuron_pool_size; i += 1){
		//if (pn != 2) {
			//neuron_pool.push_back(Neuron(input_weights_static[i],input_func_static[i],new_neuron_id()));
			//continue;
			//}
		const uint8_t dna_donor = (std::rand() % 6 > 0) ? parent : parent2;
		
		switch ( dna_donor ) {
			case 0 : 
				neuron_pool.push_back(Neuron(input_weights_A[i],input_func_A[i],new_neuron_id()));
				break;
			case 1 : 
				neuron_pool.push_back(Neuron(input_weights_B[i],input_func_B[i],new_neuron_id()));
				break;
			case 2 : 
				neuron_pool.push_back(Neuron(input_weights_C[i],input_func_C[i],new_neuron_id()));
				break;
			case 3 : 
				neuron_pool.push_back(Neuron(input_weights_D[i],input_func_D[i],new_neuron_id()));
				break;
			//case 4 : 
				//neuron_pool.push_back(Neuron(input_weights_E[i],input_func_E[i],new_neuron_id()));
				//break;
			//case 5 : 
				//neuron_pool.push_back(Neuron(input_weights_F[i],input_func_F[i],new_neuron_id()));
				//break;
			//case 6 : 
				//neuron_pool.push_back(Neuron(input_weights_G[i],input_func_G[i],new_neuron_id()));
				//break;
			//case 7 : 
				//neuron_pool.push_back(Neuron(input_weights_H[i],input_func_H[i],new_neuron_id()));
				//break;
			default:
				printf ("parent %d?\n", dna_donor);
				NEVER_HERE();
		}
	}


	for (uint16_t i = 0; i <f_neuron_pool_size; i += 1){
		//if (pn != 2) {
			//f_neuron_pool.push_back(FNeuron(f_neurons_static[i]));
			//continue;
			//}
		const uint8_t dna_donor = (std::rand() % 6 > 0) ? parent : parent2;
		switch ( dna_donor ) {
			case 0 : 
				f_neuron_pool.push_back(FNeuron(f_neurons_A[i]));				
				break;
			case 1 : 
				f_neuron_pool.push_back(FNeuron(f_neurons_B[i]));	
				break;
			case 2 : 
				f_neuron_pool.push_back(FNeuron(f_neurons_C[i]));					
				break;
			case 3 : 
				f_neuron_pool.push_back(FNeuron(f_neurons_D[i]));	
				break;
			//case 4 : 
				//f_neuron_pool.push_back(FNeuron(f_neurons_E[i]));				
				//break;
			//case 5 : 
				//f_neuron_pool.push_back(FNeuron(f_neurons_F[i]));	
				//break;
			//case 6 : 
				//f_neuron_pool.push_back(FNeuron(f_neurons_G[i]));					
				//break;
			//case 7 : 
				//f_neuron_pool.push_back(FNeuron(f_neurons_H[i]));	
				//break;
			default:
				printf ("parent %d?\n", dna_donor);
				NEVER_HERE();
		}
	}
	
	//printf (" ... initialize 2.5, pool size: %lu\n", neuron_pool.size());
	assert(pd->neuron_weights.empty());
	assert(pd->neuron_functs.empty());	
	assert(pd->f_neurons.empty());
		
	for (uint32_t i = 0; i < neuron_pool_size; i = i+1){
		pd->neuron_weights.push_back(neuron_pool[i].get_weight());
		pd->neuron_functs.push_back(neuron_pool[i].get_type());	
	}

	for (uint32_t i = 0; i < f_neuron_pool_size; i = i+1){
		pd->f_neurons.push_back(f_neuron_pool[i].get_int());
	}

	//printf (" ... initialize 3\n");
	
	pd->magic_numbers_size = magic_numbers_size;
	pd->neuron_pool_size = neuron_pool_size;
	pd->f_neuron_pool_size = f_neuron_pool_size;	
	
	test_consistency();
	printf (" %d: DNA initialized\n", pn);
			
}

bool ManagementData::test_consistency() {

	assert (pd->neuron_weights.size() == pd->neuron_pool_size);
	assert (pd->neuron_functs.size() == pd->neuron_pool_size);
	assert (neuron_pool.size() == pd->neuron_pool_size);
	assert (neuron_pool.size() == neuron_pool_size);
	
	assert (pd->magic_numbers_size == magic_numbers_size);			
	assert (pd->magic_numbers.size() == magic_numbers_size);
	
	assert (pd->f_neurons.size() == pd->f_neuron_pool_size);
	assert (f_neuron_pool.size() == pd->f_neuron_pool_size);
	assert (f_neuron_pool.size() == f_neuron_pool_size);	
	return true; //?
}

void ManagementData::dump_data() {
		//dumping new numbers
	printf ("     actual military_numbers (%lu):\n      {", pd->magic_numbers.size());
	uint16_t itemcounter = 1;
	uint16_t line_counter = 1;
	for (const auto& item : pd->magic_numbers) {
		printf (" %3d %s",item,(&item != &pd->magic_numbers.back())?", ":"");
		if (itemcounter % 10 == 0) {
			printf (" //AutoContent_%02d\n       ", line_counter);
			line_counter +=1;
		}
		++itemcounter;
	}
	printf ("}\n");
	
	printf ("     actual neuron setup:\n      ");
	printf ("{ ");
	itemcounter = 1;
	for (auto& item : neuron_pool) {
		printf (" %3d %s",item.get_weight(),(&item != &neuron_pool.back())?", ":"");
		if (itemcounter % 10 == 0) {
			printf (" //AutoContent_%02d\n       ", line_counter);
			line_counter +=1;
		}
		++itemcounter;
	}
	printf ("}\n      { ");
	itemcounter = 1;	
	for (auto& item : neuron_pool) {
		printf (" %3d %s",item.get_type(),(&item != &neuron_pool.back())?", ":"");
		if (itemcounter % 10 == 0) {
			printf (" //AutoContent_%02d\n       ", line_counter);
			line_counter +=1;
		}
		++itemcounter;
	}
	printf ("}\n");


	printf ("     actual f-neuron setup:\n      ");
	printf ("{ ");
	itemcounter = 1;
	for (auto& item : f_neuron_pool) {
		printf (" %8u %s",item.get_int(),(&item != &f_neuron_pool.back())?", ":"");
		if (itemcounter % 10 == 0) {
			printf (" //AutoContent_%02d\n       ", line_counter);
			line_counter +=1;
		}
		++itemcounter;
	}
	printf ("}\n");
}

int16_t ManagementData::get_military_number_at(uint8_t pos) {
	assert (pos < magic_numbers_size);
	return pd->magic_numbers[pos];
}

void ManagementData::set_military_number_at(const uint8_t pos, const int16_t value) {
	assert (pos < magic_numbers_size);
	assert (pos < pd->magic_numbers.size());
	assert (value >= -100 && value <= 100);
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

bool SchedulerTask::operator<(SchedulerTask other) const {
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

bool FlagsForRoads::get_winner(uint32_t* winner_hash, uint32_t pos) {
	assert(pos == 1 || pos == 2);
	uint32_t counter = 1;
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

		if (counter == pos) {
			return true;
		} else if (counter < pos) {
			counter += 1;
		} else {
			break;
		}
	}
	if (has_winner) {
		return true;
	}
	return false;
}

// This is an struct that stores strength of players, info on teams and provides some outputs from
// these data
PlayersStrengths::PlayerStat::PlayerStat() : team_number(0), is_enemy(false), players_power(0), old_players_power(0), old60_players_power(0),players_casualities(0) {
}
PlayersStrengths::PlayerStat::PlayerStat(Widelands::TeamNumber tc, bool e, uint32_t pp, uint32_t op, uint32_t o60p, uint32_t cs, uint32_t land, uint32_t oland)
   : team_number(tc), is_enemy(e), players_power(pp),  old_players_power(op), old60_players_power(o60p), players_casualities(cs), players_land(land), old_players_land(oland)  {
	 last_time_seen = kNever;  
}

// Inserting/updating data
void PlayersStrengths::add(Widelands::PlayerNumber pn, Widelands::PlayerNumber opn, Widelands::TeamNumber mytn,
   Widelands::TeamNumber pltn, uint32_t pp, uint32_t op, uint32_t o60p, uint32_t cs, uint32_t land, uint32_t oland) {
	if (all_stats.count(opn) == 0) {
		bool enemy = false;
		if ( pn == opn ) {
			;
		} else if (pltn == 0 || mytn == 0) {
			enemy = true;
		} else if (pltn != mytn) {
			enemy = true;			
		}
		this_player_number = pn;
		printf (" %d PlayersStrengths: player %d / team: %d - is%s enemy\n", pn, opn, pltn, (enemy)?"":" not");
		all_stats.insert(std::pair<Widelands::PlayerNumber, PlayerStat>(opn, PlayerStat(pltn, enemy, pp, op, o60p, cs, land, oland)));
	} else {
		all_stats[opn].players_power = pp;
		all_stats[opn].old_players_power = op;
		all_stats[opn].old60_players_power = o60p;
		all_stats[opn].players_casualities = cs;
		all_stats[opn].players_land = land;
		all_stats[opn].old_players_land = oland;
	}
}

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

bool PlayersStrengths::any_enemy_seen_lately(const uint32_t gametime){
	for (auto& item : all_stats) {
		if (item.second.is_enemy && player_seen_lately(item.first, gametime)) {
			return true;
		}  // is a member of a team
	}
	return false;
}

uint8_t PlayersStrengths::enemies_seen_lately_count(const uint32_t gametime){
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (item.second.is_enemy && player_seen_lately(item.first, gametime)) {
			count +=1;
		}  // is a member of a team
	}
	return count;
}

uint32_t PlayersStrengths::enemy_last_seen(){
	uint32_t time = 0;
	for (auto& item : all_stats) {
		if (item.second.last_time_seen == kNever){
			continue;
		}
		if (item.second.is_enemy && item.second.last_time_seen > time) {
			time = item.second.last_time_seen;
		}  
	}
	if (time == 0) {return kNever;}
	return time;
}



void PlayersStrengths::set_last_time_seen(const uint32_t seentime, Widelands::PlayerNumber pn){
	assert(all_stats.count(pn) > 0);
	all_stats[pn].last_time_seen = seentime;
}

bool PlayersStrengths::get_is_enemy(Widelands::PlayerNumber pn){
	assert(all_stats.count(pn) > 0);
	return all_stats[pn].is_enemy;

}

bool PlayersStrengths::player_seen_lately( Widelands::PlayerNumber pn, const uint32_t gametime){
	assert(all_stats.count(pn) > 0);
	if (all_stats[pn].last_time_seen == kNever){
		return false;
	}
	if (all_stats[pn].last_time_seen + static_cast<uint32_t>(2 * 60 * 1000) > gametime){
		return true;
	}
	return false;
}

// This is strength of player
uint32_t PlayersStrengths::get_player_power(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_power;
	};
	return 0;
}

// This is land size owned by player
uint32_t PlayersStrengths::get_player_land(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].players_land;
	};
	return 0;
}

uint32_t PlayersStrengths::get_visible_enemies_power(const uint32_t gametime){
	uint32_t pw = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first) && player_seen_lately(item.first, gametime)) {
			pw += item.second.players_power;
		}
	}
	return pw;
}

uint32_t PlayersStrengths::get_enemies_average_power(){
	uint32_t sum = 0;
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			sum += item.second.players_power;
			count += 1;
		}
	}
	if (count > 0) {
		return sum/count;
	}
	return 0;
}

uint32_t PlayersStrengths::get_enemies_average_land(){
	uint32_t sum = 0;
	uint8_t count = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			sum += item.second.players_land;
			count += 1;
		}
	}
	if (count > 0) {
		return sum/count;
	}
	return 0;
}

uint32_t PlayersStrengths::get_enemies_max_power(){
	uint32_t power = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			power=std::max<uint32_t>(power, item.second.players_power);
		}
	}
	return power;
}

uint32_t PlayersStrengths::get_enemies_max_land(){
	uint32_t land = 0;
	for (auto& item : all_stats) {
		if (get_is_enemy(item.first)) {
			land=std::max<uint32_t>(land, item.second.players_land);
		}
	}
	return land;
}

uint32_t PlayersStrengths::get_old_player_power(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].old_players_power;
	};
	return 0;
}

uint32_t PlayersStrengths::get_old60_player_power(Widelands::PlayerNumber pn) {
	if (all_stats.count(pn) > 0) {
		return all_stats[pn].old60_players_power;
	};
	return 0;
}

uint32_t PlayersStrengths::get_old_player_land(Widelands::PlayerNumber pn) {
	assert(all_stats.count(pn) > 0);
	return all_stats[pn].old_players_land;
}


uint32_t PlayersStrengths::get_old_visible_enemies_power(const uint32_t gametime){
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
	};
	return 0;
}

// This is strength of player plus third of strength of other members of his team
uint32_t PlayersStrengths::get_modified_player_power(Widelands::PlayerNumber pn) {
	uint32_t result = 0;
	Widelands::TeamNumber team = 0;
	if (all_stats.count(pn) > 0) {
		result = all_stats[pn].players_power;
		team = all_stats[pn].team_number;
	};
	if (team > 0 && team_powers.count(team) > 0) {
		result = result + (team_powers[team] - result) / 3;
	};
	return result;
}

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

}  // namespace WIdelands
