/*
 * Copyright (C) 2002-2004, 2006-2007 by the Widelands Development Team
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
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "attack_controller.h"

#include "battle.h"
#include "error.h"
#include "game.h"
#include "geometry.h"
#include "immovable.h"
#include "instances.h"
#include "map.h"
#include "militarysite.h"
#include "player.h"
#include "soldier.h"
#include "transport.h"


void getCloseMilitarySites
(const Game & game,
 const Flag & flag,
 const Player_Number player,
 std::set<MilitarySite *> & militarySites)
{
	Map & map = game.map();

   std::vector<ImmovableFound> immovables;

	map.find_reachable_immovables
		(Area<FCoords>(game.map().get_fcoords(flag.get_position()), 25),
		 &immovables,
		 CheckStepWalkOn(MOVECAPS_WALK, false));

   if (!immovables.size())
      return;

   /* Find all friendly MS */
   for (std::vector<ImmovableFound>::const_iterator it=immovables.begin();it != immovables.end();++it) {
		if
			(MilitarySite * const ms =
			 dynamic_cast<MilitarySite * const>(it->object))
			if (ms->owner().get_player_number() == player) {
				militarySites.insert(ms);
			}
	}
}

uint getMaxAttackSoldiers
(const Game & game, const Flag & flag, const Player_Number player)
{
   uint maxAttackSoldiers = 0;

	std::set<MilitarySite *> militarySites;
	getCloseMilitarySites(game, flag, player, militarySites);

	const std::set<MilitarySite *>::const_iterator militarySites_end =
		militarySites.end();
	for
		(std::set<MilitarySite *>::const_iterator it = militarySites.begin();
		 it != militarySites_end;
		 ++it)
      maxAttackSoldiers+= (*it)->nr_attack_soldiers();
   log("Got %i attackers available for attack.\n", maxAttackSoldiers);
   return maxAttackSoldiers;
}

Map_Object_Descr globalAttackControllerDescr;

AttackController::AttackController(Game & the_game) :
BaseImmovable  (globalAttackControllerDescr),
attackedMsEmpty(false),
game           (&the_game)
{Map_Object::init(&the_game);}

AttackController::AttackController(Game* _game, Flag* _flag, int _attacker, int _defender) :
BaseImmovable(globalAttackControllerDescr)
{

   Map_Object::init(_game);

   this->game = _game;
   this->flag = _flag;
   this->attackingPlayer = _attacker;
   this->defendingPlayer = _defender;
   this->totallyLaunched = 0;
   this->attackedMsEmpty = false;

}

AttackController::~AttackController() {
}

//Methods inherited by BaseImmovable
void AttackController::act (Game*, uint) {
   schedule_act(game, 10000); // Check every 10sec if the battle is deadlocked
}

void AttackController::cleanup (Editor_Game_Base* eg)
{
   log ("AttackController::cleanup\n");
   Map_Object::cleanup(eg);
}
//end inherited

void AttackController::launchAttack(uint nrAttackers) {
   launchAllSoldiers(true, nrAttackers);
   //always try to launch two more defenders than attackers
   if (!launchAllSoldiers(false, nrAttackers+2)) {
      attackedMsEmpty = true;
	}
}

bool AttackController::launchAllSoldiers(bool attackers, int max) {
	std::set<MilitarySite *> militarySites;
	getCloseMilitarySites
		(*game,
		 *flag,
		 (attackers ? attackingPlayer : defendingPlayer),
		 militarySites);

   bool launchedSoldier = false;

	const std::set<MilitarySite *>::const_iterator militarySites_end =
		militarySites.end();
	for
		(std::set<MilitarySite *>::const_iterator it = militarySites.begin();
		 it != militarySites_end;
		 ++it)
	{
      uint soldiersOfMs = (*it)->nr_attack_soldiers();
      uint nrLaunch = ((max > -1) && (soldiersOfMs > (uint)max) ? (uint)max : soldiersOfMs);
		if (nrLaunch == 0) continue;

		launchedSoldier = true;

      launchSoldiersOfMilitarySite(*it, nrLaunch, attackers);

      if (max > -1) {
         max-=nrLaunch;
         if (max <= 0)
            break;
		}

	}
   return launchedSoldier;
}

void AttackController::launchSoldiersOfMilitarySite(MilitarySite* militarySite, uint nrLaunch, bool attackers) {
	assert(nrLaunch);
   uint launched = 0;

	const std::vector<Soldier *> & soldiers = militarySite->get_soldiers();
	std::vector<Soldier *>::const_iterator soldiers_end = soldiers.end();
	for
		(std::vector<Soldier *>::const_iterator it = soldiers.begin();
		 it != soldiers_end;
		 ++it)
		if (moveToBattle(*it, militarySite, attackers)) {
			++launched;
			if (launched == nrLaunch) break;
		}

   if (launched > 0) {
		involvedMilitarySites.insert(militarySite);
      militarySite->set_in_battle(true);
	}
}

bool AttackController::moveToBattle(Soldier* soldier, MilitarySite* militarySite, bool attackers) {
   if (!soldier->is_marked()) {
      soldier->set_attack_ctrl(this);
      soldier->mark(true);
      soldier->reset_tasks(this->game);

      BattleSoldier bs = {
         soldier,
         militarySite,
         Coords::Null(),
         attackers,
         false,
         false,
		};

      calcBattleGround(&bs, totallyLaunched);

      soldier->startTaskMoveToBattle(this->game, this->flag, bs.battleGround);

      involvedSoldiers.push_back(bs);
      totallyLaunched++;
      return true;
	}
   return false;
}

