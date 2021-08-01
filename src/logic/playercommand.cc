/*
 * Copyright (C) 2004-2021 by the Widelands Development Team
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

#include "logic/playercommand.h"

#include "base/log.h"
#include "base/macros.h"
#include "base/wexception.h"
#include "economy/economy.h"
#include "economy/expedition_bootstrap.h"
#include "economy/input_queue.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "io/streamwrite.h"
#include "logic/game.h"
#include "logic/map_objects/tribes/market.h"
#include "logic/map_objects/tribes/soldier.h"
#include "logic/map_objects/tribes/tribe_descr.h"
#include "logic/player.h"
#include "logic/widelands_geometry_io.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"

namespace Widelands {

namespace {

// Returns the serial number of the object with the given "object_index" as
// registered by the mol. If object_index is zero (because the object that was
// saved to the file did no longer exist on saving) zero will be returned. That
// means that get_object will always return nullptr and all upcasts will fail -
// so the commands will never do anything when executed.
template <typename T>
Serial get_object_serial_or_zero(uint32_t object_index, MapObjectLoader& mol) {
	if (!object_index) {
		return 0;
	}
	return mol.get<T>(object_index).serial();
}

void serialize_bill_of_materials(const BillOfMaterials& bill, StreamWrite* ser) {
	ser->unsigned_32(bill.size());
	for (const WareAmount& amount : bill) {
		ser->unsigned_8(amount.first);
		ser->unsigned_32(amount.second);
	}
}

BillOfMaterials deserialize_bill_of_materials(StreamRead* des) {
	BillOfMaterials bill;
	const int count = des->unsigned_32();
	for (int i = 0; i < count; ++i) {
		const auto index = des->unsigned_8();
		const auto amount = des->unsigned_32();
		bill.push_back(std::make_pair(index, amount));
	}
	return bill;
}

}  // namespace

/*** class PlayerCommand ***/

PlayerCommand::PlayerCommand(const Time& time, const PlayerNumber s)
   : GameLogicCommand(time), sender_(s), cmdserial_(0) {
}

void PlayerCommand::write_id_and_sender(StreamWrite& ser) {
	ser.unsigned_8(static_cast<uint8_t>(id()));
	ser.unsigned_8(sender());
}

PlayerCommand* PlayerCommand::deserialize(StreamRead& des) {
	const uint8_t command_id = des.unsigned_8();
	switch (static_cast<QueueCommandTypes>(command_id)) {
	case QueueCommandTypes::kBulldoze:
		return new CmdBulldoze(des);
	case QueueCommandTypes::kBuild:
		return new CmdBuild(des);
	case QueueCommandTypes::kBuildFlag:
		return new CmdBuildFlag(des);
	case QueueCommandTypes::kBuildRoad:
		return new CmdBuildRoad(des);
	case QueueCommandTypes::kBuildWaterway:
		return new CmdBuildWaterway(des);
	case QueueCommandTypes::kFlagAction:
		return new CmdFlagAction(des);
	case QueueCommandTypes::kStartStopBuilding:
		return new CmdStartStopBuilding(des);
	case QueueCommandTypes::kEnhanceBuilding:
		return new CmdEnhanceBuilding(des);

	case QueueCommandTypes::kChangeTrainingOptions:
		return new CmdChangeTrainingOptions(des);
	case QueueCommandTypes::kDropSoldier:
		return new CmdDropSoldier(des);
	case QueueCommandTypes::kChangeSoldierCapacity:
		return new CmdChangeSoldierCapacity(des);
	case QueueCommandTypes::kEnemyFlagAction:
		return new CmdEnemyFlagAction(des);

	case QueueCommandTypes::kSetWarePriority:
		return new CmdSetWarePriority(des);
	case QueueCommandTypes::kSetWareTargetQuantity:
		return new CmdSetWareTargetQuantity(des);
	case QueueCommandTypes::kSetWorkerTargetQuantity:
		return new CmdSetWorkerTargetQuantity(des);

	case QueueCommandTypes::kMessageSetStatusRead:
		return new CmdMessageSetStatusRead(des);
	case QueueCommandTypes::kMessageSetStatusArchived:
		return new CmdMessageSetStatusArchived(des);

	case QueueCommandTypes::kSetStockPolicy:
		return new CmdSetStockPolicy(des);
	case QueueCommandTypes::kSetInputMaxFill:
		return new CmdSetInputMaxFill(des);
	case QueueCommandTypes::kDismantleBuilding:
		return new CmdDismantleBuilding(des);
	case QueueCommandTypes::kEvictWorker:
		return new CmdEvictWorker(des);
	case QueueCommandTypes::kMilitarysiteSetSoldierPreference:
		return new CmdMilitarySiteSetSoldierPreference(des);
	case QueueCommandTypes::kToggleMuteMessages:
		return new CmdToggleMuteMessages(des);

	case QueueCommandTypes::kStartOrCancelExpedition:
		return new CmdStartOrCancelExpedition(des);
	case QueueCommandTypes::kShipScoutDirection:
		return new CmdShipScoutDirection(des);
	case QueueCommandTypes::kShipExploreIsland:
		return new CmdShipExploreIsland(des);
	case QueueCommandTypes::kShipConstructPort:
		return new CmdShipConstructPort(des);
	case QueueCommandTypes::kShipSink:
		return new CmdShipSink(des);
	case QueueCommandTypes::kShipCancelExpedition:
		return new CmdShipCancelExpedition(des);
	case QueueCommandTypes::kExpeditionConfig:
		return new CmdExpeditionConfig(des);
	case QueueCommandTypes::kPickCustomStartingPosition:
		return new CmdPickCustomStartingPosition(des);
	case QueueCommandTypes::kMarkMapObjectForRemoval:
		return new CmdMarkMapObjectForRemoval(des);

	default:
		throw wexception("PlayerCommand::deserialize(): Encountered invalid command id: %d",
		                 static_cast<unsigned>(command_id));
	}
}

/**
 * Write this player command to a file. Call this from base classes
 */
constexpr uint16_t kCurrentPacketVersionPlayerCommand = 3;

void PlayerCommand::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionPlayerCommand);

	GameLogicCommand::write(fw, egbase, mos);
	// Now sender
	fw.unsigned_8(sender());
	fw.unsigned_32(cmdserial());
}

void PlayerCommand::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionPlayerCommand) {
			GameLogicCommand::read(fr, egbase, mol);
			sender_ = fr.unsigned_8();
			if (!egbase.get_player(sender_)) {
				throw GameDataError("player %u does not exist", sender_);
			}
			cmdserial_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError(
			   "PlayerCommand", packet_version, kCurrentPacketVersionPlayerCommand);
		}
	} catch (const WException& e) {
		throw GameDataError("player command: %s", e.what());
	}
}

/*** class Cmd_Bulldoze ***/

CmdBulldoze::CmdBulldoze(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()),
     serial(des.unsigned_32()),
     recurse(des.unsigned_8()) {
}

void CmdBulldoze::execute(Game& game) {
	if (upcast(PlayerImmovable, pimm, game.objects().get_object(serial))) {
		game.get_player(sender())->bulldoze(*pimm, recurse);
	}
}

void CmdBulldoze::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(recurse);
}

constexpr uint16_t kCurrentPacketVersionCmdBulldoze = 2;

