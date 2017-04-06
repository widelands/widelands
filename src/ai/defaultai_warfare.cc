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

#include "ai/defaultai.h"
//#include "ai/tmp_constants.h" //Import decreasor should be removed

using namespace Widelands;

bool DefaultAI::check_enemy_sites(uint32_t const gametime) {

	Map& map = game().map();

	// define which players are attackable
	std::vector<Attackable> player_attackable;
	PlayerNumber const nr_players = map.get_nrplayers();
	player_attackable.resize(nr_players);
	uint32_t plr_in_game = 0;
	Widelands::PlayerNumber const pn = player_number();

	iterate_players_existing_novar(p, nr_players, game())++ plr_in_game;

	update_player_stat();

	// defining treshold ratio of own_strength/enemy's strength
	uint32_t treshold_ratio = 100;
	if (type_ == DefaultAI::Type::kNormal) {
		treshold_ratio = 80;
	}
	if (type_ == DefaultAI::Type::kVeryWeak) {
		treshold_ratio = 120;
	}

	// let's say a 'campaign' is a series of attacks,
	// if there is more then 3 minutes without attack after last
	// attack, then a campaign is over.
	// To start new campaign (=attack again), our strenth must exceed
	// target values (calculated above) by some treshold =
	// ai_personality_attack_margin
	// Once a new campaign started we will fight until
	// we get below above treshold or there will be 3
	// minutes gap since last attack
	// note - AI is not aware of duration of attacks
	// everywhere we consider time when an attack is ordered.
	if (last_attack_time_ < gametime - kCampaignDuration) {
		treshold_ratio += persistent_data->ai_personality_attack_margin;
	}

	const uint32_t my_power = player_statistics.get_modified_player_power(pn);

	// now we test all players to identify 'attackable' ones
	for (Widelands::PlayerNumber j = 1; j <= plr_in_game; ++j) {
		// if we are the same team, or it is just me
		if (player_statistics.players_in_same_team(pn, j) || pn == j) {
			player_attackable[j - 1] = Attackable::kNotAttackable;
			continue;
		}

		// now we compare strength
		// strength of the other player (considering his team)
		uint32_t players_power = player_statistics.get_modified_player_power(j);

		if (players_power == 0) {
			player_attackable.at(j - 1) = Attackable::kAttackable;
		} else if (my_power * 100 / players_power > treshold_ratio * 8) {
			player_attackable.at(j - 1) = Attackable::kAttackableVeryWeak;
		} else if (my_power * 100 / players_power > treshold_ratio * 4) {
			player_attackable.at(j - 1) = Attackable::kAttackableAndWeak;
		} else if (my_power * 100 / players_power > treshold_ratio) {
			player_attackable.at(j - 1) = Attackable::kAttackable;
		} else {
			player_attackable.at(j - 1) = Attackable::kNotAttackable;
		}
	}

	// first we scan vicitnity of couple of militarysites to get new enemy sites
	// Militarysites rotate (see check_militarysites())
	int32_t i = 0;
	for (MilitarySiteObserver mso : militarysites) {
		i += 1;
		if (i % 4 == 0)
			continue;
		if (i > 20)
			continue;

		MilitarySite* ms = mso.site;
		uint32_t const vision = ms->descr().vision_range();
		FCoords f = map.get_fcoords(ms->get_position());

		// get list of immovable around this our military site
		std::vector<ImmovableFound> immovables;
		map.find_immovables(Area<FCoords>(f, (vision + 3 < 13) ? 13 : vision + 3), &immovables,
		                    FindImmovableAttackable());

		for (uint32_t j = 0; j < immovables.size(); ++j) {
			if (upcast(MilitarySite const, bld, immovables.at(j).object)) {
				const PlayerNumber opn = bld->owner().player_number();
				if (player_statistics.get_is_enemy(opn)) {
					assert(opn != pn);
					player_statistics.set_last_time_seen(gametime, opn);
					if (enemy_sites.count(bld->get_position().hash()) == 0) {
						enemy_sites[bld->get_position().hash()] = EnemySiteObserver();
					}
				}
			}
			if (upcast(Warehouse const, wh, immovables.at(j).object)) {
				const PlayerNumber opn = wh->owner().player_number();
				if (player_statistics.get_is_enemy(opn)) {
					assert(opn != pn);
					player_statistics.set_last_time_seen(gametime, opn);
					if (enemy_sites.count(wh->get_position().hash()) == 0) {
						enemy_sites[wh->get_position().hash()] = EnemySiteObserver();
					}
				}
			}
		}
	}

	// now we update some of them
	uint32_t best_target = std::numeric_limits<uint32_t>::max();
	uint8_t best_score = 0;
	uint32_t count = 0;
	// sites that were either conquered or destroyed
	std::vector<uint32_t> disappeared_sites;

	// Willingness to attack depend on how long ago the last soldier has been trained. This is used
	// as
	// indicator how busy our trainingsites are.
	// Moreover the stronger AI the more sensitive to it it is (a score of attack willingness is more
	// decreased if promotion of soldiers is stalled)
	int8_t general_score = 0;
	if (persistent_data->last_soldier_trained > gametime) {
		// No soldier was ever trained ...
		switch (type_) {
		case DefaultAI::Type::kNormal:
			general_score = 1;
			break;
		case DefaultAI::Type::kWeak:
			general_score = 0;
			break;
		case DefaultAI::Type::kVeryWeak:
			general_score = -1;
		}
	} 

	const bool strong_enough = player_statistics.strong_enough(pn);

	for (std::map<uint32_t, EnemySiteObserver>::iterator site = enemy_sites.begin();
	     site != enemy_sites.end(); ++site) {

		// we test max 12 sites and prefer ones tested more then 1 min ago
		if (((site->second.last_tested + (enemysites_check_delay_ * 1000)) > gametime && count > 4) ||
		    count > 12) {
			continue;
		}
		count += 1;

		site->second.last_tested = gametime;
		uint8_t defenders_strength = 0;
		bool is_warehouse = false;
		bool is_attackable = false;
		// we cannot attack unvisible site and there is no other way to find out
		const bool is_visible =
		   (1 < player_->vision(Map::get_index(Coords::unhash(site->first), map.get_width())));
		uint16_t owner_number = 100;

		// testing if we can attack the building - result is a flag
		// if we dont get a flag, we remove the building from observers list
		FCoords f = map.get_fcoords(Coords::unhash(site->first));
		uint32_t site_to_be_removed = std::numeric_limits<uint32_t>::max();
		Flag* flag = nullptr;

		if (upcast(MilitarySite, bld, f.field->get_immovable())) {
			if (player_->is_hostile(bld->owner())) {
				std::vector<Soldier*> defenders;
				defenders = bld->present_soldiers();
				defenders_strength = calculate_strength(defenders);

				flag = &bld->base_flag();
				if (is_visible && bld->can_attack()) {
					is_attackable = true;
				}
				owner_number = bld->owner().player_number();
			}
		}
		if (upcast(Warehouse, Wh, f.field->get_immovable())) {
			if (player_->is_hostile(Wh->owner())) {

				std::vector<Soldier*> defenders;
				defenders = Wh->present_soldiers();
				defenders_strength = calculate_strength(defenders);

				flag = &Wh->base_flag();
				is_warehouse = true;
				if (is_visible && Wh->can_attack()) {
					is_attackable = true;
				}
				owner_number = Wh->owner().player_number();
			}
		}

		// if flag is defined it is a good taget
		if (flag) {
			// updating some info
			// updating info on mines nearby if needed
			if (site->second.mines_nearby == ExtendedBool::kUnset) {
				FindNodeMineable find_mines_spots_nearby(game(), f.field->get_resources());
				const int32_t minescount =
				   map.find_fields(Area<FCoords>(f, 6), nullptr, find_mines_spots_nearby);
				if (minescount > 0) {
					site->second.mines_nearby = ExtendedBool::kTrue;
				} else {
					site->second.mines_nearby = ExtendedBool::kFalse;
				}
			}

			site->second.is_warehouse = is_warehouse;

			// getting rid of default
			if (site->second.last_time_attackable == kNever) {
				site->second.last_time_attackable = gametime;
			}

			// can we attack:
			if (is_attackable) {
				std::vector<Soldier*> attackers;
				player_->find_attack_soldiers(*flag, &attackers);
				if (attackers.empty()) {
					site->second.attack_soldiers_strength = 0;
				} else {
					int32_t strength = calculate_strength(attackers);

					site->second.attack_soldiers_strength = strength;
					assert(!attackers.empty());
					site->second.attack_soldiers_competency = strength * 10 / attackers.size();
					// printf ("soldiers :%d, competency: %d\n", attackers.size(),
					// site->second.attack_soldiers_competency);
				}
			} else {
				site->second.attack_soldiers_strength = 0;
			}

			site->second.defenders_strength = defenders_strength;

			if (site->second.attack_soldiers_strength > 0 &&
			    (player_attackable[owner_number - 1] == Attackable::kAttackable ||
			     player_attackable[owner_number - 1] == Attackable::kAttackableAndWeak ||
			     player_attackable[owner_number - 1] == Attackable::kAttackableVeryWeak)) {

				// treating no attack score
				if (site->second.no_attack_counter < 0) { //NOCOM move this higher
					// we cannot attack yet
					site->second.score = 0;
					// but increase the counter by 1
					site->second.no_attack_counter += 1;
				} else {					 
					 
					int16_t inputs[f_neuron_bit_size] = {0};
					inputs[0] = (site->second.attack_soldiers_strength - site->second.defenders_strength) *
						std::abs(management_data.get_military_number_at(114)) / 30;	 
					inputs[1] = (site->second.attack_soldiers_strength - site->second.defenders_strength) *
						std::abs(management_data.get_military_number_at(115)) / 30;	 					 
					inputs[2] = (is_warehouse) ? 4 : 0;	 
					inputs[3] = (is_warehouse) ? 2 : 0;						 
					inputs[4] = (site->second.attack_soldiers_competency > 15)  ? 2 : 0;
					inputs[5] = (site->second.attack_soldiers_competency > 25)  ? 4 : 0	;	;			 
					inputs[6] = (2 * site->second.defenders_strength > 3 * site->second.attack_soldiers_strength) ? 2 : 0;
					inputs[7] = (3 * site->second.defenders_strength > 2 * site->second.attack_soldiers_strength) ? 2 : 0;
					inputs[8] =  (soldier_status_ == SoldiersStatus::kBadShortage || soldier_status_ == SoldiersStatus::kShortage) ? -2 : 0;
					inputs[8] =  (soldier_status_ == SoldiersStatus::kBadShortage) ? -2 : 0;
					inputs[9] =  (soldier_status_ == SoldiersStatus::kBadShortage || soldier_status_ == SoldiersStatus::kShortage) ? -3 : 0;
					inputs[10] = (site->second.mines_nearby == ExtendedBool::kTrue) ? 3 : -1;
					inputs[11] = (site->second.mines_nearby == ExtendedBool::kTrue) ? 0 : 0;
					inputs[12] = (owner_number == persistent_data->last_attacked_player) ? 2 : -2;
					inputs[13] = (owner_number == persistent_data->last_attacked_player) ? 4 : -4;	
					inputs[14] = (strong_enough)	? 1 : -1;
					inputs[15] = (strong_enough)	? 3 : -3;
					inputs[16] = (player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn)) ? 2 : - 2;
					inputs[17] = (player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn)) ? 3 : - 3;
					inputs[18] = (player_statistics.get_visible_enemies_power(pn) > player_statistics.get_old_visible_enemies_power(pn))? - 1 : 1;
					inputs[19] = (player_statistics.get_visible_enemies_power(pn) > player_statistics.get_old_visible_enemies_power(pn))? - 3 : 3;				
					inputs[20] = (player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(owner_number)) ? 2 : - 2;
					inputs[21] = (player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(owner_number)) ? 1 : - 1;						
					inputs[22] = (player_attackable[owner_number - 1] == Attackable::kAttackableVeryWeak) ? 2 : 0;
					inputs[23] = (player_attackable[owner_number - 1] == Attackable::kAttackableAndWeak) ? 2 : 0;
					inputs[24] = (player_attackable[owner_number - 1] == Attackable::kAttackableVeryWeak) ? 6 : 0;
					inputs[25] = (player_attackable[owner_number - 1] == Attackable::kAttackableAndWeak) ? 3 : 0;
					inputs[26] = management_data.get_military_number_at(62) / 10;
					inputs[27] = (ts_finished_count_ - ts_without_trainers_) * 2; 
					inputs[28] = general_score * 3;
					inputs[29] = general_score;
					inputs[30] = ((mines_per_type[iron_ore_id].in_construction + mines_per_type[iron_ore_id].finished) > 0) ? 1 : -1;
					inputs[31] = (player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn) + 5) ? 2 : -2;
	
					site->second.score = 0;
					for (uint8_t j = 0; j < f_neuron_bit_size; j +=1) {
						if (management_data.f_neuron_pool[47].get_position(j)){
							site->second.score += inputs[j];
							
						}
					}
				}				
				

			} else {
				site->second.score = 0;
			}  // or the score will remain 0

			if (site->second.score > 0) {
				if (site->second.score > best_score) {
					best_score = site->second.score;
					best_target = site->first;
				}
			}

			if (site->second.attack_soldiers_strength > 0) {
				site->second.last_time_attackable = gametime;
			}
			if (site->second.last_time_attackable + 20 * 60 * 1000 < gametime) {
				site_to_be_removed = site->first;
			}
		} else {  // we dont have a flag, let remove the site from out observer list
			site_to_be_removed = site->first;
		}

		if (site_to_be_removed < std::numeric_limits<uint32_t>::max()) {
			disappeared_sites.push_back(site_to_be_removed);
		}
	}

	while (!disappeared_sites.empty()) {
		enemy_sites.erase(disappeared_sites.back());
		disappeared_sites.pop_back();
	}

	// modifying enemysites_check_delay_,this depends on the count
	// of enemysites in observer
	if (count >= 13 && enemysites_check_delay_ < 180) {
		enemysites_check_delay_ += 3;
	}
	if (count < 10 && enemysites_check_delay_ > 45) {
		enemysites_check_delay_ -= 2;
	}

	// if coordinates hash is not set
	if (best_target == std::numeric_limits<uint32_t>::max()) {
		return false;
	}

	assert(enemy_sites.count(best_target) > 0);

	// attacking
	FCoords f = map.get_fcoords(Coords::unhash(best_target));
	// setting no attack counter here
	// this gauranties that it will not be attacked in next 3
	// turns
	enemy_sites[best_target].no_attack_counter =
	   -3 + management_data.get_military_number_at(20) / 40;
	assert(enemy_sites[best_target].no_attack_counter > -7 &&
	       enemy_sites[best_target].no_attack_counter < 0);

	Flag* flag = nullptr;  // flag of a building to be attacked
	if (upcast(MilitarySite, bld, f.field->get_immovable())) {
		flag = &bld->base_flag();
	} else if (upcast(Warehouse, Wh, f.field->get_immovable())) {
		flag = &Wh->base_flag();
	} else {
		return false;  // this should not happen
	}

	// how many attack soldiers we can send?
	int32_t attackers = player_->find_attack_soldiers(*flag);
	assert (attackers < 500);

	// Of course not all of them:
	// reduce by 0-3 for attackers below 10
	// but for soldiers in range 10-40 reduce by much more.
	// Soldiers above 40 are ignored for calculation

	// Number of soldiers in the range 10-40, random portion of
	// them will be used
	uint32_t above_ten = (attackers > 10) ? attackers - 10 : 0;
	above_ten = (above_ten > 30) ? 30 : above_ten;

	attackers = attackers - (std::rand() % 3) - ((above_ten > 0) ? std::rand() % above_ten : 0);
	//attackers += management_data.neuron_pool[48].get_result_safe(training_score / 2, kAbsValue) / 10;
	assert (attackers < 500);

	if (attackers <= 0) {
		return false;
	}

	attackers_count_ += static_cast<uint16_t>(attackers);
	assert (attackers_count_ < 5000);

	game().send_player_enemyflagaction(*flag, player_number(), static_cast<uint16_t>(attackers));

	last_attack_time_ = gametime;
	persistent_data->last_attacked_player = flag->owner().player_number();

	return true;
}
// this just counts free positions in military and training sites
void DefaultAI::count_military_vacant_positions(const uint32_t gametime) {
	// counting vacant positions
	int32_t vacant_mil_positions_ = 0;
	for (TrainingSiteObserver tso : trainingsites) {
		vacant_mil_positions_ +=
		   5 * std::min<int32_t>(
		          (tso.site->soldier_capacity() - tso.site->stationed_soldiers().size()), 2);
	}
	for (MilitarySiteObserver mso : militarysites) {
		vacant_mil_positions_ += mso.site->soldier_capacity() - mso.site->stationed_soldiers().size();
	}

	SoldiersStatus soldier_status_tmp;

	if (vacant_mil_positions_ <= 1) {
		soldier_status_tmp = SoldiersStatus::kFull;
	} else if (vacant_mil_positions_ * 4 <= static_cast<int32_t>(militarysites.size())) {
		soldier_status_tmp = SoldiersStatus::kEnough;
	} else if (vacant_mil_positions_ > static_cast<int32_t>(militarysites.size())) {
		soldier_status_tmp = SoldiersStatus::kBadShortage;
	} else {
		soldier_status_tmp = SoldiersStatus::kShortage;
	}

	// Never increase soldier status too soon
	if (soldier_status_tmp >= soldier_status_) {
		soldier_status_ = soldier_status_tmp;
		military_status_last_updated = gametime;
	} else if (soldier_status_tmp < soldier_status_ &&
	           military_status_last_updated +
	                 std::abs(management_data.get_military_number_at(60)) * 60 * 1000 / 10 <
	              gametime) {
		// printf("%d / %d: finaly increasing soldier status %d -> %d\n", player_number(),
		// gametime / 1000, soldier_status_, soldier_status_tmp);
		soldier_status_ = soldier_status_tmp;
	}
}

