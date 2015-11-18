/*
 * Copyright (C) 2004, 2007-2011, 2013 by the Widelands Development Team
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
#include "economy/wares_queue.h"
#include "io/fileread.h"
#include "io/filewrite.h"
#include "io/streamwrite.h"
#include "logic/game.h"
#include "logic/instances.h"
#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/tribes/tribe_descr.h"
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
template<typename T>
Serial get_object_serial_or_zero(uint32_t object_index, MapObjectLoader& mol) {
	if (!object_index)
		return 0;
	return mol.get<T>(object_index).serial();
}

}  // namespace

// NOTE keep numbers of existing entries as they are to ensure backward compatible savegame loading
enum {
	PLCMD_UNUSED                           = 0,
	PLCMD_BULLDOZE                         = 1,
	PLCMD_BUILD                            = 2,
	PLCMD_BUILDFLAG                        = 3,
	PLCMD_BUILDROAD                        = 4,
	PLCMD_FLAGACTION                       = 5,
	PLCMD_STARTSTOPBUILDING                = 6,
	PLCMD_ENHANCEBUILDING                  = 7,
	PLCMD_CHANGETRAININGOPTIONS            = 8,
	PLCMD_DROPSOLDIER                      = 9,
	PLCMD_CHANGESOLDIERCAPACITY            = 10,
	PLCMD_ENEMYFLAGACTION                  = 11,
	PLCMD_SETWAREPRIORITY                  = 12,
	PLCMD_SETWARETARGETQUANTITY            = 13,
	PLCMD_RESETWARETARGETQUANTITY          = 14,
	PLCMD_SETWORKERTARGETQUANTITY          = 15,
	PLCMD_RESETWORKERTARGETQUANTITY        = 16,
	// Used to be PLCMD_CHANGEMILITARYCONFIG
	PLCMD_MESSAGESETSTATUSREAD             = 18,
	PLCMD_MESSAGESETSTATUSARCHIVED         = 19,
	PLCMD_SETSTOCKPOLICY                   = 20,
	PLCMD_SETWAREMAXFILL                   = 21,
	PLCMD_DISMANTLEBUILDING                = 22,
	PLCMD_EVICTWORKER                      = 23,
	PLCMD_MILITARYSITESETSOLDIERPREFERENCE = 24,
	PLCMD_SHIP_EXPEDITION                  = 25,
	PLCMD_SHIP_SCOUT                       = 26,
	PLCMD_SHIP_EXPLORE                     = 27,
	PLCMD_SHIP_CONSTRUCT                   = 28,
	PLCMD_SHIP_SINK                        = 29,
	PLCMD_SHIP_CANCELEXPEDITION            = 30
};

/*** class PlayerCommand ***/

PlayerCommand::PlayerCommand (const int32_t time, const PlayerNumber s)
	: GameLogicCommand (time), m_sender(s), m_cmdserial(0)
{}

PlayerCommand * PlayerCommand::deserialize (StreamRead & des)
{
	switch (des.unsigned_8()) {
	case PLCMD_BULLDOZE:                  return new CmdBulldoze                 (des);
	case PLCMD_BUILD:                     return new CmdBuild                    (des);
	case PLCMD_BUILDFLAG:                 return new CmdBuildFlag                (des);
	case PLCMD_BUILDROAD:                 return new CmdBuildRoad                (des);
	case PLCMD_FLAGACTION:                return new CmdFlagAction               (des);
	case PLCMD_STARTSTOPBUILDING:         return new CmdStartStopBuilding        (des);
	case PLCMD_SHIP_EXPEDITION:           return new CmdStartOrCancelExpedition  (des);
	case PLCMD_SHIP_SCOUT:                return new CmdShipScoutDirection       (des);
	case PLCMD_SHIP_EXPLORE:              return new CmdShipExploreIsland        (des);
	case PLCMD_SHIP_CONSTRUCT:            return new CmdShipConstructPort        (des);
	case PLCMD_SHIP_SINK:                 return new CmdShipSink                 (des);
	case PLCMD_SHIP_CANCELEXPEDITION:     return new CmdShipCancelExpedition     (des);
	case PLCMD_ENHANCEBUILDING:           return new CmdEnhanceBuilding          (des);
	case PLCMD_CHANGETRAININGOPTIONS:     return new CmdChangeTrainingOptions    (des);
	case PLCMD_DROPSOLDIER:               return new CmdDropSoldier              (des);
	case PLCMD_CHANGESOLDIERCAPACITY:     return new CmdChangeSoldierCapacity    (des);
	case PLCMD_ENEMYFLAGACTION:           return new CmdEnemyFlagAction          (des);
	case PLCMD_SETWAREPRIORITY:           return new CmdSetWarePriority          (des);
	case PLCMD_SETWARETARGETQUANTITY:     return new CmdSetWareTargetQuantity    (des);
	case PLCMD_RESETWARETARGETQUANTITY:   return new CmdResetWareTargetQuantity  (des);
	case PLCMD_SETWORKERTARGETQUANTITY:   return new CmdSetWorkerTargetQuantity  (des);
	case PLCMD_RESETWORKERTARGETQUANTITY: return new CmdResetWorkerTargetQuantity(des);
	case PLCMD_MESSAGESETSTATUSREAD:      return new CmdMessageSetStatusRead     (des);
	case PLCMD_MESSAGESETSTATUSARCHIVED:  return new CmdMessageSetStatusArchived (des);
	case PLCMD_SETSTOCKPOLICY:            return new CmdSetStockPolicy           (des);
	case PLCMD_SETWAREMAXFILL:            return new CmdSetWareMaxFill           (des);
	case PLCMD_DISMANTLEBUILDING:         return new CmdDismantleBuilding        (des);
	case PLCMD_EVICTWORKER:               return new CmdEvictWorker              (des);
	case PLCMD_MILITARYSITESETSOLDIERPREFERENCE: return new CmdMilitarySiteSetSoldierPreference(des);
	default:
		throw wexception
			("PlayerCommand::deserialize(): Invalid command id encountered");
	}
}

/**
 * Write this player command to a file. Call this from base classes
 */
