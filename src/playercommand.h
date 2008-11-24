/*
 * Copyright (C) 2004, 2006-2008 by the Widelands Development Team
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

#ifndef PLAYERCOMMAND_H
#define PLAYERCOMMAND_H


#include "cmd_queue.h"
#include "building.h"
#include "transport.h"

namespace Widelands {

/**
 * PlayerCommand is for commands issued by players. It has the additional
 * ability to send itself over the network
 *
 * PlayerCommands are given serial numbers once they become authoritative
 * (e.g. after being acked by the server). The serial numbers must then be
 * reasonably unique (to be precise, they must be unique per duetime) and
 * the same across all hosts, to ensure parallel simulation.
 */
struct PlayerCommand : public GameLogicCommand {
	PlayerCommand (int32_t time, Player_Number);
	PlayerCommand() : GameLogicCommand(0), sender(0), cmdserial(0) {} // For savegame loading
	virtual ~PlayerCommand ();

	char get_sender() const {return sender;}
	uint32_t get_cmdserial() const {return cmdserial;}
	void set_cmdserial(uint32_t s) {cmdserial = s;}

	virtual void serialize (StreamWrite &) = 0;
	static Widelands::PlayerCommand * deserialize (StreamRead &);

	// Call these from child classes
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

private:
	Player_Number sender;
	uint32_t cmdserial;
};

struct Cmd_Bulldoze:public PlayerCommand {
	Cmd_Bulldoze() : PlayerCommand() {} // For savegame loading
	Cmd_Bulldoze (int32_t const t, int32_t const p, PlayerImmovable* pi)
		: PlayerCommand(t, p)
	{serial=pi->get_serial();}

	Cmd_Bulldoze (StreamRead &);

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_BULLDOZE;} // Get this command id

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Serial serial;
};

struct Cmd_Build:public PlayerCommand {
	Cmd_Build() : PlayerCommand() {} // For savegame loading
	Cmd_Build (int32_t t, int32_t p, const Coords& c, Building_Index i):PlayerCommand(t, p)
	{coords=c; id=i;}

	Cmd_Build (StreamRead &);

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_BUILD;} // Get this command id

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Coords         coords;
	Building_Index id;
};

struct Cmd_BuildFlag:public PlayerCommand {
	Cmd_BuildFlag() : PlayerCommand() {} // For savegame loading
	Cmd_BuildFlag (int32_t t, int32_t p, const Coords& c):PlayerCommand(t, p)
	{coords=c;}

	Cmd_BuildFlag (StreamRead &);

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_FLAG;} // Get this command id

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Coords coords;
};

struct Cmd_BuildRoad:public PlayerCommand {
	Cmd_BuildRoad() : PlayerCommand() {} // For savegame loading
	Cmd_BuildRoad (int32_t, int32_t, Path &);
	Cmd_BuildRoad (StreamRead &);

	virtual ~Cmd_BuildRoad ();

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_BUILDROAD;} // Get this command id

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Path                       * path;
	Coords                       start;
	Path::Step_Vector::size_type nsteps;
	char                       * steps;
};

struct Cmd_FlagAction:public PlayerCommand {
	Cmd_FlagAction() : PlayerCommand() {} // For savegame loading
	Cmd_FlagAction (int32_t t, int32_t p, Flag* f, int32_t a):PlayerCommand(t, p)
	{serial=f->get_serial(); action=a;}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_FLAGACTION;} // Get this command id


	Cmd_FlagAction (StreamRead &);

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Serial serial;
	int32_t action;
};

struct Cmd_StartStopBuilding : public PlayerCommand {
	Cmd_StartStopBuilding() : PlayerCommand() {} // For savegame loading
	Cmd_StartStopBuilding (int32_t t, int32_t p, Building* b):PlayerCommand(t, p)
	{serial=b->get_serial();}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_STOPBUILDING;} // Get this command id

	Cmd_StartStopBuilding (StreamRead &);

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Serial serial;
};

struct Cmd_EnhanceBuilding:public PlayerCommand {
	Cmd_EnhanceBuilding() : PlayerCommand() {} // For savegame loading
	Cmd_EnhanceBuilding (int32_t t, int32_t p, Building* b, Building_Index i):PlayerCommand(t, p)
	{serial=b->get_serial(); id=i;}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_ENHANCEBUILDING;} // Get this command id

	Cmd_EnhanceBuilding (StreamRead &);

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Serial serial;
	Building_Index id;
};