// this function only check with trainingsites
// manipulates input queues and soldier capacity
bool DefaultAI::check_trainingsites(uint32_t gametime) {

	if (trainingsites.empty()) {
		set_taskpool_task_time(
		   gametime + 2 * kTrainingSitesCheckInterval, SchedulerTaskId::kCheckTrainingsites);
		return false;
	}

	trainingsites.push_back(trainingsites.front());
	trainingsites.pop_front();

	TrainingSite* ts = trainingsites.front().site;
	TrainingSiteObserver& tso = trainingsites.front();

	const DescriptionIndex enhancement = ts->descr().enhancement();

	if (enhancement != INVALID_INDEX && ts_without_trainers_ == 0 && mines_.size() > 3 &&
	    ts_finished_count_ > 1) {

		if (player_->is_building_type_allowed(enhancement)) {
			game().send_player_enhance_building(*tso.site, enhancement);
		}
	}

	// changing capacity to 0 - this will happen only once.....
	if (tso.site->soldier_capacity() > 1) {
		game().send_player_change_soldier_capacity(*ts, -tso.site->soldier_capacity());
		return true;
	}

	// reducing ware queues
	// - for armours and weapons to 1
	// - for others to 6
	// - for others to 6
	for (InputQueue* queue : tso.site->inputqueues()) {

		if (queue->get_type() != wwWARE) {
			continue;
		}

		// if it was decreased yet
		if (queue->get_max_fill() <= 1) {
			continue;
		}

		// now modifying max_fill of armors and weapons
		for (std::string pattern : armors_and_weapons) {

			if (tribe_->get_ware_descr(queue->get_index())->name().find(pattern) !=
			    std::string::npos) {
				if (queue->get_max_fill() > 1) {
					game().send_player_set_input_max_fill(*ts, queue->get_index(), wwWARE, 1);
					continue;
				}
			}
		}
	}

	//// changing priority if basic
	//if (tso.bo->trainingsite_type == TrainingSiteType::kBasic) {
		//for (uint32_t k = 0; k < tso.bo->inputs.size(); ++k) {
			//game().send_player_set_ware_priority(*ts, wwWARE, tso.bo->inputs.at(k), HIGH_PRIORITY);
		//}
	//}

	// are we willing to train another soldier
	// bool want_train = true;
	const PlayerNumber pn = player_number();

	// if soldier capacity is set to 0, we need to find out if the site is
	// supplied enough to incrase the capacity to 1
	if (tso.site->soldier_capacity() == 0) {

		// First subsitute wares
		int32_t filled = 0;
		// We call a soldier to a trainingsite only if it is stocked. Shortage is deficit of wares
		// Generally we accept shortage 1, but if training is stalled (no trained soldier in last 1
		// minutes)
		// we can accept also shortage up to 3
		int32_t shortage = 0;
		for (InputQueue* queue : tso.site->inputqueues()) {
			if (queue->get_type() != wwWARE) {
				continue;
			}
			if (tso.bo->substitute_inputs.count(queue->get_index()) > 0) {
				filled += queue->get_filled();
			}
		}
		if (filled < 5) {
			shortage += 5 - filled;
		}

		// checking non subsitutes
		for (InputQueue* queue : tso.site->inputqueues()) {
			if (queue->get_type() != wwWARE) {
				continue;
			}
			if (tso.bo->substitute_inputs.count(queue->get_index()) == 0) {
				const uint32_t required_amount =
				   (queue->get_max_fill() < 5) ? queue->get_max_fill() : 5;
				if (queue->get_filled() < required_amount) {
					shortage += required_amount - queue->get_filled();
				}
			}
		}

		if (shortage <= 3) {  // training only if supplied

			
			int16_t inputs[f_neuron_bit_size] = {0};
			inputs[0] = -shortage;
			inputs[1] = (player_statistics.get_visible_enemies_power(gametime) >
			                        player_statistics.get_old_visible_enemies_power(gametime)) ? 1 : 0;
			inputs[2] = (mines_.size() < 3) ? -1 : 0;			
			inputs[3] = (mines_per_type[iron_ore_id].total_count() == 0) ? -1 : 0;	
			inputs[4] = (player_statistics.get_player_power(pn) * 2 >
			                player_statistics.get_visible_enemies_power(gametime)) ? -1 : 0;
			inputs[5] = (player_statistics.get_player_power(pn) * 2 >
			                player_statistics.get_enemies_average_power()) ? -1 : 0;
			inputs[6] = (player_statistics.get_player_power(pn) >
			                player_statistics.get_visible_enemies_power(gametime)) ? -1 : 1;
			inputs[7] = (player_statistics.get_player_power(pn) >
			                player_statistics.get_enemies_average_power()) ? -1 : 1;
			inputs[8] = (player_statistics.get_player_power(pn) * 2 >
			                player_statistics.get_visible_enemies_power(gametime)) ? -1 : 1;
			inputs[9] = (player_statistics.get_player_power(pn) * 2 >
			                player_statistics.get_enemies_average_power()) ? -1 : 1;
			inputs[10] = (3 - shortage) * 1;
			inputs[11] = (3 - shortage) * 2;
			inputs[12] = +1;	
			inputs[13] = +2;	
			inputs[14] = -1;	
			inputs[15] = -2;	
			inputs[16] = (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn)) ? 1 : 0;		
			inputs[17] = (player_statistics.get_player_power(pn) < player_statistics.get_old_player_power(pn)) ? 1 : 0;
			inputs[18] = (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn) + 4) ? 1 : 0;		
			inputs[19] = (player_statistics.get_player_power(pn) < player_statistics.get_old_player_power(pn) + 1) ? 1 : 0;
			inputs[20] = (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn) + 4) ? 0 : 1;		
			inputs[21] = (player_statistics.get_player_power(pn) < player_statistics.get_old_player_power(pn) + 2) ? 0 : 1;

			if (player_statistics.any_enemy_seen_lately(gametime)) {
				inputs[20] = (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn) + 4) ? 0 : 1;		
				inputs[21] = (player_statistics.get_player_power(pn) < player_statistics.get_old_player_power(pn) + 2) ? 0 : 1;
				inputs[22] = (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn) + 4) ? 1 : 0;		
				inputs[23] = (player_statistics.get_player_power(pn) < player_statistics.get_old_player_power(pn) + 2) ? 1 : 0;
			}
			
			int16_t tmp_score = 0;
			for (uint8_t i = 0; i < f_neuron_bit_size; i +=1) {
				if (management_data.f_neuron_pool[29].get_position(i)){
					tmp_score += inputs[i];
				}
			}

			
			if (tmp_score > 0) { 
				game().send_player_change_soldier_capacity(*ts, 1);
			}
		}
	}

	ts_without_trainers_ = 0;  // zeroing
	for (std::list<TrainingSiteObserver>::iterator site = trainingsites.begin();
	     site != trainingsites.end(); ++site) {

		if (!site->site->can_start_working()) {
			ts_without_trainers_ += 1;
		}
	}
	return true;
}



