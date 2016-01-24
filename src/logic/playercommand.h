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

#ifndef WL_LOGIC_PLAYERCOMMAND_H
#define WL_LOGIC_PLAYERCOMMAND_H

#include <memory>

#include "logic/cmd_queue.h"
#include "economy/flag.h"
#include "logic/message_id.h"
#include "logic/path.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"

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
	PlayerCommand (uint32_t time, PlayerNumber);

	/// For savegame loading
	PlayerCommand() : GameLogicCommand(0), m_sender(0), m_cmdserial(0) {}

	PlayerNumber sender   () const {return m_sender;}
	uint32_t      cmdserial() const {return m_cmdserial;}
	void set_cmdserial(const uint32_t s) {m_cmdserial = s;}

	virtual void serialize (StreamWrite &) = 0;
	static Widelands::PlayerCommand * deserialize (StreamRead &);

	// Call these from child classes
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

private:
	PlayerNumber m_sender;
	uint32_t      m_cmdserial;
};

struct CmdBulldoze:public PlayerCommand {
	CmdBulldoze() : PlayerCommand(), serial(0), recurse(0) {} // For savegame loading
	CmdBulldoze
		(const uint32_t t, const int32_t p,
		 PlayerImmovable & pi,
		 const bool _recurse = false)
		: PlayerCommand(t, p), serial(pi.serial()), recurse(_recurse)
	{}

	CmdBulldoze (StreamRead &);

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kBulldoze;}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	bool   recurse;
};

struct CmdBuild:public PlayerCommand {
	CmdBuild() : PlayerCommand() {} // For savegame loading
	CmdBuild
		(const uint32_t        _duetime,
		 const int32_t        p,
		 const Coords         c,
		 const DescriptionIndex i)
		: PlayerCommand(_duetime, p), coords(c), bi(i)
	{}

	CmdBuild (StreamRead &);

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kBuild;}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Coords         coords;
	DescriptionIndex bi;
};

struct CmdBuildFlag:public PlayerCommand {
	CmdBuildFlag() : PlayerCommand() {} // For savegame loading
	CmdBuildFlag (const uint32_t t, const int32_t p, const Coords c) :
		PlayerCommand(t, p), coords(c)
	{}

	CmdBuildFlag (StreamRead &);

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kFlag;}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Coords coords;
};

struct CmdBuildRoad:public PlayerCommand {
	CmdBuildRoad() :
		PlayerCommand(), path(nullptr), start(), nsteps(0), steps(nullptr) {} // For savegame loading
	CmdBuildRoad (uint32_t, int32_t, Path &);
	CmdBuildRoad (StreamRead &);

	virtual ~CmdBuildRoad ();

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kBuildRoad;}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Path                       * path;
	Coords                       start;
	Path::StepVector::size_type nsteps;
	char                       * steps;
};