void AttackController::moveToReached(Soldier* soldier) {
   if (this->attackedMsEmpty) {
      soldierWon(soldier);
	}
   else {
      startBattle(soldier, true);
	}
   log("Soldier %dP reached flag.\n", soldier->get_serial());
}

void AttackController::soldierDied(Soldier* soldier) {
   removeSoldier(soldier);
   soldier->schedule_destroy(game);
}

void AttackController::soldierWon(Soldier* soldier) {
   uint idx = getBattleSoldierIndex(soldier);
   involvedSoldiers[idx].fighting = false;

   if (opponentsLeft(soldier)) {
      startBattle(soldier, true);
      return;
	}

   //if the last remaing was an attacker, check for
   //remaining soldiers in the building
	if (involvedSoldiers[idx].attacker)
		if
			(MilitarySite * const ms =
			 dynamic_cast<MilitarySite * const>(flag->get_building()))
      //There are defending soldiers left in the building.
			if (const uint n = ms->nr_not_marked_soldiers()) {
				launchSoldiersOfMilitarySite(ms, n, false);
				return;
			}

   log("finishing battle...\n");

	for
		(std::set<Object_Ptr>::const_iterator it = involvedMilitarySites.begin();
		 it != involvedMilitarySites.end();
		 ++it)
		static_cast<MilitarySite * const>(static_cast<Object_Ptr>(*it).get(game))->set_in_battle(false);

	if (involvedSoldiers[idx].attacker) {
      log("attackers won, destroying building.\n");
		Building & building = *flag->get_building();
		building.set_defeating_player(attackingPlayer);
		building.destroy(game);
	}

	for (uint i=0;i<involvedSoldiers.size();i++) {
		//involvedSoldiers[i].soldier->set_economy(0);
		involvedSoldiers[i].soldier->set_location(involvedSoldiers[i].origin);
		involvedSoldiers[i].soldier->send_signal(game, "return_home");
	}

   log("battle finished. removing attack controller.\n");
   game->remove_attack_controller(this->get_serial());
}

bool AttackController::startBattle(Soldier* soldier, bool isArrived) {
   uint s1Index = getBattleSoldierIndex(soldier);
   involvedSoldiers[s1Index].arrived = isArrived;

   for (uint i=0;i<involvedSoldiers.size();i++) {
      if (involvedSoldiers[i].arrived && !involvedSoldiers[i].fighting && (involvedSoldiers[i].attacker != involvedSoldiers[s1Index].attacker)) {
         involvedSoldiers[i].fighting = true;
         involvedSoldiers[s1Index].fighting = true;

         Battle* battle = game->create_battle();
         uint rnd = game->logic_rand() % 11;
         if (rnd <= 5)
            battle->soldiers(involvedSoldiers[i].soldier, involvedSoldiers[s1Index].soldier);
         else
            battle->soldiers(involvedSoldiers[s1Index].soldier, involvedSoldiers[i].soldier);
         log("Created battle, rnd was: %i\n", rnd);
         return true;
		}
	}
   log("Could not create battle. No opponents found.\n");
   return false;
}

bool AttackController::opponentsLeft(Soldier* soldier) {
   uint idx = getBattleSoldierIndex(soldier);
   for (uint i=0;i<involvedSoldiers.size();i++) {
      if (involvedSoldiers[i].attacker != involvedSoldiers[idx].attacker)
         return true;
	}
   return false;
}

uint AttackController::getBattleSoldierIndex(Soldier* soldier) {
  for (uint i=0;i<involvedSoldiers.size();i++) {
      if (involvedSoldiers[i].soldier == soldier)
         return i;
	}
   throw wexception("No BattleSoldier structure found for %dP!\n", soldier->get_serial());
}

void AttackController::removeSoldier(Soldier* soldier) {
   uint idx = getBattleSoldierIndex(soldier);
   involvedSoldiers[idx].battleGround = Coords::Null();
   if (idx < (involvedSoldiers.size()-1)) {
      involvedSoldiers[idx] = involvedSoldiers[involvedSoldiers.size() -1];
	}
   involvedSoldiers.pop_back();
}

bool AttackController::battleGroundOccupied(Coords coords) {
   for (uint i=0;i<involvedSoldiers.size();i++) {
      if (involvedSoldiers[i].battleGround == coords) {
         return true;
		}
	}
   return false;
}

void AttackController::calcBattleGround(BattleSoldier* battleSoldier, int soldierNr) {
   log("Calculating battle ground for soldier\n");
   if (soldierNr == 0) {
      battleSoldier->battleGround = flag->get_position();
      return;
	}

   Map* map = game->get_map();

   FCoords prevCoords = map->get_fcoords(flag->get_position());
   FCoords newCoords = map->get_fcoords(flag->get_position());

   int walkDir[] = {Map_Object::WALK_NE, Map_Object::WALK_E, Map_Object::WALK_SE,
                    Map_Object::WALK_SW, Map_Object::WALK_W};
   int walkDirIndex = soldierNr % 5;

   CheckStepDefault step(battleSoldier->soldier->get_movecaps());

   for (uint i=0;i<20;i++) {
      map->get_neighbour(prevCoords, walkDir[walkDirIndex], &newCoords);

      if (step.allowed(map, prevCoords, newCoords, walkDir[walkDirIndex], CheckStep::stepNormal)) {
         if (!battleGroundOccupied(newCoords)) {
            battleSoldier->battleGround = newCoords;
            return;
			}
         prevCoords = newCoords;
		}
      else {
         walkDirIndex = (soldierNr+1) % 5;
		}
	}
   battleSoldier->battleGround = flag->get_position();
}
