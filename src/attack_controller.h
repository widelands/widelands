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

#ifndef __S__WIDELANDS_ATTACK_CTRL_H
#define __S__WIDELANDS_ATTACK_CTRL_H

#include "immovable.h"
#include "instances.h"

#include "widelands.h"

#include "point.h"

#include <set>
#include <vector>

class Editor_Game_Base;
class Flag;
class MilitarySite;
class Soldier;
class Coords;
class Widelands_Map_Attack_Controller_Data_Packet;


uint32_t getMaxAttackSoldiers(const Editor_Game_Base &, const Flag &, const Player_Number);

struct AttackController : public BaseImmovable {
	friend class Widelands_Map_Attack_Controller_Data_Packet;

	AttackController(Editor_Game_Base* eg, Flag* flag, int32_t attacker, int32_t defender);
	AttackController(Editor_Game_Base &);
	~AttackController();
	void launchAttack(uint32_t nrAttackers);

	//Methods inherited by BaseImmovable
	virtual int32_t  get_type    () const throw () {return ATTACKCONTROLLER;}
	virtual int32_t  get_size    () const throw () {return SMALL;}
	virtual bool get_passable() const throw () {return false;}
	virtual void draw (const Editor_Game_Base &, RenderTarget &, const FCoords, const Point) {}
	virtual void act (Game*, uint32_t);
	virtual void init(Editor_Game_Base*);
	virtual void cleanup (Editor_Game_Base*);
	//end inherited

	void moveToReached(Soldier* soldier);
	void soldierDied(Soldier* soldier);
	void soldierWon(Soldier* soldier);

	int32_t getAttackingPlayer() {return attackingPlayer;};
	int32_t getDefendingPlayer() {return defendingPlayer;};
	Flag* getFlag() {return flag;};
	Editor_Game_Base & egbase() {return *m_egbase;}

private:
	struct BattleSoldier {
		Soldier* soldier;
		MilitarySite* origin;
		Coords battleGround;
		bool attacker;
		bool arrived;
		bool fighting;
	};

	bool battleGroundOccupied(Coords coords);
	void calcBattleGround(BattleSoldier*, int32_t);

	bool launchAllSoldiers(bool attackers, int32_t nrLaunch);
	void launchSoldiersOfMilitarySite(MilitarySite* militarySite, uint32_t nrLaunch, bool attackers);
	bool moveToBattle(Soldier* soldier, MilitarySite* militarySite, bool attackers);

	bool startBattle(Soldier*, bool);
	void removeSoldier(Soldier*);
	uint32_t getBattleSoldierIndex(Soldier*);
	bool opponentsLeft(Soldier* soldier);

	std::vector<BattleSoldier> involvedSoldiers;

	typedef std::set<OPtr<MilitarySite> > MilitarySiteSet;
	MilitarySiteSet involvedMilitarySites;

	int32_t attackingPlayer;
	int32_t defendingPlayer;
	uint32_t totallyLaunched;
	bool attackedMsEmpty;
	Flag* flag;
	Editor_Game_Base* m_egbase;

	// Load/save support
protected:
	struct Loader : public BaseImmovable::Loader {
		virtual void load(FileRead&);
		virtual void load_pointers();

		struct BattleSoldierData {
			uint32_t soldier;
			uint32_t origin;
		};

		uint32_t flag;
		std::vector<BattleSoldierData> soldiers;
		std::vector<uint32_t> militarySites;
	};

public:
	// Remove as soon as we fully support the new system
	virtual bool has_new_save_support() {return true;}

	virtual void save(Editor_Game_Base*, Widelands_Map_Map_Object_Saver*, FileWrite&);
	static Map_Object::Loader* load(Editor_Game_Base*, Widelands_Map_Map_Object_Loader*, FileRead&);
};

#endif
