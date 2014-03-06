/*
 * Copyright (C) 2004, 2006-2011, 2013 by the Widelands Development Team
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

#ifndef PLAYERCOMMAND_H
#define PLAYERCOMMAND_H

#include "logic/cmd_queue.h"
#include "economy/flag.h"
#include "logic/message_id.h"
#include "logic/path.h"
#include "logic/trainingsite.h"
#include "logic/warehouse.h"
#include "logic/worker.h"

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
class PlayerCommand : public GameLogicCommand {
public:
	PlayerCommand (int32_t time, Player_Number);

	/// For savegame loading
	PlayerCommand() : GameLogicCommand(0), m_sender(0), m_cmdserial(0) {}

	Player_Number sender   () const {return m_sender;}
	uint32_t      cmdserial() const {return m_cmdserial;}
	void set_cmdserial(const uint32_t s) {m_cmdserial = s;}

	virtual void serialize (StreamWrite &) = 0;
	static Widelands::PlayerCommand * deserialize (StreamRead &);

	// Call these from child classes
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

private:
	Player_Number m_sender;
	uint32_t      m_cmdserial;
};

struct Cmd_Bulldoze:public PlayerCommand {
	Cmd_Bulldoze() : PlayerCommand(), serial(0), recurse(0) {} // For savegame loading
	Cmd_Bulldoze
		(const int32_t t, const int32_t p,
		 PlayerImmovable & pi,
		 const bool _recurse = false)
		: PlayerCommand(t, p), serial(pi.serial()), recurse(_recurse)
	{}

	Cmd_Bulldoze (StreamRead &);

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_BULLDOZE;}

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
	bool   recurse;
};

struct Cmd_Build:public PlayerCommand {
	Cmd_Build() : PlayerCommand() {} // For savegame loading
	Cmd_Build
		(const int32_t        _duetime,
		 const int32_t        p,
		 const Coords         c,
		 const Building_Index i)
		: PlayerCommand(_duetime, p), coords(c), bi(i)
	{}

	Cmd_Build (StreamRead &);

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_BUILD;}

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Coords         coords;
	Building_Index bi;
};

struct Cmd_BuildFlag:public PlayerCommand {
	Cmd_BuildFlag() : PlayerCommand() {} // For savegame loading
	Cmd_BuildFlag (const int32_t t, const int32_t p, const Coords c) :
		PlayerCommand(t, p), coords(c)
	{}

	Cmd_BuildFlag (StreamRead &);

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_FLAG;}

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Coords coords;
};

struct Cmd_BuildRoad:public PlayerCommand {
	Cmd_BuildRoad() :
		PlayerCommand(), path(nullptr), start(), nsteps(0), steps(nullptr) {} // For savegame loading
	Cmd_BuildRoad (int32_t, int32_t, Path &);
	Cmd_BuildRoad (StreamRead &);

	virtual ~Cmd_BuildRoad ();

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_BUILDROAD;}

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Path                       * path;
	Coords                       start;
	Path::Step_Vector::size_type nsteps;
	char                       * steps;
};

struct Cmd_FlagAction : public PlayerCommand {
	Cmd_FlagAction() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_FlagAction (const int32_t t, const int32_t p, const Flag & f) :
		PlayerCommand(t, p), serial(f.serial())
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_FLAGACTION;}


	Cmd_FlagAction (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct Cmd_StartStopBuilding : public PlayerCommand {
	Cmd_StartStopBuilding() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_StartStopBuilding (const int32_t t, const Player_Number p, Building & b)
		: PlayerCommand(t, p), serial(b.serial())
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_STOPBUILDING;}

	Cmd_StartStopBuilding (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct Cmd_MilitarySiteSetSoldierPreference : public PlayerCommand {
	Cmd_MilitarySiteSetSoldierPreference() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_MilitarySiteSetSoldierPreference (const int32_t t, const Player_Number p, Building & b, uint8_t prefs)
		: PlayerCommand(t, p), serial(b.serial()), preference(prefs)
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_MILITARYSITESETSOLDIERPREFERENCE;}

	Cmd_MilitarySiteSetSoldierPreference (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
	uint8_t preference;
};
struct Cmd_StartOrCancelExpedition : public PlayerCommand {
	Cmd_StartOrCancelExpedition() : PlayerCommand() {} // For savegame loading
	Cmd_StartOrCancelExpedition (int32_t const t, Player_Number const p, Building & b)
		: PlayerCommand(t, p), serial(b.serial())
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_PORT_START_EXPEDITION;}

	Cmd_StartOrCancelExpedition (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct Cmd_EnhanceBuilding:public PlayerCommand {
	Cmd_EnhanceBuilding() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_EnhanceBuilding
		(const int32_t        _duetime,
		 const int32_t        p,
		 Building           & b,
		 const Building_Index i)
		: PlayerCommand(_duetime, p), serial(b.serial()), bi(i)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_ENHANCEBUILDING;}

	Cmd_EnhanceBuilding (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
	Building_Index bi;
};

struct Cmd_DismantleBuilding:public PlayerCommand {
	Cmd_DismantleBuilding() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_DismantleBuilding
		(const int32_t t, const int32_t p,
		 PlayerImmovable & pi)
		: PlayerCommand(t, p), serial(pi.serial())
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_DISMANTLEBUILDING;}

	Cmd_DismantleBuilding (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct Cmd_EvictWorker : public PlayerCommand {
	Cmd_EvictWorker() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_EvictWorker
		(const int32_t t, const int32_t p,
		 Worker & w)
		: PlayerCommand(t, p), serial(w.serial())
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_EVICTWORKER;}

	Cmd_EvictWorker (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct Cmd_ShipScoutDirection : public PlayerCommand {
	Cmd_ShipScoutDirection() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_ShipScoutDirection
		(int32_t const t, Player_Number const p, Serial s, uint8_t direction)
		: PlayerCommand(t, p), serial(s), dir(direction)
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_SHIP_SCOUT;}

	Cmd_ShipScoutDirection (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
	uint8_t dir;
};

struct Cmd_ShipConstructPort : public PlayerCommand {
	Cmd_ShipConstructPort() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_ShipConstructPort
		(int32_t const t, Player_Number const p, Serial s, Coords c)
		: PlayerCommand(t, p), serial(s), coords(c)
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_SHIP_CONSTRUCT_PORT;}

	Cmd_ShipConstructPort (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
	Coords coords;
};

struct Cmd_ShipExploreIsland : public PlayerCommand {
	Cmd_ShipExploreIsland() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_ShipExploreIsland
		(int32_t const t, Player_Number const p, Serial s, bool cw)
		: PlayerCommand(t, p), serial(s), clockwise(cw)
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_SHIP_EXPLORE;}

	Cmd_ShipExploreIsland (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
	bool clockwise;
};

struct Cmd_ShipSink : public PlayerCommand {
	Cmd_ShipSink() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_ShipSink
		(int32_t const t, Player_Number const p, Serial s)
		: PlayerCommand(t, p), serial(s)
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_SHIP_SINK;}

	Cmd_ShipSink(StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct Cmd_ShipCancelExpedition : public PlayerCommand {
	Cmd_ShipCancelExpedition() : PlayerCommand(), serial(0) {} // For savegame loading
	Cmd_ShipCancelExpedition
		(int32_t const t, Player_Number const p, Serial s)
		: PlayerCommand(t, p), serial(s)
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_SHIP_CANCELEXPEDITION;}

	Cmd_ShipCancelExpedition(StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct Cmd_SetWarePriority : public PlayerCommand {
	// For savegame loading
	Cmd_SetWarePriority() :
		PlayerCommand(),
		m_serial(0),
		m_type(0),
		m_index(),
		m_priority(0)
	{}
	Cmd_SetWarePriority
		(int32_t duetime, Player_Number sender,
		 PlayerImmovable &,
		 int32_t type, Ware_Index index, int32_t priority);

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_SETWAREPRIORITY;}

	Cmd_SetWarePriority(StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial m_serial;
	int32_t m_type; ///< this is always WARE right now
	Ware_Index m_index;
	int32_t m_priority;
};

struct Cmd_SetWareMaxFill : public PlayerCommand {
	Cmd_SetWareMaxFill() : PlayerCommand(), m_serial(0), m_index(), m_max_fill(0) {} // For savegame loading
	Cmd_SetWareMaxFill
		(int32_t duetime, Player_Number,
		 PlayerImmovable &,
		 Ware_Index, uint32_t maxfill);

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_SETWAREMAXFILL;}

	Cmd_SetWareMaxFill(StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial m_serial;
	Ware_Index m_index;
	uint32_t m_max_fill;
};

struct Cmd_ChangeTargetQuantity : public PlayerCommand {
	Cmd_ChangeTargetQuantity() : PlayerCommand(), m_economy(0), m_ware_type() {} //  For savegame loading.
	Cmd_ChangeTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	Cmd_ChangeTargetQuantity(StreamRead &);

	virtual void serialize (StreamWrite &) override;

protected:
	uint32_t   economy  () const {return m_economy;}
	Ware_Index ware_type() const {return m_ware_type;}

private:
	uint32_t   m_economy;
	Ware_Index m_ware_type;
};


struct Cmd_SetWareTargetQuantity : public Cmd_ChangeTargetQuantity {
	Cmd_SetWareTargetQuantity() : Cmd_ChangeTargetQuantity(), m_permanent(0) {}
	Cmd_SetWareTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index,
		 uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_SETWARETARGETQUANTITY;}

	Cmd_SetWareTargetQuantity(StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	uint32_t m_permanent;
};

struct Cmd_ResetWareTargetQuantity : public Cmd_ChangeTargetQuantity {
	Cmd_ResetWareTargetQuantity() : Cmd_ChangeTargetQuantity(), m_ware_type() {}
	Cmd_ResetWareTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_RESETWARETARGETQUANTITY;}

	Cmd_ResetWareTargetQuantity(StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Ware_Index m_ware_type;
};

struct Cmd_SetWorkerTargetQuantity : public Cmd_ChangeTargetQuantity {
	Cmd_SetWorkerTargetQuantity() : Cmd_ChangeTargetQuantity(), m_permanent(0) {}
	Cmd_SetWorkerTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index,
		 uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_SETWORKERTARGETQUANTITY;}

	Cmd_SetWorkerTargetQuantity(StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	uint32_t m_permanent;
};

struct Cmd_ResetWorkerTargetQuantity : public Cmd_ChangeTargetQuantity {
	Cmd_ResetWorkerTargetQuantity() : Cmd_ChangeTargetQuantity(), m_ware_type() {}
	Cmd_ResetWorkerTargetQuantity
		(int32_t duetime, Player_Number sender,
		 uint32_t economy, Ware_Index index);

	//  Write/Read these commands to/from a file (for savegames).
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_RESETWORKERTARGETQUANTITY;}

	Cmd_ResetWorkerTargetQuantity(StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Ware_Index m_ware_type;
};

struct Cmd_ChangeTrainingOptions : public PlayerCommand {
	Cmd_ChangeTrainingOptions() : PlayerCommand(), serial(0), attribute(0), value(0) {} // For savegame loading
	Cmd_ChangeTrainingOptions
		(const int32_t    t,
		 const Player_Number p,
		 TrainingSite &   ts,
		 const int32_t    at,
		 const int32_t    val)
		: PlayerCommand(t, p), serial(ts.serial()), attribute(at), value(val)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_CHANGETRAININGOPTIONS;}

	Cmd_ChangeTrainingOptions (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
	int32_t attribute;
	int32_t value;
};

struct Cmd_DropSoldier : public PlayerCommand {
	Cmd_DropSoldier () : PlayerCommand(), serial(0), soldier(0) {} //  for savegames
	Cmd_DropSoldier
		(const int32_t    t,
		 const int32_t    p,
		 Building &       b,
		 const int32_t    _soldier)
		: PlayerCommand(t, p), serial(b.serial()), soldier(_soldier)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_DROPSOLDIER;}

	Cmd_DropSoldier(StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
	Serial soldier;
};

struct Cmd_ChangeSoldierCapacity : public PlayerCommand {
	Cmd_ChangeSoldierCapacity () : PlayerCommand(), serial(0), val(0) {} //  for savegames
	Cmd_ChangeSoldierCapacity
		(const int32_t t, const int32_t p, Building & b, const int32_t i)
		: PlayerCommand(t, p), serial(b.serial()), val(i)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_CHANGESOLDIERCAPACITY;}

	Cmd_ChangeSoldierCapacity (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial serial;
	int32_t val;
};

/////////////TESTING STUFF
struct Cmd_EnemyFlagAction : public PlayerCommand {
	Cmd_EnemyFlagAction() : PlayerCommand(), serial(0), number(0), retreat(0) {} // For savegame loading
	Cmd_EnemyFlagAction
		(const int32_t      t,
		 const int32_t      p,
		 const Flag &       f,
		 const uint32_t     num,
		 const uint32_t     ret)
		: PlayerCommand(t, p), serial(f.serial()), number(num), retreat(ret)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_ENEMYFLAGACTION;}

	Cmd_EnemyFlagAction (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	Serial        serial;
	uint8_t       number;
	uint8_t       retreat;
};

// This is at very early stage, more vars should be added
struct Cmd_ChangeMilitaryConfig : public PlayerCommand {
	Cmd_ChangeMilitaryConfig() : PlayerCommand(), retreat(0) {} // For savegame loading
	Cmd_ChangeMilitaryConfig
		(const int32_t      t,
		 const int32_t      p,
		 const uint32_t     ret)
		: PlayerCommand(t, p), retreat(ret)
	{}

	// Write these commands to a file (for savegames)
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	virtual uint8_t id() const override {return QUEUE_CMD_CHANGEMILITARYCONFIG;}

	Cmd_ChangeMilitaryConfig (StreamRead &);

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;

private:
	// By now only retreat info is stored
	uint8_t       retreat;
};


/// Abstract base for commands about a message.
struct PlayerMessageCommand : public PlayerCommand {
	PlayerMessageCommand () : PlayerCommand() {} //  for savegames
	PlayerMessageCommand
		(const uint32_t t, const Player_Number p, const Message_Id i)
		: PlayerCommand(t, p), m_message_id(i)
	{}

	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

	PlayerMessageCommand(StreamRead &);

	Message_Id message_id() const {return m_message_id;}

private:
	Message_Id m_message_id;
};

struct Cmd_MessageSetStatusRead : public PlayerMessageCommand {
	Cmd_MessageSetStatusRead () : PlayerMessageCommand() {}
	Cmd_MessageSetStatusRead
		(const uint32_t t, const Player_Number p, const Message_Id i)
		: PlayerMessageCommand(t, p, i)
	{}

	virtual uint8_t id() const override {return QUEUE_CMD_MESSAGESETSTATUSREAD;}

	Cmd_MessageSetStatusRead(StreamRead & des) : PlayerMessageCommand(des) {}

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;
};

struct Cmd_MessageSetStatusArchived : public PlayerMessageCommand {
	Cmd_MessageSetStatusArchived () : PlayerMessageCommand() {}
	Cmd_MessageSetStatusArchived
		(const uint32_t t, const Player_Number p, const Message_Id i)
		: PlayerMessageCommand(t, p, i)
	{}

	virtual uint8_t id() const override {return QUEUE_CMD_MESSAGESETSTATUSARCHIVED;}

	Cmd_MessageSetStatusArchived(StreamRead & des) : PlayerMessageCommand(des) {
	}

	virtual void execute (Game &) override;
	virtual void serialize (StreamWrite &) override;
};

/**
 * Command to change the stock policy for a ware or worker in a warehouse.
 */
struct Cmd_SetStockPolicy : PlayerCommand {
	Cmd_SetStockPolicy
		(int32_t time, Player_Number p,
		 Warehouse & wh, bool isworker, Ware_Index ware,
		 Warehouse::StockPolicy policy);

	virtual uint8_t id() const override;

	virtual void execute(Game & game) override;

	// Network (de-)serialization
	Cmd_SetStockPolicy(StreamRead & des);
	virtual void serialize(StreamWrite & ser) override;

	// Savegame functions
	Cmd_SetStockPolicy();
	void Write(FileWrite &, Editor_Game_Base &, Map_Map_Object_Saver  &) override;
	void Read (FileRead  &, Editor_Game_Base &, Map_Map_Object_Loader &) override;

private:
	Serial m_warehouse;
	bool m_isworker;
	Ware_Index m_ware;
	Warehouse::StockPolicy m_policy;
};

}

#endif
