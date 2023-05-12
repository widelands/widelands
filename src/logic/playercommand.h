/*
 * Copyright (C) 2004-2023 by the Widelands Development Team
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
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

	[[nodiscard]] PlayerNumber sender() const {
		return sender_;
	}
	[[nodiscard]] uint32_t cmdserial() const {
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
	CmdBulldoze() = default;  // For savegame loading
	CmdBulldoze(const Time& t, const int32_t p, PlayerImmovable& pi, const bool init_recurse = false)
	   : PlayerCommand(t, p), serial(pi.serial()), recurse(init_recurse) {
	}

	explicit CmdBulldoze(StreamRead&);

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kBulldoze;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
	bool recurse{false};
};

struct CmdBuild : public PlayerCommand {
	CmdBuild() = default;  // For savegame loading
	CmdBuild(const Time& init_duetime, const int32_t p, const Coords& c, const DescriptionIndex i)
	   : PlayerCommand(init_duetime, p), coords(c), bi(i) {
	}

	explicit CmdBuild(StreamRead&);

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuild;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Coords coords;
	DescriptionIndex bi{0U};
};

struct CmdBuildFlag : public PlayerCommand {
	CmdBuildFlag() = default;  // For savegame loading
	CmdBuildFlag(const Time& t, const int32_t p, const Coords& c) : PlayerCommand(t, p), coords(c) {
	}

	explicit CmdBuildFlag(StreamRead&);

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuildFlag;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Coords coords;
};

struct CmdBuildRoad : public PlayerCommand {
	CmdBuildRoad() = default;  // For savegame loading
	CmdBuildRoad(const Time&, int32_t, Path&);
	explicit CmdBuildRoad(StreamRead&);

	~CmdBuildRoad() override = default;

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuildRoad;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	std::unique_ptr<Path> path;
	Coords start;
	Path::StepVector::size_type nsteps{0U};
	std::unique_ptr<uint8_t[]> steps;
};

struct CmdBuildWaterway : public PlayerCommand {
	CmdBuildWaterway() = default;  // For savegame loading
	CmdBuildWaterway(const Time&, int32_t, Path&);
	explicit CmdBuildWaterway(StreamRead&);

	~CmdBuildWaterway() override = default;

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kBuildWaterway;
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	std::unique_ptr<Path> path;
	Coords start;
	Path::StepVector::size_type nsteps{0U};
	std::unique_ptr<uint8_t[]> steps;
};

struct CmdFlagAction : public PlayerCommand {
	CmdFlagAction() = default;  // For savegame loading
	CmdFlagAction(const Time& t, const int32_t p, const Flag& f, FlagJob::Type y)
	   : PlayerCommand(t, p), serial_(f.serial()), type_(y) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kFlagAction;
	}

	explicit CmdFlagAction(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_{0U};
	FlagJob::Type type_{FlagJob::Type::kGeologist};
};

struct CmdStartStopBuilding : public PlayerCommand {
	CmdStartStopBuilding() = default;  // For savegame loading
	CmdStartStopBuilding(const Time& t, const PlayerNumber p, Building& b)
	   : PlayerCommand(t, p), serial(b.serial()) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kStartStopBuilding;
	}

	explicit CmdStartStopBuilding(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
};

struct CmdToggleInfiniteProduction : public PlayerCommand {
	CmdToggleInfiniteProduction() = default;  // For savegame loading
	CmdToggleInfiniteProduction(const Time& t, const PlayerNumber p, Building& b)
	   : PlayerCommand(t, p), serial_(b.serial()) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kToggleInfiniteProduction;
	}

	explicit CmdToggleInfiniteProduction(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_{0U};
};

struct CmdMilitarySiteSetSoldierPreference : public PlayerCommand {
	CmdMilitarySiteSetSoldierPreference() = default;  // For savegame loading
	CmdMilitarySiteSetSoldierPreference(const Time& t,
	                                    const PlayerNumber p,
	                                    Building& b,
	                                    SoldierPreference prefs)
	   : PlayerCommand(t, p), serial(b.serial()), preference(prefs) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kMilitarysiteSetSoldierPreference;
	}

	explicit CmdMilitarySiteSetSoldierPreference(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
	Widelands::SoldierPreference preference{SoldierPreference::kRookies};
};

struct CmdStartOrCancelExpedition : public PlayerCommand {
	CmdStartOrCancelExpedition() = default;  // For savegame loading
	CmdStartOrCancelExpedition(const Time& t, PlayerNumber const p, Building& b)
	   : PlayerCommand(t, p), serial(b.serial()) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kStartOrCancelExpedition;
	}

	explicit CmdStartOrCancelExpedition(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{kInvalidSerial};
};

struct CmdExpeditionConfig : public PlayerCommand {
	CmdExpeditionConfig() = default;  // For savegame loading
	CmdExpeditionConfig(
	   const Time& t, PlayerNumber const p, PortDock& pd, WareWorker ww, DescriptionIndex di, bool a)
	   : PlayerCommand(t, p), serial(pd.serial()), type(ww), index(di), add(a) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kExpeditionConfig;
	}

	explicit CmdExpeditionConfig(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{kInvalidSerial};
	WareWorker type{wwWARE};
	DescriptionIndex index{0U};
	bool add{false};
};

struct CmdEnhanceBuilding : public PlayerCommand {
	CmdEnhanceBuilding() = default;  // For savegame loading
	CmdEnhanceBuilding(
	   const Time& init_duetime, const int32_t p, Building& b, const DescriptionIndex i, bool kw)
	   : PlayerCommand(init_duetime, p), serial_(b.serial()), bi_(i), keep_wares_(kw) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kEnhanceBuilding;
	}

	explicit CmdEnhanceBuilding(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_{0U};
	DescriptionIndex bi_{0U};
	bool keep_wares_{false};
};

struct CmdDismantleBuilding : public PlayerCommand {
	CmdDismantleBuilding() = default;  // For savegame loading
	CmdDismantleBuilding(const Time& t, const int32_t p, PlayerImmovable& pi, bool kw)
	   : PlayerCommand(t, p), serial_(pi.serial()), keep_wares_(kw) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kDismantleBuilding;
	}

	explicit CmdDismantleBuilding(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_{0U};
	bool keep_wares_{false};
};

struct CmdEvictWorker : public PlayerCommand {
	CmdEvictWorker() = default;  // For savegame loading
	CmdEvictWorker(const Time& t, const int32_t p, Worker& w)
	   : PlayerCommand(t, p), serial(w.serial()) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kEvictWorker;
	}

	explicit CmdEvictWorker(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
};

struct CmdShipScoutDirection : public PlayerCommand {
	CmdShipScoutDirection() = default;  // For savegame loading
	CmdShipScoutDirection(const Time& t, PlayerNumber const p, Serial s, WalkingDir direction)
	   : PlayerCommand(t, p), serial(s), dir(direction) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipScoutDirection;
	}

	explicit CmdShipScoutDirection(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
	WalkingDir dir{WalkingDir::IDLE};
};

struct CmdShipConstructPort : public PlayerCommand {
	CmdShipConstructPort() = default;  // For savegame loading
	CmdShipConstructPort(const Time& t, PlayerNumber const p, Serial s, Coords c)
	   : PlayerCommand(t, p), serial(s), coords(c) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipConstructPort;
	}

	explicit CmdShipConstructPort(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
	Coords coords;
};

struct CmdShipExploreIsland : public PlayerCommand {
	CmdShipExploreIsland() = default;  // For savegame loading
	CmdShipExploreIsland(const Time& t,
	                     PlayerNumber const p,
	                     Serial s,
	                     IslandExploreDirection direction)
	   : PlayerCommand(t, p), serial(s), island_explore_direction(direction) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipExploreIsland;
	}

	explicit CmdShipExploreIsland(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
	IslandExploreDirection island_explore_direction{IslandExploreDirection::kNotSet};
};

struct CmdShipSink : public PlayerCommand {
	CmdShipSink() = default;  // For savegame loading
	CmdShipSink(const Time& t, PlayerNumber const p, Serial s) : PlayerCommand(t, p), serial(s) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipSink;
	}

	explicit CmdShipSink(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
};

struct CmdShipCancelExpedition : public PlayerCommand {
	CmdShipCancelExpedition() = default;  // For savegame loading
	CmdShipCancelExpedition(const Time& t, PlayerNumber const p, Serial s)
	   : PlayerCommand(t, p), serial(s) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipCancelExpedition;
	}

	explicit CmdShipCancelExpedition(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
};

struct CmdSetWarePriority : public PlayerCommand {
	CmdSetWarePriority() = default;  // For savegame loading.
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

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetWarePriority;
	}

	explicit CmdSetWarePriority(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_{0U};
	WareWorker type_{wwWARE};  ///< this is always wwWARE right now
	DescriptionIndex index_{INVALID_INDEX};
	WarePriority priority_{WarePriority::kNormal};
	bool is_constructionsite_setting_{false};
};

struct CmdSetInputMaxFill : public PlayerCommand {
	CmdSetInputMaxFill() = default;  // For savegame loading
	CmdSetInputMaxFill(const Time& duetime,
	                   PlayerNumber,
	                   PlayerImmovable&,
	                   DescriptionIndex,
	                   WareWorker,
	                   uint32_t max_fill,
	                   bool cs);

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetInputMaxFill;
	}

	explicit CmdSetInputMaxFill(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_{0U};
	DescriptionIndex index_{INVALID_INDEX};
	WareWorker type_{wwWARE};
	uint32_t max_fill_{0U};
	bool is_constructionsite_setting_{false};
};

struct CmdChangeTargetQuantity : public PlayerCommand {
	CmdChangeTargetQuantity() = default;  //  For savegame loading.
	CmdChangeTargetQuantity(const Time& duetime,
	                        PlayerNumber sender,
	                        uint32_t economy,
	                        DescriptionIndex init_ware_type);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	explicit CmdChangeTargetQuantity(StreamRead&);

	void serialize(StreamWrite&) override;

protected:
	[[nodiscard]] Serial economy() const {
		return economy_;
	}
	[[nodiscard]] DescriptionIndex ware_type() const {
		return ware_type_;
	}

private:
	Serial economy_{0U};
	DescriptionIndex ware_type_{INVALID_INDEX};
};

struct CmdSetWareTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWareTargetQuantity() = default;
	CmdSetWareTargetQuantity(const Time& duetime,
	                         PlayerNumber sender,
	                         uint32_t economy,
	                         DescriptionIndex init_ware_type,
	                         uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetWareTargetQuantity;
	}

	explicit CmdSetWareTargetQuantity(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	uint32_t permanent_{0U};
};

struct CmdSetWorkerTargetQuantity : public CmdChangeTargetQuantity {
	CmdSetWorkerTargetQuantity() = default;
	CmdSetWorkerTargetQuantity(const Time& duetime,
	                           PlayerNumber sender,
	                           uint32_t economy,
	                           DescriptionIndex init_ware_type,
	                           uint32_t permanent);

	//  Write/Read these commands to/from a file (for savegames).
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kSetWorkerTargetQuantity;
	}

	explicit CmdSetWorkerTargetQuantity(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	uint32_t permanent_{0U};
};

struct CmdChangeTrainingOptions : public PlayerCommand {
	CmdChangeTrainingOptions() = default;  // For savegame loading
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

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kChangeTrainingOptions;
	}

	explicit CmdChangeTrainingOptions(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
	TrainingAttribute attribute{TrainingAttribute::kHealth};
	int32_t value{0};
};

struct CmdDropSoldier : public PlayerCommand {
	CmdDropSoldier() = default;  //  for savegames
	CmdDropSoldier(const Time& t, const int32_t p, Building& b, const int32_t init_soldier)
	   : PlayerCommand(t, p), serial(b.serial()), soldier(init_soldier) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kDropSoldier;
	}

	explicit CmdDropSoldier(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
	Serial soldier{0U};
};

struct CmdChangeSoldierCapacity : public PlayerCommand {
	CmdChangeSoldierCapacity() = default;  //  for savegames
	CmdChangeSoldierCapacity(const Time& t, const int32_t p, Building& b, const int32_t i)
	   : PlayerCommand(t, p), serial(b.serial()), val(i) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kChangeSoldierCapacity;
	}

	explicit CmdChangeSoldierCapacity(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial{0U};
	int32_t val{0};
};

struct CmdEnemyFlagAction : public PlayerCommand {
	CmdEnemyFlagAction() = default;  // For savegame loading
	CmdEnemyFlagAction(const Time& t, int32_t p, const Flag& f, const std::vector<Serial>& s, bool c)
	   : PlayerCommand(t, p), serial_(f.serial()), soldiers_(s), allow_conquer_(c) {
	}

	// Write these commands to a file (for savegames)
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kEnemyFlagAction;
	}

	explicit CmdEnemyFlagAction(StreamRead&);

	void execute(Game&) override;
	void serialize(StreamWrite&) override;

private:
	Serial serial_{0U};
	std::vector<Serial> soldiers_;
	bool allow_conquer_{true};
};

/// Abstract base for commands about a message.
struct PlayerMessageCommand : public PlayerCommand {
	PlayerMessageCommand() = default;  //  for savegames
	PlayerMessageCommand(const Time& t, const PlayerNumber p, const MessageId& i)
	   : PlayerCommand(t, p), message_id_(i) {
	}

	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

	explicit PlayerMessageCommand(StreamRead&);

	[[nodiscard]] MessageId message_id() const {
		return message_id_;
	}

private:
	MessageId message_id_;
};

struct CmdMessageSetStatusRead : public PlayerMessageCommand {
	CmdMessageSetStatusRead() = default;
	CmdMessageSetStatusRead(const Time& t, const PlayerNumber p, const MessageId& i)
	   : PlayerMessageCommand(t, p, i) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kMessageSetStatusRead;
	}

	explicit CmdMessageSetStatusRead(StreamRead& des) : PlayerMessageCommand(des) {
	}

	void execute(Game&) override;
	void serialize(StreamWrite&) override;
};

struct CmdMessageSetStatusArchived : public PlayerMessageCommand {
	CmdMessageSetStatusArchived() = default;
	CmdMessageSetStatusArchived(const Time& t, const PlayerNumber p, const MessageId& i)
	   : PlayerMessageCommand(t, p, i) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
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

	[[nodiscard]] QueueCommandTypes id() const override {
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

	[[nodiscard]] QueueCommandTypes id() const override {
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

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kToggleMuteMessages;
	}

	void execute(Game& game) override;

	explicit CmdToggleMuteMessages(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdToggleMuteMessages() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial building_{kInvalidSerial};
	bool all_{false};
};

struct CmdMarkMapObjectForRemoval : PlayerCommand {
	CmdMarkMapObjectForRemoval(const Time& t, PlayerNumber p, const Immovable& mo, bool m)
	   : PlayerCommand(t, p), object_(mo.serial()), mark_(m) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kMarkMapObjectForRemoval;
	}

	void execute(Game& game) override;

	explicit CmdMarkMapObjectForRemoval(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdMarkMapObjectForRemoval() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial object_{kInvalidSerial};
	bool mark_{false};
};

struct CmdDiplomacy : PlayerCommand {
	CmdDiplomacy(const Time& t, PlayerNumber p, DiplomacyAction a, PlayerNumber o)
	   : PlayerCommand(t, p), action_(a), other_player_(o) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kDiplomacy;
	}

	void execute(Game& game) override;

	explicit CmdDiplomacy(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdDiplomacy() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	DiplomacyAction action_{DiplomacyAction::kResign};
	PlayerNumber other_player_{0U};  // Ignored for kLeaveTeam and kResign
};

struct CmdPinnedNote : PlayerCommand {
	CmdPinnedNote(const Time& t,
	              PlayerNumber p,
	              const std::string& text,
	              Coords pos,
	              const RGBColor& rgb,
	              bool del)
	   : PlayerCommand(t, p), text_(text), pos_(pos), rgb_(rgb), delete_(del) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kPinnedNote;
	}

	void execute(Game& game) override;

	explicit CmdPinnedNote(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdPinnedNote() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	std::string text_;
	Coords pos_;
	RGBColor rgb_;
	bool delete_;
};

struct CmdShipPortName : PlayerCommand {
	CmdShipPortName(const Time& t, PlayerNumber p, Serial s, const std::string& n)
	   : PlayerCommand(t, p), serial_(s), name_(n) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kShipPortName;
	}

	void execute(Game& game) override;

	explicit CmdShipPortName(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdShipPortName() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial serial_{0U};
	std::string name_;
};

struct CmdFleetTargets : PlayerCommand {
	CmdFleetTargets(const Time& t, PlayerNumber p, Serial i, Quantity q)
	   : PlayerCommand(t, p), interface_(i), target_(q) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kFleetTargets;
	}

	void execute(Game& game) override;

	explicit CmdFleetTargets(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdFleetTargets() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Serial interface_{0U};
	Quantity target_{0U};
};

struct CmdPickCustomStartingPosition : PlayerCommand {
	CmdPickCustomStartingPosition(const Time& t, PlayerNumber p, const Coords& c)
	   : PlayerCommand(t, p), coords_(c) {
	}

	[[nodiscard]] QueueCommandTypes id() const override {
		return QueueCommandTypes::kPickCustomStartingPosition;
	}

	void execute(Game& game) override;

	explicit CmdPickCustomStartingPosition(StreamRead& des);
	void serialize(StreamWrite& ser) override;

	CmdPickCustomStartingPosition() = default;
	void write(FileWrite&, EditorGameBase&, MapObjectSaver&) override;
	void read(FileRead&, EditorGameBase&, MapObjectLoader&) override;

private:
	Coords coords_;
};

}  // namespace Widelands

#endif  // end of include guard: WL_LOGIC_PLAYERCOMMAND_H
