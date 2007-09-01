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
#include "fileread.h"
#include "filewrite.h"
#include "game.h"
#include "geometry.h"
#include "immovable.h"
#include "instances.h"
#include "map.h"
#include "militarysite.h"
#include "player.h"
#include "soldier.h"
#include "transport.h"
#include "widelands_map_map_object_loader.h"
#include "widelands_map_map_object_saver.h"


void getCloseMilitarySites
(const Editor_Game_Base & eg,
 const Flag & flag,
 const Player_Number player,
 std::set<MilitarySite *> & militarySites)
{
	Map & map = eg.map();

	std::vector<ImmovableFound> immovables;

	map.find_reachable_immovables
		(Area<FCoords>(eg.map().get_fcoords(flag.get_position()), 25),
		 &immovables,
		 CheckStepWalkOn(MOVECAPS_WALK, false));

	if (!immovables.size())
		return;

	/* Find all friendly MS */
	for (std::vector<ImmovableFound>::const_iterator it=immovables.begin();it != immovables.end();++it) {
		if
			(MilitarySite * const ms =
			 dynamic_cast<MilitarySite *>(it->object))
			if (ms->owner().get_player_number() == player) {
				militarySites.insert(ms);
			}
	}
}

uint getMaxAttackSoldiers
(const Editor_Game_Base & eg, const Flag & flag, const Player_Number player)
{
	uint maxAttackSoldiers = 0;

	std::set<MilitarySite *> militarySites;
	getCloseMilitarySites(eg, flag, player, militarySites);

	const std::set<MilitarySite *>::const_iterator militarySites_end =
		militarySites.end();
	for
		(std::set<MilitarySite *>::const_iterator it = militarySites.begin();
		 it != militarySites_end;
		 ++it)
	{
		maxAttackSoldiers+= (*it)->nr_attack_soldiers();
	}

	log("Got %i attackers available for attack.\n", maxAttackSoldiers);
	return maxAttackSoldiers;
}

Map_Object_Descr globalAttackControllerDescr;

AttackController::AttackController(Editor_Game_Base & eg) :
	BaseImmovable  (globalAttackControllerDescr),
	attackedMsEmpty(false),
	m_egbase(&eg)
{
}

AttackController::AttackController(Editor_Game_Base* eg, Flag* _flag, int _attacker, int _defender) :
	BaseImmovable(globalAttackControllerDescr)
{
	this->m_egbase = eg;
	this->flag = _flag;
	this->attackingPlayer = _attacker;
	this->defendingPlayer = _defender;
	this->totallyLaunched = 0;
	this->attackedMsEmpty = false;
}

AttackController::~AttackController() {
}

//Methods inherited by BaseImmovable
void AttackController::act (Game* game, uint) {
	schedule_act(game, 10000); // Check every 10sec if the battle is deadlocked
}

