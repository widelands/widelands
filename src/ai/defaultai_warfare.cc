/*
 * Copyright (C) 2009-2019 by the Widelands Development Team
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

#include "economy/wares_queue.h"
#include "logic/map_objects/tribes/militarysite.h"

using namespace Widelands;

bool DefaultAI::check_enemy_sites(uint32_t const gametime) {

	const Map& map = game().map();

	PlayerNumber const nr_players = map.get_nrplayers();
	uint32_t plr_in_game = 0;
	Widelands::PlayerNumber const pn = player_number();

	iterate_players_existing_novar(p, nr_players, game())++ plr_in_game;

	update_player_stat(gametime);

	const uint32_t my_power = player_statistics.get_modified_player_power(pn);

	// first we scan vicitnity of couple of militarysites to get new enemy sites
	// Militarysites rotate (see check_militarysites())
	int32_t i = 0;
	for (MilitarySiteObserver mso : militarysites) {
		++i;
		if (i % 4 == 0)
			continue;
		if (i > 20)
			continue;

		MilitarySite* ms = mso.site;
		uint32_t const vision = ms->descr().vision_range();
		FCoords f = map.get_fcoords(ms->get_position());

		// get list of immovable around this our military site
		static std::vector<ImmovableFound> immovables;
		immovables.clear();
		immovables.reserve(40);
		map.find_immovables(game(), Area<FCoords>(f, (vision + 3 < 13) ? 13 : vision + 3),
		                    &immovables, FindImmovableAttackTarget());

		for (uint32_t j = 0; j < immovables.size(); ++j) {
			if (upcast(MilitarySite const, bld, immovables.at(j).object)) {
				const PlayerNumber opn = bld->owner().player_number();
				if (player_statistics.get_is_enemy(opn)) {
					assert(opn != pn);
					player_statistics.set_last_time_seen(gametime, opn);
					if (enemy_sites.count(bld->get_position().hash()) == 0) {
						enemy_sites[bld->get_position().hash()] = EnemySiteObserver();
					} else {
						enemy_sites[bld->get_position().hash()].last_time_seen = gametime;
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
					} else {
						enemy_sites[wh->get_position().hash()].last_time_seen = gametime;
					}
				}
			}
		}
	}

	// now we update some of them
	Widelands::Serial best_target = Widelands::kInvalidSerial;
	uint8_t best_score = 0;
	uint32_t count = 0;
	// sites that were either conquered or destroyed
	static std::vector<uint32_t> disappeared_sites;
	disappeared_sites.clear();
	disappeared_sites.reserve(6);

	// Willingness to attack depend on how long ago the last soldier has been trained. This is used
	// as indicator how busy our trainingsites are.
	// Moreover the stronger AI the more sensitive to it it is (a score of attack willingness is more
	// decreased if promotion of soldiers is stalled)
	int8_t general_score = 0;
	if (soldier_trained_log.count(gametime) == 0) {
		// No soldier was trained lately ...
		switch (type_) {
		case Widelands::AiType::kNormal:
			general_score = 1;
			break;
		case Widelands::AiType::kWeak:
			general_score = 0;
			break;
		case Widelands::AiType::kVeryWeak:
			general_score = -1;
		}
	}

	const bool strong_enough = player_statistics.strong_enough(pn);

	// removing sites we saw too long ago
	for (std::map<uint32_t, EnemySiteObserver>::iterator site = enemy_sites.begin();
	     site != enemy_sites.end(); ++site) {
		if (site->second.last_time_seen + 20 * 60 * 1000 < gametime) {
			disappeared_sites.push_back(site->first);
		}
	}
	while (!disappeared_sites.empty()) {
		enemy_sites.erase(disappeared_sites.back());
		disappeared_sites.pop_back();
	}

	for (std::map<uint32_t, EnemySiteObserver>::iterator site = enemy_sites.begin();
	     site != enemy_sites.end(); ++site) {

		assert(site->second.last_time_attacked <= gametime);
		// Do not attack too soon
		if (std::min<uint32_t>(site->second.attack_counter, 10) * 20 * 1000 >
		    (gametime - site->second.last_time_attacked)) {
			continue;
		}

		// we test max 12 sites and prefer ones tested more then 1 min ago
		if (((site->second.last_tested + (enemysites_check_delay_ * 1000)) > gametime && count > 4) ||
		    count > 12) {
			continue;
		}

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

		Flag* flag = nullptr;

		if (upcast(MilitarySite, bld, f.field->get_immovable())) {
			if (player_->is_hostile(bld->owner())) {
				std::vector<Soldier*> defenders;
				defenders = bld->soldier_control()->present_soldiers();
				defenders_strength = calculate_strength(defenders);

				flag = &bld->base_flag();
				if (is_visible && bld->attack_target()->can_be_attacked()) {
					is_attackable = true;
				}
				owner_number = bld->owner().player_number();
			}
		}
		if (upcast(Warehouse, wh, f.field->get_immovable())) {
			if (player_->is_hostile(wh->owner())) {

				std::vector<Soldier*> defenders;
				defenders = wh->soldier_control()->present_soldiers();
				defenders_strength = calculate_strength(defenders);

				flag = &wh->base_flag();
				is_warehouse = true;
				if (is_visible && wh->attack_target()->can_be_attacked()) {
					is_attackable = true;
				}
				owner_number = wh->owner().player_number();
			}
		}

		// if flag is defined it is a good taget
		if (flag) {

			// Site is still there but not visible for us
			if (!is_visible) {
				if (site->second.last_time_seen + 20 * 60 * 1000 < gametime) {
					log("site %d not visible for more than 20 minutes\n", site->first);
					disappeared_sites.push_back(site->first);
				}
				continue;
			}

			// updating some info
			// updating info on mines nearby if needed
			if (site->second.mines_nearby == ExtendedBool::kUnset) {
				FindNodeMineable find_mines_spots_nearby(game(), f.field->get_resources());
				const int32_t minescount =
				   map.find_fields(game(), Area<FCoords>(f, 6), nullptr, find_mines_spots_nearby);
				if (minescount > 0) {
					site->second.mines_nearby = ExtendedBool::kTrue;
				} else {
					site->second.mines_nearby = ExtendedBool::kFalse;
				}
			}

			site->second.is_warehouse = is_warehouse;

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
				}
			} else {
				site->second.attack_soldiers_strength = 0;
			}

			site->second.defenders_strength = defenders_strength;

			site->second.score = 0;

			if (site->second.attack_soldiers_strength > 0 &&
			    !player_statistics.players_in_same_team(pn, owner_number)) {

				const uint16_t enemys_power = player_statistics.get_modified_player_power(owner_number);
				uint16_t my_to_enemy_power_ratio = 100;
				if (enemys_power) {
					my_to_enemy_power_ratio = my_power * 100 / enemys_power;
				}
				uint16_t enemys_power_growth = 10;
				if (player_statistics.get_old60_player_land(owner_number)) {
					enemys_power_growth = player_statistics.get_player_power(owner_number) * 100 /
					                      player_statistics.get_old60_player_land(owner_number);
				}
				uint16_t own_power_growth = 10;
				if (player_statistics.get_old60_player_land(pn)) {
					enemys_power_growth = player_statistics.get_player_power(pn) * 100 /
					                      player_statistics.get_old60_player_land(pn);
				}

				static int16_t inputs[3 * kFNeuronBitSize] = {0};
				// Reseting values as the variable is static
				for (int j = 0; j < 3 * kFNeuronBitSize; j++) {
					inputs[j] = 0;
				}
				inputs[0] = (site->second.attack_soldiers_strength - site->second.defenders_strength) *
				            std::abs(management_data.get_military_number_at(114)) / 30;
				inputs[1] = (site->second.attack_soldiers_strength - site->second.defenders_strength) *
				            std::abs(management_data.get_military_number_at(115)) / 30;
				inputs[2] = (is_warehouse) ? 4 : 0;
				inputs[3] = (is_warehouse) ? 2 : 0;
				inputs[4] = (site->second.attack_soldiers_competency > 15) ? 2 : 0;
				inputs[5] = (site->second.attack_soldiers_competency > 25) ? 4 : 0;
				inputs[6] =
				   (2 * site->second.defenders_strength > 3 * site->second.attack_soldiers_strength) ?
				      2 :
				      0;
				inputs[7] =
				   (3 * site->second.defenders_strength > 2 * site->second.attack_soldiers_strength) ?
				      2 :
				      0;
				inputs[8] = (soldier_status_ == SoldiersStatus::kBadShortage ||
				             soldier_status_ == SoldiersStatus::kShortage) ?
				               -2 :
				               0;
				inputs[8] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -2 : 0;
				inputs[9] = (soldier_status_ == SoldiersStatus::kBadShortage ||
				             soldier_status_ == SoldiersStatus::kShortage) ?
				               -3 :
				               0;
				inputs[10] = (site->second.mines_nearby == ExtendedBool::kTrue) ? 3 : -1;
				inputs[11] = (site->second.mines_nearby == ExtendedBool::kTrue) ? 0 : 0;
				inputs[12] = (owner_number == persistent_data->last_attacked_player) ? 2 : -2;
				inputs[13] = (owner_number == persistent_data->last_attacked_player) ? 4 : -4;
				inputs[14] = (strong_enough) ? 1 : -1;
				inputs[15] = (strong_enough) ? 3 : -3;
				inputs[16] = (player_statistics.get_player_power(pn) >
				              player_statistics.get_old_player_power(pn)) ?
				                2 :
				                -2;
				inputs[17] = (player_statistics.get_player_power(pn) >
				              player_statistics.get_old60_player_power(pn)) ?
				                3 :
				                -3;
				inputs[18] = (player_statistics.get_visible_enemies_power(pn) >
				              player_statistics.get_old_visible_enemies_power(pn)) ?
				                -1 :
				                1;
				inputs[19] = (player_statistics.get_visible_enemies_power(pn) >
				              player_statistics.get_old_visible_enemies_power(pn)) ?
				                -3 :
				                3;
				inputs[20] = (player_statistics.get_player_power(owner_number) >
				              player_statistics.get_old_player_power(owner_number)) ?
				                -2 :
				                2;
				inputs[21] = (player_statistics.get_player_power(owner_number) >
				              player_statistics.get_old_player_power(owner_number)) ?
				                -1 :
				                1;
				inputs[22] = (my_to_enemy_power_ratio > 80) ? 2 : -2;
				inputs[23] = (my_to_enemy_power_ratio > 90) ? 2 : -2;
				inputs[24] = (my_to_enemy_power_ratio > 110) ? 2 : -2;
				inputs[55] = (my_to_enemy_power_ratio > 120) ? 2 : -2;
				inputs[26] = management_data.get_military_number_at(62) / 10;
				inputs[27] = (ts_finished_count_ - ts_without_trainers_) * 2;
				inputs[28] = general_score * 3;
				inputs[29] = general_score;
				inputs[30] = ((mines_per_type[iron_resource_id].in_construction +
				               mines_per_type[iron_resource_id].finished) > 0) ?
				                1 :
				                -1;
				inputs[31] = (player_statistics.get_player_power(pn) >
				              player_statistics.get_old60_player_power(pn) + 5) ?
				                2 :
				                -2;
				inputs[32] = soldier_trained_log.count(gametime);
				inputs[33] = soldier_trained_log.count(gametime) / 2;
				inputs[34] = general_score * 2;
				inputs[35] = -1;
				inputs[36] = (gametime < 15 * 60 * 1000) ? -1 : 0;
				inputs[37] = (gametime < 20 * 60 * 1000) ? -1 : 0;
				inputs[38] = (gametime < 25 * 60 * 1000) ? -1 : 0;
				inputs[39] = (gametime < 30 * 60 * 1000) ? -1 : 0;
				inputs[40] = (gametime < 35 * 60 * 1000) ? -1 : 0;
				inputs[41] = (gametime < 40 * 60 * 1000) ? -1 : 0;
				inputs[42] = (site->second.last_time_attacked + 1 * 60 * 1000 > gametime) ? -3 : 0;
				inputs[43] = (site->second.last_time_attacked + 30 * 1000 > gametime) ? -1 : 0;
				inputs[44] = (site->second.last_time_attacked + 2 * 60 * 1000 > gametime) ? -2 : 0;
				inputs[45] = (site->second.last_time_attacked + 40 * 1000 > gametime) ? -1 : 0;
				inputs[46] = (site->second.last_time_attacked + 3 * 60 * 1000 > gametime) ? -1 : 0;
				inputs[47] = (site->second.last_time_attacked + 30 * 1000 > gametime) ? -1 : 0;
				inputs[48] = (site->second.last_time_attacked + 90 * 1000 > gametime) ? -1 : 0;
				inputs[49] = (site->second.last_time_attacked + 2 * 60 * 1000 > gametime) ? -1 : 0;
				inputs[50] = soldier_trained_log.count(gametime);
				inputs[51] = soldier_trained_log.count(gametime) / 2;
				inputs[52] = (my_to_enemy_power_ratio - 100) / 50;
				inputs[53] = (my_to_enemy_power_ratio > 60) ? 0 : -4;
				inputs[54] = (my_to_enemy_power_ratio > 70) ? 0 : -3;
				inputs[55] = (my_to_enemy_power_ratio > 80) ? 2 : -2;
				inputs[56] = (my_to_enemy_power_ratio > 90) ? 2 : -2;
				inputs[57] = (my_to_enemy_power_ratio > 100) ? 2 : -2;
				inputs[58] = (my_to_enemy_power_ratio > 110) ? 2 : -2;
				inputs[59] = (my_to_enemy_power_ratio > 120) ? 2 : -2;
				inputs[60] = (my_to_enemy_power_ratio > 130) ? 2 : -2;
				inputs[61] = (my_to_enemy_power_ratio > 140) ? 3 : 0;
				inputs[62] = (my_to_enemy_power_ratio > 150) ? 4 : 0;
				inputs[63] = (my_to_enemy_power_ratio - 100) / 50;
				inputs[64] = (enemys_power_growth > 105) ? -1 : 0;
				inputs[65] = (enemys_power_growth > 110) ? -2 : 0;
				inputs[66] = (enemys_power_growth > 115) ? -1 : 0;
				inputs[67] = (enemys_power_growth > 120) ? -2 : 0;
				inputs[68] = (enemys_power_growth < 95) ? 1 : 0;
				inputs[69] = (enemys_power_growth < 90) ? 2 : 0;
				inputs[70] = (enemys_power_growth < 85) ? 1 : 0;
				inputs[71] = (enemys_power_growth < 80) ? 2 : 0;
				inputs[72] = (own_power_growth > 105) ? 1 : 0;
				inputs[73] = (own_power_growth > 110) ? 2 : 0;
				inputs[74] = (own_power_growth > 115) ? 1 : 0;
				inputs[75] = (own_power_growth > 120) ? 2 : 0;
				inputs[76] = (own_power_growth < 95) ? -1 : 0;
				inputs[77] = (own_power_growth < 90) ? -2 : 0;
				inputs[77] = (own_power_growth < 85) ? -1 : 0;
				inputs[78] = (own_power_growth < 80) ? -2 : 0;
				inputs[79] = ((gametime - last_attack_time_) < kCampaignDuration) ? +2 : -2;
				inputs[80] = -1;
				inputs[81] = +1;
				inputs[82] = -1;
				inputs[83] = (soldier_status_ == SoldiersStatus::kBadShortage ||
				              soldier_status_ == SoldiersStatus::kShortage) ?
				                -3 :
				                1;
				inputs[84] = (soldier_status_ == SoldiersStatus::kBadShortage ||
				              soldier_status_ == SoldiersStatus::kShortage) ?
				                -4 :
				                1;
				inputs[85] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -2 : 1;
				inputs[86] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -4 : 1;
				inputs[87] = (soldier_status_ == SoldiersStatus::kBadShortage ||
				              soldier_status_ == SoldiersStatus::kShortage) ?
				                -2 :
				                1;
				inputs[88] = (site->second.attack_soldiers_strength < 2) ? -3 : 0;
				inputs[89] = (site->second.attack_soldiers_strength < 4) ? -2 : 0;
				inputs[90] = (site->second.attack_soldiers_strength < 5) ? -3 : 0;
				inputs[90] = (site->second.attack_soldiers_strength < 7) ? -3 : 0;
				inputs[91] = (site->second.attack_soldiers_competency < 15) ? -4 : 0;
				inputs[92] = (site->second.attack_soldiers_competency < 20) ? -2 : 0;
				inputs[93] = ((gametime - last_attack_time_) < kCampaignDuration) ? +2 : -2;
				inputs[94] = ((gametime - last_attack_time_) < kCampaignDuration) ? +2 : -2;
				inputs[95] = -player_statistics.enemies_seen_lately_count(gametime);

				site->second.score = 0;
				for (uint8_t j = 0; j < kFNeuronBitSize; ++j) {
					if (management_data.f_neuron_pool[47].get_position(j)) {
						site->second.score += inputs[j];
						if (inputs[j] < -10 || inputs[j] > 10) {
							log(" pos: %d - value %d\n", j, inputs[j]);
						}
					}
					if (management_data.f_neuron_pool[0].get_position(j)) {
						site->second.score += inputs[j + kFNeuronBitSize];
						if (inputs[j + kFNeuronBitSize] < -10 || inputs[j + kFNeuronBitSize] > 10) {
							log(" pos: %d - value %d\n", j + kFNeuronBitSize, inputs[j + kFNeuronBitSize]);
						}
					}
					if (management_data.f_neuron_pool[16].get_position(j)) {
						site->second.score += inputs[j + 2 * kFNeuronBitSize];
						if (inputs[j + 2 * kFNeuronBitSize] < -10 ||
						    inputs[j + 2 * kFNeuronBitSize] > 10) {
							log(" pos: %d - value %d\n", j + 2 * kFNeuronBitSize,
							    inputs[j + 2 * kFNeuronBitSize]);
						}
					}
				}
			}
			site->second.score += (management_data.get_military_number_at(138) +
			                       management_data.get_military_number_at(159)) /
			                      8;

			if (site->second.score > 0) {
				assert(is_visible);
				if (site->second.score > best_score) {
					best_score = site->second.score;
					best_target = site->first;
				}
			}

		} else {  // we don't have a flag = site does not exist anymore, let's remove the site from
			       // our observer list
			disappeared_sites.push_back(site->first);
		}
	}

	while (!disappeared_sites.empty()) {
		enemy_sites.erase(disappeared_sites.back());
		disappeared_sites.pop_back();
	}

	// modifying enemysites_check_delay_,this depends on the count
	// of enemysites in observer
	if (enemy_sites.size() >= 13 && enemysites_check_delay_ < 180) {
		enemysites_check_delay_ += 3;
	}
	if (enemy_sites.size() < 10 && enemysites_check_delay_ > 30) {
		enemysites_check_delay_ -= 2;
	}

	// if coordinates hash is not set
	if (best_target == Widelands::kInvalidSerial) {
		return false;
	}

	assert(enemy_sites.count(best_target) > 0);

	// attacking
	FCoords f = map.get_fcoords(Coords::unhash(best_target));

	Flag* flag = nullptr;  // flag of a building to be attacked
	if (upcast(MilitarySite, bld, f.field->get_immovable())) {
		flag = &bld->base_flag();
	} else if (upcast(Warehouse, Wh, f.field->get_immovable())) {
		flag = &Wh->base_flag();
	} else {
		return false;  // this should not happen
	}

	// how many attack soldiers we can send?
	std::vector<Soldier*> soldiers;
	int32_t attackers = player_->find_attack_soldiers(*flag, &soldiers);
	assert(attackers < 500);

	if (attackers > 5) {
		attackers = 5 + std::rand() % (attackers - 5);
	}

	assert(attackers < 500);

	if (attackers <= 0) {
		return false;
	}

	log("%2d: attacking site at %3dx%3d, score %3d, with %2d soldiers, attacking %2d times, after "
	    "%5d seconds\n",
	    player_number(), flag->get_position().x, flag->get_position().y, best_score, attackers,
	    enemy_sites[best_target].attack_counter + 1,
	    (gametime - enemy_sites[best_target].last_time_attacked) / 1000);
	std::vector<Serial> attacking_soldiers;
	for (int a = 0; a < attackers; ++a) {
		// TODO(Nordfriese): We could now choose the soldiers we want to send
		attacking_soldiers.push_back(soldiers[a]->serial());
	}
	game().send_player_enemyflagaction(*flag, player_number(), attacking_soldiers);
	assert(1 <
	       player_->vision(Map::get_index(flag->get_building()->get_position(), map.get_width())));
	attackers_count_ += attackers;
	enemy_sites[best_target].last_time_attacked = gametime;
	++enemy_sites[best_target].attack_counter;

	last_attack_time_ = gametime;
	for (int j = 0; j < attackers; ++j) {
		soldier_attacks_log.push(gametime);
	}
	persistent_data->last_attacked_player = flag->owner().player_number();

	return true;
}
// this just counts free positions in military and training sites
void DefaultAI::count_military_vacant_positions() {
	// counting vacant positions
	int32_t vacant_mil_positions_ = 0;
	int32_t understaffed_ = 0;
	int32_t on_stock_ = 0;
	for (TrainingSiteObserver tso : trainingsites) {
		vacant_mil_positions_ +=
		   5 * std::min<int32_t>((tso.site->soldier_control()->soldier_capacity() -
		                          tso.site->soldier_control()->stationed_soldiers().size()),
		                         2);
	}
	for (MilitarySiteObserver mso : militarysites) {
		vacant_mil_positions_ += mso.site->soldier_control()->soldier_capacity() -
		                         mso.site->soldier_control()->stationed_soldiers().size();
		understaffed_ += mso.understaffed;
	}
	vacant_mil_positions_ += understaffed_;

	// also available in warehouses
	for (auto wh : warehousesites) {
		on_stock_ += wh.site->soldier_control()->stationed_soldiers().size();
	}

	vacant_mil_positions_ += on_stock_;

	// to avoid floats this is actual number * 100
	vacant_mil_positions_average_ =
	   vacant_mil_positions_average_ * 8 / 10 + 20 * vacant_mil_positions_;

	if (vacant_mil_positions_ <= 1 || on_stock_ > 4) {
		soldier_status_ = SoldiersStatus::kFull;
	} else if (vacant_mil_positions_ * 4 <= static_cast<int32_t>(militarysites.size()) ||
	           on_stock_ > 2) {
		soldier_status_ = SoldiersStatus::kEnough;
	} else if (vacant_mil_positions_ > static_cast<int32_t>(militarysites.size())) {
		soldier_status_ = SoldiersStatus::kBadShortage;
	} else {
		soldier_status_ = SoldiersStatus::kShortage;
	}

	assert(soldier_status_ == SoldiersStatus::kFull || soldier_status_ == SoldiersStatus::kEnough ||
	       soldier_status_ == SoldiersStatus::kShortage ||
	       soldier_status_ == SoldiersStatus::kBadShortage);
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

	// Inform if we are above ai type limit.
	if (tso.bo->total_count() > tso.bo->cnt_limit_by_aimode) {
		log("AI check_trainingsites: AI player %d: count of %s exceeds an AI limit %d: actual count: "
		    "%d\n",
		    player_number(), tso.bo->name, tso.bo->cnt_limit_by_aimode, tso.bo->total_count());
	}

	const DescriptionIndex enhancement = ts->descr().enhancement();

	if (enhancement != INVALID_INDEX && ts_without_trainers_ == 0 && mines_.size() > 3 &&
	    ts_finished_count_ > 1 && ts_in_const_count_ == 0) {

		// Make sure that:
		// 1. Building is allowed
		// 2. AI limit for weaker AI is not to be exceeded
		BuildingObserver& en_bo =
		   get_building_observer(tribe_->get_building_descr(enhancement)->name().c_str());
		uint16_t current_proportion =
		   en_bo.total_count() * 100 / (ts_finished_count_ + ts_in_const_count_);
		if (player_->is_building_type_allowed(enhancement) &&
		    en_bo.aimode_limit_status() == AiModeBuildings::kAnotherAllowed &&
		    en_bo.max_trainingsites_proportion > current_proportion) {
			game().send_player_enhance_building(*tso.site, enhancement);
		}
	}

	// changing capacity to 0 - this will happen only once.....
	if (tso.site->soldier_control()->soldier_capacity() > 1) {
		game().send_player_change_soldier_capacity(
		   *ts, -tso.site->soldier_control()->soldier_capacity());
		return true;
	}

	// reducing ware queues
	// - for armours and weapons to 1
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

	// are we willing to train another soldier?
	// bool want_train = true;
	const PlayerNumber pn = player_number();

	// if soldier capacity is set to 0, we need to find out if the site is
	// supplied enough to incrase the capacity to 1
	if (tso.site->soldier_control()->soldier_capacity() == 0) {

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

			int16_t inputs[kFNeuronBitSize] = {0};
			inputs[0] = -shortage;
			inputs[1] = (player_statistics.get_visible_enemies_power(gametime) >
			             player_statistics.get_old_visible_enemies_power(gametime)) ?
			               1 :
			               0;
			inputs[2] = (mines_.size() < 3) ? -1 : 0;
			inputs[3] = (mines_per_type[iron_resource_id].total_count() == 0) ? -1 : 0;
			inputs[4] = (player_statistics.get_player_power(pn) * 2 >
			             player_statistics.get_visible_enemies_power(gametime)) ?
			               -1 :
			               0;
			inputs[5] = (player_statistics.get_player_power(pn) * 2 >
			             player_statistics.get_enemies_average_power()) ?
			               -1 :
			               0;
			inputs[6] = (player_statistics.get_player_power(pn) >
			             player_statistics.get_visible_enemies_power(gametime)) ?
			               -1 :
			               1;
			inputs[7] = (player_statistics.get_player_power(pn) >
			             player_statistics.get_enemies_average_power()) ?
			               -1 :
			               1;
			inputs[8] = (player_statistics.get_player_power(pn) * 2 >
			             player_statistics.get_visible_enemies_power(gametime)) ?
			               -1 :
			               1;
			inputs[9] = (player_statistics.get_player_power(pn) * 2 >
			             player_statistics.get_enemies_average_power()) ?
			               -1 :
			               1;
			inputs[10] = (3 - shortage) * 1;
			inputs[11] = (3 - shortage) * 2;
			inputs[12] = +1;
			inputs[13] = +2;
			inputs[14] = -1;
			inputs[15] = -2;
			inputs[16] = (player_statistics.get_player_power(pn) <
			              player_statistics.get_old60_player_power(pn)) ?
			                1 :
			                0;
			inputs[17] =
			   (player_statistics.get_player_power(pn) < player_statistics.get_old_player_power(pn)) ?
			      1 :
			      0;
			inputs[18] = (player_statistics.get_player_power(pn) <
			              player_statistics.get_old60_player_power(pn) + 4) ?
			                1 :
			                0;
			inputs[19] = (player_statistics.get_player_power(pn) <
			              player_statistics.get_old_player_power(pn) + 1) ?
			                1 :
			                0;
			inputs[20] = (player_statistics.get_player_power(pn) <
			              player_statistics.get_old60_player_power(pn) + 4) ?
			                0 :
			                1;
			inputs[21] = (player_statistics.get_player_power(pn) <
			              player_statistics.get_old_player_power(pn) + 2) ?
			                0 :
			                1;

			if (player_statistics.any_enemy_seen_lately(gametime)) {
				inputs[20] = (player_statistics.get_player_power(pn) <
				              player_statistics.get_old60_player_power(pn) + 4) ?
				                0 :
				                1;
				inputs[21] = (player_statistics.get_player_power(pn) <
				              player_statistics.get_old_player_power(pn) + 2) ?
				                0 :
				                1;
				inputs[22] = (player_statistics.get_player_power(pn) <
				              player_statistics.get_old60_player_power(pn) + 4) ?
				                1 :
				                0;
				inputs[23] = (player_statistics.get_player_power(pn) <
				              player_statistics.get_old_player_power(pn) + 2) ?
				                1 :
				                0;
			}

			int16_t tmp_score = 0;
			for (uint8_t i = 0; i < kFNeuronBitSize; ++i) {
				if (management_data.f_neuron_pool[29].get_position(i)) {
					tmp_score += inputs[i];
				}
			}

			if (tmp_score > 0) {
				game().send_player_change_soldier_capacity(*ts, 1);
			}
		}
	}

	ts_without_trainers_ = 0;  // zeroing
	for (std::deque<TrainingSiteObserver>::iterator site = trainingsites.begin();
	     site != trainingsites.end(); ++site) {

		if (!site->site->can_start_working()) {
			++ts_without_trainers_;
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

	// Check next militarysite
	bool changed = false;
	MilitarySite* ms = militarysites.front().site;

	// Don't do anything if last change took place lately
	if (militarysites.front().last_change + 2 * 60 * 1000 > gametime) {
		militarysites.push_back(militarysites.front());
		militarysites.pop_front();
		return false;
	}

	// Make sure we have statistics about our enemies up-to-date
	update_player_stat(gametime);

	// Inform if we are above ai type limit.
	if (militarysites.front().bo->total_count() > militarysites.front().bo->cnt_limit_by_aimode) {
		log("AI check_militarysites: Too many %s: %d, ai limit: %d\n", militarysites.front().bo->name,
		    militarysites.front().bo->total_count(), militarysites.front().bo->cnt_limit_by_aimode);
	}

	FCoords f = game().map().get_fcoords(ms->get_position());

	BuildableField bf(f);
	update_buildable_field(bf);

	Quantity const total_capacity = ms->soldier_control()->max_soldier_capacity();
	Quantity const current_target = ms->soldier_control()->soldier_capacity();
	Quantity const current_soldiers = ms->soldier_control()->present_soldiers().size();
	Quantity target_occupancy = total_capacity;
	if (soldier_status_ == SoldiersStatus::kBadShortage) {
		target_occupancy = total_capacity / 3 + 1;
	} else if (soldier_status_ == SoldiersStatus::kShortage) {
		target_occupancy = total_capacity * 2 / 3 + 1;
	}

	militarysites.front().understaffed = 0;

	const bool can_be_dismantled =
	   (current_soldiers == 1 || militarysites.front().built_time + 10 * 60 * 1000 < gametime) &&
	   bf.military_loneliness < 1000 - 2 * std::abs(management_data.get_military_number_at(14));

	bool should_be_dismantled = false;
	const int32_t enemy_military_capacity = std::max<int32_t>(
	   {bf.enemy_military_presence,
	    bf.enemy_military_sites * (1 + std::abs(management_data.get_military_number_at(77) / 20)),
	    (bf.enemy_owned_land_nearby) ?
	       4 + std::abs(management_data.get_military_number_at(99) / 20) :
	       0});
	if (bf.enemy_owned_land_nearby) {
		if (bf.military_score_ < std::abs(management_data.get_military_number_at(91) * 10) &&
		    bf.area_military_capacity - static_cast<int16_t>(total_capacity) -
		          std::abs(management_data.get_military_number_at(84) / 10) >
		       (std::abs(management_data.get_military_number_at(24) / 25) + 1) *
		          enemy_military_capacity) {
			should_be_dismantled = true;
		}
	} else {
		const uint16_t size_bonus =
		   total_capacity * std::abs(management_data.get_military_number_at(89)) / 5;
		if (bf.military_score_ + size_bonus < management_data.get_military_number_at(88) * 5 &&
		    bf.area_military_capacity > static_cast<int16_t>(total_capacity)) {
			should_be_dismantled = true;
		}
	}

	if (bf.enemy_accessible_ && !should_be_dismantled) {

		assert(total_capacity >= target_occupancy);

		militarysites.front().understaffed = total_capacity - target_occupancy;

		if (current_target < target_occupancy) {
			game().send_player_change_soldier_capacity(*ms, 1);
			changed = true;
		}
		if (current_target > target_occupancy) {
			game().send_player_change_soldier_capacity(*ms, -1);
			changed = true;
		}
		if (ms->get_soldier_preference() == SoldierPreference::kRookies) {
			game().send_player_militarysite_set_soldier_preference(*ms, SoldierPreference::kHeroes);
			changed = true;
		}
	} else if (should_be_dismantled && can_be_dismantled) {
		changed = true;
		if (ms->get_playercaps() & Widelands::Building::PCap_Dismantle) {
			game().send_player_dismantle(*ms);
			military_last_dismantle_ = game().get_gametime();
		} else {
			game().send_player_bulldoze(*ms);
			military_last_dismantle_ = game().get_gametime();
		}
	} else {
		if (current_target > 1) {  // reduce number of soldiers here at least....
			game().send_player_change_soldier_capacity(*ms, -1);
			changed = true;
		}
		if (ms->get_soldier_preference() == SoldierPreference::kHeroes) {
			game().send_player_militarysite_set_soldier_preference(*ms, SoldierPreference::kRookies);
			changed = true;
		}
	}
	if (changed) {
		militarysites.front().last_change = gametime;
	}

	// reorder:;
	militarysites.push_back(militarysites.front());
	militarysites.pop_front();
	return changed;
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

	const SoldierDescr& descr = soldiers.front()->descr();

	for (Soldier* soldier : soldiers) {
		health = soldier->get_current_health();
		attack = (descr.get_base_max_attack() - descr.get_base_min_attack()) / 2.f +
		         descr.get_base_min_attack() +
		         descr.get_attack_incr_per_level() * soldier->get_attack_level();
		defense = 100 - descr.get_base_defense() -
		          descr.get_defense_incr_per_level() * soldier->get_defense_level();
		evade = 100 - descr.get_base_evade() -
		        descr.get_evade_incr_per_level() / 100.f * soldier->get_evade_level();
		final += (attack * health) / (defense * evade);
	}

	assert(final >= 0);
	assert(final <=
	       soldiers.size() * (descr.get_base_max_attack() * descr.get_base_health() +
	                          descr.get_max_attack_level() * descr.get_attack_incr_per_level() +
	                          descr.get_max_health_level() * descr.get_health_incr_per_level()));

	// We divide the result by the aproximate strength of one unpromoted soldier
	const int average_unpromoted_strength =
	   (descr.get_base_min_attack() +
	    (descr.get_base_max_attack() - descr.get_base_min_attack()) / 2) *
	   descr.get_base_health() / (descr.get_base_defense() * descr.get_base_evade());

	return static_cast<int32_t>(final / average_unpromoted_strength);
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

	if (military_last_build_ >
	    gametime - (10 + std::abs(management_data.get_military_number_at(43)) * 1000 / 2)) {
		return BuildingNecessity::kForbidden;
	}

	// Skip if the building is prohibited for now
	if (bo.prohibited_till > gametime) {
		return BuildingNecessity::kForbidden;
	}

	bo.primary_priority = 0;

	const uint32_t msites_total = msites_built() + msites_in_constr();
	const uint16_t scores[3] = {
	   static_cast<uint16_t>(msites_per_size[1].in_construction + msites_per_size[1].finished),
	   static_cast<uint16_t>((msites_per_size[2].in_construction + msites_per_size[2].finished) * 2),
	   static_cast<uint16_t>((msites_per_size[3].in_construction + msites_per_size[3].finished) *
	                         3)};
	const uint16_t total_score = scores[0] + scores[1] + scores[2];

	static int32_t inputs[4 * kFNeuronBitSize] = {0};
	// Reseting values as the variable is static
	for (int i = 0; i < 4 * kFNeuronBitSize; i++) {
		inputs[i] = 0;
	}
	inputs[0] = (msites_total < 1) ? 1 : 0;
	inputs[1] = (msites_total < 2) ? 1 : 0;
	inputs[2] = (msites_total < 3) ? 1 : 0;
	inputs[3] = (msites_total < 4) ? 1 : 0;
	inputs[3] = (msites_total < 5) ? 1 : 0;
	inputs[4] = (msites_in_constr() > msites_built()) ? -1 : 0;
	inputs[5] = -3;
	inputs[6] = (msites_in_constr() > msites_built() / 2) ? -1 : 0;
	inputs[7] = (msites_in_constr() > msites_built() / 3) ? -1 : 0;
	inputs[8] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -2 : 0;
	inputs[9] = (soldier_status_ == SoldiersStatus::kShortage) ? -1 : 0;
	inputs[10] = (scores[size - 1] > total_score) ? -2 : 0;
	inputs[11] = (scores[size - 1] > total_score / 2) ? -2 : 0;
	inputs[12] = (scores[size - 1] > total_score / 3) ? -2 : 0;
	inputs[13] =
	   (player_statistics.get_enemies_max_land() < player_statistics.get_player_land(pn)) ? -1 : 0;
	inputs[14] = (mines_per_type[iron_resource_id].total_count() == 0) ? +1 : 0;
	inputs[15] = (spots_ < kSpotsTooLittle) ? +1 : 0;
	inputs[16] = +1;
	inputs[17] = +2;
	inputs[18] = -1;
	inputs[19] = -2;
	inputs[20] = (scores[size - 1] > total_score / 2) ? -1 : 0;
	inputs[21] = (msites_in_constr() > msites_built() / 3) ? -1 : 0;
	inputs[22] = (scores[size - 1] > total_score / 4) ? -1 : 0;
	inputs[23] = (((3 - size) * msites_in_constr()) < 1) ? +1 : 0;
	inputs[24] = (3 - size) * ((msites_in_constr() < 3) ? +1 : 0);
	inputs[25] = (((3 - size) * msites_in_constr()) < 5) ? +1 : 0;
	inputs[26] = (msites_in_constr() < 7) ? +1 : 0;
	inputs[27] = +5;
	inputs[28] = -5;
	inputs[29] = +3;
	inputs[30] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -4 : 0;
	inputs[31] = (soldier_status_ == SoldiersStatus::kShortage) ? -2 : 0;

	inputs[32] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -3 : 0;
	inputs[33] = (soldier_status_ == SoldiersStatus::kShortage) ? -2 : 0;

	inputs[34] =
	   (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land()) ? 1 : 0;
	inputs[35] =
	   (!player_statistics.any_enemy_seen_lately(gametime) &&
	    (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land())) ?
	      2 :
	      0;
	inputs[36] =
	   (!player_statistics.any_enemy_seen_lately(gametime)) &&
	         (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land() * 2) ?
	      1 :
	      0;
	inputs[37] =
	   (!player_statistics.any_enemy_seen_lately(gametime)) &&
	         (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land() / 2) ?
	      1 :
	      0;

	inputs[38] = (!player_statistics.any_enemy_seen_lately(gametime)) &&
	                   (player_statistics.get_player_land(pn) <
	                    player_statistics.get_old_player_land(pn) * 105 / 100) ?
	                2 :
	                0;
	inputs[39] = (!player_statistics.any_enemy_seen_lately(gametime)) &&
	                   (player_statistics.get_player_land(pn) <
	                    player_statistics.get_old_player_land(pn) + 110) ?
	                3 :
	                0;

	inputs[40] =
	   (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn)) ? 1 :
	                                                                                             0;
	inputs[41] =
	   (player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn)) ? 1 :
	                                                                                             0;
	inputs[42] = (!player_statistics.any_enemy_seen_lately(gametime)) &&
	                   (player_statistics.get_player_power(pn) <
	                    player_statistics.get_old60_player_power(pn)) ?
	                1 :
	                0;
	inputs[43] = (!player_statistics.any_enemy_seen_lately(gametime)) &&
	                   (player_statistics.get_player_power(pn) >
	                    player_statistics.get_old60_player_power(pn)) ?
	                1 :
	                0;

	inputs[44] =
	   (player_statistics.get_player_land(pn) < player_statistics.get_enemies_average_land()) ? 1 :
	                                                                                            0;
	inputs[45] =
	   (player_statistics.get_player_land(pn) > player_statistics.get_enemies_average_land()) ? 1 :
	                                                                                            0;

	inputs[46] =
	   (!player_statistics.any_enemy_seen_lately(gametime)) &&
	         (player_statistics.get_player_land(pn) < player_statistics.get_enemies_average_land()) ?
	      2 :
	      0;
	inputs[47] =
	   (!player_statistics.any_enemy_seen_lately(gametime)) &&
	         (player_statistics.get_player_land(pn) > player_statistics.get_enemies_average_land()) ?
	      2 :
	      0;

	inputs[48] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -3 : 0;
	inputs[49] = (soldier_status_ == SoldiersStatus::kShortage) ? -2 : 0;

	inputs[50] = (!player_statistics.any_enemy_seen_lately(gametime)) &&
	                   (player_statistics.get_player_land(pn) <
	                    player_statistics.get_old_player_land(pn) * 110 / 100) ?
	                1 :
	                0;
	inputs[51] = (!player_statistics.any_enemy_seen_lately(gametime)) &&
	                   (player_statistics.get_player_land(pn) <
	                    player_statistics.get_old_player_land(pn) * 105 / 100) ?
	                2 :
	                0;

	inputs[52] =
	   (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land()) ? 1 : 0;
	inputs[53] =
	   (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land()) ? 3 : 0;
	inputs[54] =
	   (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land() * 2) ? 2 :
	                                                                                            0;
	inputs[55] =
	   (player_statistics.get_player_land(pn) < player_statistics.get_enemies_max_land() / 2) ? 2 :
	                                                                                            0;
	inputs[56] =
	   !player_statistics.any_enemy_seen_lately(gametime) && (spots_ < kSpotsTooLittle) ? +2 : 0;
	inputs[57] =
	   player_statistics.any_enemy_seen_lately(gametime) && (spots_ < kSpotsTooLittle) ? +2 : 0;
	inputs[58] = ((mines_per_type[iron_resource_id].in_construction +
	               mines_per_type[iron_resource_id].finished) == 0) ?
	                +3 :
	                0;
	inputs[59] = ((mines_per_type[iron_resource_id].in_construction +
	               mines_per_type[iron_resource_id].finished) == 0) ?
	                +1 :
	                0;
	inputs[60] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ? -2 : 0;
	inputs[61] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy ||
	              expansion_type.get_expansion_type() == ExpansionMode::kBoth) ?
	                -4 :
	                0;
	inputs[62] = (soldier_status_ == SoldiersStatus::kBadShortage) ? -3 : 0;
	inputs[63] = (soldier_status_ == SoldiersStatus::kShortage) ? -2 : 0;

	inputs[64] = (bo.build_material_shortage) ? -3 : 0;
	inputs[65] = (bo.build_material_shortage) ? -1 : 0;
	inputs[66] = (bo.build_material_shortage) ? -2 : 0;
	inputs[67] = (bo.build_material_shortage) ? -8 : 0;
	inputs[68] = (gametime < 15 * 60 * 1000) ? (size - 1) * -1 : 0;
	inputs[69] = (gametime < 30 * 60 * 1000) ? (size - 1) * -1 : 0;
	inputs[70] = (gametime < 45 * 60 * 1000) ? (size - 1) * -1 : 0;
	inputs[71] = (gametime < 15 * 60 * 1000) ? (size - 1) * -2 : 0;
	inputs[72] = (gametime < 30 * 60 * 1000) ? (size - 1) * -2 : 0;
	inputs[73] = (gametime < 45 * 60 * 1000) ? (size - 1) * -2 : 0;
	inputs[74] = (gametime < 15 * 60 * 1000) ? (size - 1) * -3 : 0;
	inputs[75] = (gametime < 30 * 60 * 1000) ? (size - 1) * -3 : 0;
	inputs[76] = (gametime < 45 * 60 * 1000) ? (size - 1) * -3 : 0;
	inputs[77] =
	   (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn) + 2) ?
	      1 :
	      0;
	inputs[78] =
	   (player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn) + 5) ?
	      1 :
	      0;
	inputs[79] =
	   (player_statistics.get_player_power(pn) < player_statistics.get_old60_player_power(pn) + 10) ?
	      1 :
	      0;
	inputs[80] =
	   (player_statistics.get_player_power(pn) > player_statistics.get_old60_player_power(pn) + 20) ?
	      1 :
	      0;
	if (!player_statistics.any_enemy_seen_lately(gametime)) {
		inputs[81] = (player_statistics.get_player_land(pn) * 130 / 100 <
		              player_statistics.get_old60_player_land(pn)) ?
		                2 :
		                0;
		inputs[82] = (player_statistics.get_old60_player_land(pn) * 130 / 100 <
		              player_statistics.get_player_land(pn)) ?
		                0 :
		                2;
	} else {
		inputs[83] = (player_statistics.get_player_land(pn) * 130 / 100 <
		              player_statistics.get_old60_player_land(pn)) ?
		                2 :
		                0;
		inputs[84] = (player_statistics.get_old60_player_land(pn) * 130 / 100 <
		              player_statistics.get_player_land(pn)) ?
		                0 :
		                2;
	}
	inputs[85] = -1 * static_cast<int32_t>(msites_in_constr());
	inputs[86] = -1 * static_cast<int32_t>(msites_in_constr()) / 2;
	inputs[87] = -1 * static_cast<int32_t>(msites_in_constr()) / 3;
	inputs[88] = (msites_in_constr() > 2 && msites_in_constr() > msites_built() / 2) ? -1 : 0;
	inputs[89] = (msites_in_constr() > 2 && msites_in_constr() > msites_built() / 3) ? -1 : 0;
	inputs[90] = (msites_in_constr() > 2 && msites_in_constr() > msites_built() / 4) ? -1 : 0;
	inputs[91] = -static_cast<int32_t>(msites_in_constr()) / 4;
	inputs[92] = (player_statistics.get_player_land(pn) <
	              player_statistics.get_old_player_land(pn) * 130 / 100) ?
	                2 :
	                0;
	inputs[93] = (player_statistics.get_player_land(pn) <
	              player_statistics.get_old_player_land(pn) * 140 / 100) ?
	                2 :
	                0;
	inputs[94] = (!player_statistics.any_enemy_seen_lately(gametime)) &&
	                   (player_statistics.get_player_land(pn) <
	                    player_statistics.get_old_player_land(pn) * 120 / 100) ?
	                2 :
	                0;
	inputs[95] = (!player_statistics.any_enemy_seen_lately(gametime)) &&
	                   (player_statistics.get_player_land(pn) <
	                    player_statistics.get_old_player_land(pn) * 140 / 100) ?
	                2 :
	                0;
	if (msites_built() > 5) {
		inputs[96] = -1 * static_cast<int32_t>(msites_in_constr());
		inputs[97] = -1 * static_cast<int32_t>(msites_in_constr()) / 2;
		inputs[98] = -1 * static_cast<int32_t>(msites_in_constr()) / 3;
		inputs[99] = (msites_in_constr() > msites_built() / 2) ? -2 : 0;
		inputs[100] = (msites_in_constr() > msites_built() / 3) ? -2 : 0;
		inputs[101] = (msites_in_constr() > msites_built() / 4) ? -2 : 0;
	}
	inputs[102] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ? -4 : 0;
	inputs[104] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy ||
	               expansion_type.get_expansion_type() == ExpansionMode::kBoth) ?
	                 -3 :
	                 0;
	inputs[105] = (expansion_type.get_expansion_type() == ExpansionMode::kEconomy) ? -1 : 0;
	inputs[106] = (wood_policy_ == WoodPolicy::kAllowRangers) ? -1 * (size - 1) : 0;
	inputs[107] = (wood_policy_ == WoodPolicy::kAllowRangers) ? -3 * (size - 1) : 0;
	inputs[108] = (wood_policy_ == WoodPolicy::kAllowRangers) ? -5 * size : 0;
	inputs[109] = (wood_policy_ == WoodPolicy::kAllowRangers) ? -5 * (size - 1) : 0;
	if (!bo.critical_building_material.empty() && buil_material_mines_count == 0) {
		inputs[110] = -5;
		inputs[111] = -2;
		inputs[111] = -10;
	}
	if (bo.build_material_shortage) {
		inputs[112] = -5;
		inputs[113] = -2;
		inputs[114] = -10;
	}

	if (!mine_fields_stat.has_critical_ore_fields()) {
		inputs[115] = -3;
		inputs[116] = -6;
		inputs[117] = -8;
	}
	inputs[118] = -mine_fields_stat.count_types();
	inputs[119] = -mine_fields_stat.count_types() * 3;

	for (int i = 0; i < 4 * kFNeuronBitSize; i = i + 1) {
		if (inputs[i] < -35 || inputs[i] > 6) {
			log("Warning check_building_necessity score on position %2d too high %2d\n", i, inputs[i]);
		}
	}

	int32_t final_score = 0;
	for (int i = 0; i < kFNeuronBitSize; i = i + 1) {
		if (management_data.f_neuron_pool[56].get_position(i)) {
			final_score += inputs[i];
		}
		if (management_data.f_neuron_pool[57].get_position(i)) {
			final_score += inputs[kFNeuronBitSize + i];
		}
		if (management_data.f_neuron_pool[58].get_position(i)) {
			final_score += inputs[2 * kFNeuronBitSize + i];
		}
		if (management_data.f_neuron_pool[13].get_position(i)) {
			final_score += inputs[3 * kFNeuronBitSize + i];
		}
	}

	final_score += std::abs(management_data.get_military_number_at(76) / 10);
	final_score += management_data.get_military_number_at(100) / 5;

	if (final_score > 0) {
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

	const uint32_t gametime = game().get_gametime();

	for (TrainingSiteObserver& trainingsite_obs : trainingsites) {
		if (trainingsite_obs.site == &site) {
			soldier_trained_log.push(gametime, trainingsite_obs.bo->id);
			if (trainingsite_obs.site->soldier_control()->soldier_capacity() > 0) {
				game().send_player_change_soldier_capacity(
				   *trainingsite_obs.site,
				   -trainingsite_obs.site->soldier_control()->soldier_capacity());
			}
			return;
		}
	}

	log(" %d: Computer player error - trainingsite not found\n", player_number());
}