void CmdBulldoze::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBulldoze) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PlayerImmovable>(fr.unsigned_32(), mol);
			recurse = 2 <= packet_version ? fr.unsigned_8() : false;
		} else {
			throw UnhandledVersionError(
			   "CmdBulldoze", packet_version, kCurrentPacketVersionCmdBulldoze);
		}
	} catch (const WException& e) {
		throw GameDataError("bulldoze: %s", e.what());
	}
}
void CmdBulldoze::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBulldoze);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
	fw.unsigned_8(recurse);
}

/*** class Cmd_Build ***/

CmdBuild::CmdBuild(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	bi = des.signed_16();
	coords = read_coords_32(&des);
}

void CmdBuild::execute(Game& game) {
	// Empty former_buildings vector since it's a new csite.
	FormerBuildings former_buildings;
	game.get_player(sender())->build(coords, bi, true, former_buildings);
}

void CmdBuild::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.signed_16(bi);
	write_coords_32(&ser, coords);
}

constexpr uint16_t kCurrentPacketVersionCmdBuild = 1;

void CmdBuild::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBuild) {
			PlayerCommand::read(fr, egbase, mol);
			bi = fr.unsigned_16();
			coords = read_coords_32(&fr, egbase.map().extent());
		} else {
			throw UnhandledVersionError("CmdBuild", packet_version, kCurrentPacketVersionCmdBuild);
		}

	} catch (const WException& e) {
		throw GameDataError("build: %s", e.what());
	}
}

void CmdBuild::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBuild);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_16(bi);
	write_coords_32(&fw, coords);
}

/*** class Cmd_BuildFlag ***/

CmdBuildFlag::CmdBuildFlag(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()), coords(read_coords_32(&des)) {
}

void CmdBuildFlag::execute(Game& game) {
	game.get_player(sender())->build_flag(coords);
}

void CmdBuildFlag::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	write_coords_32(&ser, coords);
}

constexpr uint16_t kCurrentPacketVersionCmdBuildFlag = 1;

void CmdBuildFlag::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBuildFlag) {
			PlayerCommand::read(fr, egbase, mol);
			coords = read_coords_32(&fr, egbase.map().extent());
		} else {
			throw UnhandledVersionError(
			   "CmdBuildFlag", packet_version, kCurrentPacketVersionCmdBuildFlag);
		}
	} catch (const WException& e) {
		throw GameDataError("build flag: %s", e.what());
	}
}
void CmdBuildFlag::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBuildFlag);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	write_coords_32(&fw, coords);
}

/*** class Cmd_BuildRoad ***/

CmdBuildRoad::CmdBuildRoad(const Time& t, int32_t p, Path& pa)
   : PlayerCommand(t, p),
     path(&pa),
     start(pa.get_start()),
     nsteps(pa.get_nsteps()),
     steps(nullptr) {
}

CmdBuildRoad::CmdBuildRoad(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()),
     // We cannot completely deserialize the path here because we don't have a Map
     path(nullptr),
     start(read_coords_32(&des)),
     nsteps(des.unsigned_16()),
     steps(new uint8_t[nsteps]) {
	for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
		steps[i] = des.unsigned_8();
	}
}

void CmdBuildRoad::execute(Game& game) {
	if (path == nullptr) {
		assert(steps);

		path.reset(new Path(start));
		for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
			path->append(game.map(), steps[i]);
		}
	}

	game.get_player(sender())->build_road(*path);
}

void CmdBuildRoad::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	write_coords_32(&ser, start);
	ser.unsigned_16(nsteps);

	assert(path || steps);

	for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
		ser.unsigned_8(path ? (*path)[i] : steps[i]);
	}
}

constexpr uint16_t kCurrentPacketVersionCmdBuildRoad = 1;

void CmdBuildRoad::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBuildRoad) {
			PlayerCommand::read(fr, egbase, mol);
			start = read_coords_32(&fr, egbase.map().extent());
			nsteps = fr.unsigned_16();
			path.reset(nullptr);
			steps.reset(new uint8_t[nsteps]);
			for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
				steps[i] = fr.unsigned_8();
			}
		} else {
			throw UnhandledVersionError(
			   "CmdBuildRoad", packet_version, kCurrentPacketVersionCmdBuildRoad);
		}
	} catch (const WException& e) {
		throw GameDataError("build road: %s", e.what());
	}
}
void CmdBuildRoad::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBuildRoad);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	write_coords_32(&fw, start);
	fw.unsigned_16(nsteps);
	for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
		fw.unsigned_8(path ? (*path)[i] : steps[i]);
	}
}

/*** class Cmd_BuildWaterway ***/

CmdBuildWaterway::CmdBuildWaterway(const Time& t, int32_t p, Path& pa)
   : PlayerCommand(t, p),
     path(&pa),
     start(pa.get_start()),
     nsteps(pa.get_nsteps()),
     steps(nullptr) {
}

CmdBuildWaterway::CmdBuildWaterway(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()),
     // We cannot completely deserialize the path here because we don't have a Map
     path(nullptr),
     start(read_coords_32(&des)),
     nsteps(des.unsigned_16()),
     steps(new uint8_t[nsteps]) {
	for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
		steps[i] = des.unsigned_8();
	}
}

void CmdBuildWaterway::execute(Game& game) {
	if (path == nullptr) {
		assert(steps);

		path.reset(new Path(start));
		for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
			path->append(game.map(), steps[i]);
		}
	}

	game.get_player(sender())->build_waterway(*path);
}

void CmdBuildWaterway::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	write_coords_32(&ser, start);
	ser.unsigned_16(nsteps);

	assert(path || steps);

	for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
		ser.unsigned_8(path ? (*path)[i] : steps[i]);
	}
}

constexpr uint16_t kCurrentPacketVersionCmdBuildWaterway = 1;

void CmdBuildWaterway::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBuildWaterway) {
			PlayerCommand::read(fr, egbase, mol);
			start = read_coords_32(&fr, egbase.map().extent());
			nsteps = fr.unsigned_16();
			path.reset(nullptr);
			steps.reset(new uint8_t[nsteps]);
			for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
				steps[i] = fr.unsigned_8();
			}
		} else {
			throw UnhandledVersionError(
			   "CmdBuildWaterway", packet_version, kCurrentPacketVersionCmdBuildWaterway);
		}
	} catch (const WException& e) {
		throw GameDataError("build waterway: %s", e.what());
	}
}
void CmdBuildWaterway::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBuildWaterway);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	write_coords_32(&fw, start);
	fw.unsigned_16(nsteps);
	for (Path::StepVector::size_type i = 0; i < nsteps; ++i) {
		fw.unsigned_8(path ? (*path)[i] : steps[i]);
	}
}

/*** Cmd_FlagAction ***/
CmdFlagAction::CmdFlagAction(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	type_ = static_cast<FlagJob::Type>(des.unsigned_8());
	serial_ = des.unsigned_32();
}

void CmdFlagAction::execute(Game& game) {
	Player* player = game.get_player(sender());
	if (upcast(Flag, flag, game.objects().get_object(serial_))) {
		if (flag->get_owner() == player) {
			player->flagaction(*flag, type_);
		}
	}
}

void CmdFlagAction::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_8(static_cast<uint8_t>(type_));
	ser.unsigned_32(serial_);
}

constexpr uint16_t kCurrentPacketVersionCmdFlagAction = 3;

void CmdFlagAction::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		// TODO(Nordfriese): Savegame compatibility
		if (packet_version >= 2 && packet_version <= kCurrentPacketVersionCmdFlagAction) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Flag>(fr.unsigned_32(), mol);
			// TODO(Nordfriese): Savegame compatibility
			type_ = packet_version < 3 ? FlagJob::Type::kGeologist :
                                      static_cast<FlagJob::Type>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError(
			   "CmdFlagAction", packet_version, kCurrentPacketVersionCmdFlagAction);
		}
	} catch (const WException& e) {
		throw GameDataError("flag action: %s", e.what());
	}
}
void CmdFlagAction::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdFlagAction);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_8(static_cast<uint8_t>(type_));
}