struct CmdFlagAction : public PlayerCommand {
	CmdFlagAction() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdFlagAction (const uint32_t t, const int32_t p, const Flag & f) :
		PlayerCommand(t, p), serial(f.serial())
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kFlagAction;}


	CmdFlagAction (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdStartStopBuilding : public PlayerCommand {
	CmdStartStopBuilding() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdStartStopBuilding (const uint32_t t, const PlayerNumber p, Building & b)
		: PlayerCommand(t, p), serial(b.serial())
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kStopBuilding;}

	CmdStartStopBuilding (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdMilitarySiteSetSoldierPreference : public PlayerCommand {
	CmdMilitarySiteSetSoldierPreference() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdMilitarySiteSetSoldierPreference (const uint32_t t, const PlayerNumber p, Building & b, uint8_t prefs)
		: PlayerCommand(t, p), serial(b.serial()), preference(prefs)
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kMilitarysiteSetSoldierPreference;}

	CmdMilitarySiteSetSoldierPreference (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	uint8_t preference;
};
struct CmdStartOrCancelExpedition : public PlayerCommand {
	CmdStartOrCancelExpedition() : PlayerCommand() {} // For savegame loading
	CmdStartOrCancelExpedition (uint32_t const t, PlayerNumber const p, Building & b)
		: PlayerCommand(t, p), serial(b.serial())
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kPortStartExpedition;}

	CmdStartOrCancelExpedition (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdEnhanceBuilding:public PlayerCommand {
	CmdEnhanceBuilding() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdEnhanceBuilding
		(const uint32_t        _duetime,
		 const int32_t        p,
		 Building           & b,
		 const DescriptionIndex i)
		: PlayerCommand(_duetime, p), serial(b.serial()), bi(i)
	{}

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kEnhanceBuilding;}

	CmdEnhanceBuilding (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	DescriptionIndex bi;
};

struct CmdDismantleBuilding:public PlayerCommand {
	CmdDismantleBuilding() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdDismantleBuilding
		(const uint32_t t, const int32_t p,
		 PlayerImmovable & pi)
		: PlayerCommand(t, p), serial(pi.serial())
	{}

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kDismantleBuilding;}

	CmdDismantleBuilding (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdEvictWorker : public PlayerCommand {
	CmdEvictWorker() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdEvictWorker
		(const uint32_t t, const int32_t p,
		 Worker & w)
		: PlayerCommand(t, p), serial(w.serial())
	{}

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kEvictWorker;}

	CmdEvictWorker (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdShipScoutDirection : public PlayerCommand {
	CmdShipScoutDirection() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipScoutDirection
		(uint32_t const t, PlayerNumber const p, Serial s, WalkingDir direction)
		: PlayerCommand(t, p), serial(s), dir(direction)
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kShipScout;}

	CmdShipScoutDirection (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	WalkingDir dir;
};

struct CmdShipConstructPort : public PlayerCommand {
	CmdShipConstructPort() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipConstructPort
		(uint32_t const t, PlayerNumber const p, Serial s, Coords c)
		: PlayerCommand(t, p), serial(s), coords(c)
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kShipConstructPort;}

	CmdShipConstructPort (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	Coords coords;
};

struct CmdShipExploreIsland : public PlayerCommand {
	CmdShipExploreIsland() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipExploreIsland
		(uint32_t const t, PlayerNumber const p, Serial s, IslandExploreDirection direction)
		: PlayerCommand(t, p), serial(s), island_explore_direction(direction)
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kShipExplore;}

	CmdShipExploreIsland (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	IslandExploreDirection island_explore_direction;
};

struct CmdShipSink : public PlayerCommand {
	CmdShipSink() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipSink
		(uint32_t const t, PlayerNumber const p, Serial s)
		: PlayerCommand(t, p), serial(s)
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kSinkShip;}

	CmdShipSink(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdShipCancelExpedition : public PlayerCommand {
	CmdShipCancelExpedition() : PlayerCommand(), serial(0) {} // For savegame loading
	CmdShipCancelExpedition
		(uint32_t const t, PlayerNumber const p, Serial s)
		: PlayerCommand(t, p), serial(s)
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kShipCancelExpedition;}

	CmdShipCancelExpedition(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
};

struct CmdSetWarePriority : public PlayerCommand {
	// For savegame loading
	CmdSetWarePriority() :
		PlayerCommand(),
		m_serial(0),
		m_type(0),
		m_index(),
		m_priority(0)
	{}
	CmdSetWarePriority
		(uint32_t duetime, PlayerNumber sender,
		 PlayerImmovable &,
		 int32_t type, DescriptionIndex index, int32_t priority);

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kSetWarePriority;}

	CmdSetWarePriority(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial m_serial;
	int32_t m_type; ///< this is always WARE right now
	DescriptionIndex m_index;
	int32_t m_priority;
};

struct CmdSetWareMaxFill : public PlayerCommand {
	CmdSetWareMaxFill() : PlayerCommand(), m_serial(0), m_index(), m_max_fill(0) {} // For savegame loading
	CmdSetWareMaxFill
		(uint32_t duetime, PlayerNumber,
		 PlayerImmovable &,
		 DescriptionIndex, uint32_t maxfill);

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kSetWareMaxFill;}

	CmdSetWareMaxFill(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial m_serial;
	DescriptionIndex m_index;
	uint32_t m_max_fill;
};

struct CmdChangeTargetQuantity : public PlayerCommand {
	CmdChangeTargetQuantity() : PlayerCommand(), m_economy(0), m_ware_type() {} //  For savegame loading.
	CmdChangeTargetQuantity
		(uint32_t duetime, PlayerNumber sender,
		 uint32_t economy, DescriptionIndex index);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	CmdChangeTargetQuantity(StreamRead &);

	void serialize (StreamWrite &) override;

protected:
	uint32_t   economy  () const {return m_economy;}
	DescriptionIndex ware_type() const {return m_ware_type;}

private:
	uint32_t   m_economy;
	DescriptionIndex m_ware_type;
};


struct CmdSetWareTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWareTargetQuantity() : CmdChangeTargetQuantity(), m_permanent(0) {}
	CmdSetWareTargetQuantity
		(uint32_t duetime, PlayerNumber sender,
		 uint32_t economy, DescriptionIndex index,
		 uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kSetWareTargetQuantity;}

	CmdSetWareTargetQuantity(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	uint32_t m_permanent;
};

struct CmdResetWareTargetQuantity : public CmdChangeTargetQuantity {
	CmdResetWareTargetQuantity() : CmdChangeTargetQuantity() {}
	CmdResetWareTargetQuantity
		(uint32_t duetime, PlayerNumber sender,
		 uint32_t economy, DescriptionIndex index);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kResetWareTargetQuantity;}

	CmdResetWareTargetQuantity(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;
};

struct CmdSetWorkerTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWorkerTargetQuantity() : CmdChangeTargetQuantity(), m_permanent(0) {}
	CmdSetWorkerTargetQuantity
		(uint32_t duetime, PlayerNumber sender,
		 uint32_t economy, DescriptionIndex index,
		 uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kSetWorkerTargetQuantity;}

	CmdSetWorkerTargetQuantity(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	uint32_t m_permanent;
};

struct CmdResetWorkerTargetQuantity : public CmdChangeTargetQuantity {
	CmdResetWorkerTargetQuantity() : CmdChangeTargetQuantity() {}
	CmdResetWorkerTargetQuantity
		(uint32_t duetime, PlayerNumber sender,
		 uint32_t economy, DescriptionIndex index);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kResetWorkerTargetQuantity;}

	CmdResetWorkerTargetQuantity(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;
};

struct CmdChangeTrainingOptions : public PlayerCommand {
	CmdChangeTrainingOptions() : PlayerCommand(), serial(0), attribute(0), value(0) {} // For savegame loading
	CmdChangeTrainingOptions
		(const uint32_t    t,
		 const PlayerNumber p,
		 TrainingSite &   ts,
		 const int32_t    at,
		 const int32_t    val)
		: PlayerCommand(t, p), serial(ts.serial()), attribute(at), value(val)
	{}

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kChangeTrainingOptions;}

	CmdChangeTrainingOptions (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	int32_t attribute;
	int32_t value;
};

struct CmdDropSoldier : public PlayerCommand {
	CmdDropSoldier () : PlayerCommand(), serial(0), soldier(0) {} //  for savegames
	CmdDropSoldier
		(const uint32_t    t,
		 const int32_t    p,
		 Building &       b,
		 const int32_t    _soldier)
		: PlayerCommand(t, p), serial(b.serial()), soldier(_soldier)
	{}

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kDropSoldier;}

	CmdDropSoldier(StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	Serial soldier;
};

struct CmdChangeSoldierCapacity : public PlayerCommand {
	CmdChangeSoldierCapacity () : PlayerCommand(), serial(0), val(0) {} //  for savegames
	CmdChangeSoldierCapacity
		(const uint32_t t, const int32_t p, Building & b, const int32_t i)
		: PlayerCommand(t, p), serial(b.serial()), val(i)
	{}

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kChangeSoldierCapacity;}

	CmdChangeSoldierCapacity (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial serial;
	int32_t val;
};

/////////////TESTING STUFF
struct CmdEnemyFlagAction : public PlayerCommand {
	CmdEnemyFlagAction() : PlayerCommand(), serial(0), number(0) {} // For savegame loading
	CmdEnemyFlagAction(uint32_t t, int32_t p, const Flag& f, uint32_t num)
	   : PlayerCommand(t, p), serial(f.serial()), number(num) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	QueueCommandTypes id() const override {return QueueCommandTypes::kEnemyFlagAction;}

	CmdEnemyFlagAction (StreamRead &);

	void execute (Game &) override;
	void serialize (StreamWrite &) override;

private:
	Serial        serial;
	uint8_t       number;
};

/// Abstract base for commands about a message.
struct PlayerMessageCommand : public PlayerCommand {
	PlayerMessageCommand () : PlayerCommand() {} //  for savegames
	PlayerMessageCommand
		(const uint32_t t, const PlayerNumber p, const MessageId i)
		: PlayerCommand(t, p), m_message_id(i)
	{}

	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

	PlayerMessageCommand(StreamRead &);

	MessageId message_id() const {return m_message_id;}

private:
	MessageId m_message_id;
};

struct CmdMessageSetStatusRead : public PlayerMessageCommand {
	CmdMessageSetStatusRead () : PlayerMessageCommand() {}
	CmdMessageSetStatusRead
		(const uint32_t t, const PlayerNumber p, const MessageId i)
		: PlayerMessageCommand(t, p, i)
	{}

	QueueCommandTypes id() const override {return QueueCommandTypes::kMessageSetStatusRead;}

	CmdMessageSetStatusRead(StreamRead & des) : PlayerMessageCommand(des) {}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;
};

struct CmdMessageSetStatusArchived : public PlayerMessageCommand {
	CmdMessageSetStatusArchived () : PlayerMessageCommand() {}
	CmdMessageSetStatusArchived
		(const uint32_t t, const PlayerNumber p, const MessageId i)
		: PlayerMessageCommand(t, p, i)
	{}

	QueueCommandTypes id() const override {return QueueCommandTypes::kMessageSetStatusArchived;}

	CmdMessageSetStatusArchived(StreamRead & des) : PlayerMessageCommand(des) {
	}

	void execute (Game &) override;
	void serialize (StreamWrite &) override;
};

/**
 * Command to change the stock policy for a ware or worker in a warehouse.
 */
struct CmdSetStockPolicy : PlayerCommand {
	CmdSetStockPolicy
		(uint32_t time, PlayerNumber p,
		 Warehouse & wh, bool isworker, DescriptionIndex ware,
		 Warehouse::StockPolicy policy);

	QueueCommandTypes id() const override {return QueueCommandTypes::kSetStockPolicy;}

	void execute(Game & game) override;

	// Network (de-)serialization
	CmdSetStockPolicy(StreamRead & des);
	void serialize(StreamWrite & ser) override;

	// Savegame functions
	CmdSetStockPolicy();
	void write(FileWrite &, EditorGameBase &, MapObjectSaver  &) override;
	void read (FileRead  &, EditorGameBase &, MapObjectLoader &) override;

private:
	Serial m_warehouse;
	bool m_isworker;
	DescriptionIndex m_ware;
	Warehouse::StockPolicy m_policy;
};

}

#endif  // end of include guard: WL_LOGIC_PLAYERCOMMAND_H