constexpr uint16_t kCurrentPacketVersionPlayerCommand = 2;

void PlayerCommand::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionPlayerCommand);

	GameLogicCommand::write(fw, egbase, mos);
	// Now sender
	fw.unsigned_8  (sender   ());
	fw.unsigned_32 (cmdserial());
}

void PlayerCommand::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionPlayerCommand) {
			GameLogicCommand::read(fr, egbase, mol);
			m_sender    = fr.unsigned_8 ();
			if (!egbase.get_player(m_sender))
				throw GameDataError("player %u does not exist", m_sender);
			m_cmdserial = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("PlayerCommand", packet_version, kCurrentPacketVersionPlayerCommand);
		}
	} catch (const WException & e) {
		throw GameDataError("player command: %s", e.what());
	}
}

/*** class Cmd_Bulldoze ***/

CmdBulldoze::CmdBulldoze (StreamRead & des) :
	PlayerCommand (0, des.unsigned_8()),
	serial        (des.unsigned_32()),
	recurse       (des.unsigned_8())
{}

void CmdBulldoze::execute (Game & game)
{
	if (upcast(PlayerImmovable, pimm, game.objects().get_object(serial)))
		game.player(sender()).bulldoze(*pimm, recurse);
}

void CmdBulldoze::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_BULLDOZE);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
	ser.unsigned_8 (recurse);
}

constexpr uint16_t kCurrentPacketVersionCmdBulldoze = 2;

void CmdBulldoze::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBulldoze) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PlayerImmovable>(fr.unsigned_32(), mol);
			recurse = 2 <= packet_version ? fr.unsigned_8() : false;
		} else {
			throw UnhandledVersionError("CmdBulldoze",
												 packet_version, kCurrentPacketVersionCmdBulldoze);
		}
	} catch (const WException & e) {
		throw GameDataError("bulldoze: %s", e.what());
	}
}
void CmdBulldoze::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBulldoze);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
	fw.unsigned_8(recurse);
}

/*** class Cmd_Build ***/

CmdBuild::CmdBuild (StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	bi = des.signed_16();
	coords = read_coords_32(&des);
}

void CmdBuild::execute (Game & game)
{
	// Empty former vector since its a new csite.
	Building::FormerBuildings former_buildings;
	game.player(sender()).build(coords, bi, true, former_buildings);
}

void CmdBuild::serialize (StreamWrite & ser) {
	ser.unsigned_8 (PLCMD_BUILD);
	ser.unsigned_8 (sender());
	ser.signed_16  (bi);
	write_coords_32  (&ser, coords);
}

constexpr uint16_t kCurrentPacketVersionCmdBuild = 1;

void CmdBuild::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBuild) {
			PlayerCommand::read(fr, egbase, mol);
			bi = fr.unsigned_16();
			coords = read_coords_32(&fr, egbase.map().extent());
		} else {
			throw UnhandledVersionError("CmdBuild",
												 packet_version, kCurrentPacketVersionCmdBuild);
		}

	} catch (const WException & e) {
		throw GameDataError("build: %s", e.what());
	}
}

void CmdBuild::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBuild);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_16(bi);
	write_coords_32  (&fw, coords);
}


/*** class Cmd_BuildFlag ***/

CmdBuildFlag::CmdBuildFlag (StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	coords = read_coords_32(&des);
}

void CmdBuildFlag::execute (Game & game)
{
	game.player(sender()).build_flag(coords);
}

void CmdBuildFlag::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_BUILDFLAG);
	ser.unsigned_8 (sender());
	write_coords_32  (&ser, coords);
}

constexpr uint16_t kCurrentPacketVersionCmdBuildFlag = 1;

void CmdBuildFlag::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBuildFlag) {
			PlayerCommand::read(fr, egbase, mol);
			coords = read_coords_32(&fr, egbase.map().extent());
		} else {
			throw UnhandledVersionError("CmdBuildFlag",
												 packet_version, kCurrentPacketVersionCmdBuildFlag);
		}
	} catch (const WException & e) {
		throw GameDataError("build flag: %s", e.what());
	}
}
void CmdBuildFlag::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBuildFlag);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	write_coords_32  (&fw, coords);
}

/*** class Cmd_BuildRoad ***/

CmdBuildRoad::CmdBuildRoad (int32_t t, int32_t p, Path & pa) :
PlayerCommand(t, p),
path         (&pa),
start        (pa.get_start()),
nsteps       (pa.get_nsteps()),
steps        (nullptr)
{}

CmdBuildRoad::CmdBuildRoad (StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	start = read_coords_32(&des);
	nsteps = des.unsigned_16();

	// we cannot completely deserialize the path here because we don't have a Map
	path = nullptr;
	steps = new char[nsteps];

	for (Path::StepVector::size_type i = 0; i < nsteps; ++i)
		steps[i] = des.unsigned_8();
}

CmdBuildRoad::~CmdBuildRoad ()
{
	delete path;

	delete[] steps;
}

void CmdBuildRoad::execute (Game & game)
{
	if (path == nullptr) {
		assert (steps);

		path = new Path(start);
		for (Path::StepVector::size_type i = 0; i < nsteps; ++i)
			path->append (game.map(), steps[i]);
	}

	game.player(sender()).build_road(*path);
}

void CmdBuildRoad::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_BUILDROAD);
	ser.unsigned_8 (sender());
	write_coords_32  (&ser, start);
	ser.unsigned_16(nsteps);

	assert (path || steps);

	for (Path::StepVector::size_type i = 0; i < nsteps; ++i)
		ser.unsigned_8(path ? (*path)[i] : steps[i]);
}

constexpr uint16_t kCurrentPacketVersionCmdBuildRoad = 1;