/*** Cmd_StartStopBuilding ***/

CmdStartStopBuilding::CmdStartStopBuilding(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
}

void CmdStartStopBuilding::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial);
	if (upcast(ConstructionSite, cs, mo)) {
		if (upcast(ProductionsiteSettings, s, cs->get_settings())) {
			s->stopped = !s->stopped;
		}
	} else if (upcast(Building, building, mo)) {
		game.get_player(sender())->start_stop_building(*building);
	}
}

void CmdStartStopBuilding::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionCmdStartStopBuilding = 1;

void CmdStartStopBuilding::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdStartStopBuilding) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError(
			   "CmdStartStopBuilding", packet_version, kCurrentPacketVersionCmdStartStopBuilding);
		}
	} catch (const WException& e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}
void CmdStartStopBuilding::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdStartStopBuilding);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

CmdMilitarySiteSetSoldierPreference::CmdMilitarySiteSetSoldierPreference(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	preference = static_cast<Widelands::SoldierPreference>(des.unsigned_8());
}

void CmdMilitarySiteSetSoldierPreference::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(static_cast<uint8_t>(preference));
}

void CmdMilitarySiteSetSoldierPreference::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial);
	if (upcast(ConstructionSite, cs, mo)) {
		if (upcast(MilitarysiteSettings, s, cs->get_settings())) {
			s->prefer_heroes = preference == SoldierPreference::kHeroes;
		}
	} else if (upcast(MilitarySite, building, mo)) {
		game.get_player(sender())->military_site_set_soldier_preference(*building, preference);
	}
}

constexpr uint16_t kCurrentPacketVersionSoldierPreference = 1;

void CmdMilitarySiteSetSoldierPreference::write(FileWrite& fw,
                                                EditorGameBase& egbase,
                                                MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionSoldierPreference);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_8(static_cast<uint8_t>(preference));

	// Now serial.
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

void CmdMilitarySiteSetSoldierPreference::read(FileRead& fr,
                                               EditorGameBase& egbase,
                                               MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionSoldierPreference) {
			PlayerCommand::read(fr, egbase, mol);
			preference = static_cast<Widelands::SoldierPreference>(fr.unsigned_8());
			serial = get_object_serial_or_zero<MilitarySite>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdMilitarySiteSetSoldierPreference", packet_version,
			                            kCurrentPacketVersionSoldierPreference);
		}
	} catch (const WException& e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}

/*** Cmd_StartOrCancelExpedition ***/

CmdStartOrCancelExpedition::CmdStartOrCancelExpedition(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
}

void CmdStartOrCancelExpedition::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial);
	if (upcast(ConstructionSite, cs, mo)) {
		if (upcast(WarehouseSettings, s, cs->get_settings())) {
			s->launch_expedition = !s->launch_expedition;
		}
	} else if (upcast(Warehouse, warehouse, game.objects().get_object(serial))) {
		game.get_player(sender())->start_or_cancel_expedition(*warehouse);
	}
}

void CmdStartOrCancelExpedition::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionExpedition = 1;

void CmdStartOrCancelExpedition::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionExpedition) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Warehouse>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError(
			   "CmdStartOrCancelExpedition", packet_version, kCurrentPacketVersionExpedition);
		}
	} catch (const WException& e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}
void CmdStartOrCancelExpedition::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionExpedition);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

/*** Cmd_ExpeditionConfig ***/

CmdExpeditionConfig::CmdExpeditionConfig(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	type = des.unsigned_8() == 0 ? wwWARE : wwWORKER;
	index = des.unsigned_32();
	add = des.unsigned_8();
}

void CmdExpeditionConfig::execute(Game& game) {
	if (upcast(PortDock, pd, game.objects().get_object(serial))) {
		if (ExpeditionBootstrap* x = pd->expedition_bootstrap()) {
			x->demand_additional_item(game, type, index, add);
		}
	}
}

void CmdExpeditionConfig::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(type == wwWARE ? 0 : 1);
	ser.unsigned_32(index);
	ser.unsigned_8(add ? 1 : 0);
}

constexpr uint16_t kCurrentPacketVersionCmdExpeditionConfig = 1;

void CmdExpeditionConfig::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdExpeditionConfig) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PortDock>(fr.unsigned_32(), mol);
			type = fr.unsigned_8() == 0 ? wwWARE : wwWORKER;
			index = fr.unsigned_32();
			add = fr.unsigned_8();
		} else {
			throw UnhandledVersionError(
			   "CmdExpeditionConfig", packet_version, kCurrentPacketVersionCmdExpeditionConfig);
		}
	} catch (const WException& e) {
		throw GameDataError("enhance building: %s", e.what());
	}
}
void CmdExpeditionConfig::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionCmdExpeditionConfig);
	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
	fw.unsigned_8(type == wwWARE ? 0 : 1);
	fw.unsigned_32(index);
	fw.unsigned_8(add ? 1 : 0);
}

/*** Cmd_EnhanceBuilding ***/

CmdEnhanceBuilding::CmdEnhanceBuilding(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	bi_ = des.unsigned_16();
	keep_wares_ = des.unsigned_8();
}

void CmdEnhanceBuilding::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial_);
	if (upcast(ConstructionSite, cs, mo)) {
		if (bi_ == cs->building().enhancement()) {
			cs->enhance(game);
		}
	} else if (upcast(Building, building, mo)) {
		game.get_player(sender())->enhance_building(building, bi_, keep_wares_);
	}
}

void CmdEnhanceBuilding::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_16(bi_);
	ser.unsigned_8(keep_wares_ ? 1 : 0);
}

constexpr uint16_t kCurrentPacketVersionCmdEnhanceBuilding = 2;

void CmdEnhanceBuilding::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdEnhanceBuilding) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			bi_ = fr.unsigned_16();
			keep_wares_ = fr.unsigned_8();
		} else {
			throw UnhandledVersionError(
			   "CmdEnhanceBuilding", packet_version, kCurrentPacketVersionCmdEnhanceBuilding);
		}
	} catch (const WException& e) {
		throw GameDataError("enhance building: %s", e.what());
	}
}
void CmdEnhanceBuilding::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionCmdEnhanceBuilding);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_16(bi_);
	fw.unsigned_8(keep_wares_ ? 1 : 0);
}

/*** Cmd_DismantleBuilding ***/
CmdDismantleBuilding::CmdDismantleBuilding(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	keep_wares_ = des.unsigned_8();
}

void CmdDismantleBuilding::execute(Game& game) {
	if (upcast(Building, building, game.objects().get_object(serial_))) {
		game.get_player(sender())->dismantle_building(building, keep_wares_);
	}
}

void CmdDismantleBuilding::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_8(keep_wares_ ? 1 : 0);
}

constexpr uint16_t kCurrentPacketVersionDismantleBuilding = 2;

void CmdDismantleBuilding::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionDismantleBuilding) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			keep_wares_ = fr.unsigned_8();
		} else {
			throw UnhandledVersionError(
			   "CmdDismantleBuilding", packet_version, kCurrentPacketVersionDismantleBuilding);
		}
	} catch (const WException& e) {
		throw GameDataError("dismantle building: %s", e.what());
	}
}
void CmdDismantleBuilding::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionDismantleBuilding);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_8(keep_wares_ ? 1 : 0);
}