/**
 * Updates the first military building in list and reenques it at the end of
 * the list afterwards. If a militarysite is in secure area but holds more than
 * one soldier, the number of stationed soldiers is decreased. If the building
 * is near a border, the number of stationed soldiers is maximized
 *
 * \returns true if something was changed
 */
bool DefaultAI::check_militarysites(uint32_t gametime) {

	// Only useable, if defaultAI owns at least one militarysite
	if (militarysites.empty()) {
		return false;
	}


	//if (gametime % 100 == 0) {
		//for (int i = 0; i < f_neuron_bit_size; i = i + 1) {
			//if ( !management_data.f_neuron_pool[5 3].get_position(i) &&
			 //!management_data.f_neuron_pool[5 4].get_position(i) &&
			 //!management_data.f_neuron_pool[5 5].get_position(i) &&
			 //!management_data.f_neuron_pool[5 6].get_position(i)) {
				 //printf (" neuron bit on position %d not used\n", i);
			//}
			//if ( !management_data.f_neuron_pool[7].get_position(i) &&
			 //!management_data.f_neuron_pool[9 ].get_position(i) &&
			 //!management_data.f_neuron_pool[1 2].get_position(i) &&
			 //!management_data.f_neuron_pool[7 4].get_position(i)) {
				 //printf (" neuron bit on position %d not used\n", i+32);
			//}		
		//}
	//}


	// Check next militarysite
	bool changed = false;
	Map& map = game().map();
	MilitarySite* ms = militarysites.front().site;
	// MilitarySiteObserver& mso = militarysites.front();
	// uint32_t const vision = ms->descr().vision_range();
	FCoords f = map.get_fcoords(ms->get_position());
	// look if there are any enemies building
	FindNodeEnemiesBuilding find_enemy(player_, game());

	int16_t usefullness_score = 0;
	BuildableField bf(f);
	update_buildable_field(bf);
	usefullness_score += bf.military_score_ / 10;
	if (military_last_dismantle_ == 0 || military_last_dismantle_ + 2 * 60 * 1000 > gametime) {
		usefullness_score += std::abs(management_data.get_military_number_at(99)) / 2;
	}
	
	if (militarysites.front().built_time + 2 * 60 * 1000 > gametime) {
		usefullness_score += std::abs(management_data.get_military_number_at(99)) / 2;
	}
	
	usefullness_score -= static_cast<int16_t>(soldier_status_) * std::abs(management_data.get_military_number_at(84));
	usefullness_score += ((bf.enemy_accessible_) ? (std::abs(management_data.get_military_number_at(91))) / 2 : 0);
	
	// Also size is consideration, bigger buildings are to be preffered
	usefullness_score += (ms->get_size() - 2) * std::abs(management_data.get_military_number_at(77) / 2);
	
	const int32_t dism_treshold = 20 - management_data.get_military_number_at(89)  / 2;
	const int32_t pref_treshold_upper = dism_treshold + std::abs(management_data.get_military_number_at(90));
	const int32_t pref_treshold_lower = dism_treshold + std::abs(management_data.get_military_number_at(90) / 2);
	//printf ("current score %3d, dism: tresh %3d, pref tresholds: %3d   %3d %s\n",
		//usefullness_score, dism_treshold, pref_treshold_lower, pref_treshold_upper, (usefullness_score > pref_treshold_upper) ? "*" : "");
	
	Quantity const total_capacity = ms->max_soldier_capacity();
	Quantity const current_target = ms->soldier_capacity();

	if (usefullness_score < dism_treshold) {
		changed = true;
		if (ms->get_playercaps() & Widelands::Building::PCap_Dismantle) {
			flags_to_be_removed.push_back(ms->base_flag().get_position());
			game().send_player_dismantle(*ms);
			military_last_dismantle_ = game().get_gametime();
		} else {
			game().send_player_bulldoze(*ms);
			military_last_dismantle_ = game().get_gametime();
		}		
	} else if (usefullness_score < pref_treshold_lower) {
		// this site is not that important but is to be preserved
		if (current_target > 1){
			game().send_player_change_soldier_capacity(*ms, -1);
			changed = true;
			}
		if (ms->get_soldier_preference() == MilitarySite::kPrefersHeroes) {
			game().send_player_militarysite_set_soldier_preference(*ms, MilitarySite::kPrefersRookies);
			changed = true;
		}
	} else if (usefullness_score > pref_treshold_upper) {
		// this is important military site
		if (current_target < total_capacity) {
			game().send_player_change_soldier_capacity(*ms, 1);
			changed = true;
		}
		if (ms->get_soldier_preference() == MilitarySite::kPrefersRookies) {
			game().send_player_militarysite_set_soldier_preference(*ms, MilitarySite::kPrefersHeroes);
			changed = true;
		}
	}
	
	// reorder:;
	militarysites.push_back(militarysites.front());
	militarysites.pop_front();
	return changed;
}