void AttackController::init(Editor_Game_Base* eg)
{
	Map_Object::init(eg);
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
		(egbase(),
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
		const uint nrLaunch = (max > -1 and (soldiersOfMs > static_cast<uint>(max)) ? static_cast<uint>(max) : soldiersOfMs);
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

void AttackController::launchSoldiersOfMilitarySite
		(MilitarySite* militarySite,
		 uint nrLaunch,
		 bool attackers)
{
	assert(nrLaunch);
	uint launched = 0;

	const std::vector<Soldier *> & soldiers = militarySite->get_soldiers();
	std::vector<Soldier *>::const_iterator soldiers_end = soldiers.end();
	for
		(std::vector<Soldier *>::const_iterator it = soldiers.begin();
		 it != soldiers_end;
		 ++it)
	{
		if (moveToBattle(*it, militarySite, attackers)) {
			++launched;
			if (launched == nrLaunch) break;
		}
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
		soldier->reset_tasks(dynamic_cast<Game*>(&egbase()));

		BattleSoldier bs = {
			soldier,
			militarySite,
			Coords::Null(),
			attackers,
			false,
			false,
		};

		calcBattleGround(&bs, totallyLaunched);

		soldier->startTaskMoveToBattle(dynamic_cast<Game*>(&egbase()), this->flag, bs.battleGround);

		involvedSoldiers.push_back(bs);
		totallyLaunched++;
		return true;
	}

	return false;
}

void AttackController::moveToReached(Soldier* soldier)
{
	if (this->attackedMsEmpty) {
		soldierWon(soldier);
	} else {
		startBattle(soldier, true);
	}
	log("Soldier %dP reached flag.\n", soldier->get_serial());
}

void AttackController::soldierDied(Soldier* soldier)
{
	removeSoldier(soldier);
	soldier->schedule_destroy(dynamic_cast<Game*>(&egbase()));
}

void AttackController::soldierWon(Soldier* soldier)
{
	uint idx = getBattleSoldierIndex(soldier);
	involvedSoldiers[idx].fighting = false;

	if (opponentsLeft(soldier)) {
		startBattle(soldier, true);
		return;
	}

	//if the last remaing was an attacker, check for
	//remaining soldiers in the building
	if (involvedSoldiers[idx].attacker) {
		if
			(MilitarySite * const ms =
			 dynamic_cast<MilitarySite *>(flag->get_building()))
		{
			//  There are defending soldiers left in the building.
			if (const uint n = ms->nr_not_marked_soldiers()) {
				launchSoldiersOfMilitarySite(ms, n, false);
				return;
			}
		}
	}

	log("finishing battle...\n");

	for
		(MilitarySiteSet::iterator it = involvedMilitarySites.begin();
		 it != involvedMilitarySites.end();
		 ++it)
	{
		OPtr<MilitarySite> ptr = *it;
		ptr.get(&egbase())->set_in_battle(false);
	}

	if (involvedSoldiers[idx].attacker) {
		log("attackers won, destroying building.\n");
		Building & building = *flag->get_building();
		building.set_defeating_player(attackingPlayer);
		building.destroy(&egbase());
	}

	for (uint i=0;i<involvedSoldiers.size();i++) {
		//involvedSoldiers[i].soldier->set_economy(0);
		involvedSoldiers[i].soldier->set_location(involvedSoldiers[i].origin);
		involvedSoldiers[i].soldier->send_signal(dynamic_cast<Game*>(&egbase()), "return_home");
	}

	log("battle finished. removing attack controller.\n");
	egbase().remove_attack_controller(this->get_serial());
}

bool AttackController::startBattle(Soldier* soldier, bool isArrived)
{
	uint s1Index = getBattleSoldierIndex(soldier);
	involvedSoldiers[s1Index].arrived = isArrived;

	for (uint i=0;i<involvedSoldiers.size();i++) {
		if (involvedSoldiers[i].arrived && !involvedSoldiers[i].fighting && (involvedSoldiers[i].attacker != involvedSoldiers[s1Index].attacker)) {
			involvedSoldiers[i].fighting = true;
			involvedSoldiers[s1Index].fighting = true;

			Battle* battle = egbase().create_battle();
			uint rnd = dynamic_cast<Game*>(&egbase())->logic_rand() % 11;
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

bool AttackController::opponentsLeft(Soldier* soldier)
{
	uint idx = getBattleSoldierIndex(soldier);
	for (uint i=0;i<involvedSoldiers.size();i++) {
		if (involvedSoldiers[i].attacker != involvedSoldiers[idx].attacker)
			return true;
	}
	return false;
}

uint AttackController::getBattleSoldierIndex(Soldier* soldier)
{
	for (uint i=0;i<involvedSoldiers.size();i++) {
		if (involvedSoldiers[i].soldier == soldier)
			return i;
	}
	throw wexception("No BattleSoldier structure found for %dP!\n", soldier->get_serial());
}

void AttackController::removeSoldier(Soldier* soldier)
{
	uint idx = getBattleSoldierIndex(soldier);
	involvedSoldiers[idx].battleGround = Coords::Null();
	if (idx < (involvedSoldiers.size()-1)) {
		involvedSoldiers[idx] = involvedSoldiers[involvedSoldiers.size() -1];
	}
	involvedSoldiers.pop_back();
}

bool AttackController::battleGroundOccupied(Coords coords)
{
	for (uint i=0;i<involvedSoldiers.size();i++) {
		if (involvedSoldiers[i].battleGround == coords) {
			return true;
		}
	}
	return false;
}

void AttackController::calcBattleGround(BattleSoldier* battleSoldier, int soldierNr)
{
	log("Calculating battle ground for soldier\n");
	if (soldierNr == 0) {
		battleSoldier->battleGround = flag->get_position();
		return;
	}

	Map* map = egbase().get_map();

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
		} else {
			walkDirIndex = (soldierNr+1) % 5;
		}
	}

	battleSoldier->battleGround = flag->get_position();
}


/*
=============================

Load/save support

=============================
*/

#define ATTACKCONTROLLER_SAVEGAME_VERSION 1

void AttackController::Loader::load(FileRead& fr)
{
	BaseImmovable::Loader::load(fr);

	AttackController* ctrl = dynamic_cast<AttackController*>(get_object());

	egbase().register_attack_controller(ctrl);

	flag = fr.Unsigned32();

	ctrl->attackingPlayer = fr.Unsigned8();
	ctrl->defendingPlayer = fr.Unsigned8();
	ctrl->totallyLaunched = fr.Unsigned32();
	ctrl->attackedMsEmpty = fr.Unsigned8();

	uint numBs = fr.Unsigned32();

	for (uint j = 0; j < numBs; ++j) {
		uint soldier = fr.Unsigned32();
		uint origin = fr.Unsigned32();

		Coords battleGround;
		try {
			battleGround = fr.Coords32(egbase().map().extent());
		} catch (const FileRead::Width_Exceeded e) {
			throw wexception
					("AttackController::load: in "
					 "binary/mapobjects:%u: battleGround has x "
					 "coordinate %i, but the map width is only %u",
					  e.position, e.x, e.w);
		} catch (const FileRead::Height_Exceeded e) {
			throw wexception
					("AttackController::load: in "
					 "binary/mapobjects:%u: battleGround has y "
					 "coordinate %i, but the map height is only %u",
					 e.position, e.y, e.h);
		}

		bool attacker = fr.Unsigned8();
		bool arrived = fr.Unsigned8();
		bool fighting = fr.Unsigned8();

		BattleSoldierData bsd = {
			soldier,
			origin
		};
		BattleSoldier bs = {
			0,
			0,
			battleGround,
			attacker,
			arrived,
			fighting
		};

		ctrl->involvedSoldiers.push_back(bs);
		soldiers.push_back(bsd);
	}

	uint numInMs = fr.Unsigned32();
	for (uint j = 0; j < numInMs; ++j)
		militarySites.push_back(fr.Unsigned32());
}


void AttackController::Loader::load_pointers()
{
	BaseImmovable::Loader::load_pointers();

	AttackController* ctrl = dynamic_cast<AttackController*>(get_object());

	ctrl->flag = dynamic_cast<Flag*>(mol().get_object_by_file_index(flag));
	assert(ctrl->flag);

	for (uint j = 0; j < soldiers.size(); ++j) {
		const BattleSoldierData& bsd = soldiers[j];
		BattleSoldier& bs = ctrl->involvedSoldiers[j];

		bs.soldier = dynamic_cast<Soldier*>(mol().get_object_by_file_index(bsd.soldier));
		assert(bs.soldier);
		bs.origin = dynamic_cast<MilitarySite*>(mol().get_object_by_file_index(bsd.origin));
		assert(bs.origin);

		bs.soldier->set_attack_ctrl(ctrl);
	}

	for (uint j = 0; j < militarySites.size(); ++j) {
		MilitarySite* ms =
				dynamic_cast<MilitarySite*>
				(mol().get_object_by_file_index(militarySites[j]));

		assert(ms);
		ctrl->involvedMilitarySites.insert(ms);
		ms->set_in_battle(true);
	}
}


void AttackController::save
		(Editor_Game_Base* eg,
		 Widelands_Map_Map_Object_Saver* mos,
		 FileWrite& fw)
{
	fw.Unsigned8(header_AttackController);
	fw.Unsigned8(ATTACKCONTROLLER_SAVEGAME_VERSION);

	BaseImmovable::save(eg, mos, fw);

	fw.Unsigned32(mos->get_object_file_index(flag));

	fw.Unsigned8(attackingPlayer);
	fw.Unsigned8(defendingPlayer);
	fw.Unsigned32(totallyLaunched);
	fw.Unsigned8(attackedMsEmpty);

	//write battle soldier structure of involved soldiers
	fw.Unsigned32(involvedSoldiers.size());

	for (uint j = 0; j < involvedSoldiers.size(); ++j) {
		BattleSoldier bs = involvedSoldiers[j];

		fw.Unsigned32(mos->get_object_file_index(bs.soldier));
		fw.Unsigned32(mos->get_object_file_index(bs.origin));

		fw.Coords32(bs.battleGround);

		fw.Unsigned8(bs.attacker);
		fw.Unsigned8(bs.arrived);
		fw.Unsigned8(bs.fighting);
	}

	//write involved military sites
	fw.Unsigned32(involvedMilitarySites.size());
	for (MilitarySiteSet::iterator it = involvedMilitarySites.begin();
	     it != involvedMilitarySites.end();
	     ++it)
	{
		OPtr<MilitarySite> ptr = *it;
		MilitarySite* ms = ptr.get(eg);
		fw.Unsigned32(mos->get_object_file_index(ms));
	}
}


Map_Object::Loader* AttackController::load
		(Editor_Game_Base* egbase,
		 Widelands_Map_Map_Object_Loader* mol,
		 FileRead& fr)
{
	Loader* loader = new Loader;

	try {
		Uint8 version = fr.Unsigned8();
		if (version != ATTACKCONTROLLER_SAVEGAME_VERSION)
			throw wexception("Unknown version %u", version);

		loader->init(egbase, mol, new AttackController(*dynamic_cast<Game*>(egbase)));
		loader->load(fr);
	} catch (const std::exception& e) {
		delete loader;
		throw wexception("Loading AttackController: %s", e.what());
	} catch (...) {
		delete loader;
		throw;
	}

	return loader;
}
