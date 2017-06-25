/*
 * Copyright (C) 2004-2017 by the Widelands Development Team
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

#include "economy/flag.h"
#include "logic/cmd_queue.h"
#include "logic/map_objects/tribes/militarysite.h"
#include "logic/map_objects/tribes/ship.h"
#include "logic/map_objects/tribes/trainingsite.h"
#include "logic/map_objects/tribes/warehouse.h"
#include "logic/map_objects/tribes/worker.h"
#include "logic/message_id.h"
#include "logic/path.h"

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
	PlayerCommand(uint32_t time, PlayerNumber);

	/// For savegame loading
	PlayerCommand() : GameLogicCommand(0), sender_(0), cmdserial_(0) {
	}

	PlayerNumber sender() const {
		return sender_;
	}
	uint32_t cmdserial() const {
		return cmdserial_;
	}
	void set_cmdserial(const uint32_t s) {
		cmdserial_ = s;
	}

	virtual void serialize(StreamWrite&) = 0;
	static Widelands::PlayerCommand* deserialize(StreamRead&);

	// Call these from child classes
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	PlayerNumber sender_;
	uint32_t cmdserial_;
};

struct CmdBulldoze : public PlayerCommand {
	CmdBulldoze() : PlayerCommand(), serial(0), recurse(0) {
	}  // For savegame loading
	CmdBulldoze(const uint32_t t,
	            const int32_t p,
	            PlayerImmovable& pi,
	            const bool init_recurse = false)
	   : PlayerCommand(t, p), serial(pi.serial()), recurse(init_recurse) {
	}

	explicit CmdBulldoze(StreamRead&);

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kBulldoze;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	bool recurse;
};

struct CmdBuild : public PlayerCommand {
	CmdBuild() : PlayerCommand() {
	}  // For savegame loading
	CmdBuild(const uint32_t init_duetime, const int32_t p, const Coords& c, const DescriptionIndex i)
	   : PlayerCommand(init_duetime, p), coords(c), bi(i) {
	}

	explicit CmdBuild(StreamRead&);

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuild;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Coords coords;
	DescriptionIndex bi;
};

struct CmdBuildFlag : public PlayerCommand {
	CmdBuildFlag() : PlayerCommand() {
	}  // For savegame loading
	CmdBuildFlag(const uint32_t t, const int32_t p, const Coords& c)
	   : PlayerCommand(t, p), coords(c) {
	}

	explicit CmdBuildFlag(StreamRead&);

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kFlag;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Coords coords;
};

struct CmdBuildRoad : public PlayerCommand {
	CmdBuildRoad() : PlayerCommand(), path(nullptr), start(), nsteps(0), steps(nullptr) {
	}  // For savegame loading
	CmdBuildRoad(uint32_t, int32_t, Path&);
	explicit CmdBuildRoad(StreamRead&);

	virtual ~CmdBuildRoad();

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuildRoad;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Path* path;
	Coords start;
	Path::StepVector::size_type nsteps;
	char* steps;
};

struct CmdFlagAction : public PlayerCommand {
	CmdFlagAction() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdFlagAction(const uint32_t t, const int32_t p, const Flag& f)
	   : PlayerCommand(t, p), serial(f.serial()) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kFlagAction;
	}

	explicit CmdFlagAction(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
};

struct CmdStartStopBuilding : public PlayerCommand {
	CmdStartStopBuilding() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdStartStopBuilding(const uint32_t t, const PlayerNumber p, Building& b)
	   : PlayerCommand(t, p), serial(b.serial()) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kStopBuilding;
	}

	explicit CmdStartStopBuilding(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
};

struct CmdMilitarySiteSetSoldierPreference : public PlayerCommand {
	CmdMilitarySiteSetSoldierPreference() : PlayerCommand(), serial(0), preference(SoldierPreference::kNotSet) {
	}  // For savegame loading
	CmdMilitarySiteSetSoldierPreference(const uint32_t t,
	                                    const PlayerNumber p,
	                                    Building& b,
	                                    SoldierPreference prefs)
	   : PlayerCommand(t, p), serial(b.serial()), preference(prefs) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kMilitarysiteSetSoldierPreference;
	}

	explicit CmdMilitarySiteSetSoldierPreference(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	Widelands::SoldierPreference preference;
};
struct CmdStartOrCancelExpedition : public PlayerCommand {
	CmdStartOrCancelExpedition() : PlayerCommand() {
	}  // For savegame loading
	CmdStartOrCancelExpedition(uint32_t const t, PlayerNumber const p, Building& b)
	   : PlayerCommand(t, p), serial(b.serial()) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kPortStartExpedition;
	}

	explicit CmdStartOrCancelExpedition(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
};

struct CmdEnhanceBuilding : public PlayerCommand {
	CmdEnhanceBuilding() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdEnhanceBuilding(const uint32_t init_duetime,
	                   const int32_t p,
	                   Building& b,
	                   const DescriptionIndex i)
	   : PlayerCommand(init_duetime, p), serial(b.serial()), bi(i) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kEnhanceBuilding;
	}

	explicit CmdEnhanceBuilding(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	DescriptionIndex bi;
};

struct CmdDismantleBuilding : public PlayerCommand {
	CmdDismantleBuilding() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdDismantleBuilding(const uint32_t t, const int32_t p, PlayerImmovable& pi)
	   : PlayerCommand(t, p), serial(pi.serial()) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kDismantleBuilding;
	}

	explicit CmdDismantleBuilding(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
};

struct CmdEvictWorker : public PlayerCommand {
	CmdEvictWorker() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdEvictWorker(const uint32_t t, const int32_t p, Worker& w)
	   : PlayerCommand(t, p), serial(w.serial()) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kEvictWorker;
	}

	explicit CmdEvictWorker(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
};

struct CmdShipScoutDirection : public PlayerCommand {
	CmdShipScoutDirection() : PlayerCommand(), serial(0), dir(WalkingDir::IDLE) {
	}  // For savegame loading
	CmdShipScoutDirection(uint32_t const t, PlayerNumber const p, Serial s, WalkingDir direction)
	   : PlayerCommand(t, p), serial(s), dir(direction) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipScout;
	}

	explicit CmdShipScoutDirection(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	WalkingDir dir;
};

struct CmdShipConstructPort : public PlayerCommand {
	CmdShipConstructPort() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdShipConstructPort(uint32_t const t, PlayerNumber const p, Serial s, Coords c)
	   : PlayerCommand(t, p), serial(s), coords(c) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipConstructPort;
	}

	explicit CmdShipConstructPort(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	Coords coords;
};

struct CmdShipExploreIsland : public PlayerCommand {
	CmdShipExploreIsland() : PlayerCommand(), serial(0), island_explore_direction(IslandExploreDirection::kNotSet) {
	}  // For savegame loading
	CmdShipExploreIsland(uint32_t const t,
	                     PlayerNumber const p,
	                     Serial s,
	                     IslandExploreDirection direction)
	   : PlayerCommand(t, p), serial(s), island_explore_direction(direction) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipExplore;
	}

	explicit CmdShipExploreIsland(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	IslandExploreDirection island_explore_direction;
};

struct CmdShipSink : public PlayerCommand {
	CmdShipSink() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdShipSink(uint32_t const t, PlayerNumber const p, Serial s) : PlayerCommand(t, p), serial(s) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kSinkShip;
	}

	explicit CmdShipSink(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
};

struct CmdShipCancelExpedition : public PlayerCommand {
	CmdShipCancelExpedition() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdShipCancelExpedition(uint32_t const t, PlayerNumber const p, Serial s)
	   : PlayerCommand(t, p), serial(s) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipCancelExpedition;
	}

	explicit CmdShipCancelExpedition(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
};

struct CmdSetWarePriority : public PlayerCommand {
	// For savegame loading
	CmdSetWarePriority() : PlayerCommand(), serial_(0), type_(0), index_(), priority_(0) {
	}
	CmdSetWarePriority(uint32_t duetime,
	                   PlayerNumber sender,
	                   PlayerImmovable&,
	                   int32_t type,
	                   DescriptionIndex index,
	                   int32_t priority);

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetWarePriority;
	}

	explicit CmdSetWarePriority(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_;
	int32_t type_;  ///< this is always WARE right now
	DescriptionIndex index_;
	int32_t priority_;
};

struct CmdSetInputMaxFill : public PlayerCommand {
	CmdSetInputMaxFill() : PlayerCommand(), serial_(0), index_(), type_(wwWARE), max_fill_(0) {
	}  // For savegame loading
	CmdSetInputMaxFill(uint32_t duetime,
	                   PlayerNumber,
	                   PlayerImmovable&,
	                   DescriptionIndex,
	                   WareWorker,
	                   uint32_t maxfill);

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetInputMaxFill;
	}

	explicit CmdSetInputMaxFill(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_;
	DescriptionIndex index_;
	WareWorker type_;
	uint32_t max_fill_;
};

struct CmdChangeTargetQuantity : public PlayerCommand {
	CmdChangeTargetQuantity() : PlayerCommand(), economy_(0), ware_type_() {
	}  //  For savegame loading.
	CmdChangeTargetQuantity(uint32_t duetime,
	                        PlayerNumber sender,
	                        uint32_t economy,
	                        DescriptionIndex index);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	explicit CmdChangeTargetQuantity(StreamRead&);

	void serialize(StreamWrite&) override;

protected:
	uint32_t economy() const {
		return economy_;
	}
	DescriptionIndex ware_type() const {
		return ware_type_;
	}

private:
	uint32_t economy_;
	DescriptionIndex ware_type_;
};

struct CmdSetWareTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWareTargetQuantity() : CmdChangeTargetQuantity(), permanent_(0) {
	}
	CmdSetWareTargetQuantity(uint32_t duetime,
	                         PlayerNumber sender,
	                         uint32_t economy,
	                         DescriptionIndex index,
	                         uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetWareTargetQuantity;
	}

	explicit CmdSetWareTargetQuantity(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	uint32_t permanent_;
};

struct CmdResetWareTargetQuantity : public CmdChangeTargetQuantity {
	CmdResetWareTargetQuantity() : CmdChangeTargetQuantity() {
	}
	CmdResetWareTargetQuantity(uint32_t duetime,
	                           PlayerNumber sender,
	                           uint32_t economy,
	                           DescriptionIndex index);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kResetWareTargetQuantity;
	}

	explicit CmdResetWareTargetQuantity(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;
};

struct CmdSetWorkerTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWorkerTargetQuantity() : CmdChangeTargetQuantity(), permanent_(0) {
	}
	CmdSetWorkerTargetQuantity(uint32_t duetime,
	                           PlayerNumber sender,
	                           uint32_t economy,
	                           DescriptionIndex index,
	                           uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetWorkerTargetQuantity;
	}

	explicit CmdSetWorkerTargetQuantity(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	uint32_t permanent_;
};

struct CmdResetWorkerTargetQuantity : public CmdChangeTargetQuantity {
	CmdResetWorkerTargetQuantity() : CmdChangeTargetQuantity() {
	}
	CmdResetWorkerTargetQuantity(uint32_t duetime,
	                             PlayerNumber sender,
	                             uint32_t economy,
	                             DescriptionIndex index);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kResetWorkerTargetQuantity;
	}

	explicit CmdResetWorkerTargetQuantity(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;
};

struct CmdChangeTrainingOptions : public PlayerCommand {
	CmdChangeTrainingOptions()
	   : PlayerCommand(), serial(0), attribute(TrainingAttribute::kHealth), value(0) {
	}  // For savegame loading
	CmdChangeTrainingOptions(const uint32_t t,
	                         const PlayerNumber p,
	                         TrainingSite& ts,
	                         const TrainingAttribute at,
	                         const int32_t val)
	   : PlayerCommand(t, p), serial(ts.serial()), attribute(at), value(val) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kChangeTrainingOptions;
	}

	explicit CmdChangeTrainingOptions(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	TrainingAttribute attribute;
	int32_t value;
};

struct CmdDropSoldier : public PlayerCommand {
	CmdDropSoldier() : PlayerCommand(), serial(0), soldier(0) {
	}  //  for savegames
	CmdDropSoldier(const uint32_t t, const int32_t p, Building& b, const int32_t init_soldier)
	   : PlayerCommand(t, p), serial(b.serial()), soldier(init_soldier) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kDropSoldier;
	}

	explicit CmdDropSoldier(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	Serial soldier;
};

struct CmdChangeSoldierCapacity : public PlayerCommand {
	CmdChangeSoldierCapacity() : PlayerCommand(), serial(0), val(0) {
	}  //  for savegames
	CmdChangeSoldierCapacity(const uint32_t t, const int32_t p, Building& b, const int32_t i)
	   : PlayerCommand(t, p), serial(b.serial()), val(i) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kChangeSoldierCapacity;
	}

	explicit CmdChangeSoldierCapacity(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	int32_t val;
};

/////////////TESTING STUFF
struct CmdEnemyFlagAction : public PlayerCommand {
	CmdEnemyFlagAction() : PlayerCommand(), serial(0), number(0) {
	}  // For savegame loading
	CmdEnemyFlagAction(uint32_t t, int32_t p, const Flag& f, uint32_t num)
	   : PlayerCommand(t, p), serial(f.serial()), number(num) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kEnemyFlagAction;
	}

	explicit CmdEnemyFlagAction(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	uint8_t number;
};

/// Abstract base for commands about a message.
struct PlayerMessageCommand : public PlayerCommand {
	PlayerMessageCommand() : PlayerCommand() {
	}  //  for savegames
	PlayerMessageCommand(const uint32_t t, const PlayerNumber p, const MessageId& i)
	   : PlayerCommand(t, p), message_id_(i) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	explicit PlayerMessageCommand(StreamRead&);

	MessageId message_id() const {
		return message_id_;
	}

private:
	MessageId message_id_;
};

struct CmdMessageSetStatusRead : public PlayerMessageCommand {
	CmdMessageSetStatusRead() : PlayerMessageCommand() {
	}
	CmdMessageSetStatusRead(const uint32_t t, const PlayerNumber p, const MessageId& i)
	   : PlayerMessageCommand(t, p, i) {
	}

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kMessageSetStatusRead;
	}

	explicit CmdMessageSetStatusRead(StreamRead& des) : PlayerMessageCommand(des) {
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;
};

struct CmdMessageSetStatusArchived : public PlayerMessageCommand {
	CmdMessageSetStatusArchived() : PlayerMessageCommand() {
	}
	CmdMessageSetStatusArchived(const uint32_t t, const PlayerNumber p, const MessageId& i)
	   : PlayerMessageCommand(t, p, i) {
	}

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kMessageSetStatusArchived;
	}

	explicit CmdMessageSetStatusArchived(StreamRead& des) : PlayerMessageCommand(des) {
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;
};

/**
 * Command to change the stock policy for a ware or worker in a warehouse.
 */
struct CmdSetStockPolicy : PlayerCommand {
	CmdSetStockPolicy(uint32_t time,
	                  PlayerNumber p,
	                  Warehouse& wh,
	                  bool isworker,
	                  DescriptionIndex ware,
	                  Warehouse::StockPolicy policy);

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetStockPolicy;
	}

	void execute(Game& game) override;

	// Network (de-)serialization
	explicit CmdSetStockPolicy(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	// Savegame functions
	CmdSetStockPolicy();
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial warehouse_;
	bool isworker_;
	DescriptionIndex ware_;
	Warehouse::StockPolicy policy_;
};
}

#endif  // end of include guard: WL_LOGIC_PLAYERCOMMAND_H
