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

	// receiving games statistics and parsing it (reading latest entry)
	const Game::GeneralStatsVector& genstats = game().get_general_statistics();

	// Collecting statistics and saving them in player_statistics object
	for (Widelands::TeamNumber j = 1; j <= plr_in_game; ++j) {
		const Player* this_player = game().get_player(j);
		if (this_player) {
			try {
				player_statistics.add(
				   j, this_player->team_number(), genstats.at(j - 1).miltary_strength.back());
			} catch (const std::out_of_range&) {
				log("ComputerPlayer(%d): genstats entry missing - size :%d\n",
				    static_cast<unsigned int>(player_number()),
				    static_cast<unsigned int>(genstats.size()));
			}
		}
	}

	player_statistics.recalculate_team_power();

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
				if (player_->is_hostile(bld->owner())) {
					if (enemy_sites.count(bld->get_position().hash()) == 0) {
						enemy_sites[bld->get_position().hash()] = EnemySiteObserver();
					}
				}
			}
			if (upcast(Warehouse const, wh, immovables.at(j).object)) {
				if (player_->is_hostile(wh->owner())) {
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
	} else if (persistent_data->last_soldier_trained + 10 * 60 * 1000 < gametime) {
		// was any soldier trained within last 10 minutes
		switch (type_) {
		case DefaultAI::Type::kNormal:
			training_score = -4;
			break;
		case DefaultAI::Type::kWeak:
			training_score = -2;
			break;
		case DefaultAI::Type::kVeryWeak:
			training_score = -1;
		}
	}
	// Also we should have at least some training sites to be more willing to attack
	// Of course, very weak AI can have only one trainingsite so will be always penalized by this
	switch (ts_basic_count_ + ts_advanced_count_ - ts_without_trainers_) {
	case 0:
		training_score -= 6;
		break;
	case 1:
		training_score -= 3;
		break;
	case 2:
		training_score -= 1;
		break;
	default:;
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
				int32_t strength = calculate_strength(attackers);

				site->second.attack_soldiers_strength = strength;
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

				site->second.score -= vacant_mil_positions_ / 8;

				if (site->second.mines_nearby == ExtendedBool::kFalse) {
					site->second.score -= 1;
				} else {
					site->second.score += 1;
				}
				// we dont want to attack multiple players at the same time too eagerly
				if (owner_number != persistent_data->last_attacked_player) {
					site->second.score -= 3;
				}
				// if we dont have mines yet
				if (mines_.size() <= 2) {
					site->second.score -= 8;
				}

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
	enemy_sites[best_target].no_attack_counter = -3;

	Flag* flag = nullptr;  // flag of a building to be attacked
	if (upcast(MilitarySite, bld, f.field->get_immovable())) {
		flag = &bld->base_flag();
	} else if (upcast(Warehouse, Wh, f.field->get_immovable())) {
		flag = &Wh->base_flag();
	} else {
		return false;  // this should not happen
	}

	// how many attack soldiers we can send?
	uint32_t attackers = player_->find_attack_soldiers(*flag);

	// Of course not all of them:
	// reduce by 0-3 for attackers below 10
	// but for soldiers in range 10-40 reduce by much more.
	// Soldiers above 40 are ignored for calculation

	// Number of soldiers in the range 10-40, random portion of
	// them will be used
	uint32_t above_ten = (attackers > 10) ? attackers - 10 : 0;
	above_ten = (above_ten > 30) ? 30 : above_ten;

	attackers = attackers - (gametime % 3) - ((above_ten > 0) ? gametime % above_ten : 0);

	if (attackers <= 0) {
		return false;
	}

	game().send_player_enemyflagaction(*flag, player_number(), attackers);

	last_attack_time_ = gametime;
	persistent_data->last_attacked_player = flag->owner().player_number();

	return true;
}

// this just counts free positions in military and training sites
void DefaultAI::count_military_vacant_positions() {
	// counting vacant positions
	vacant_mil_positions_ = 0;
	for (TrainingSiteObserver tso : trainingsites) {
		vacant_mil_positions_ +=
		   10 * (tso.site->soldier_capacity() - tso.site->stationed_soldiers().size());
		vacant_mil_positions_ += (tso.site->can_start_working()) ? 0 : 10;
	}
	for (MilitarySiteObserver mso : militarysites) {
		vacant_mil_positions_ += mso.site->soldier_capacity() - mso.site->stationed_soldiers().size();
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
	std::vector<WaresQueue*> const warequeues1 = tso.site->warequeues();
	size_t nr_warequeues = warequeues1.size();
	for (size_t i = 0; i < nr_warequeues; ++i) {

		// if it was decreased yet
		if (warequeues1[i]->get_max_fill() <= 1) {
			continue;
		}

		// now modifying max_fill of armors and weapons
		for (std::string pattern : armors_and_weapons) {

			if (tribe_->get_ware_descr(warequeues1[i]->get_ware())->name().find(pattern) !=
			    std::string::npos) {
				if (warequeues1[i]->get_max_fill() > 1) {
					game().send_player_set_ware_max_fill(*ts, warequeues1[i]->get_ware(), 1);
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
		std::vector<WaresQueue*> const warequeues2 = tso.site->warequeues();
		nr_warequeues = warequeues2.size();
		for (size_t i = 0; i < nr_warequeues; ++i) {
			if (tso.bo->substitute_inputs.count(warequeues2[i]->get_ware()) > 0) {
				filled += warequeues2[i]->get_filled();
			}
		}
		if (filled < 5) {
			shortage += 5 - filled;
		}

		// checking non subsitutes
		for (size_t i = 0; i < nr_warequeues; ++i) {
			if (tso.bo->substitute_inputs.count(warequeues2[i]->get_ware()) == 0) {
				const uint32_t required_amount =
				   (warequeues2[i]->get_max_fill() < 5) ? warequeues2[i]->get_max_fill() : 5;
				if (warequeues2[i]->get_filled() < required_amount) {
					shortage += required_amount - warequeues2[i]->get_filled();
				}
			}
		}

		// Has any soldier been trained up to now?
		if (gametime > 10 * 60 * 1000 && persistent_data->last_soldier_trained < gametime) {
			if (persistent_data->last_soldier_trained + 10 * 60 * 1000 < gametime) {
				if (shortage <= 3) {
					game().send_player_change_soldier_capacity(*ts, 1);
				}
			} else {
				if (shortage <= 1) {
					game().send_player_change_soldier_capacity(*ts, 1);
				}
			}
			// or this can be very first soldier to be trained
		} else if (shortage <= 3) {
			game().send_player_change_soldier_capacity(*ts, 1);
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

	// Check next militarysite
	bool changed = false;
	Map& map = game().map();
	MilitarySite* ms = militarysites.front().site;
	MilitarySiteObserver& mso = militarysites.front();
	uint32_t const vision = ms->descr().vision_range();
	FCoords f = map.get_fcoords(ms->get_position());
	// look if there are any enemies building
	FindNodeEnemiesBuilding find_enemy(player_, game());

	// first we make sure there is no enemy at all
	if (map.find_fields(Area<FCoords>(f, vision + 4), nullptr, find_enemy) == 0) {

		mso.enemies_nearby = false;

		// If no enemy in sight - decrease the number of stationed soldiers
		// as long as it is > 1 - BUT take care that there is a warehouse in the
		// same economy where the thrown out soldiers can go to.

		if (ms->economy().warehouses().size()) {
			uint32_t const j = ms->soldier_capacity();

			if (MilitarySite::kPrefersRookies != ms->get_soldier_preference()) {
				game().send_player_militarysite_set_soldier_preference(
				   *ms, MilitarySite::kPrefersRookies);
			} else if (j > 1) {
				game().send_player_change_soldier_capacity(*ms, (j > 2) ? -2 : -1);
			}
			// if the building is in inner land and other militarysites still
			// hold the miliary influence of the field, consider to destruct the
			// building to free some building space.
			else {
				// treat this field like a buildable and write military info to it.
				BuildableField bf(f);
				update_buildable_field(bf, vision, true);
				const int32_t size_penalty = ms->get_size() - 1;
				FindNodeAllyOwned find_ally(player_, game(), player_number());
				const int32_t allyOwnedFields =
				   map.find_fields(Area<FCoords>(f, vision), nullptr, find_ally);

				int16_t score = 0;
				score += (bf.area_military_capacity > 6);
				score += (bf.area_military_capacity > 22);
				score += (bf.area_military_presence > 4);
				score += (bf.military_loneliness <
				          (180 + persistent_data->ai_personality_military_loneliness));
				score += (bf.military_stationed > 2);
				score -= size_penalty;
				score += ((bf.unowned_land_nearby + allyOwnedFields) < 10);
				score -= (mso.built_time + 10 * 60 * 1000 > gametime);

				if (score >= 4) {
					if (ms->get_playercaps() & Widelands::Building::PCap_Dismantle) {
						flags_to_be_removed.push_back(ms->base_flag().get_position());
						game().send_player_dismantle(*ms);
						military_last_dismantle_ = game().get_gametime();
					} else {
						game().send_player_bulldoze(*ms);
						military_last_dismantle_ = game().get_gametime();
					}
				}
			}
		}
	} else {

		uint32_t unused1 = 0;
		uint16_t unused2 = 0;

		mso.enemies_nearby = false;

		// yes enemy is nearby, but still we must distinguish whether
		// he is accessible (over the land)
		if (other_player_accessible(
		       vision + 4, &unused1, &unused2, ms->get_position(), WalkSearch::kEnemy)) {

			Quantity const total_capacity = ms->max_soldier_capacity();
			Quantity const target_capacity = ms->soldier_capacity();

			game().send_player_change_soldier_capacity(*ms, total_capacity - target_capacity);
			changed = true;

			// and also set preference to Heroes
			if (MilitarySite::kPrefersHeroes != ms->get_soldier_preference()) {
				game().send_player_militarysite_set_soldier_preference(
				   *ms, MilitarySite::kPrefersHeroes);
				changed = true;
			}

			mso.enemies_nearby = true;
			enemy_last_seen_ = gametime;
		} else {  // otherwise decrease soldiers
			uint32_t const j = ms->soldier_capacity();

			if (MilitarySite::kPrefersRookies != ms->get_soldier_preference()) {
				game().send_player_militarysite_set_soldier_preference(
				   *ms, MilitarySite::kPrefersRookies);
			} else if (j > 1) {
				game().send_player_change_soldier_capacity(*ms, (j > 2) ? -2 : -1);
			}
		}
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

	for (Soldier* soldier : soldiers) {
		const SoldierDescr& descr = soldier->descr();
		health = (descr.get_base_health() + descr.get_health_incr_per_level() * soldier->get_health_level()) / 100;
		attack = ((descr.get_base_max_attack() - descr.get_base_min_attack()) / 2 + descr.get_base_min_attack() + descr.get_attack_incr_per_level() * soldier->get_attack_level()) / 100;
		defense = static_cast<float>(100 - descr.get_base_defense() - 8 * soldier->get_defense_level()) / 100;
		evade = static_cast<float>(100 - descr.get_base_evade() - descr.get_evade_incr_per_level() / 100 * soldier->get_evade_level()) / 100;
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
BuildingNecessity DefaultAI::check_building_necessity(const uint8_t size, const uint32_t gametime) {

	assert(militarysites.size() == msites_built());
	// logically size of militarysite must in between 1 and 3 (including)
	assert(size >= BaseImmovable::SMALL && size <= BaseImmovable::BIG);

	if (size == BaseImmovable::SMALL) {  // this function is intended for medium and bigger sites
		return BuildingNecessity::kAllowed;
	}

	uint32_t const big_buildings_score =
	   msites_per_size[2].in_construction + msites_per_size[2].finished +
	   msites_per_size[3].in_construction * 2 + msites_per_size[3].finished * 2;

	const uint32_t msites_total = msites_built() + msites_in_constr();

	// this is final proportion of big_buildings_score / msites_total
	// two exeptions:
	// if enemy nearby - can be higher
	// for early game - must be lower
	uint32_t limit = (msites_built() + msites_in_constr()) * 2 / 3;

	// exemption first
	if (militarysites.size() > 3 && vacant_mil_positions_ == 0 && msites_in_constr() == 0) {
		return BuildingNecessity::kAllowed;  // it seems the expansion is stuck so we allow big
		                                     // buildings
	} else if (gametime > enemy_last_seen_ && gametime < enemy_last_seen_ + 30 * 60 * 1000 &&
	           mines_.size() > 2) {  // if enemies were nearby in last 30 minutes
		// we allow more big buidings
		limit *= 2;
	} else if (msites_total < persistent_data->ai_personality_early_militarysites) {
		// for the beginning of the game (first 30 military sites)
		limit = limit * msites_total / persistent_data->ai_personality_early_militarysites;
	}

	if (big_buildings_score + size - 1 > limit) {
		return BuildingNecessity::kNotNeeded;
	} else {
		return BuildingNecessity::kAllowed;
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