void CmdBuildRoad::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdBuildRoad) {
			PlayerCommand::read(fr, egbase, mol);
			start = read_coords_32(&fr, egbase.map().extent());
			nsteps = fr.unsigned_16();
			path = nullptr;
			steps = new char[nsteps];
			for (Path::StepVector::size_type i = 0; i < nsteps; ++i)
			steps[i] = fr.unsigned_8();
		} else {
			throw UnhandledVersionError("CmdBuildRoad",
												 packet_version, kCurrentPacketVersionCmdBuildRoad);
		}
	} catch (const WException & e) {
		throw GameDataError("build road: %s", e.what());
	}
}
void CmdBuildRoad::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdBuildRoad);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	write_coords_32  (&fw, start);
	fw.unsigned_16(nsteps);
	for (Path::StepVector::size_type i = 0; i < nsteps; ++i)
		fw.unsigned_8(path ? (*path)[i] : steps[i]);
}


/*** Cmd_FlagAction ***/
CmdFlagAction::CmdFlagAction (StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	des         .unsigned_8 ();
	serial = des.unsigned_32();
}

void CmdFlagAction::execute (Game & game)
{
	Player & player = game.player(sender());
	if (upcast(Flag, flag, game.objects().get_object(serial)))
		if (&flag->owner() == &player)
			player.flagaction (*flag);
}

void CmdFlagAction::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_FLAGACTION);
	ser.unsigned_8 (sender());
	ser.unsigned_8 (0);
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionCmdFlagAction = 1;

void CmdFlagAction::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdFlagAction) {
			PlayerCommand::read(fr, egbase, mol);
			fr                             .unsigned_8 ();
			serial = get_object_serial_or_zero<Flag>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdFlagAction",
												 packet_version, kCurrentPacketVersionCmdFlagAction);
		}
	} catch (const WException & e) {
		throw GameDataError("flag action: %s", e.what());
	}
}
void CmdFlagAction::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdFlagAction);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	// Now action
	fw.unsigned_8 (0);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

/*** Cmd_StartStopBuilding ***/

CmdStartStopBuilding::CmdStartStopBuilding (StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
}

void CmdStartStopBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).start_stop_building(*building);
}

void CmdStartStopBuilding::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_STARTSTOPBUILDING);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionCmdStartStopBuilding = 1;

void CmdStartStopBuilding::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdStartStopBuilding) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdStartStopBuilding",
												 packet_version, kCurrentPacketVersionCmdStartStopBuilding);
		}
	} catch (const WException & e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}
void CmdStartStopBuilding::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdStartStopBuilding);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


CmdMilitarySiteSetSoldierPreference::CmdMilitarySiteSetSoldierPreference (StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
	preference = des.unsigned_8();
}

void CmdMilitarySiteSetSoldierPreference::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_MILITARYSITESETSOLDIERPREFERENCE);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
	ser.unsigned_8 (preference);
}

void CmdMilitarySiteSetSoldierPreference::execute (Game & game)
{
	if (upcast(MilitarySite, building, game.objects().get_object(serial)))
		game.player(sender()).military_site_set_soldier_preference(*building, preference);

}

constexpr uint16_t kCurrentPacketVersionSoldierPreference = 1;

void CmdMilitarySiteSetSoldierPreference::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionSoldierPreference);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_8(preference);

	// Now serial.
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

void CmdMilitarySiteSetSoldierPreference::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try
	{
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionSoldierPreference) {
			PlayerCommand::read(fr, egbase, mol);
			preference = fr.unsigned_8();
			serial = get_object_serial_or_zero<MilitarySite>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdMilitarySiteSetSoldierPreference",
												 packet_version, kCurrentPacketVersionSoldierPreference);
		}
	} catch (const WException & e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}


/*** Cmd_StartOrCancelExpedition ***/

CmdStartOrCancelExpedition::CmdStartOrCancelExpedition (StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
}

void CmdStartOrCancelExpedition::execute (Game & game)
{
	if (upcast(Warehouse, warehouse, game.objects().get_object(serial)))
		game.player(sender()).start_or_cancel_expedition(*warehouse);
}

void CmdStartOrCancelExpedition::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_SHIP_EXPEDITION);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionExpedition = 1;

void CmdStartOrCancelExpedition::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionExpedition) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Warehouse>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdStartOrCancelExpedition",
												 packet_version, kCurrentPacketVersionExpedition);
		}
	} catch (const WException & e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}
void CmdStartOrCancelExpedition::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionExpedition);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** Cmd_EnhanceBuilding ***/

CmdEnhanceBuilding::CmdEnhanceBuilding (StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
	bi = des.unsigned_16();
}

void CmdEnhanceBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).enhance_building(building, bi);
}

void CmdEnhanceBuilding::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_ENHANCEBUILDING);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
	ser.unsigned_16(bi);
}

constexpr uint16_t kCurrentPacketVersionCmdEnhanceBuilding = 1;

void CmdEnhanceBuilding::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdEnhanceBuilding) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			bi = fr.unsigned_16();
		} else {
			throw UnhandledVersionError("CmdEnhanceBuilding",
												 packet_version, kCurrentPacketVersionCmdEnhanceBuilding);
		}
	} catch (const WException & e) {
		throw GameDataError("enhance building: %s", e.what());
	}
}
void CmdEnhanceBuilding::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdEnhanceBuilding);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Now id
	fw.unsigned_16(bi);
}


/*** Cmd_DismantleBuilding ***/
CmdDismantleBuilding::CmdDismantleBuilding (StreamRead & des) :
	PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
}

void CmdDismantleBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).dismantle_building(building);
}

void CmdDismantleBuilding::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_DISMANTLEBUILDING);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionDismantleBuilding = 1;

void CmdDismantleBuilding::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionDismantleBuilding) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdDismantleBuilding",
												 packet_version, kCurrentPacketVersionDismantleBuilding);
		}
	} catch (const WException & e) {
		throw GameDataError("dismantle building: %s", e.what());
	}
}
void CmdDismantleBuilding::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionDismantleBuilding);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

/*** Cmd_EvictWorker ***/
CmdEvictWorker::CmdEvictWorker (StreamRead& des) :
	PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
}

void CmdEvictWorker::execute (Game & game)
{
	upcast(Worker, worker, game.objects().get_object(serial));
	if (worker && worker->owner().player_number() == sender()) {
		worker->evict(game);
	}
}

void CmdEvictWorker::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_EVICTWORKER);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionCmdEvictWorker = 1;