/*** Cmd_EvictWorker ***/
CmdEvictWorker::CmdEvictWorker(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
}

void CmdEvictWorker::execute(Game& game) {
	upcast(Worker, worker, game.objects().get_object(serial));
	if (worker && worker->owner().player_number() == sender()) {
		worker->evict(game);
	}
}

void CmdEvictWorker::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionCmdEvictWorker = 1;

void CmdEvictWorker::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdEvictWorker) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Worker>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError(
			   "CmdEvictWorker", packet_version, kCurrentPacketVersionCmdEvictWorker);
		}
	} catch (const WException& e) {
		throw GameDataError("evict worker: %s", e.what());
	}
}
void CmdEvictWorker::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdEvictWorker);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

/*** Cmd_ShipScoutDirection ***/
CmdShipScoutDirection::CmdShipScoutDirection(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	dir = static_cast<WalkingDir>(des.unsigned_8());
}

void CmdShipScoutDirection::execute(Game& game) {
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		if (!(ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionWaiting ||
		      ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionPortspaceFound ||
		      ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionScouting)) {
			log_warn_time(
			   game.get_gametime(),
			   " %1d:ship on %3dx%3d received scout command but not in "
			   "kExpeditionWaiting or kExpeditionPortspaceFound or kExpeditionScouting status "
			   "(expedition: %s), ignoring...\n",
			   ship->get_owner()->player_number(), ship->get_position().x, ship->get_position().y,
			   (ship->state_is_expedition()) ? "Y" : "N");
			return;
		}
		ship->exp_scouting_direction(game, dir);
	}
}

void CmdShipScoutDirection::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(static_cast<uint8_t>(dir));
}

constexpr uint16_t kCurrentPacketVersionShipScoutDirection = 1;

void CmdShipScoutDirection::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipScoutDirection) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			// direction
			dir = static_cast<WalkingDir>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError(
			   "CmdShipScoutDirection", packet_version, kCurrentPacketVersionShipScoutDirection);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship scout: %s", e.what());
	}
}
void CmdShipScoutDirection::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionShipScoutDirection);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// direction
	fw.unsigned_8(static_cast<uint8_t>(dir));
}

/*** Cmd_ShipConstructPort ***/
CmdShipConstructPort::CmdShipConstructPort(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	coords = read_coords_32(&des);
}

void CmdShipConstructPort::execute(Game& game) {
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		if (ship->get_ship_state() != Widelands::Ship::ShipStates::kExpeditionPortspaceFound) {
			log_warn_time(game.get_gametime(),
			              " %1d:ship on %3dx%3d received build port command but "
			              "not in kExpeditionPortspaceFound status (expedition: %s), ignoring...\n",
			              ship->get_owner()->player_number(), ship->get_position().x,
			              ship->get_position().y, (ship->state_is_expedition()) ? "Y" : "N");
			return;
		}
		ship->exp_construct_port(game, coords);
	}
}

void CmdShipConstructPort::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	write_coords_32(&ser, coords);
}

constexpr uint16_t kCurrentPacketVersionShipConstructPort = 1;

void CmdShipConstructPort::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipConstructPort) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			// Coords
			coords = read_coords_32(&fr);
		} else {
			throw UnhandledVersionError(
			   "CmdShipConstructPort", packet_version, kCurrentPacketVersionShipConstructPort);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship construct port: %s", e.what());
	}
}
void CmdShipConstructPort::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionShipConstructPort);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Coords
	write_coords_32(&fw, coords);
}

/*** Cmd_ShipExploreIsland ***/
CmdShipExploreIsland::CmdShipExploreIsland(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	island_explore_direction = static_cast<IslandExploreDirection>(des.unsigned_8());
}

void CmdShipExploreIsland::execute(Game& game) {
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		if (!(ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionWaiting ||
		      ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionPortspaceFound ||
		      ship->get_ship_state() == Widelands::Ship::ShipStates::kExpeditionScouting)) {
			log_warn_time(
			   game.get_gametime(),
			   " %1d:ship on %3dx%3d received explore island command "
			   "but not in kExpeditionWaiting or kExpeditionPortspaceFound or kExpeditionScouting "
			   "status (expedition: %s), ignoring...\n",
			   ship->get_owner()->player_number(), ship->get_position().x, ship->get_position().y,
			   (ship->state_is_expedition()) ? "Y" : "N");
			return;
		}
		ship->exp_explore_island(game, island_explore_direction);
	}
}

void CmdShipExploreIsland::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(static_cast<uint8_t>(island_explore_direction));
}

constexpr uint16_t kCurrentPacketVersionShipExploreIsland = 1;

void CmdShipExploreIsland::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipExploreIsland) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			island_explore_direction = static_cast<IslandExploreDirection>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError(
			   "CmdShipExploreIsland", packet_version, kCurrentPacketVersionShipExploreIsland);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipExploreIsland::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionShipExploreIsland);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Direction of exploration
	fw.unsigned_8(static_cast<uint8_t>(island_explore_direction));
}

/*** Cmd_ShipSink ***/
CmdShipSink::CmdShipSink(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
}

void CmdShipSink::execute(Game& game) {
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->sink_ship(game);
	}
}

void CmdShipSink::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionCmdShipSink = 1;

void CmdShipSink::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdShipSink) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError(
			   "CmdShipSink", packet_version, kCurrentPacketVersionCmdShipSink);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipSink::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdShipSink);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

/*** Cmd_ShipCancelExpedition ***/
CmdShipCancelExpedition::CmdShipCancelExpedition(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
}

void CmdShipCancelExpedition::execute(Game& game) {
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->exp_cancel(game);
	}
}

void CmdShipCancelExpedition::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionShipCancelExpedition = 1;

void CmdShipCancelExpedition::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipCancelExpedition) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError(
			   "CmdShipCancelExpedition", packet_version, kCurrentPacketVersionShipCancelExpedition);
		}
	} catch (const WException& e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipCancelExpedition::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionShipCancelExpedition);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

/*** class Cmd_SetWarePriority ***/
CmdSetWarePriority::CmdSetWarePriority(const Time& init_duetime,
                                       const PlayerNumber init_sender,
                                       PlayerImmovable& imm,
                                       const WareWorker init_type,
                                       const DescriptionIndex i,
                                       const WarePriority& init_priority,
                                       bool cs_setting)
   : PlayerCommand(init_duetime, init_sender),
     serial_(imm.serial()),
     type_(init_type),
     index_(i),
     priority_(init_priority),
     is_constructionsite_setting_(cs_setting) {
}

void CmdSetWarePriority::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial_);
	if (is_constructionsite_setting_) {
		if (upcast(ConstructionSite, cs, mo)) {
			if (upcast(ProductionsiteSettings, s, cs->get_settings())) {
				for (auto& pair : s->ware_queues) {
					if (pair.first == index_) {
						pair.second.priority = priority_;
						return;
					}
				}
				NEVER_HERE();
			}
		}
	} else if (upcast(Building, psite, mo)) {
		if (psite->owner().player_number() == sender()) {
			psite->set_priority(WareWorker(type_), index_, priority_);
		}
	}
}

constexpr uint16_t kCurrentPacketVersionCmdSetWarePriority = 2;

void CmdSetWarePriority::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionCmdSetWarePriority);

	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.unsigned_8(static_cast<uint8_t>(type_));
	fw.signed_32(index_);
	priority_.write(fw);
	fw.unsigned_8(is_constructionsite_setting_ ? 1 : 0);
}