struct Cmd_SetWarePriority : public PlayerCommand {
	Cmd_SetWarePriority() : PlayerCommand() {} // For savegame loading
	Cmd_SetWarePriority
		(int32_t duetime, Player_Number sender,
		 PlayerImmovable* imm,
		 int32_t type, Ware_Index index, int32_t priority);

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_SETWAREPRIORITY;}

	Cmd_SetWarePriority(StreamRead &);

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Serial m_serial;
	int32_t m_type; ///< this is always WARE right now
	Ware_Index m_index;
	int32_t m_priority;
};


struct Cmd_ChangeTargetQuantity : public PlayerCommand {
	Cmd_ChangeTargetQuantity() : PlayerCommand() {} //  For savegame loading.
	Cmd_ChangeTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	Cmd_ChangeTargetQuantity(StreamRead &);

	virtual void serialize (StreamWrite &);

protected:
	uint32_t   economy  () const {return m_economy;}
	Ware_Index ware_type() const {return m_ware_type;}

private:
	uint32_t   m_economy;
	Ware_Index m_ware_type;
};


struct Cmd_SetTargetQuantity : public Cmd_ChangeTargetQuantity {
	Cmd_SetTargetQuantity() : Cmd_ChangeTargetQuantity() {}
	Cmd_SetTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index,
		 uint32_t permanent, uint32_t temporary);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_SETTARGETQUANTITY;}

	Cmd_SetTargetQuantity(StreamRead &);

	virtual void execute (Game *);
	virtual void serialize (StreamWrite &);

private:
	uint32_t m_permanent, m_temporary;
};

struct Cmd_ResetTargetQuantity : public Cmd_ChangeTargetQuantity {
	Cmd_ResetTargetQuantity() : Cmd_ChangeTargetQuantity() {}
	Cmd_ResetTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_RESETTARGETQUANTITY;}

	Cmd_ResetTargetQuantity(StreamRead &);

	virtual void execute (Game *);
	virtual void serialize (StreamWrite &);

private:
	uint32_t m_economy;
	Ware_Index m_ware_type;
};

struct Cmd_ChangeTrainingOptions : public PlayerCommand {
	Cmd_ChangeTrainingOptions() : PlayerCommand() {} // For savegame loading
	Cmd_ChangeTrainingOptions(int32_t t, int32_t p, Building* b, int32_t at, int32_t val):PlayerCommand(t, p)
	{serial=b->get_serial(); attribute=at; value=val;}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_CHANGETRAININGOPTIONS;} // Get this command id

	Cmd_ChangeTrainingOptions (StreamRead &);

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Serial serial;
	int32_t attribute;
	int32_t value;
};

struct Cmd_DropSoldier : public PlayerCommand {
	Cmd_DropSoldier () : PlayerCommand() {} //  for savegames
	Cmd_DropSoldier(int32_t t, int32_t p, Building* b, int32_t _soldier):PlayerCommand(t, p)
	{serial=b->get_serial(); soldier=_soldier;}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_DROPSOLDIER;} // Get this command id

	Cmd_DropSoldier(StreamRead &);

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Serial serial;
	Serial soldier;
};

struct Cmd_ChangeSoldierCapacity : public PlayerCommand {
	Cmd_ChangeSoldierCapacity () : PlayerCommand() {} //  for savegames
	Cmd_ChangeSoldierCapacity (int32_t t, int32_t p, Building* b, int32_t i):PlayerCommand(t, p)
	{serial=b->get_serial(); val=i;}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_CHANGESOLDIERCAPACITY;} // Get this command id

	Cmd_ChangeSoldierCapacity (StreamRead &);

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Serial serial;
	int32_t val;
};

/////////////TESTING STUFF
struct Cmd_EnemyFlagAction : public PlayerCommand {
	Cmd_EnemyFlagAction() : PlayerCommand() {} // For savegame loading
	Cmd_EnemyFlagAction
		(int32_t      const t,
		 int32_t      const p,
		 Flag const * const f,
		 int32_t      const a,
		 int32_t      const at,
		 int32_t      const num,
		 int32_t      const ty)
		:
		PlayerCommand(t, p),
		serial(f->get_serial()), action(a), attacker(at), number(num), type(ty)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &);
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &);

	virtual int32_t get_id() {return QUEUE_CMD_ENEMYFLAGACTION;} // Get this command id

	Cmd_EnemyFlagAction (StreamRead &);

	virtual void execute (Game* g);
	virtual void serialize (StreamWrite &);

private:
	Serial        serial;
	uint8_t       action;
	Player_Number attacker;
	uint8_t       number;
	uint8_t       type;
};

};

#endif