void CmdEvictWorker::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdEvictWorker) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Worker>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdEvictWorker",
												 packet_version, kCurrentPacketVersionCmdEvictWorker);
		}
	} catch (const WException & e) {
		throw GameDataError("evict worker: %s", e.what());
	}
}
void CmdEvictWorker::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdEvictWorker);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** Cmd_ShipScoutDirection ***/
CmdShipScoutDirection::CmdShipScoutDirection (StreamRead& des) :
	PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
	dir    = static_cast<WalkingDir>(des.unsigned_8());
}

void CmdShipScoutDirection::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		if (!(ship->get_ship_state() == Widelands::Ship::EXP_WAITING ||
			ship->get_ship_state() == Widelands::Ship::EXP_FOUNDPORTSPACE ||
			ship->get_ship_state() == Widelands::Ship::EXP_SCOUTING)) {
			log (" %1d:ship on %3dx%3d received scout command but not in "
				"EXP_WAITING or PORTSPACE_FOUND or EXP_SCOUTING status "
				"(expedition: %s), ignoring...\n",
				ship->get_owner()->player_number(),
				ship->get_position().x,
				ship->get_position().y,
				(ship->state_is_expedition())?"Y":"N");
			return;
		}
		ship->exp_scouting_direction(game, dir);
	}
}

void CmdShipScoutDirection::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_SHIP_SCOUT);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
	ser.unsigned_8 (static_cast<uint8_t>(dir));
}

constexpr uint16_t kCurrentPacketVersionShipScoutDirection = 1;

void CmdShipScoutDirection::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipScoutDirection) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			// direction
			dir = static_cast<WalkingDir>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError("CmdShipScoutDirection",
												 packet_version, kCurrentPacketVersionShipScoutDirection);
		}
	} catch (const WException & e) {
		throw GameDataError("Ship scout: %s", e.what());
	}
}
void CmdShipScoutDirection::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
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
CmdShipConstructPort::CmdShipConstructPort (StreamRead& des) :
	PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
	coords = read_coords_32(&des);
}

void CmdShipConstructPort::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		if (ship->get_ship_state() != Widelands::Ship::EXP_FOUNDPORTSPACE) {
			log (" %1d:ship on %3dx%3d received build port command but "
			"not in PORTSPACE_FOUND status (expedition: %s), ignoring...\n",
				ship->get_owner()->player_number(),
				ship->get_position().x,
				ship->get_position().y,
				(ship->state_is_expedition())?"Y":"N");
			return;
		}
		ship->exp_construct_port(game, coords);
	}
}

void CmdShipConstructPort::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_SHIP_CONSTRUCT);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
	write_coords_32  (&ser, coords);
}

constexpr uint16_t kCurrentPacketVersionShipConstructPort = 1;

void CmdShipConstructPort::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipConstructPort) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			// Coords
			coords = read_coords_32(&fr);
		} else {
			throw UnhandledVersionError("CmdShipConstructPort",
												 packet_version, kCurrentPacketVersionShipConstructPort);
		}
	} catch (const WException & e) {
		throw GameDataError("Ship construct port: %s", e.what());
	}
}
void CmdShipConstructPort::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
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
CmdShipExploreIsland::CmdShipExploreIsland (StreamRead& des) :
	PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
	island_explore_direction = static_cast<IslandExploreDirection>(des.unsigned_8());
}

void CmdShipExploreIsland::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		if (!(ship->get_ship_state() == Widelands::Ship::EXP_WAITING ||
			ship->get_ship_state() == Widelands::Ship::EXP_FOUNDPORTSPACE ||
			ship->get_ship_state() == Widelands::Ship::EXP_SCOUTING)) {
			log (" %1d:ship on %3dx%3d received explore island command "
			"but not in EXP_WAITING or PORTSPACE_FOUND or EXP_SCOUTING "
			"status (expedition: %s), ignoring...\n",
				ship->get_owner()->player_number(),
				ship->get_position().x,
				ship->get_position().y,
				(ship->state_is_expedition())?"Y":"N");
			return;
		}
		ship->exp_explore_island(game, island_explore_direction);
	}
}

void CmdShipExploreIsland::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_SHIP_EXPLORE);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
	ser.unsigned_8 (static_cast<uint8_t>(island_explore_direction));
}

constexpr uint16_t kCurrentPacketVersionShipExploreIsland = 1;

void CmdShipExploreIsland::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipExploreIsland) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
			island_explore_direction = static_cast<IslandExploreDirection>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError("CmdShipExploreIsland",
												 packet_version, kCurrentPacketVersionShipExploreIsland);
		}
	} catch (const WException & e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipExploreIsland::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
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
CmdShipSink::CmdShipSink (StreamRead& des) :
	PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
}

void CmdShipSink::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->sink_ship(game);
	}
}

void CmdShipSink::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_SHIP_SINK);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionCmdShipSink = 1;

void CmdShipSink::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdShipSink) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdShipSink", packet_version,
												 kCurrentPacketVersionCmdShipSink);
		}
	} catch (const WException & e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipSink::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdShipSink);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** Cmd_ShipCancelExpedition ***/
CmdShipCancelExpedition::CmdShipCancelExpedition (StreamRead& des) :
	PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
}

void CmdShipCancelExpedition::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->exp_cancel(game);
	}
}

void CmdShipCancelExpedition::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_SHIP_CANCELEXPEDITION);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
}

constexpr uint16_t kCurrentPacketVersionShipCancelExpedition = 1;

void CmdShipCancelExpedition::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionShipCancelExpedition) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdShipCancelExpedition",
												 packet_version, kCurrentPacketVersionShipCancelExpedition);
		}
	} catch (const WException & e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipCancelExpedition::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionShipCancelExpedition);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** class Cmd_SetWarePriority ***/
CmdSetWarePriority::CmdSetWarePriority
	(const int32_t _duetime, const PlayerNumber _sender,
	 PlayerImmovable & imm,
	 const int32_t type, const DescriptionIndex index, const int32_t priority)
	:
	PlayerCommand(_duetime, _sender),
	m_serial     (imm.serial()),
	m_type       (type),
	m_index      (index),
	m_priority   (priority)
{}