void CmdSetWarePriority::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdSetWarePriority) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			type_ = WareWorker(fr.unsigned_8());
			index_ = fr.signed_32();
			priority_ = WarePriority(fr);
			is_constructionsite_setting_ = fr.unsigned_8();
		} else {
			throw UnhandledVersionError(
			   "CmdSetWarePriority", packet_version, kCurrentPacketVersionCmdSetWarePriority);
		}

	} catch (const WException& e) {
		throw GameDataError("set ware priority: %s", e.what());
	}
}

CmdSetWarePriority::CmdSetWarePriority(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()),
     serial_(des.unsigned_32()),
     type_(WareWorker(des.unsigned_8())),
     index_(des.signed_32()),
     priority_(des),
     is_constructionsite_setting_(des.unsigned_8()) {
}

void CmdSetWarePriority::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_8(static_cast<uint8_t>(type_));
	ser.signed_32(index_);
	priority_.write(ser);
	ser.unsigned_8(is_constructionsite_setting_ ? 1 : 0);
}

/*** class Cmd_SetWareMaxFill ***/
CmdSetInputMaxFill::CmdSetInputMaxFill(const Time& init_duetime,
                                       const PlayerNumber init_sender,
                                       PlayerImmovable& imm,
                                       const DescriptionIndex index,
                                       const WareWorker type,
                                       const uint32_t max_fill,
                                       bool cs_setting)
   : PlayerCommand(init_duetime, init_sender),
     serial_(imm.serial()),
     index_(index),
     type_(type),
     max_fill_(max_fill),
     is_constructionsite_setting_(cs_setting) {
}

void CmdSetInputMaxFill::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial_);
	if (is_constructionsite_setting_) {
		if (upcast(ConstructionSite, cs, mo)) {
			if (upcast(ProductionsiteSettings, s, cs->get_settings())) {
				switch (type_) {
				case wwWARE:
					for (auto& pair : s->ware_queues) {
						if (pair.first == index_) {
							assert(pair.second.max_fill >= max_fill_);
							pair.second.desired_fill = max_fill_;
							return;
						}
					}
					NEVER_HERE();
				case wwWORKER:
					for (auto& pair : s->worker_queues) {
						if (pair.first == index_) {
							assert(pair.second.max_fill >= max_fill_);
							pair.second.desired_fill = max_fill_;
							return;
						}
					}
					NEVER_HERE();
				}
				NEVER_HERE();
			}
		}
	} else if (upcast(Building, b, mo)) {
		if (b->owner().player_number() == sender()) {
			try {
				b->inputqueue(index_, type_, nullptr).set_max_fill(max_fill_);
				if (upcast(Warehouse, wh, b)) {
					if (PortDock* p = wh->get_portdock()) {
						// Update in case the expedition was ready previously and now lacks a ware again
						p->expedition_bootstrap()->check_is_ready(game);
					}
				}
			} catch (const std::exception& e) {
				// TODO(matthiakl): This exception is only caught to ensure b21 savegame compatibility
				// and should be removed after v1.0
				log_err("Skipped CmdSetInputMaxFill command: %s", e.what());
			}
		}
	}
}

constexpr uint16_t kCurrentPacketVersionCmdSetInputMaxFill = 3;

void CmdSetInputMaxFill::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionCmdSetInputMaxFill);

	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));
	fw.signed_32(index_);
	fw.unsigned_8(type_ == wwWARE ? 0 : 1);
	fw.unsigned_32(max_fill_);
	fw.unsigned_8(is_constructionsite_setting_ ? 1 : 0);
}

void CmdSetInputMaxFill::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdSetInputMaxFill) {
			PlayerCommand::read(fr, egbase, mol);
			serial_ = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			index_ = fr.signed_32();
			if (fr.unsigned_8() == 0) {
				type_ = wwWARE;
			} else {
				type_ = wwWORKER;
			}
			max_fill_ = fr.unsigned_32();
			is_constructionsite_setting_ = fr.unsigned_8();
		} else {
			throw UnhandledVersionError(
			   "CmdSetInputMaxFill", packet_version, kCurrentPacketVersionCmdSetInputMaxFill);
		}
	} catch (const WException& e) {
		throw GameDataError("set ware max fill: %s", e.what());
	}
}

CmdSetInputMaxFill::CmdSetInputMaxFill(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	index_ = des.signed_32();
	if (des.unsigned_8() == 0) {
		type_ = wwWARE;
	} else {
		type_ = wwWORKER;
	}
	max_fill_ = des.unsigned_32();
	is_constructionsite_setting_ = des.unsigned_8();
}

void CmdSetInputMaxFill::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.signed_32(index_);
	ser.unsigned_8(type_ == wwWARE ? 0 : 1);
	ser.unsigned_32(max_fill_);
	ser.unsigned_8(is_constructionsite_setting_ ? 1 : 0);
}

CmdChangeTargetQuantity::CmdChangeTargetQuantity(const Time& init_duetime,
                                                 const PlayerNumber init_sender,
                                                 const uint32_t init_economy,
                                                 const DescriptionIndex init_ware_type)
   : PlayerCommand(init_duetime, init_sender), economy_(init_economy), ware_type_(init_ware_type) {
}

void CmdChangeTargetQuantity::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(economy());
	fw.c_string(egbase.player(sender()).tribe().get_ware_descr(ware_type())->name());
}

void CmdChangeTargetQuantity::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		PlayerCommand::read(fr, egbase, mol);
		economy_ = fr.unsigned_32();
		ware_type_ = egbase.player(sender()).tribe().ware_index(fr.c_string());
	} catch (const WException& e) {
		throw GameDataError("change target quantity: %s", e.what());
	}
}

CmdChangeTargetQuantity::CmdChangeTargetQuantity(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()),
     economy_(des.unsigned_32()),
     ware_type_(des.unsigned_8()) {
}

void CmdChangeTargetQuantity::serialize(StreamWrite& ser) {
	// Subclasses take care of writing their id() and sender()
	ser.unsigned_32(economy());
	ser.unsigned_8(ware_type());
}

CmdSetWareTargetQuantity::CmdSetWareTargetQuantity(const Time& init_duetime,
                                                   const PlayerNumber init_sender,
                                                   const uint32_t init_economy,
                                                   const DescriptionIndex init_ware_type,
                                                   const uint32_t init_permanent)
   : CmdChangeTargetQuantity(init_duetime, init_sender, init_economy, init_ware_type),
     permanent_(init_permanent) {
}

void CmdSetWareTargetQuantity::execute(Game& game) {
	Player* player = game.get_player(sender());
	if (player->has_economy(economy()) && game.descriptions().ware_exists(ware_type())) {
		player->get_economy(economy())->set_target_quantity(
		   wwWARE, ware_type(), permanent_, duetime());
	}
}

constexpr uint16_t kCurrentPacketVersionSetWareTargetQuantity = 2;

void CmdSetWareTargetQuantity::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionSetWareTargetQuantity);
	CmdChangeTargetQuantity::write(fw, egbase, mos);
	fw.unsigned_32(permanent_);
}

void CmdSetWareTargetQuantity::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionSetWareTargetQuantity) {
			CmdChangeTargetQuantity::read(fr, egbase, mol);
			permanent_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError(
			   "CmdSetWareTargetQuantity", packet_version, kCurrentPacketVersionSetWareTargetQuantity);
		}
	} catch (const WException& e) {
		throw GameDataError("set ware target quantity: %s", e.what());
	}
}