uint32_t DefaultAI::barracks_count() {
	uint32_t count = 0;
	for (auto ps : productionsites) {
		if (ps.bo->is_barracks) {
			count += ps.bo->total_count();
		}
	}
	return count;
}

// This calculates strength of vector of soldiers, f.e. soldiers in a building or
// ones ready to attack
int32_t DefaultAI::calculate_strength(const std::vector<Widelands::Soldier*>& soldiers) {

	if (soldiers.empty()) {
		return 0;
	}


	float health = 0;
	float attack = 0;
	float defense = 0;
	float evade = 0;
	float final = 0;

	for (Soldier* soldier : soldiers) {
		const SoldierDescr& descr = soldier->descr();
		health =
		   descr.get_base_health() + descr.get_health_incr_per_level() * soldier->get_health_level();
		attack = (descr.get_base_max_attack() - descr.get_base_min_attack()) / 2.f +
		         descr.get_base_min_attack() +
		         descr.get_attack_incr_per_level() * soldier->get_attack_level();
		defense = 100 - descr.get_base_defense() - 8 * soldier->get_defense_level();
		evade = 100 - descr.get_base_evade() -
		        descr.get_evade_incr_per_level() / 100.f * soldier->get_evade_level();
		final += (attack * health) / (defense * evade);
	}

	// 2500 is aproximate strength of one unpromoted soldier
	return static_cast<int32_t>(final / 2500);
}