void CmdSetWarePriority::execute(Game & game)
{
	upcast(Building, psite, game.objects().get_object(m_serial));

	if (!psite)
		return;
	if (psite->owner().player_number() != sender())
		return;

	psite->set_priority(m_type, m_index, m_priority);
}

constexpr uint16_t kCurrentPacketVersionCmdSetWarePriority = 1;

void CmdSetWarePriority::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersionCmdSetWarePriority);

	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(m_serial)));
	fw.unsigned_8(m_type);
	fw.signed_32(m_index);
	fw.signed_32(m_priority);
}

void CmdSetWarePriority::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdSetWarePriority) {
			PlayerCommand::read(fr, egbase, mol);
			m_serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			m_type = fr.unsigned_8();
			m_index = fr.signed_32();
			m_priority = fr.signed_32();
		} else {
			throw UnhandledVersionError("CmdSetWarePriority",
												 packet_version, kCurrentPacketVersionCmdSetWarePriority);
		}

	} catch (const WException & e) {
		throw GameDataError("set ware priority: %s", e.what());
	}
}

CmdSetWarePriority::CmdSetWarePriority(StreamRead & des) :
	PlayerCommand(0, des.unsigned_8()),
	m_serial     (des.unsigned_32()),
	m_type       (des.unsigned_8()),
	m_index      (des.signed_32()),
	m_priority   (des.signed_32())
{}

void CmdSetWarePriority::serialize(StreamWrite & ser)
{
	ser.unsigned_8(PLCMD_SETWAREPRIORITY);
	ser.unsigned_8(sender());
	ser.unsigned_32(m_serial);
	ser.unsigned_8(m_type);
	ser.signed_32(m_index);
	ser.signed_32(m_priority);
}

/*** class Cmd_SetWareMaxFill ***/
CmdSetWareMaxFill::CmdSetWareMaxFill
	(const int32_t _duetime, const PlayerNumber _sender,
	 PlayerImmovable & imm,
	 const DescriptionIndex index, const uint32_t max_fill)
	:
	PlayerCommand(_duetime, _sender),
	m_serial     (imm.serial()),
	m_index      (index),
	m_max_fill   (max_fill)
{}

void CmdSetWareMaxFill::execute(Game & game)
{
	upcast(Building, b, game.objects().get_object(m_serial));

	if (!b)
		return;
	if (b->owner().player_number() != sender())
		return;

	b->waresqueue(m_index).set_max_fill(m_max_fill);
}

constexpr uint16_t kCurrentPacketVersionCmdSetWareMaxFill = 1;

void CmdSetWareMaxFill::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersionCmdSetWareMaxFill);

	PlayerCommand::write(fw, egbase, mos);

	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(m_serial)));
	fw.signed_32(m_index);
	fw.unsigned_32(m_max_fill);
}

void CmdSetWareMaxFill::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdSetWareMaxFill) {
			PlayerCommand::read(fr, egbase, mol);
			m_serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			m_index = fr.signed_32();
			m_max_fill = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("CmdSetWareMaxFill",
												 packet_version, kCurrentPacketVersionCmdSetWareMaxFill);
		}
	} catch (const WException & e) {
		throw GameDataError("set ware max fill: %s", e.what());
	}
}

CmdSetWareMaxFill::CmdSetWareMaxFill(StreamRead & des) :
	PlayerCommand(0, des.unsigned_8()),
	m_serial     (des.unsigned_32()),
	m_index      (des.signed_32()),
	m_max_fill(des.unsigned_32())
{}

void CmdSetWareMaxFill::serialize(StreamWrite & ser)
{
	ser.unsigned_8(PLCMD_SETWAREMAXFILL);
	ser.unsigned_8(sender());
	ser.unsigned_32(m_serial);
	ser.signed_32(m_index);
	ser.unsigned_32(m_max_fill);
}


CmdChangeTargetQuantity::CmdChangeTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy, const DescriptionIndex _ware_type)
	:
	PlayerCommand(_duetime, _sender),
	m_economy (_economy), m_ware_type(_ware_type)
{}

void CmdChangeTargetQuantity::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(economy());
	fw.c_string
		(egbase.player(sender()).tribe().get_ware_descr(ware_type())->name());
}

void CmdChangeTargetQuantity::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		PlayerCommand::read(fr, egbase, mol);
		m_economy   = fr.unsigned_32();
		m_ware_type =
			egbase.player(sender()).tribe().ware_index(fr.c_string());
	} catch (const WException & e) {
		throw GameDataError("change target quantity: %s", e.what());
	}
}

CmdChangeTargetQuantity::CmdChangeTargetQuantity(StreamRead & des)
	:
	PlayerCommand(0, des.unsigned_8()),
	m_economy    (des.unsigned_32()),
	m_ware_type  (des.unsigned_8())
{}

void CmdChangeTargetQuantity::serialize(StreamWrite & ser)
{
	ser.unsigned_8 (sender());
	ser.unsigned_32(economy());
	ser.unsigned_8 (ware_type());
}


CmdSetWareTargetQuantity::CmdSetWareTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy,
	 const DescriptionIndex _ware_type,
	 const uint32_t _permanent)
	:
	CmdChangeTargetQuantity(_duetime, _sender, _economy, _ware_type),
	m_permanent(_permanent)
{}

void CmdSetWareTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	if (economy() < player.get_nr_economies() && game.tribes().ware_exists(ware_type())) {
		player.get_economy_by_number(economy())->set_ware_target_quantity
			(ware_type(), m_permanent, duetime());
	}
}

constexpr uint16_t kCurrentPacketVersionSetWareTargetQuantity = 2;

void CmdSetWareTargetQuantity::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersionSetWareTargetQuantity);
	CmdChangeTargetQuantity::write(fw, egbase, mos);
	fw.unsigned_32(m_permanent);
}

