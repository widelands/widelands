/*
 * Copyright (C) 2004-2022 by the Widelands Development Team
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
	PlayerCommand(const Time& time, PlayerNumber);

	/// For savegame loading
	PlayerCommand() : GameLogicCommand(Time(0)), sender_(0), cmdserial_(0) {
	}

	void write_id_and_sender(StreamWrite& ser);

	PlayerNumber sender() const {
		return sender_;
	}
	uint32_t cmdserial() const {
		return cmdserial_;
	}
	void set_cmdserial(const uint32_t s) {
		cmdserial_ = s;
	}

	// For networking and replays
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
	CmdBulldoze(const Time& t, const int32_t p, PlayerImmovable& pi, const bool init_recurse = false)
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
	CmdBuild() : PlayerCommand(), bi(0) {
	}  // For savegame loading
	CmdBuild(const Time& init_duetime, const int32_t p, const Coords& c, const DescriptionIndex i)
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
	CmdBuildFlag(const Time& t, const int32_t p, const Coords& c) : PlayerCommand(t, p), coords(c) {
	}

	explicit CmdBuildFlag(StreamRead&);

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuildFlag;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Coords coords;
};

struct CmdBuildRoad : public PlayerCommand {
	CmdBuildRoad() : PlayerCommand(), path(nullptr), start(), nsteps(0), steps(nullptr) {
	}  // For savegame loading
	CmdBuildRoad(const Time&, int32_t, Path&);
	explicit CmdBuildRoad(StreamRead&);

	~CmdBuildRoad() override = default;

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuildRoad;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	std::unique_ptr<Path> path;
	Coords start;
	Path::StepVector::size_type nsteps;
	std::unique_ptr<uint8_t[]> steps;
};

struct CmdBuildWaterway : public PlayerCommand {
	CmdBuildWaterway() : PlayerCommand(), path(nullptr), start(), nsteps(0), steps(nullptr) {
	}  // For savegame loading
	CmdBuildWaterway(const Time&, int32_t, Path&);
	explicit CmdBuildWaterway(StreamRead&);

	~CmdBuildWaterway() override = default;

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuildWaterway;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	std::unique_ptr<Path> path;
	Coords start;
	Path::StepVector::size_type nsteps;
	std::unique_ptr<uint8_t[]> steps;
};

struct CmdFlagAction : public PlayerCommand {
	CmdFlagAction() : PlayerCommand(), serial_(0), type_(FlagJob::Type::kGeologist) {
	}  // For savegame loading
	CmdFlagAction(const Time& t, const int32_t p, const Flag& f, FlagJob::Type y)
	   : PlayerCommand(t, p), serial_(f.serial()), type_(y) {
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
	Serial serial_;
	FlagJob::Type type_;
};

struct CmdStartStopBuilding : public PlayerCommand {
	CmdStartStopBuilding() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdStartStopBuilding(const Time& t, const PlayerNumber p, Building& b)
	   : PlayerCommand(t, p), serial(b.serial()) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kStartStopBuilding;
	}

	explicit CmdStartStopBuilding(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
};

struct CmdMilitarySiteSetSoldierPreference : public PlayerCommand {
	CmdMilitarySiteSetSoldierPreference()
	   : PlayerCommand(), serial(0), preference(SoldierPreference::kRookies) {
	}  // For savegame loading
	CmdMilitarySiteSetSoldierPreference(const Time& t,
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
	CmdStartOrCancelExpedition() : PlayerCommand(), serial(kInvalidSerial) {
	}  // For savegame loading
	CmdStartOrCancelExpedition(const Time& t, PlayerNumber const p, Building& b)
	   : PlayerCommand(t, p), serial(b.serial()) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kStartOrCancelExpedition;
	}

	explicit CmdStartOrCancelExpedition(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
};

struct CmdExpeditionConfig : public PlayerCommand {
	CmdExpeditionConfig()
	   : PlayerCommand(), serial(kInvalidSerial), type(wwWARE), index(0), add(false) {
	}  // For savegame loading
	CmdExpeditionConfig(
	   const Time& t, PlayerNumber const p, PortDock& pd, WareWorker ww, DescriptionIndex di, bool a)
	   : PlayerCommand(t, p), serial(pd.serial()), type(ww), index(di), add(a) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kExpeditionConfig;
	}

	explicit CmdExpeditionConfig(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial;
	WareWorker type;
	DescriptionIndex index;
	bool add;
};

struct CmdEnhanceBuilding : public PlayerCommand {
	CmdEnhanceBuilding() : PlayerCommand(), serial_(0), bi_(0), keep_wares_(false) {
	}  // For savegame loading
	CmdEnhanceBuilding(
	   const Time& init_duetime, const int32_t p, Building& b, const DescriptionIndex i, bool kw)
	   : PlayerCommand(init_duetime, p), serial_(b.serial()), bi_(i), keep_wares_(kw) {
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
	Serial serial_;
	DescriptionIndex bi_;
	bool keep_wares_;
};

struct CmdDismantleBuilding : public PlayerCommand {
	CmdDismantleBuilding() : PlayerCommand(), serial_(0), keep_wares_(false) {
	}  // For savegame loading
	CmdDismantleBuilding(const Time& t, const int32_t p, PlayerImmovable& pi, bool kw)
	   : PlayerCommand(t, p), serial_(pi.serial()), keep_wares_(kw) {
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
	Serial serial_;
	bool keep_wares_;
};

struct CmdEvictWorker : public PlayerCommand {
	CmdEvictWorker() : PlayerCommand(), serial(0) {
	}  // For savegame loading
	CmdEvictWorker(const Time& t, const int32_t p, Worker& w)
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
	CmdShipScoutDirection(const Time& t, PlayerNumber const p, Serial s, WalkingDir direction)
	   : PlayerCommand(t, p), serial(s), dir(direction) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipScoutDirection;
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
	CmdShipConstructPort(const Time& t, PlayerNumber const p, Serial s, Coords c)
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
	CmdShipExploreIsland()
	   : PlayerCommand(), serial(0), island_explore_direction(IslandExploreDirection::kNotSet) {
	}  // For savegame loading
	CmdShipExploreIsland(const Time& t,
	                     PlayerNumber const p,
	                     Serial s,
	                     IslandExploreDirection direction)
	   : PlayerCommand(t, p), serial(s), island_explore_direction(direction) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipExploreIsland;
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
	CmdShipSink(const Time& t, PlayerNumber const p, Serial s) : PlayerCommand(t, p), serial(s) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipSink;
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
	CmdShipCancelExpedition(const Time& t, PlayerNumber const p, Serial s)
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
	CmdSetWarePriority()
	   : PlayerCommand(),
	     serial_(0),
	     type_(wwWARE),
	     priority_(WarePriority::kNormal),
	     is_constructionsite_setting_(false) {
	}
	CmdSetWarePriority(const Time& duetime,
	                   PlayerNumber sender,
	                   PlayerImmovable&,
	                   WareWorker type,
	                   DescriptionIndex index,
	                   const WarePriority& priority,
	                   bool cs);

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
	WareWorker type_;  ///< this is always wwWARE right now
	DescriptionIndex index_;
	WarePriority priority_;
	bool is_constructionsite_setting_;
};

struct CmdSetInputMaxFill : public PlayerCommand {
	CmdSetInputMaxFill()
	   : PlayerCommand(),
	     serial_(0),
	     type_(wwWARE),
	     max_fill_(0),
	     is_constructionsite_setting_(false) {
	}  // For savegame loading
	CmdSetInputMaxFill(const Time& duetime,
	                   PlayerNumber,
	                   PlayerImmovable&,
	                   DescriptionIndex,
	                   WareWorker,
	                   uint32_t maxfill,
	                   bool cs);

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
	bool is_constructionsite_setting_;
};

struct CmdChangeTargetQuantity : public PlayerCommand {
	CmdChangeTargetQuantity() : PlayerCommand(), economy_(0), ware_type_() {
	}  //  For savegame loading.
	CmdChangeTargetQuantity(const Time& duetime,
	                        PlayerNumber sender,
	                        uint32_t economy,
	                        DescriptionIndex index);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	explicit CmdChangeTargetQuantity(StreamRead&);

	void serialize(StreamWrite&) override;

protected:
	Serial economy() const {
		return economy_;
	}
	DescriptionIndex ware_type() const {
		return ware_type_;
	}

private:
	Serial economy_;
	DescriptionIndex ware_type_;
};

struct CmdSetWareTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWareTargetQuantity() : CmdChangeTargetQuantity(), permanent_(0) {
	}
	CmdSetWareTargetQuantity(const Time& duetime,
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

struct CmdSetWorkerTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWorkerTargetQuantity() : CmdChangeTargetQuantity(), permanent_(0) {
	}
	CmdSetWorkerTargetQuantity(const Time& duetime,
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

struct CmdChangeTrainingOptions : public PlayerCommand {
	CmdChangeTrainingOptions()
	   : PlayerCommand(), serial(0), attribute(TrainingAttribute::kHealth), value(0) {
	}  // For savegame loading
	CmdChangeTrainingOptions(const Time& t,
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
	CmdDropSoldier(const Time& t, const int32_t p, Building& b, const int32_t init_soldier)
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
	CmdChangeSoldierCapacity(const Time& t, const int32_t p, Building& b, const int32_t i)
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

struct CmdEnemyFlagAction : public PlayerCommand {
	CmdEnemyFlagAction() : PlayerCommand(), serial_(0), allow_conquer_(true) {
	}  // For savegame loading
	CmdEnemyFlagAction(const Time& t, int32_t p, const Flag& f, const std::vector<Serial>& s, bool c)
	   : PlayerCommand(t, p), serial_(f.serial()), soldiers_(s), allow_conquer_(c) {
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
	Serial serial_;
	std::vector<Serial> soldiers_;
	bool allow_conquer_;
};

/// Abstract base for commands about a message.
struct PlayerMessageCommand : public PlayerCommand {
	PlayerMessageCommand() : PlayerCommand() {
	}  //  for savegames
	PlayerMessageCommand(const Time& t, const PlayerNumber p, const MessageId& i)
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
	CmdMessageSetStatusRead(const Time& t, const PlayerNumber p, const MessageId& i)
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
	CmdMessageSetStatusArchived(const Time& t, const PlayerNumber p, const MessageId& i)
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
	CmdSetStockPolicy(const Time& time,
	                  PlayerNumber p,
	                  const Building& wh,
	                  bool isworker,
	                  DescriptionIndex ware,
	                  StockPolicy policy);

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
	StockPolicy policy_;
};

struct CmdProposeTrade : PlayerCommand {
	CmdProposeTrade(const Time& time, PlayerNumber pn, const Trade& trade);

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kProposeTrade;
	}

	void execute(Game& game) override;

	// Network (de-)serialization
	explicit CmdProposeTrade(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	// Savegame functions
	CmdProposeTrade();
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Trade trade_;
};

struct CmdToggleMuteMessages : PlayerCommand {
	CmdToggleMuteMessages(const Time& t, PlayerNumber p, const Building& b, bool a)
	   : PlayerCommand(t, p), building_(b.serial()), all_(a) {
	}

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kToggleMuteMessages;
	}

	void execute(Game& game) override;

	explicit CmdToggleMuteMessages(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdToggleMuteMessages() : PlayerCommand(), building_(kInvalidSerial), all_(false) {
	}
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial building_;
	bool all_;
};

struct CmdMarkMapObjectForRemoval : PlayerCommand {
	CmdMarkMapObjectForRemoval(const Time& t, PlayerNumber p, const Immovable& mo, bool m)
	   : PlayerCommand(t, p), object_(mo.serial()), mark_(m) {
	}

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kMarkMapObjectForRemoval;
	}

	void execute(Game& game) override;

	explicit CmdMarkMapObjectForRemoval(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdMarkMapObjectForRemoval() : PlayerCommand(), object_(kInvalidSerial), mark_(false) {
	}
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial object_;
	bool mark_;
};

struct CmdPickCustomStartingPosition : PlayerCommand {
	CmdPickCustomStartingPosition(const Time& t, PlayerNumber p, const Coords& c)
	   : PlayerCommand(t, p), coords_(c) {
	}

	QueueCommandTypes id() const override {
		return QueueCommandTypes::kPickCustomStartingPosition;
	}

	void execute(Game& game) override;

	explicit CmdPickCustomStartingPosition(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdPickCustomStartingPosition() : PlayerCommand() {
	}
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Coords coords_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_PLAYERCOMMAND_H