// Now we can prohibit some militarysites, based on size, the goal is not to
// exhaust AI resources on the beginning of the game
// We count bigger buildings, medium ones get 1 points, big ones 2 points
// and we force some proportion to the number of military sites
// sidenote: function can return kNotNeeded, but it means 'not allowed'
BuildingNecessity DefaultAI::check_building_necessity(BuildingObserver& bo,
                                                      const uint32_t gametime) {

	assert(militarysites.size() == msites_built());
	
	const PlayerNumber pn = player_number();
	
	// logically size of militarysite must in between 1 and 3 (including)
	const uint8_t size = bo.desc->get_size();
	assert(size >= BaseImmovable::SMALL && size <= BaseImmovable::BIG);

	if (military_last_build_ > gametime - (10 + std::abs(management_data.get_military_number_at(43)) * 1000 / 2)) {
		return BuildingNecessity::kForbidden;
	}

	bo.primary_priority = 0;
	
	
	// Inputs
	// in construction
	// proportion of sizes
	// available fields
	// soldier status
	
	const uint32_t msites_total = msites_built() + msites_in_constr();
	const uint16_t scores[3] = {static_cast<uint16_t>(msites_per_size[1].in_construction + msites_per_size[1].finished),
		static_cast<uint16_t>((msites_per_size[2].in_construction + msites_per_size[2].finished) * 2),
		static_cast<uint16_t>((msites_per_size[3].in_construction + msites_per_size[3].finished) * 3)};
	const uint16_t total_score = scores[0]+scores[1]+scores[2];

	//printf ("stat sample %d  %d\n", player_statistics.get_player_land(pn), player_statistics.get_enemies_max_land()); 
	
	int32_t inputs[4 * f_neuron_bit_size] = {0};
	inputs[0] = (msites_total < 1) ? 1 : 0;
	inputs[1] = (msites_total < 2) ? 1 : 0;
	inputs[2] = (msites_total < 3) ? 1 : 0;
	inputs[3] = (msites_total < 4) ? 1 : 0;
	inputs[3] = (msites_total < 5) ? 1 : 0;
	inputs[4] = (msites_in_constr() >  msites_built()) ? -1 : 0;
	inputs[5] = - 3;
	inputs[6] = (msites_in_constr() >  msites_built() / 2) ? -1 : 0;
	inputs[7] = (msites_in_constr() >  msites_built() / 3) ? -1 : 0;
	inputs[8] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -2 : 0;
	inputs[9] = (soldier_status_ == SoldiersStatus::kShortage) ? -1 : 0;
	inputs[10] = (scores[size - 1] > total_score ) ? -2 : 0;
	inputs[11] = (scores[size - 1] > total_score / 2) ? -2 : 0;
	inputs[12] = (scores[size - 1] > total_score / 3) ? -2 : 0;
	inputs[13] = (player_statistics.get_enemies_max_land() < player_statistics.get_player_land(pn)) ? -1 : 0;
	inputs[14] = (mines_per_type[iron_ore_id].total_count() == 0) ? +1 : 0;
	inputs[15] = (spots_ < kSpotsTooLittle) ? +1 : 0;
	inputs[16] = +1 ;
	inputs[17] = +2 ;
	inputs[18] = -1;
	inputs[19] = -2;
	inputs[20] = (scores[size - 1] > total_score / 2) ? -1 : 0;
	inputs[21] = (msites_in_constr() >  msites_built() / 3) ? -1 : 0;
	inputs[22] = (scores[size - 1] > total_score / 4) ? -1 : 0;	
	inputs[23] = (3 - size) * (msites_in_constr() < 1) ? +1 : 0;	
	inputs[24] = (3 - size) * (msites_in_constr() < 3) ? +1 : 0;	
	inputs[25] = (3 - size) * (msites_in_constr() < 5) ? +1 : 0;		
	inputs[26] = (msites_in_constr() < 7) ? +1 : 0;	
	inputs[27] = +5;
	inputs[28] = -5;
	inputs[29] = +3;
	inputs[30] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -4 : 0;
	inputs[31] = (soldier_status_ == SoldiersStatus::kShortage) ? -2 : 0;
	
	inputs[32] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -3 : 0;
	inputs[33] = (soldier_status_ == SoldiersStatus::kShortage) ? -2 : 0;	
	
	inputs[34] = (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land()) ? 1 : 0;
	inputs[35] = (!player_statistics.any_enemy_seen_lately(gametime)) *
					(player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land()) ? 2 : 0;
	inputs[36] = (!player_statistics.any_enemy_seen_lately(gametime)) *
					(player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land() * 2) ? 1 : 0;
	inputs[37] = (!player_statistics.any_enemy_seen_lately(gametime)) *
					(player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land() / 2) ? 1 : 0;

	inputs[38] = (!player_statistics.any_enemy_seen_lately(gametime)) *
					(player_statistics.get_player_land(pn) < player_statistics.get_old_player_land(pn) * 105 / 100) ? 2 : 0;
	inputs[39] = (!player_statistics.any_enemy_seen_lately(gametime)) *
					(player_statistics.get_player_land(pn) < player_statistics.get_old_player_land(pn) + 110) ? 3 : 0;

	inputs[40] = (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn)) ? 1 : 0;
	inputs[41] = (player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn)) ? 1 : 0;
	inputs[42] = (!player_statistics.any_enemy_seen_lately(gametime)) *
					(player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn)) ? 1 : 0;
	inputs[43] = (!player_statistics.any_enemy_seen_lately(gametime)) *
					(player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn)) ? 1 : 0;

	inputs[44] = (player_statistics.get_player_land(pn) < player_statistics.get_enemies_average_land()) ? 1 : 0;
	inputs[45] = (player_statistics.get_player_land(pn) > player_statistics.get_enemies_average_land()) ? 1 : 0;

	inputs[46] = (!player_statistics.any_enemy_seen_lately(gametime)) *
					(player_statistics.get_player_land(pn) < player_statistics.get_enemies_average_land()) ? 2 : 0;
	inputs[47] = (!player_statistics.any_enemy_seen_lately(gametime)) *
					(player_statistics.get_player_land(pn) > player_statistics.get_enemies_average_land()) ? 2 : 0;

	inputs[48] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -3 : 0;
	inputs[49] = (soldier_status_ == SoldiersStatus::kShortage) ? -2 : 0;

	inputs[50] = (!player_statistics.any_enemy_seen_lately(gametime)) *
				(player_statistics.get_player_land(pn) < player_statistics.get_old_player_land(pn) * 110 / 100) ? 1 : 0;
	inputs[51] = (!player_statistics.any_enemy_seen_lately(gametime)) *
				(player_statistics.get_player_land(pn) < player_statistics.get_old_player_land(pn) * 105 / 100) ? 2 : 0;

	inputs[52] = (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land()) ? 1 : 0;
	inputs[53] = (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land()) ? 3 : 0;
	inputs[54] = (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land() * 2) ? 2 : 0;
	inputs[55] = (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land() / 2) ? 2 : 0;
	inputs[56] = (!player_statistics.any_enemy_seen_lately(gametime)) * (spots_ < kSpotsTooLittle) ? +2 : 0;
	inputs[57] = (player_statistics.any_enemy_seen_lately(gametime)) * (spots_ < kSpotsTooLittle) ? +2 : 0;
	inputs[58] = ((mines_per_type[iron_ore_id].in_construction + mines_per_type[iron_ore_id].finished) == 0) ? +3 : 0;
	inputs[59] = ((mines_per_type[iron_ore_id].in_construction + mines_per_type[iron_ore_id].finished) == 0) ? +1 : 0;
	inputs[60] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ? -2 : 0;
	inputs[61] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy || expansion_type.get_expansion_type() == ExpansionMode::kBoth) ? -4 : 0;
	inputs[62] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -3 : 0;
	inputs[63] = (soldier_status_ == SoldiersStatus::kShortage) ? -2 : 0;	
	
	inputs[64] = (bo.build_material_shortage) ? -3 : 0;
	inputs[65] = (bo.build_material_shortage) ? -1 : 0;
	inputs[66] = (bo.build_material_shortage) ? -2 : 0;
	inputs[67] = (bo.build_material_shortage) ? -2 : 0;
	inputs[68] = (gametime < 15 * 60 * 1000) ? (size - 1) * -1 : 0;
	inputs[69] = (gametime < 30 * 60 * 1000) ? (size - 1) * -1 : 0;
	inputs[70] = (gametime < 45 * 60 * 1000) ? (size - 1) * -1 : 0;
	inputs[71] = (gametime < 15 * 60 * 1000) ? (size - 1) * -2 : 0;
	inputs[72] = (gametime < 30 * 60 * 1000) ? (size - 1) * -2 : 0;
	inputs[73] = (gametime < 45 * 60 * 1000) ? (size - 1) * -2 : 0;
	inputs[74] = (gametime < 15 * 60 * 1000) ? (size - 1) * -3 : 0;
	inputs[75] = (gametime < 30 * 60 * 1000) ? (size - 1) * -3 : 0;
	inputs[76] = (gametime < 45 * 60 * 1000) ? (size - 1) * -3 : 0;
	inputs[77] = (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn) + 2) ? 1 : 0;
	inputs[78] = (player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn) + 5) ? 1 : 0;
	inputs[79] = (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn) + 10) ? 1 : 0;
	inputs[80] = (player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn) + 20) ? 1 : 0;
	if(!player_statistics.any_enemy_seen_lately(gametime)){
		inputs[81] = (player_statistics.get_player_land(pn) * 130 / 100 < player_statistics.get_old60_player_land(pn)) ? 2 : 0;
		inputs[82] = (player_statistics.get_old60_player_land(pn) * 130 / 100 < player_statistics.get_player_land(pn)) ? 0 : 2;
	} else {
		inputs[83] = (player_statistics.get_player_land(pn) * 130 / 100 < player_statistics.get_old60_player_land(pn)) ? 2 : 0;
		inputs[84] = (player_statistics.get_old60_player_land(pn) * 130 / 100 < player_statistics.get_player_land(pn)) ? 0 : 2;	
	}
	inputs[85] = -1 * static_cast<int32_t>(msites_in_constr());
	inputs[86] = -1 * static_cast<int32_t>(msites_in_constr())/ 2;
	inputs[87] = -1 * static_cast<int32_t>(msites_in_constr()) / 3;
	inputs[88] = (msites_in_constr() > 2 && msites_in_constr() >  msites_built() / 2) ? -1 : 0;
	inputs[89] = (msites_in_constr() > 2 && msites_in_constr() >  msites_built() / 3) ? -1 : 0;
	inputs[90] = (msites_in_constr() > 2 && msites_in_constr() >  msites_built() / 4) ? -1 : 0;
	inputs[91] = -static_cast<int32_t>(msites_in_constr()) / 4;
	inputs[92] = (player_statistics.get_player_land(pn) < player_statistics.get_old_player_land(pn) * 130 / 100) ? 2 : 0;
	inputs[93] = (player_statistics.get_player_land(pn) < player_statistics.get_old_player_land(pn) * 140 / 100) ? 2 : 0;
	inputs[94] = (!player_statistics.any_enemy_seen_lately(gametime)) *
				(player_statistics.get_player_land(pn) < player_statistics.get_old_player_land(pn) * 120 / 100) ? 2 : 0;
	inputs[95] = (!player_statistics.any_enemy_seen_lately(gametime)) *
				(player_statistics.get_player_land(pn) < player_statistics.get_old_player_land(pn) * 140 / 100) ? 2 : 0;
	if (msites_built() > 5) {
		inputs[96] = -1 * static_cast<int32_t>(msites_in_constr());
		inputs[97] = -1 * static_cast<int32_t>(msites_in_constr()) / 2;
		inputs[98] = -1 * static_cast<int32_t>(msites_in_constr()) / 3;
		inputs[99] = (msites_in_constr() >  msites_built() / 2) ? -2 : 0;
		inputs[100] = (msites_in_constr() >  msites_built() / 3) ? -2 : 0;
		inputs[101] = (msites_in_constr() >  msites_built() / 4) ? -2 : 0;
	}

	for (int i = 0; i < 4 * f_neuron_bit_size; i = i + 1) {
		if (inputs[i] < -35 || inputs[i] > 6) {
			printf ("Warning check_building_necessity score on position %2d too high %2d\n", i, inputs[i]);
		}
	}

	int32_t final_score = 0;
	for (int i = 0; i < f_neuron_bit_size; i = i + 1) {
		if (management_data.f_neuron_pool[56].get_position(i)) {
			final_score += inputs[i];
		}
		if (management_data.f_neuron_pool[57].get_position(i)) {
			final_score += inputs[f_neuron_bit_size + i];
		}
		if (management_data.f_neuron_pool[58].get_position(i)) {
			final_score += inputs[2 * f_neuron_bit_size + i];
		}
		if (management_data.f_neuron_pool[13].get_position(i)) {
			final_score += inputs[3 * f_neuron_bit_size + i];
		}
	}

	final_score += std::abs(management_data.get_military_number_at(76) / 10);
	final_score += management_data.get_military_number_at(100) / 5;

	if (final_score >0) {
		bo.primary_priority = final_score * std::abs(management_data.get_military_number_at(79) / 2);
		return BuildingNecessity::kAllowed;
	} else {
		return BuildingNecessity::kForbidden;
		}

}

// This is called when soldier left the trainingsite
// the purpose is to set soldier capacity to 0
// (AI will then wait till training site is stocked)
void DefaultAI::soldier_trained(const TrainingSite& site) {

	persistent_data->last_soldier_trained = game().get_gametime();

	for (TrainingSiteObserver& trainingsite_obs : trainingsites) {
		if (trainingsite_obs.site == &site) {
			if (trainingsite_obs.site->soldier_capacity() > 0) {
				game().send_player_change_soldier_capacity(
				   *trainingsite_obs.site, -trainingsite_obs.site->soldier_capacity());
			}
			return;
		}
	}

	log(" %d: Computer player error - trainingsite not found\n", player_number());
}