CmdSetWareTargetQuantity::CmdSetWareTargetQuantity(StreamRead& des)
   : CmdChangeTargetQuantity(des), permanent_(des.unsigned_32()) {
	if (cmdserial() == 1) {
		des.unsigned_32();
	}
}

void CmdSetWareTargetQuantity::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	CmdChangeTargetQuantity::serialize(ser);
	ser.unsigned_32(permanent_);
}

CmdSetWorkerTargetQuantity::CmdSetWorkerTargetQuantity(const Time& init_duetime,
                                                       const PlayerNumber init_sender,
                                                       const uint32_t init_economy,
                                                       const DescriptionIndex init_ware_type,
                                                       const uint32_t init_permanent)
   : CmdChangeTargetQuantity(init_duetime, init_sender, init_economy, init_ware_type),
     permanent_(init_permanent) {
}

void CmdSetWorkerTargetQuantity::execute(Game& game) {
	Player* player = game.get_player(sender());
	if (player->has_economy(economy()) && game.descriptions().worker_exists(ware_type())) {
		player->get_economy(economy())->set_target_quantity(
		   wwWORKER, ware_type(), permanent_, duetime());
	}
}

constexpr uint16_t kCurrentPacketVersionSetWorkerTargetQuantity = 2;

void CmdSetWorkerTargetQuantity::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionSetWorkerTargetQuantity);
	CmdChangeTargetQuantity::write(fw, egbase, mos);
	fw.unsigned_32(permanent_);
}

void CmdSetWorkerTargetQuantity::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionSetWorkerTargetQuantity) {
			CmdChangeTargetQuantity::read(fr, egbase, mol);
			permanent_ = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("CmdSetWorkerTargetQuantity", packet_version,
			                            kCurrentPacketVersionSetWorkerTargetQuantity);
		}
	} catch (const WException& e) {
		throw GameDataError("set worker target quantity: %s", e.what());
	}
}

CmdSetWorkerTargetQuantity::CmdSetWorkerTargetQuantity(StreamRead& des)
   : CmdChangeTargetQuantity(des), permanent_(des.unsigned_32()) {
	if (cmdserial() == 1) {
		des.unsigned_32();
	}
}

void CmdSetWorkerTargetQuantity::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	CmdChangeTargetQuantity::serialize(ser);
	ser.unsigned_32(permanent_);
}

/*** class Cmd_ChangeTrainingOptions ***/
CmdChangeTrainingOptions::CmdChangeTrainingOptions(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();                                    //  Serial of the building
	attribute = static_cast<TrainingAttribute>(des.unsigned_8());  //  Attribute to modify
	value = des.unsigned_16();                                     //  New vale
}

void CmdChangeTrainingOptions::execute(Game& game) {
	if (upcast(TrainingSite, trainingsite, game.objects().get_object(serial))) {
		game.get_player(sender())->change_training_options(*trainingsite, attribute, value);
	}
}

void CmdChangeTrainingOptions::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_8(static_cast<uint8_t>(attribute));
	ser.unsigned_16(value);
}

constexpr uint16_t kCurrentPacketVersionChangeTrainingOptions = 2;

void CmdChangeTrainingOptions::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionChangeTrainingOptions) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<TrainingSite>(fr.unsigned_32(), mol);
			attribute = static_cast<TrainingAttribute>(fr.unsigned_8());
			value = fr.unsigned_16();
		} else {
			throw UnhandledVersionError(
			   "CmdChangeTrainingOptions", packet_version, kCurrentPacketVersionChangeTrainingOptions);
		}
	} catch (const WException& e) {
		throw GameDataError("change training options: %s", e.what());
	}
}

void CmdChangeTrainingOptions::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionChangeTrainingOptions);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	fw.unsigned_8(static_cast<uint8_t>(attribute));
	fw.unsigned_16(value);
}

/*** class Cmd_DropSoldier ***/

CmdDropSoldier::CmdDropSoldier(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();   //  Serial of the building
	soldier = des.unsigned_32();  //  Serial of soldier
}

void CmdDropSoldier::execute(Game& game) {
	if (upcast(PlayerImmovable, player_imm, game.objects().get_object(serial))) {
		if (upcast(Soldier, s, game.objects().get_object(soldier))) {
			game.get_player(sender())->drop_soldier(*player_imm, *s);
		}
	}
}

void CmdDropSoldier::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.unsigned_32(soldier);
}

constexpr uint16_t kCurrentPacketVersionCmdDropSoldier = 1;

void CmdDropSoldier::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdDropSoldier) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PlayerImmovable>(fr.unsigned_32(), mol);
			soldier = get_object_serial_or_zero<Soldier>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError(
			   "CmdDropSoldier", packet_version, kCurrentPacketVersionCmdDropSoldier);
		}
	} catch (const WException& e) {
		throw GameDataError("drop soldier: %s", e.what());
	}
}

void CmdDropSoldier::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdDropSoldier);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	//  site serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	//  soldier serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(soldier)));
}

/*** Cmd_ChangeSoldierCapacity ***/

CmdChangeSoldierCapacity::CmdChangeSoldierCapacity(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	serial = des.unsigned_32();
	val = des.signed_16();
}

void CmdChangeSoldierCapacity::execute(Game& game) {
	MapObject* mo = game.objects().get_object(serial);
	if (upcast(ConstructionSite, cs, mo)) {
		// Clamp the capacity between the minimum and maximum value because the player may
		// have sent multiple decrease/increase commands at the same time (bug #5006).
		if (upcast(MilitarysiteSettings, ms, cs->get_settings())) {
			ms->desired_capacity = std::max(1, std::min<int32_t>(ms->max_capacity, val));
		} else if (upcast(TrainingsiteSettings, ts, cs->get_settings())) {
			ts->desired_capacity = std::max(0, std::min<int32_t>(ts->max_capacity, val));
		}
	} else if (upcast(Building, building, mo)) {
		if (building->get_owner() == game.get_player(sender()) &&
		    building->soldier_control() != nullptr) {
			SoldierControl* soldier_control = building->mutable_soldier_control();
			Widelands::Quantity const old_capacity = soldier_control->soldier_capacity();
			Widelands::Quantity const new_capacity =
			   std::min(static_cast<Widelands::Quantity>(
			               std::max(static_cast<int32_t>(old_capacity) + val,
			                        static_cast<int32_t>(soldier_control->min_soldier_capacity()))),
			            soldier_control->max_soldier_capacity());
			if (old_capacity != new_capacity) {
				soldier_control->set_soldier_capacity(new_capacity);
			}
		}
	}
}

void CmdChangeSoldierCapacity::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial);
	ser.signed_16(val);
}

constexpr uint16_t kCurrentPacketVersionChangeSoldierCapacity = 1;

void CmdChangeSoldierCapacity::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionChangeSoldierCapacity) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			val = fr.signed_16();
		} else {
			throw UnhandledVersionError(
			   "CmdChangeSoldierCapacity", packet_version, kCurrentPacketVersionChangeSoldierCapacity);
		}
	} catch (const WException& e) {
		throw GameDataError("change soldier capacity: %s", e.what());
	}
}

void CmdChangeSoldierCapacity::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionChangeSoldierCapacity);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Now capacity
	fw.signed_16(val);
}

/*** Cmd_EnemyFlagAction ***/

CmdEnemyFlagAction::CmdEnemyFlagAction(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	serial_ = des.unsigned_32();
	const uint32_t number = des.unsigned_32();
	soldiers_.clear();
	for (uint32_t i = 0; i < number; ++i) {
		soldiers_.push_back(des.unsigned_32());
	}
	allow_conquer_ = des.unsigned_8();
}