void CmdSetWareTargetQuantity::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionSetWareTargetQuantity) {
			CmdChangeTargetQuantity::read(fr, egbase, mol);
			m_permanent = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("CmdSetWareTargetQuantity",
												 packet_version, kCurrentPacketVersionSetWareTargetQuantity);
		}
	} catch (const WException & e) {
		throw GameDataError("set ware target quantity: %s", e.what());
	}
}

CmdSetWareTargetQuantity::CmdSetWareTargetQuantity(StreamRead & des)
	:
	CmdChangeTargetQuantity(des),
	m_permanent             (des.unsigned_32())
{
	if (cmdserial() == 1) des.unsigned_32();
}

void CmdSetWareTargetQuantity::serialize(StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_SETWARETARGETQUANTITY);
	CmdChangeTargetQuantity::serialize(ser);
	ser.unsigned_32(m_permanent);
}


CmdResetWareTargetQuantity::CmdResetWareTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy,
	 const DescriptionIndex _ware_type)
	:
	CmdChangeTargetQuantity(_duetime, _sender, _economy, _ware_type)
{}

void CmdResetWareTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	const TribeDescr & tribe = player.tribe();
	if (economy() < player.get_nr_economies() && game.tribes().ware_exists(ware_type())) {
		const int count =
			tribe.get_ware_descr(ware_type())->default_target_quantity(tribe.name());
		player.get_economy_by_number(economy())->set_ware_target_quantity
			(ware_type(),  count, duetime());
	}
}

constexpr uint16_t kCurrentPacketVersionResetWareTargetQuantity = 1;

void CmdResetWareTargetQuantity::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersionResetWareTargetQuantity);
	CmdChangeTargetQuantity::write(fw, egbase, mos);
}

void CmdResetWareTargetQuantity::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionResetWareTargetQuantity) {
			CmdChangeTargetQuantity::read(fr, egbase, mol);
		} else {
			throw UnhandledVersionError("CmdResetWareTargetQuantity",
												 packet_version, kCurrentPacketVersionResetWareTargetQuantity);
		}
	} catch (const WException & e) {
		throw GameDataError("reset target quantity: %s", e.what());
	}
}

CmdResetWareTargetQuantity::CmdResetWareTargetQuantity(StreamRead & des)
	: CmdChangeTargetQuantity(des)
{}

void CmdResetWareTargetQuantity::serialize(StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_RESETWARETARGETQUANTITY);
	CmdChangeTargetQuantity::serialize(ser);
}


CmdSetWorkerTargetQuantity::CmdSetWorkerTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy,
	 const DescriptionIndex _ware_type,
	 const uint32_t _permanent)
	:
	CmdChangeTargetQuantity(_duetime, _sender, _economy, _ware_type),
	m_permanent(_permanent)
{}

void CmdSetWorkerTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	if (economy() < player.get_nr_economies() && game.tribes().ware_exists(ware_type())) {
		player.get_economy_by_number(economy())->set_worker_target_quantity
			(ware_type(),  m_permanent, duetime());
	}
}

constexpr uint16_t kCurrentPacketVersionSetWorkerTargetQuantity = 2;

void CmdSetWorkerTargetQuantity::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersionSetWorkerTargetQuantity);
	CmdChangeTargetQuantity::write(fw, egbase, mos);
	fw.unsigned_32(m_permanent);
}

void CmdSetWorkerTargetQuantity::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionSetWorkerTargetQuantity) {
			CmdChangeTargetQuantity::read(fr, egbase, mol);
			m_permanent = fr.unsigned_32();
		} else {
			throw UnhandledVersionError("CmdSetWorkerTargetQuantity",
												 packet_version, kCurrentPacketVersionSetWorkerTargetQuantity);
		}
	} catch (const WException & e) {
		throw GameDataError("set worker target quantity: %s", e.what());
	}
}

CmdSetWorkerTargetQuantity::CmdSetWorkerTargetQuantity(StreamRead & des)
	:
	CmdChangeTargetQuantity(des),
	m_permanent             (des.unsigned_32())
{
	if (cmdserial() == 1) des.unsigned_32();
}

void CmdSetWorkerTargetQuantity::serialize(StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_SETWORKERTARGETQUANTITY);
	CmdChangeTargetQuantity::serialize(ser);
	ser.unsigned_32(m_permanent);
}


CmdResetWorkerTargetQuantity::CmdResetWorkerTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy,
	 const DescriptionIndex _ware_type)
	:
	CmdChangeTargetQuantity(_duetime, _sender, _economy, _ware_type)
{}

void CmdResetWorkerTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	const TribeDescr & tribe = player.tribe();
	if (economy() < player.get_nr_economies() && game.tribes().ware_exists(ware_type())) {
		const int count =
			tribe.get_ware_descr(ware_type())->default_target_quantity(tribe.name());
		player.get_economy_by_number(economy())->set_worker_target_quantity
			(ware_type(),  count, duetime());
	}
}

constexpr uint16_t kCurrentPacketVersionResetWorkerTargetQuantity = 1;

void CmdResetWorkerTargetQuantity::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersionResetWorkerTargetQuantity);
	CmdChangeTargetQuantity::write(fw, egbase, mos);
}

void CmdResetWorkerTargetQuantity::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionResetWorkerTargetQuantity) {
			CmdChangeTargetQuantity::read(fr, egbase, mol);
		} else {
			throw UnhandledVersionError("CmdResetWorkerTargetQuantity",
												 packet_version, kCurrentPacketVersionResetWorkerTargetQuantity);
		}
	} catch (const WException & e) {
		throw GameDataError("reset worker target quantity: %s", e.what());
	}
}

CmdResetWorkerTargetQuantity::CmdResetWorkerTargetQuantity(StreamRead & des)
	: CmdChangeTargetQuantity(des)
{}

void CmdResetWorkerTargetQuantity::serialize(StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_RESETWORKERTARGETQUANTITY);
	CmdChangeTargetQuantity::serialize(ser);
}


/*** class Cmd_ChangeTrainingOptions ***/
CmdChangeTrainingOptions::CmdChangeTrainingOptions(StreamRead & des)
:
PlayerCommand (0, des.unsigned_8())
{
	serial    = des.unsigned_32();  //  Serial of the building
	attribute = des.unsigned_16();  //  Attribute to modify
	value     = des.unsigned_16();  //  New vale
}

