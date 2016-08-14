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
	: player(pl), movecaps(mc), open_end(oe) {}

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

bool CheckStepRoadAI::reachable_dest(Map& map, FCoords const dest) const {
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
FindNodeEnemy::FindNodeEnemy(Player* p, Game& g) : player(p), game(g) {}

bool FindNodeEnemy::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && fc.field->get_owned_by() != 0 &&
			 player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// We are looking for buildings owned by hostile player
// (sometimes there is a enemy's teritorry without buildings, and
// this confuses the AI)
FindNodeEnemiesBuilding::FindNodeEnemiesBuilding(Player* p, Game& g) : player(p), game(g) {}

bool FindNodeEnemiesBuilding::accept(const Map&, const FCoords& fc) const {
	return (fc.field->get_immovable()) && fc.field->get_owned_by() != 0 &&
			 player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// When looking for unowned terrain to acquire, we are actually
// only interested in fields we can walk on.
// Fields should either be completely unowned or owned by an opposing player
FindNodeUnowned::FindNodeUnowned(Player* p, Game& g, bool oe) : player(p), game(g), only_enemies(oe) {}

bool FindNodeUnowned::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) &&
			 ((fc.field->get_owned_by() == 0) ||
			  player->is_hostile(*game.get_player(fc.field->get_owned_by()))) &&
			 (!only_enemies || (fc.field->get_owned_by() != 0));
}

// Sometimes we need to know how many nodes our allies owns
FindNodeAllyOwned::FindNodeAllyOwned(Player* p, Game& g, PlayerNumber n) :
	player(p), game(g), player_number(n) {}

bool FindNodeAllyOwned::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() != 0) &&
			 (fc.field->get_owned_by() != player_number) &&
			 !player->is_hostile(*game.get_player(fc.field->get_owned_by()));
}

// When looking for unowned terrain to acquire, we must
// pay speciall attention to fields where mines can be built.
// Fields should be completely unowned
FindNodeUnownedMineable::FindNodeUnownedMineable(Player* p, Game& g) : player(p), game(g) {}

bool FindNodeUnownedMineable::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_owned_by() == 0);
}

// Unowned but walkable fields nearby
FindNodeUnownedWalkable::FindNodeUnownedWalkable(Player* p, Game& g) : player(p), game(g) {}

bool FindNodeUnownedWalkable::accept(const Map&, const FCoords& fc) const {
	return (fc.field->nodecaps() & MOVECAPS_WALK) && (fc.field->get_owned_by() == 0);
}

// Looking only for mines-capable fields nearby
// of specific type
FindNodeMineable::FindNodeMineable(Game& g, DescriptionIndex r) : game(g), res(r) {}

bool FindNodeMineable::accept(const Map&, const FCoords& fc) const {

	return (fc.field->nodecaps() & BUILDCAPS_MINE) && (fc.field->get_resources() == res);
}

// Fishers and fishbreeders must be built near water
FindNodeWater::FindNodeWater(const World& world) : world_(world) {}

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

NearFlag::NearFlag(const Flag& f, int32_t const c, int32_t const d) : flag(&f), cost(c), distance(d) {}

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
	  area_military_presence(0),
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
	  no_attack_counter(0) {}


// as all mines have 3 levels, AI does not know total count of mines per mined material
// so this observer will be used for this
MineTypesObserver::MineTypesObserver() : in_construction(0), finished(0) {}


// Is this needed? HERE
ManagementData::ManagementData() {
		old_msites = 0;
		old_psites = 0;
		old_bfields = 0;
		review_count = 0;
		re_scatter_count = 0;
		last_scatter_time = 0;
		next_neuron_id = 0;
	}