void CmdEnemyFlagAction::execute(Game& game) {
	Player* player = game.get_player(sender());

	if (upcast(Flag, flag, game.objects().get_object(serial_))) {
		verb_log_info_time(game.get_gametime(),
		                   "Cmd_EnemyFlagAction::execute player(%u): flag->owner(%d) "
		                   "number=%" PRIuS "\n",
		                   player->player_number(), flag->owner().player_number(), soldiers_.size());

		if (const Building* const building = flag->get_building()) {
			if (player->is_hostile(flag->owner())) {
				for (Widelands::Coords& coords : building->get_positions(game)) {
					if (player->is_seeing(Map::get_index(coords, game.map().get_width()))) {
						std::vector<Soldier*> result;
						for (Serial s : soldiers_) {
							if (Soldier* soldier = dynamic_cast<Soldier*>(game.objects().get_object(s))) {
								result.push_back(soldier);
							}
						}
						player->enemyflagaction(*flag, sender(), result, allow_conquer_);
						return;
					}
				}
			}
			log_warn_time(
			   game.get_gametime(),
			   "Cmd_EnemyFlagAction::execute: wrong player target not seen or not hostile.\n");
		}
	}
}

void CmdEnemyFlagAction::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(serial_);
	ser.unsigned_32(soldiers_.size());
	for (Serial s : soldiers_) {
		ser.unsigned_32(s);
	}
	ser.unsigned_8(allow_conquer_ ? 1 : 0);
}

constexpr uint16_t kCurrentPacketVersionCmdEnemyFlagAction = 5;

void CmdEnemyFlagAction::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		// TODO(Nordfriese): Savegame compatibility
		if (packet_version <= kCurrentPacketVersionCmdEnemyFlagAction && packet_version >= 3) {
			PlayerCommand::read(fr, egbase, mol);
			if (packet_version < 5) {
				fr.unsigned_8();
			}
			serial_ = get_object_serial_or_zero<Flag>(fr.unsigned_32(), mol);
			if (packet_version < 5) {
				fr.unsigned_8();
			}

			soldiers_.clear();
			if (packet_version == kCurrentPacketVersionCmdEnemyFlagAction) {
				const uint32_t number = fr.unsigned_32();
				for (uint32_t i = 0; i < number; ++i) {
					soldiers_.push_back(mol.get<Soldier>(fr.unsigned_32()).serial());
				}
			} else {
				const uint8_t number = fr.unsigned_8();
				upcast(Flag, flag, egbase.objects().get_object(serial_));
				assert(flag);
				std::vector<Soldier*> result;
				egbase.get_player(sender())->find_attack_soldiers(*flag, &result, number);
				assert(result.size() == number);
				for (const auto& s : result) {
					soldiers_.push_back(s->serial());
				}
			}
			allow_conquer_ = packet_version < 5 || fr.unsigned_8();
		} else {
			throw UnhandledVersionError(
			   "CmdEnemyFlagAction", packet_version, kCurrentPacketVersionCmdEnemyFlagAction);
		}
	} catch (const WException& e) {
		throw GameDataError("enemy flag action: %s", e.what());
	}
}

void CmdEnemyFlagAction::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdEnemyFlagAction);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial_)));

	// Now param
	fw.unsigned_32(soldiers_.size());
	for (Serial s : soldiers_) {
		fw.unsigned_32(mos.get_object_file_index(*egbase.objects().get_object(s)));
	}

	fw.unsigned_8(allow_conquer_ ? 1 : 0);
}

/*** struct PlayerMessageCommand ***/

PlayerMessageCommand::PlayerMessageCommand(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()), message_id_(des.unsigned_32()) {
}

constexpr uint16_t kCurrentPacketVersionPlayerMessageCommand = 1;

void PlayerMessageCommand::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionPlayerMessageCommand) {
			PlayerCommand::read(fr, egbase, mol);
			message_id_ = MessageId(fr.unsigned_32());
			if (!message_id_) {
				throw GameDataError("(player %u): message id is null", sender());
			}
		} else {
			throw UnhandledVersionError(
			   "PlayerMessageCommand", packet_version, kCurrentPacketVersionPlayerMessageCommand);
		}
	} catch (const WException& e) {
		throw GameDataError("player message: %s", e.what());
	}
}

void PlayerMessageCommand::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_16(kCurrentPacketVersionPlayerMessageCommand);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.message_savers[sender() - 1][message_id()].value());
}

/*** struct Cmd_MessageSetStatusRead ***/

void CmdMessageSetStatusRead::execute(Game& game) {
	game.get_player(sender())->get_messages()->set_message_status(
	   message_id(), Message::Status::kRead);
}

void CmdMessageSetStatusRead::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(message_id().value());
}

/*** struct Cmd_MessageSetStatusArchived ***/

void CmdMessageSetStatusArchived::execute(Game& game) {
	game.get_player(sender())->get_messages()->set_message_status(
	   message_id(), Message::Status::kArchived);
}

void CmdMessageSetStatusArchived::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(message_id().value());
}

/*** struct Cmd_SetStockPolicy ***/
CmdSetStockPolicy::CmdSetStockPolicy(const Time& time,
                                     PlayerNumber p,
                                     const Building& wh,
                                     bool isworker,
                                     DescriptionIndex ware,
                                     StockPolicy policy)
   : PlayerCommand(time, p) {
	warehouse_ = wh.serial();
	isworker_ = isworker;
	ware_ = ware;
	policy_ = policy;
}

CmdSetStockPolicy::CmdSetStockPolicy()
   : PlayerCommand(), warehouse_(0), isworker_(false), ware_(0), policy_() {
}

void CmdSetStockPolicy::execute(Game& game) {
	// Sanitize data that could have come from the network
	if (Player* plr = game.get_player(sender())) {
		MapObject* mo = game.objects().get_object(warehouse_);
		if (upcast(ConstructionSite, cs, mo)) {
			if (upcast(WarehouseSettings, s, cs->get_settings())) {
				if (isworker_) {
					s->worker_preferences[ware_] = policy_;
				} else {
					s->ware_preferences[ware_] = policy_;
				}
			}
		} else if (upcast(Warehouse, warehouse, mo)) {
			if (warehouse->get_owner() != plr) {
				log_warn_time(game.get_gametime(),
				              "Cmd_SetStockPolicy: sender %u, but warehouse owner %u\n", sender(),
				              warehouse->owner().player_number());
				return;
			}

			if (isworker_) {
				if (!(game.descriptions().worker_exists(ware_))) {
					log_warn_time(game.get_gametime(),
					              "Cmd_SetStockPolicy: sender %u, worker %u does not exist\n", sender(),
					              ware_);
					return;
				}
				warehouse->set_worker_policy(ware_, policy_);
			} else {
				if (!(game.descriptions().ware_exists(ware_))) {
					log_warn_time(game.get_gametime(),
					              "Cmd_SetStockPolicy: sender %u, ware %u does not exist\n", sender(),
					              ware_);
					return;
				}
				warehouse->set_ware_policy(ware_, policy_);
			}
		}
	}
}

CmdSetStockPolicy::CmdSetStockPolicy(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	warehouse_ = des.unsigned_32();
	isworker_ = des.unsigned_8();
	ware_ = DescriptionIndex(des.unsigned_8());
	policy_ = static_cast<StockPolicy>(des.unsigned_8());
}

void CmdSetStockPolicy::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(warehouse_);
	ser.unsigned_8(isworker_);
	ser.unsigned_8(ware_);
	ser.unsigned_8(static_cast<uint8_t>(policy_));
}