void CmdChangeTrainingOptions::execute (Game & game)
{
	if (upcast(TrainingSite, trainingsite, game.objects().get_object(serial)))
		game.player(sender()).change_training_options
			(*trainingsite, attribute, value);
}

void CmdChangeTrainingOptions::serialize (StreamWrite & ser) {
	ser.unsigned_8 (PLCMD_CHANGETRAININGOPTIONS);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
	ser.unsigned_16(attribute);
	ser.unsigned_16(value);
}

constexpr uint16_t kCurrentPacketVersionChangeTrainingOptions = 1;

void CmdChangeTrainingOptions::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionChangeTrainingOptions) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<TrainingSite>(fr.unsigned_32(), mol);
			attribute = fr.unsigned_16();
			value     = fr.unsigned_16();
		} else {
			throw UnhandledVersionError("CmdChangeTrainingOptions",
												 packet_version, kCurrentPacketVersionChangeTrainingOptions);
		}
	} catch (const WException & e) {
		throw GameDataError("change training options: %s", e.what());
	}
}

void CmdChangeTrainingOptions::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionChangeTrainingOptions);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	fw.unsigned_16(attribute);
	fw.unsigned_16(value);
}

/*** class Cmd_DropSoldier ***/

CmdDropSoldier::CmdDropSoldier(StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	serial  = des.unsigned_32(); //  Serial of the building
	soldier = des.unsigned_32(); //  Serial of soldier
}

void CmdDropSoldier::execute (Game & game)
{
	if (upcast(PlayerImmovable, player_imm, game.objects().get_object(serial)))
		if (upcast(Soldier, s, game.objects().get_object(soldier)))
			game.player(sender()).drop_soldier(*player_imm, *s);
}

void CmdDropSoldier::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_DROPSOLDIER);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
	ser.unsigned_32(soldier);
}

constexpr uint16_t kCurrentPacketVersionCmdDropSoldier = 1;

void CmdDropSoldier::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdDropSoldier) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PlayerImmovable>(fr.unsigned_32(), mol);
			soldier = get_object_serial_or_zero<Soldier>(fr.unsigned_32(), mol);
		} else {
			throw UnhandledVersionError("CmdDropSoldier",
												 packet_version, kCurrentPacketVersionCmdDropSoldier);
		}
	} catch (const WException & e) {
		throw GameDataError("drop soldier: %s", e.what());
	}
}

void CmdDropSoldier::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
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

CmdChangeSoldierCapacity::CmdChangeSoldierCapacity(StreamRead & des)
:
PlayerCommand (0, des.unsigned_8())
{
	serial = des.unsigned_32();
	val    = des.signed_16();
}

void CmdChangeSoldierCapacity::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		if (&building->owner() == game.get_player(sender()))
			if (upcast(SoldierControl, ctrl, building))
				ctrl->changeSoldierCapacity(val);
}

void CmdChangeSoldierCapacity::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_CHANGESOLDIERCAPACITY);
	ser.unsigned_8 (sender());
	ser.unsigned_32(serial);
	ser.signed_16(val);
}

constexpr uint16_t kCurrentPacketVersionChangeSoldierCapacity = 1;

void CmdChangeSoldierCapacity::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionChangeSoldierCapacity) {
			PlayerCommand::read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.unsigned_32(), mol);
			val = fr.signed_16();
		} else {
			throw UnhandledVersionError("CmdChangeSoldierCapacity",
												 packet_version, kCurrentPacketVersionChangeSoldierCapacity);
		}
	} catch (const WException & e) {
		throw GameDataError("change soldier capacity: %s", e.what());
	}
}

void CmdChangeSoldierCapacity::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
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

CmdEnemyFlagAction::CmdEnemyFlagAction (StreamRead & des) :
PlayerCommand (0, des.unsigned_8())
{
	des.unsigned_8();
	serial = des.unsigned_32();
	des.unsigned_8();
	number = des.unsigned_8();
}

void CmdEnemyFlagAction::execute (Game & game)
{
	Player & player = game.player(sender());

	if (upcast(Flag, flag, game.objects().get_object(serial))) {
		log
			("Cmd_EnemyFlagAction::execute player(%u): flag->owner(%d) "
			 "number=%u\n",
			 player.player_number(), flag->owner().player_number(), number);

		if (const Building * const building = flag->get_building()) {
			if
				(player.is_hostile(flag->owner())
				 &&
				 1
				 <
				 player.vision
				 	(Map::get_index
				 	 	(building->get_position(), game.map().get_width())))
				player.enemyflagaction (*flag, sender(), number);
			else
				log
					("Cmd_EnemyFlagAction::execute: ERROR: wrong player target not "
					 "seen or not hostile.\n");
		}
	}
}

void CmdEnemyFlagAction::serialize (StreamWrite & ser) {
	ser.unsigned_8 (PLCMD_ENEMYFLAGACTION);
	ser.unsigned_8 (sender());
	ser.unsigned_8 (1);
	ser.unsigned_32(serial);
	ser.unsigned_8 (sender());
	ser.unsigned_8 (number);
}

constexpr uint16_t kCurrentPacketVersionCmdEnemyFlagAction = 3;

void CmdEnemyFlagAction::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionCmdEnemyFlagAction) {
			PlayerCommand::read(fr, egbase, mol);
			fr           .unsigned_8 ();
			serial = get_object_serial_or_zero<Flag>(fr.unsigned_32(), mol);
			fr           .unsigned_8 ();
			number   = fr.unsigned_8 ();
		} else {
			throw UnhandledVersionError("CmdEnemyFlagAction",
												 packet_version, kCurrentPacketVersionCmdEnemyFlagAction);
		}
	} catch (const WException & e) {
		throw GameDataError("enemy flag action: %s", e.what());
	}
}

