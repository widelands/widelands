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

#include "ai/defaultai.h"

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

	// printf (" %d: strength: %3d -> %3d ->%3d\n",
	// pn, player_statistics.get_old60_player_power(pn), player_statistics.get_old_player_power(pn),
	// player_statistics.get_player_power(pn));

	// printf ("%d : visible enemies (%d) power: %3d -> %3d, any seen at %u\n",
	// pn,
	// player_statistics.enemies_seen_lately_count(gametime),
	// player_statistics.get_old_visible_enemies_power(gametime),
	// player_statistics.get_visible_enemies_power(gametime),
	// player_statistics.enemy_last_seen());

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
	int8_t training_score = 0;
	if (persistent_data->last_soldier_trained > gametime) {
		// No soldier was ever trained ...
		switch (type_) {
		case DefaultAI::Type::kNormal:
			training_score = -8;
			break;
		case DefaultAI::Type::kWeak:
			training_score = -4;
			break;
		case DefaultAI::Type::kVeryWeak:
			training_score = -2;
		}
	} else {
		training_score +=
		   management_data.neuron_pool[19].get_result_safe(
		      (gametime - persistent_data->last_soldier_trained) / 60 / 1000, kAbsValue) /
		   10;
	}

	// Also we should have at least some training sites to be more willing to attack
	// Of course, very weak AI can have only one trainingsite so will be always penalized by this
	training_score += management_data.neuron_pool[23].get_result_safe(
	                     (ts_basic_count_ + ts_advanced_count_ - ts_without_trainers_) * 4) /
	                  10;

	// some black box magic related to growth
	training_score +=
	   management_data.f_neuron_pool[6].get_result(
	      player_statistics.get_player_power(pn) > player_statistics.get_old_player_power(pn),
	      player_statistics.get_visible_enemies_power(pn) >
	         player_statistics.get_old_visible_enemies_power(pn),
	      player_statistics.get_player_power(pn) - player_statistics.get_old_player_power(pn) > 0) *
	   std::abs(management_data.get_military_number_at(63)) / 10;

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
				site->second.score =
				   site->second.attack_soldiers_strength - site->second.defenders_strength / 2;

				if (is_warehouse) {
					site->second.score += 2;
				} else {
					site->second.score -= 2;
				}

				// considering heroes/rokie status of soldiers
				site->second.score += management_data.neuron_pool[63].get_result_safe(
				                         site->second.attack_soldiers_competency - 10, kAbsValue) /
				                      10;
				site->second.score -= std::abs(management_data.get_military_number_at(62)) / 10;
				site->second.score +=
				   management_data.f_neuron_pool[31].get_result(
				      site->second.attack_soldiers_competency > 15,
				      site->second.attack_soldiers_competency > 25,
				      site->second.defenders_strength > site->second.attack_soldiers_strength,
				      2 * site->second.defenders_strength > 3 * site->second.attack_soldiers_strength,
				      3 * site->second.defenders_strength > 2 * site->second.attack_soldiers_strength) /
				   20;

				site->second.score -= static_cast<uint8_t>(soldier_status_);

				if (site->second.mines_nearby == ExtendedBool::kTrue) {
					site->second.score += std::abs(management_data.get_military_number_at(18)) / 15;
				}
				// we dont want to attack multiple players at the same time too eagerly
				if (owner_number != persistent_data->last_attacked_player) {
					site->second.score -= 3 + std::abs(management_data.get_military_number_at(38)) / 40;
				}

				// Let consider mines nearby and generally
				site->second.score +=
				   management_data.f_neuron_pool[5].get_result(
				      mines_.size() <= 2, mines_.size() > 2 && mines_.size() < 6, mines_.size() >= 6,
				      site->second.mines_nearby == ExtendedBool::kTrue,
				      mines_per_type[iron_ore_id].total_count() == 0) *
				   std::abs(management_data.get_military_number_at(19)) / 20;

				// Applying (decreasing score) if trainingsites are not working
				site->second.score += training_score;

				// We have an advantage over stongest opponent
				if (strong_enough) {
					site->second.score += 3;
				}

				// Enemy is too weak, be more aggressive attacking him
				if (player_attackable[owner_number - 1] == Attackable::kAttackableAndWeak) {
					site->second.score += 4;
				}
				if (player_attackable[owner_number - 1] == Attackable::kAttackableVeryWeak) {
					site->second.score += 8;
				}

				// consider change in power
				site->second.score +=
				   management_data.f_neuron_pool[4].get_result(
				      player_statistics.get_player_power(pn) >  // 1
				         player_statistics.get_old_player_power(pn),
				      player_statistics.get_player_power(owner_number) >  // 2
				         player_statistics.get_old_player_power(owner_number),
				      player_statistics.get_player_power(pn) -  // 3
				            player_statistics.get_old_player_power(pn) >
				         0,
				      static_cast<int32_t>(player_statistics.get_player_power(pn) -  // 4
				                           player_statistics.get_old_player_power(pn)) >
				         static_cast<int32_t>(player_statistics.get_player_power(owner_number) -
				                              player_statistics.get_old_player_power(owner_number)),
				      5 * player_statistics.get_player_power(pn) >  // 5
				         4 * player_statistics.get_old_player_power(pn)) *
				   std::abs(management_data.get_military_number_at(35)) / 10;
				site->second.score +=
				   management_data.f_neuron_pool[30].get_result(
				      player_statistics.get_player_power(pn) >  // 1
				         player_statistics.get_old60_player_power(pn),
				      3 * player_statistics.get_player_power(pn) >  // 2
				         2 * player_statistics.get_old60_player_power(pn),
				      2 * player_statistics.get_player_power(pn) >  // 3
				         3 * player_statistics.get_old60_player_power(pn),
				      player_statistics.get_player_power(pn) >  // 4
				         player_statistics.get_old_player_power(pn),
				      static_cast<int32_t>(player_statistics.get_player_power(pn) -  // 5
				                           player_statistics.get_old60_player_power(pn)) >
				         static_cast<int32_t>(player_statistics.get_player_power(owner_number) -
				                              player_statistics.get_old60_player_power(owner_number))) *
				   std::abs(management_data.get_military_number_at(61)) / 10;

				// treating no attack score
				if (site->second.no_attack_counter < 0) {
					// we cannot attack yet
					site->second.score = 0;
					// but increase the counter by 1
					site->second.no_attack_counter += 1;
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
	attackers += management_data.neuron_pool[48].get_result_safe(training_score / 2, kAbsValue) / 10;
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
	    (ts_basic_const_count_ + ts_advanced_const_count_) == 0 && ts_advanced_count_ > 0) {

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
	for (InputQueue *queue : tso.site->inputqueues()) {

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

	// changing priority if basic
	if (tso.bo->trainingsite_type == TrainingSiteType::kBasic) {
		for (uint32_t k = 0; k < tso.bo->inputs.size(); ++k) {
			game().send_player_set_ware_priority(*ts, wwWARE, tso.bo->inputs.at(k), HIGH_PRIORITY);
		}
	}

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
		for (InputQueue *queue : tso.site->inputqueues()) {
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
		for (InputQueue *queue : tso.site->inputqueues()) {
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

			const bool i1 = player_statistics.get_player_power(pn) -
			                   player_statistics.get_old_player_power(pn) -
			                   (player_statistics.get_visible_enemies_power(gametime) -
			                    player_statistics.get_old_visible_enemies_power(gametime)) >
			                0;
			const bool i2 =
			   static_cast<int16_t>(player_statistics.get_visible_enemies_power(gametime) -
			                        player_statistics.get_old_visible_enemies_power(gametime)) >
			   management_data.get_military_number_at(34) / 10;
			const bool i3 = player_statistics.get_player_power(pn) * 2 >
			                player_statistics.get_visible_enemies_power(gametime);
			const bool i4 = player_statistics.any_enemy_seen_lately(gametime);
			const bool i5 =
			   soldier_status_ == SoldiersStatus::kEnough || soldier_status_ == SoldiersStatus::kFull;

			const bool i6 = mines_.size() < 3;
			const bool i7 = shortage <= 1;
			const bool i8 = mines_per_type[iron_ore_id].total_count() == 0;
			const bool i9 = persistent_data->last_soldier_trained == kNever;

			const bool ts_1 = management_data.f_neuron_pool[26].get_result(i1, i3, i6, i8, i9);
			const bool ts_2 = management_data.f_neuron_pool[27].get_result(i2, i4, i5, i7, i9);
			const bool ts_3 = management_data.f_neuron_pool[28].get_result(i1, i3, i4, i5, i7);

			const bool wanna_train = management_data.f_neuron_pool[29].get_result(ts_1, ts_2, ts_3);

			if (wanna_train) {
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
	
	const int32_t dism_treshold = 20 - management_data.get_military_number_at(89) * 2 / 3;
	//const int32_t pref_treshold = dism_treshold + std::abs(management_data.get_military_number_at(90) * 3 / 2);
	const int32_t pref_treshold_upper = dism_treshold + std::abs(management_data.get_military_number_at(90) * 35 / 20);
	const int32_t pref_treshold_lower = dism_treshold + std::abs(management_data.get_military_number_at(90) * 25 / 20);
	//printf ("pref tresholds %d  %d\n", pref_treshold_lower, pref_treshold_upper);
	
	Quantity const total_capacity = ms->max_soldier_capacity(); //NOCOM
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
		health = descr.get_base_health() +
					 descr.get_health_incr_per_level() * soldier->get_health_level();
		attack = (descr.get_base_max_attack() - descr.get_base_min_attack()) / 2.f +
		          descr.get_base_min_attack() +
					 descr.get_attack_incr_per_level() * soldier->get_attack_level();
		defense =
			100 - descr.get_base_defense() - 8 * soldier->get_defense_level();
		evade =
			100 - descr.get_base_evade() -
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

	//const uint32_t msites_total = msites_built() + msites_in_constr();

	uint32_t const big_buildings_score =
	   msites_per_size[2].in_construction + msites_per_size[2].finished +
	   msites_per_size[3].in_construction * 2 + msites_per_size[3].finished * 2;

	if (size != BaseImmovable::SMALL){
		bo.primary_priority += (static_cast<int16_t>(msites_per_size[1].finished + msites_per_size[1].in_construction - 3) 
			- static_cast<int16_t>(big_buildings_score)) 
				* std::abs(management_data.get_military_number_at(14) / 5);
	}

	if (player_statistics.get_enemies_max_land() < player_statistics.get_player_land(pn)){
		bo.primary_priority += std::abs(management_data.get_military_number_at(33) / 3);
	}
	if (spots_ < kSpotsTooLittle) {
		bo.primary_priority += std::abs(management_data.get_military_number_at(78) / 3) * size;
	}

	if (mines_per_type[iron_ore_id].total_count() == 0) {
		bo.primary_priority += std::abs(management_data.get_military_number_at(77)) / 3;
	}

	if (msites_in_constr() == 0) {
		bo.primary_priority += std::abs(management_data.get_military_number_at(79)) / 3;
	}

	return BuildingNecessity::kAllowed;

	//const uint32_t msites_total = msites_built() + msites_in_constr();

	//if (size == BaseImmovable::SMALL) {  // this function is intended for medium and bigger sites
		//const bool input1 = management_data.f_neuron_pool[37].get_result(
			//mines_built() <=2,
			//mines_per_type[iron_ore_id].total_count() == 0,
			//msites_in_constr() <= static_cast<uint32_t>(std::abs(management_data.get_military_number_at(76)) / 20),
			//spots_ < kSpotsTooLittle,
			//player_statistics.get_enemies_max_land() > player_statistics.get_player_land(pn));
		//const bool input2 = management_data.f_neuron_pool[38].get_result( 
			//msites_in_constr() == 0, (military_last_build_ + 5 * 60 * 1000 > gametime),
		       //big_buildings_score >
		          //msites_total * 10 / (10 + management_data.get_military_number_at(14) / 20),
		       //soldier_status_ == SoldiersStatus::kShortage ||
		          //soldier_status_ == SoldiersStatus::kBadShortage);
		//const bool input3 = management_data.f_neuron_pool[39].get_result( 
			//mines_per_type[iron_ore_id].total_count() == 0,
			//msites_in_constr() <= static_cast<uint32_t>(std::abs(management_data.get_military_number_at(77)) / 20),
			//spots_ < kSpotsTooLittle,		
		    //big_buildings_score >
		          //msites_total * 10 / (10 + management_data.get_military_number_at(33) / 20),
		    //soldier_status_ == SoldiersStatus::kShortage ||
		          //soldier_status_ == SoldiersStatus::kBadShortage);
		
		//const bool intermed1 = management_data.f_neuron_pool[41].get_result(
		       //input1, input2, input3);
		//const bool intermed2 = management_data.f_neuron_pool[42].get_result(
		       //input1, input2, input3);
		//const bool intermed3 = management_data.f_neuron_pool[43].get_result(
		       //input1, input3, input3);		   		       
		
		//if (management_data.f_neuron_pool[44].get_result(
		       //intermed1, intermed2, intermed3)) {
			//return BuildingNecessity::kNotNeeded;
		//} else {
			//return BuildingNecessity::kAllowed;
		//}
	//} else {
		//return BuildingNecessity::kAllowed;
	//}


	//// Now for medium and big buildings
	//const bool input1 = management_data.f_neuron_pool[21].get_result( //NOCOM
		//bo.build_material_shortage,
		//mines_per_type[iron_ore_id].total_count() == 0,
		//msites_in_constr() <= static_cast<uint32_t>(std::abs(management_data.get_military_number_at(78))) / 20,
		//spots_<kSpotsTooLittle,
		//player_statistics.get_enemies_max_land() > player_statistics.get_player_land(pn));
	//const bool input2 = management_data.f_neuron_pool[22].get_result( 
		//msites_in_constr() == 0, (military_last_build_ + 5 * 60 * 1000 > gametime),
	       //big_buildings_score >
	          //msites_total * 10 / (10 + management_data.get_military_number_at(14) / 20),
	       //soldier_status_ == SoldiersStatus::kShortage ||
	          //soldier_status_ == SoldiersStatus::kBadShortage);
	//const bool input3 = management_data.f_neuron_pool[23].get_result( 
		//mines_per_type[iron_ore_id].total_count() == 0,
		//msites_in_constr() <= static_cast<uint32_t>(std::abs(management_data.get_military_number_at(79))) / 20,
		//spots_<kSpotsTooLittle,		
	    //big_buildings_score >
	          //msites_total * 10 / (10 + management_data.get_military_number_at(64) / 20),
	    //soldier_status_ == SoldiersStatus::kShortage ||
	          //soldier_status_ == SoldiersStatus::kBadShortage);

	//// Intermediate results
	//const bool intermed1 = management_data.f_neuron_pool[24].get_result(
	       //input1, input2, input3);
	//const bool intermed2 = management_data.f_neuron_pool[25].get_result(
	       //input1, input2, input3);
	//const bool intermed3 = management_data.f_neuron_pool[26].get_result(
	       //input1, input3, input3);		   		       
	
	//// Final decision
	//if (management_data.f_neuron_pool[33].get_result(
	       //intermed1, intermed2, intermed3)) {
		//return BuildingNecessity::kNotNeeded;
	//} else {
		//return BuildingNecessity::kAllowed;
	//}
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