constexpr uint8_t kCurrentPacketVersionCmdSetStockPolicy = 1;

void CmdSetStockPolicy::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdSetStockPolicy) {
			PlayerCommand::read(fr, egbase, mol);
			warehouse_ = fr.unsigned_32();
			isworker_ = fr.unsigned_8();
			ware_ = DescriptionIndex(fr.unsigned_8());
			policy_ = static_cast<StockPolicy>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError(
			   "CmdSetStockPolicy", packet_version, kCurrentPacketVersionCmdSetStockPolicy);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_SetStockPolicy: %s", e.what());
	}
}

void CmdSetStockPolicy::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdSetStockPolicy);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(warehouse_);
	fw.unsigned_8(isworker_);
	fw.unsigned_8(ware_);
	fw.unsigned_8(static_cast<uint8_t>(policy_));
}

CmdProposeTrade::CmdProposeTrade(const Time& time, PlayerNumber pn, const Trade& trade)
   : PlayerCommand(time, pn), trade_(trade) {
}

CmdProposeTrade::CmdProposeTrade() : PlayerCommand() {
}

void CmdProposeTrade::execute(Game& game) {
	Player* plr = game.get_player(sender());
	if (plr == nullptr) {
		return;
	}

	Market* initiator = dynamic_cast<Market*>(game.objects().get_object(trade_.initiator));
	if (initiator == nullptr) {
		log_warn_time(
		   game.get_gametime(), "CmdProposeTrade: initiator vanished or is not a market.\n");
		return;
	}
	if (&initiator->owner() != plr) {
		log_warn_time(game.get_gametime(), "CmdProposeTrade: sender %u, but market owner %u\n",
		              sender(), initiator->owner().player_number());
		return;
	}
	Market* receiver = dynamic_cast<Market*>(game.objects().get_object(trade_.receiver));
	if (receiver == nullptr) {
		log_warn_time(
		   game.get_gametime(), "CmdProposeTrade: receiver vanished or is not a market.\n");
		return;
	}
	if (initiator->get_owner() == receiver->get_owner()) {
		log_warn_time(
		   game.get_gametime(), "CmdProposeTrade: Sending and receiving player are the same.\n");
		return;
	}

	// TODO(sirver,trading): Maybe check connectivity between markets here and
	// report errors.
	game.propose_trade(trade_);
}

CmdProposeTrade::CmdProposeTrade(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	trade_.initiator = des.unsigned_32();
	trade_.receiver = des.unsigned_32();
	trade_.items_to_send = deserialize_bill_of_materials(&des);
	trade_.items_to_receive = deserialize_bill_of_materials(&des);
	trade_.num_batches = des.signed_32();
}

void CmdProposeTrade::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(trade_.initiator);
	ser.unsigned_32(trade_.receiver);
	serialize_bill_of_materials(trade_.items_to_send, &ser);
	serialize_bill_of_materials(trade_.items_to_receive, &ser);
	ser.signed_32(trade_.num_batches);
}

void CmdProposeTrade::read(FileRead& /* fr */,
                           EditorGameBase& /* egbase */,
                           MapObjectLoader& /* mol */) {
	// TODO(sirver,trading): Implement this.
	NEVER_HERE();
}

void CmdProposeTrade::write(FileWrite& /* fw */,
                            EditorGameBase& /* egbase */,
                            MapObjectSaver& /* mos */) {
	// TODO(sirver,trading): Implement this.
	NEVER_HERE();
}

// CmdToggleMuteMessages
void CmdToggleMuteMessages::execute(Game& game) {
	if (upcast(Building, b, game.objects().get_object(building_))) {
		if (all_) {
			const DescriptionIndex di = game.descriptions().safe_building_index(b->descr().name());
			b->get_owner()->set_muted(di, !b->owner().is_muted(di));
		} else {
			b->set_mute_messages(!b->mute_messages());
		}
	}
}

CmdToggleMuteMessages::CmdToggleMuteMessages(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	building_ = des.unsigned_32();
	all_ = des.unsigned_8();
}

void CmdToggleMuteMessages::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(building_);
	ser.unsigned_8(all_ ? 1 : 0);
}

constexpr uint8_t kCurrentPacketVersionCmdToggleMuteMessages = 1;

void CmdToggleMuteMessages::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdToggleMuteMessages) {
			PlayerCommand::read(fr, egbase, mol);
			building_ = fr.unsigned_32();
			all_ = fr.unsigned_8();
		} else {
			throw UnhandledVersionError(
			   "CmdToggleMuteMessages", packet_version, kCurrentPacketVersionCmdToggleMuteMessages);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_ToggleMuteMessages: %s", e.what());
	}
}

void CmdToggleMuteMessages::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdToggleMuteMessages);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(building_)));
	fw.unsigned_8(all_);
}

// CmdMarkMapObjectForRemoval
void CmdMarkMapObjectForRemoval::execute(Game& game) {
	if (upcast(Immovable, mo, game.objects().get_object(object_))) {
		mo->set_marked_for_removal(sender(), mark_);
	}
}

CmdMarkMapObjectForRemoval::CmdMarkMapObjectForRemoval(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	object_ = des.unsigned_32();
	mark_ = des.unsigned_8();
}

void CmdMarkMapObjectForRemoval::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_32(object_);
	ser.unsigned_8(mark_ ? 1 : 0);
}

constexpr uint8_t kCurrentPacketVersionCmdMarkMapObjectForRemoval = 1;

void CmdMarkMapObjectForRemoval::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdMarkMapObjectForRemoval) {
			PlayerCommand::read(fr, egbase, mol);
			object_ = fr.unsigned_32();
			mark_ = fr.unsigned_8();
		} else {
			throw UnhandledVersionError("CmdMarkMapObjectForRemoval", packet_version,
			                            kCurrentPacketVersionCmdMarkMapObjectForRemoval);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_MarkMapObjectForRemoval: %s", e.what());
	}
}

void CmdMarkMapObjectForRemoval::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdMarkMapObjectForRemoval);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(object_)));
	fw.unsigned_8(mark_);
}

// CmdPickCustomStartingPosition
void CmdPickCustomStartingPosition::execute(Game& game) {
	game.get_player(sender())->do_pick_custom_starting_position(coords_);
}

CmdPickCustomStartingPosition::CmdPickCustomStartingPosition(StreamRead& des)
   : PlayerCommand(Time(0), des.unsigned_8()) {
	coords_.x = des.unsigned_16();
	coords_.y = des.unsigned_16();
}

void CmdPickCustomStartingPosition::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_16(coords_.x);
	ser.unsigned_16(coords_.y);
}

constexpr uint8_t kCurrentPacketVersionCmdPickCustomStartingPosition = 1;

void CmdPickCustomStartingPosition::read(FileRead& fr,
                                         EditorGameBase& egbase,
                                         MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdPickCustomStartingPosition) {
			PlayerCommand::read(fr, egbase, mol);
			coords_.x = fr.unsigned_16();
			coords_.y = fr.unsigned_16();
		} else {
			throw UnhandledVersionError("CmdPickCustomStartingPosition", packet_version,
			                            kCurrentPacketVersionCmdPickCustomStartingPosition);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_PickCustomStartingPosition: %s", e.what());
	}
}

void CmdPickCustomStartingPosition::write(FileWrite& fw,
                                          EditorGameBase& egbase,
                                          MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdPickCustomStartingPosition);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_16(coords_.x);
	fw.unsigned_16(coords_.y);
}

}  // namespace Widelands