Neuron::Neuron(int8_t w, uint8_t f, uint16_t i) : 
	weight(w),type(f), id(i) {
	lowest_pos = std::numeric_limits<uint8_t>::max();
	highest_pos = std::numeric_limits<uint8_t>::min();
	//printf (" New neuron created: %d\n", id);
	recalculate();
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

int8_t Neuron::get_result_safe(int32_t pos){
	if (pos > highest_pos) {highest_pos = pos;};
	if (pos < lowest_pos) {lowest_pos = pos;};
	if (pos < 0) {
		pos = 0;
	}
	if (pos > 20) {
		pos = 20;
	}
	assert(pos <= 20);
	return results[pos];
}


void Neuron::set_type(uint8_t new_type) {
	type = new_type;
}


void ManagementData::scatter(const uint32_t gametime) {

	re_scatter_count +=1;
	printf ("    ... scattering (*%2d), time since last scatter %6d\n",
	re_scatter_count, (gametime - last_scatter_time) / 1000 / 60);
	last_scatter_time = gametime;
	
	int16_t diff = 0;
	

   for (auto & item : military_numbers) {
	   	if (std::rand() % 15 == 0) {
			int32_t boost = -400 + std::rand() % 800;
			if (boost != 0 ){
				//boost *= re_scatter_count;
				printf ("   special numbers boost: %4d\n", boost);
				item = item +  boost;
				
			}
		} else  if (std::rand() % 10 == 0) {
			diff = std::abs(item) / 6;
			if (diff < 2) {
				diff = 2;
			}
			int32_t boost = -diff + std::rand() % (2*diff);
			if (boost != 0 ){
				//boost *= re_scatter_count;
				printf ("   special numbers boost: %4d\n", boost);
				item = item +  boost;
			}
		}
	}

	if (military_numbers[6] < 40) {
		printf ("   Increasing military_numbers[16]...\n");
		military_numbers[6] = 40;
	}	
	if (military_numbers[7] < 70) {
		printf ("   Increasing military_numbers[17]...\n");
		military_numbers[7] = 70;
	}
	if (military_numbers[8] < 100) {
		printf ("   Increasing military_numbers[18]...\n");
		military_numbers[8] = 100;
	}	
	if (military_numbers[6] > 80) {
		printf ("   Decreasing military_numbers[16]...\n");
		military_numbers[6] = 80;
	}	
	if (military_numbers[7] > 120) {
		printf ("   Decreasing military_numbers[17]...\n");
		military_numbers[7] = 120;
	}
	if (military_numbers[8] > 150) {
		printf ("   Decreasing military_numbers[18]...\n");
		military_numbers[8] = 150;
	}
	
	// Modifying pool of neurons	
	for (auto& item : neuron_pool){
		if (std::rand() % 3 == 0) {
			item.set_weight(item.get_weight() -10 + std::rand() % 20);
			item.set_type(std::rand() % neuron_curves.size());
			printf ("     Neuron %d: re-setting type %d\n", item.get_id(), item.get_type());
			item.recalculate();
		}
	}


	//dumping new numbers
	printf ("   new military_numbers (%lu):\n    {", military_numbers.size());
	for (const auto& item : military_numbers) {
		printf ("%3d%s",item,(&item != &military_numbers.back())?", ":"");
	}
	printf ("}\n");

	
	printf ("   new neuron setup:\n    ");
	
	printf ("{");
	for (auto& item : neuron_pool) {
		printf ("%3d%s",item.get_weight(),(&item != &neuron_pool.back())?", ":"");
	}
	printf ("}\n{");	
	for (auto& item : neuron_pool) {
		printf ("%3d%s",item.get_type(),(&item != &neuron_pool.back())?", ":"");
	}
	printf ("}\n");
	
	
}
void ManagementData::review(const uint16_t msites, const uint16_t psites, const uint8_t pn, const uint16_t bfields, const uint32_t gametime) {
	printf (" %d %s: reviewig AI management data (%3d -> %3d, %3d -> %3d, %3d -> %3d )\n",
	pn, gamestring_with_leading_zeros(gametime), old_msites, msites, old_psites, psites, old_bfields, bfields);
	//militarysites are now ignored
	if ( (((psites - old_psites) * 5  + (bfields - old_bfields)) < 20) ||	
	 ((old_bfields + old_psites * 5) * 103 > (bfields + psites * 5) * 100) ) {
		printf ("  !  too WEAK performer\n");
		
		military_numbers = learned_military_numbers;
		scatter(gametime);
	} else {
		printf ("  still using scatter from %d minutes ago:\n",(gametime - last_scatter_time) / 1000 / 60); 
		
		
		if (review_count > 1){
			//learning
			printf ("   updating learning_military_numbers:\n    {");
			for (uint16_t i = 0; i < military_numbers.size(); i += 1){
				int32_t newvalue = (9 * learned_military_numbers[i] + military_numbers[i])/ 10 ;
				printf ("%3d(%3d)%s",newvalue,learned_military_numbers[i], (i + 1 !=  static_cast<int16_t>(military_numbers.size()))?", ":"");
				learned_military_numbers[i] = newvalue;
			}
			printf ("}\n");
		}

	}
	
	//review min-max values of neurons
	for (auto & item : neuron_pool) {
		printf ("  Neuron %2d, min: %3d, max: %3d\n", item.get_id(), item.get_lowest_pos(), item.get_highest_pos());
	}
	
	old_msites = msites;
	old_psites = psites;
	old_bfields = bfields;
	review_count += 1;	
}

void ManagementData::init_learned_data() {
	learned_military_numbers= military_numbers;
}

uint16_t MineTypesObserver::total_count() const {
	return in_construction + finished;
}

// this is used to count militarysites by their size
MilitarySiteSizeObserver::MilitarySiteSizeObserver() : in_construction(0), finished(0) {}

// this represents a scheduler task
SchedulerTask::SchedulerTask
	(const uint32_t time, const Widelands::SchedulerTaskId t, const uint8_t p, const char* d):
	due_time(time), id(t), priority(p), descr(d) {}

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
	for (auto field: blocked_fields_) {
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


FlagsForRoads::Candidate::Candidate(uint32_t coords, int32_t distance, bool economy):
	coords_hash(coords), air_distance(distance), different_economy(economy) {
		new_road_possible = false;
		accessed_via_roads = false;
		// Values are only very rough, and are dependant on the map size
		new_road_length = 2 * Widelands::kMapDimensions.at(Widelands::kMapDimensions.size() - 1);
		current_roads_distance = 2 *(Widelands::kMapDimensions.size() - 1); // must be big enough
		reduction_score = -air_distance; // allows reasonable ordering from the start
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
		reduction_score = kRoadNotFound - air_distance; // to have at least some ordering preserved
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

void FlagsForRoads::print() { // this is for debugging and development purposes
	for (auto& candidate_flag : queue) {
		log("   %starget: %3dx%3d, saving: %5d (%3d), air distance: %3d, new road: %6d, score: %5d %s\n",
		(candidate_flag.reduction_score>=min_reduction && candidate_flag.new_road_possible)?"+":" ",
		Coords::unhash(candidate_flag.coords_hash).x,
		Coords::unhash(candidate_flag.coords_hash).y,
		candidate_flag.current_roads_distance - candidate_flag.new_road_length,
		min_reduction,
		candidate_flag.air_distance,
		candidate_flag.new_road_length,
		candidate_flag.reduction_score,
		(candidate_flag.new_road_possible)? ", new road possible" : " ");
	}
}

// Queue is ordered but some target flags are only estimations so we take such a candidate_flag first
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
		*winner_hash=candidate_flag.coords_hash;
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

// This is an struct that stores strength of players, info on teams and provides some outputs from these data
PlayersStrengths::PlayerStat::PlayerStat() :
	team_number(0),
	players_power(0) {}
PlayersStrengths::PlayerStat::PlayerStat(Widelands::TeamNumber tc, uint32_t pp) :
	team_number(tc), players_power(pp) {}

// Inserting/updating data
void PlayersStrengths::add(Widelands::PlayerNumber pn, Widelands::TeamNumber tn, uint32_t pp) {
	if (all_stats.count(pn) == 0) {
		all_stats.insert(std::pair<Widelands::PlayerNumber, PlayerStat>(pn, PlayerStat(tn, pp)));
	} else {
		all_stats[pn].players_power = pp;
	}
}

void PlayersStrengths::recalculate_team_power() {
	team_powers.clear();
	for (auto& item: all_stats) {
		if (item.second.team_number > 0) { // is a member of a team
			if (team_powers.count(item.second.team_number) > 0) {
				team_powers[item.second.team_number] += item.second.players_power;
			} else {
				team_powers[item.second.team_number] = item.second.players_power;
			}
		}
	}
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

bool PlayersStrengths::players_in_same_team(Widelands::PlayerNumber pl1, Widelands::PlayerNumber pl2) {
	if (all_stats.count(pl1) > 0 && all_stats.count(pl2) > 0 && pl1 != pl2) {
		// team number 0 = no team
		return all_stats[pl1].team_number > 0 && all_stats[pl1].team_number == all_stats[pl2].team_number;
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

} // namespace WIdelands