void CmdEnemyFlagAction::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.unsigned_16(kCurrentPacketVersionCmdEnemyFlagAction);
	// Write base classes
	PlayerCommand::write(fw, egbase, mos);
	// Now action
	fw.unsigned_8 (0);

	// Now serial
	fw.unsigned_32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Now param
	fw.unsigned_8 (sender());
	fw.unsigned_8 (number);
}

/*** struct PlayerMessageCommand ***/

PlayerMessageCommand::PlayerMessageCommand(StreamRead & des) :
PlayerCommand (0, des.unsigned_8()), m_message_id(des.unsigned_32())
{}

constexpr uint16_t kCurrentPacketVersionPlayerMessageCommand = 1;

void PlayerMessageCommand::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.unsigned_16();
		if (packet_version == kCurrentPacketVersionPlayerMessageCommand) {
			PlayerCommand::read(fr, egbase, mol);
			m_message_id = MessageId(fr.unsigned_32());
			if (!m_message_id)
				throw GameDataError
					("(player %u): message id is null", sender());
		} else {
			throw UnhandledVersionError("PlayerMessageCommand",
												 packet_version, kCurrentPacketVersionPlayerMessageCommand);
		}
	} catch (const WException & e) {
		throw GameDataError("player message: %s", e.what());
	}
}

void PlayerMessageCommand::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.unsigned_16(kCurrentPacketVersionPlayerMessageCommand);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(mos.message_savers[sender() - 1][message_id()].value());
}


/*** struct Cmd_MessageSetStatusRead ***/

void CmdMessageSetStatusRead::execute (Game & game)
{
	game.player(sender()).messages().set_message_status
		(message_id(), Message::Status::kRead);
}

void CmdMessageSetStatusRead::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_MESSAGESETSTATUSREAD);
	ser.unsigned_8 (sender());
	ser.unsigned_32(message_id().value());
}


/*** struct Cmd_MessageSetStatusArchived ***/

void CmdMessageSetStatusArchived::execute (Game & game)
{
	game.player(sender()).messages().set_message_status
		(message_id(), Message::Status::kArchived);
}

void CmdMessageSetStatusArchived::serialize (StreamWrite & ser)
{
	ser.unsigned_8 (PLCMD_MESSAGESETSTATUSARCHIVED);
	ser.unsigned_8 (sender());
	ser.unsigned_32(message_id().value());
}

/*** struct Cmd_SetStockPolicy ***/
CmdSetStockPolicy::CmdSetStockPolicy
	(int32_t time, PlayerNumber p,
	 Warehouse & wh, bool isworker, DescriptionIndex ware,
	 Warehouse::StockPolicy policy)
: PlayerCommand(time, p)
{
	m_warehouse = wh.serial();
	m_isworker = isworker;
	m_ware = ware;
	m_policy = policy;
}

CmdSetStockPolicy::CmdSetStockPolicy()
: PlayerCommand(), m_warehouse(0), m_isworker(false), m_policy()
{
}

uint8_t CmdSetStockPolicy::id() const
{
	return QUEUE_CMD_SETSTOCKPOLICY;
}

void CmdSetStockPolicy::execute(Game & game)
{
	// Sanitize data that could have come from the network
	if (Player * plr = game.get_player(sender())) {
		if (upcast(Warehouse, warehouse, game.objects().get_object(m_warehouse)))
		{
			if (&warehouse->owner() != plr) {
				log
					("Cmd_SetStockPolicy: sender %u, but warehouse owner %u\n",
					 sender(), warehouse->owner().player_number());
				return;
			}

			switch (m_policy) {
			case Warehouse::SP_Normal:
			case Warehouse::SP_Prefer:
			case Warehouse::SP_DontStock:
			case Warehouse::SP_Remove:
				break;
			default:
				log
					("Cmd_SetStockPolicy: sender %u, bad policy %u\n",
					 sender(), m_policy);
				return;
			}

			if (m_isworker) {
				if (!(game.tribes().worker_exists(m_ware))) {
					log
						("Cmd_SetStockPolicy: sender %u, worker %u does not exist\n",
						 sender(), m_ware);
					return;
				}
				warehouse->set_worker_policy(m_ware, m_policy);
			} else {
				if (!(game.tribes().ware_exists(m_ware))) {
					log
						("Cmd_SetStockPolicy: sender %u, ware %u does not exist\n",
						 sender(), m_ware);
					return;
				}
				warehouse->set_ware_policy(m_ware, m_policy);
			}
		}
	}
}

CmdSetStockPolicy::CmdSetStockPolicy(StreamRead & des)
	: PlayerCommand(0, des.unsigned_8())
{
	m_warehouse = des.unsigned_32();
	m_isworker = des.unsigned_8();
	m_ware = DescriptionIndex(des.unsigned_8());
	m_policy = static_cast<Warehouse::StockPolicy>(des.unsigned_8());
}

void CmdSetStockPolicy::serialize(StreamWrite & ser)
{
	ser.unsigned_8(PLCMD_SETSTOCKPOLICY);
	ser.unsigned_8(sender());
	ser.unsigned_32(m_warehouse);
	ser.unsigned_8(m_isworker);
	ser.unsigned_8(m_ware);
	ser.unsigned_8(m_policy);
}

constexpr uint8_t kCurrentPacketVersionCmdSetStockPolicy = 1;

void CmdSetStockPolicy::read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdSetStockPolicy) {
			PlayerCommand::read(fr, egbase, mol);
			m_warehouse = fr.unsigned_32();
			m_isworker = fr.unsigned_8();
			m_ware = DescriptionIndex(fr.unsigned_8());
			m_policy = static_cast<Warehouse::StockPolicy>(fr.unsigned_8());
		} else {
			throw UnhandledVersionError("CmdSetStockPolicy",
												 packet_version, kCurrentPacketVersionCmdSetStockPolicy);
		}
	} catch (const std::exception & e) {
		throw GameDataError("Cmd_SetStockPolicy: %s", e.what());
	}
}

void CmdSetStockPolicy::write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.unsigned_8(kCurrentPacketVersionCmdSetStockPolicy);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_32(m_warehouse);
	fw.unsigned_8(m_isworker);
	fw.unsigned_8(m_ware);
	fw.unsigned_8(m_policy);
}

}
