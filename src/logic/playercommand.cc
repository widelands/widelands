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
#include "logic/tribe.h"
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
	switch (des.Unsigned8()) {
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
#define PLAYER_COMMAND_VERSION 2
void PlayerCommand::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_COMMAND_VERSION);

	GameLogicCommand::Write(fw, egbase, mos);
	// Now sender
	fw.Unsigned8  (sender   ());
	fw.Unsigned32 (cmdserial());
}

void PlayerCommand::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (2 <= packet_version && packet_version <= PLAYER_COMMAND_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			m_sender    = fr.Unsigned8 ();
			if (!egbase.get_player(m_sender))
				throw GameDataError("player %u does not exist", m_sender);
			m_cmdserial = fr.Unsigned32();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("player command: %s", e.what());
	}
}

/*** class Cmd_Bulldoze ***/

CmdBulldoze::CmdBulldoze (StreamRead & des) :
	PlayerCommand (0, des.Unsigned8()),
	serial        (des.Unsigned32()),
	recurse       (des.Unsigned8())
{}

void CmdBulldoze::execute (Game & game)
{
	if (upcast(PlayerImmovable, pimm, game.objects().get_object(serial)))
		game.player(sender()).bulldoze(*pimm, recurse);
}

void CmdBulldoze::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BULLDOZE);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned8 (recurse);
}
#define PLAYER_CMD_BULLDOZE_VERSION 2
void CmdBulldoze::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if
			(1 <= packet_version &&
			 packet_version <= PLAYER_CMD_BULLDOZE_VERSION)
		{
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PlayerImmovable>(fr.Unsigned32(), mol);
			recurse = 2 <= packet_version ? fr.Unsigned8() : false;
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("bulldoze: %s", e.what());
	}
}
void CmdBulldoze::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BULLDOZE_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
	fw.Unsigned8(recurse);
}

/*** class Cmd_Build ***/

CmdBuild::CmdBuild (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	bi = des.Signed16();
	coords = ReadCoords32(&des);
}

void CmdBuild::execute (Game & game)
{
	// Empty former vector since its a new csite.
	Building::FormerBuildings former_buildings;
	game.player(sender()).build(coords, bi, true, former_buildings);
}

void CmdBuild::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_BUILD);
	ser.Unsigned8 (sender());
	ser.Signed16  (bi);
	WriteCoords32  (&ser, coords);
}
#define PLAYER_CMD_BUILD_VERSION 1
void CmdBuild::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BUILD_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			bi = fr.Unsigned16();
			coords = ReadCoords32(&fr, egbase.map().extent());
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("build: %s", e.what());
	}
}

void CmdBuild::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BUILD_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned16(bi);
	WriteCoords32  (&fw, coords);
}


/*** class Cmd_BuildFlag ***/

CmdBuildFlag::CmdBuildFlag (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	coords = ReadCoords32(&des);
}

void CmdBuildFlag::execute (Game & game)
{
	game.player(sender()).build_flag(coords);
}

void CmdBuildFlag::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BUILDFLAG);
	ser.Unsigned8 (sender());
	WriteCoords32  (&ser, coords);
}
#define PLAYER_CMD_BUILDFLAG_VERSION 1
void CmdBuildFlag::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BUILDFLAG_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			coords = ReadCoords32(&fr, egbase.map().extent());
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("build flag: %s", e.what());
	}
}
void CmdBuildFlag::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BUILDFLAG_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	WriteCoords32  (&fw, coords);
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
PlayerCommand (0, des.Unsigned8())
{
	start = ReadCoords32(&des);
	nsteps = des.Unsigned16();

	// we cannot completely deserialize the path here because we don't have a Map
	path = nullptr;
	steps = new char[nsteps];

	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		steps[i] = des.Unsigned8();
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
		for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
			path->append (game.map(), steps[i]);
	}

	game.player(sender()).build_road(*path);
}

void CmdBuildRoad::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BUILDROAD);
	ser.Unsigned8 (sender());
	WriteCoords32  (&ser, start);
	ser.Unsigned16(nsteps);

	assert (path || steps);

	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		ser.Unsigned8(path ? (*path)[i] : steps[i]);
}
#define PLAYER_CMD_BUILDROAD_VERSION 1
void CmdBuildRoad::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BUILDROAD_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			start = ReadCoords32(&fr, egbase.map().extent());
			nsteps = fr.Unsigned16();
			path = nullptr;
			steps = new char[nsteps];
			for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
			steps[i] = fr.Unsigned8();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("build road: %s", e.what());
	}
}
void CmdBuildRoad::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BUILDROAD_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	WriteCoords32  (&fw, start);
	fw.Unsigned16(nsteps);
	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		fw.Unsigned8(path ? (*path)[i] : steps[i]);
}


/*** Cmd_FlagAction ***/
CmdFlagAction::CmdFlagAction (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	des         .Unsigned8 ();
	serial = des.Unsigned32();
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
	ser.Unsigned8 (PLCMD_FLAGACTION);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (0);
	ser.Unsigned32(serial);
}

#define PLAYER_CMD_FLAGACTION_VERSION 1
void CmdFlagAction::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_FLAGACTION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			fr                             .Unsigned8 ();
			serial = get_object_serial_or_zero<Flag>(fr.Unsigned32(), mol);
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("flag action: %s", e.what());
	}
}
void CmdFlagAction::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_FLAGACTION_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	// Now action
	fw.Unsigned8 (0);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

/*** Cmd_StartStopBuilding ***/

CmdStartStopBuilding::CmdStartStopBuilding (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void CmdStartStopBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).start_stop_building(*building);
}

void CmdStartStopBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_STARTSTOPBUILDING);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_STOPBUILDING_VERSION 1
void CmdStartStopBuilding::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_STOPBUILDING_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}
void CmdStartStopBuilding::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_STOPBUILDING_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


CmdMilitarySiteSetSoldierPreference::CmdMilitarySiteSetSoldierPreference (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	preference = des.Unsigned8();
}

void CmdMilitarySiteSetSoldierPreference::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_MILITARYSITESETSOLDIERPREFERENCE);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned8 (preference);
}

void CmdMilitarySiteSetSoldierPreference::execute (Game & game)
{
	if (upcast(MilitarySite, building, game.objects().get_object(serial)))
		game.player(sender()).military_site_set_soldier_preference(*building, preference);

}

#define PLAYER_CMD_SOLDIERPREFERENCE_VERSION 1
void CmdMilitarySiteSetSoldierPreference::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SOLDIERPREFERENCE_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	fw.Unsigned8(preference);

	// Now serial.
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

void CmdMilitarySiteSetSoldierPreference::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try
	{
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SOLDIERPREFERENCE_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			preference = fr.Unsigned8();
			serial = get_object_serial_or_zero<MilitarySite>(fr.Unsigned32(), mol);
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}


/*** Cmd_StartOrCancelExpedition ***/

CmdStartOrCancelExpedition::CmdStartOrCancelExpedition (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void CmdStartOrCancelExpedition::execute (Game & game)
{
	if (upcast(Warehouse, warehouse, game.objects().get_object(serial)))
		game.player(sender()).start_or_cancel_expedition(*warehouse);
}

void CmdStartOrCancelExpedition::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_EXPEDITION);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_EXPEDITION_VERSION 1
void CmdStartOrCancelExpedition::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_EXPEDITION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Warehouse>(fr.Unsigned32(), mol);
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("start/stop building: %s", e.what());
	}
}
void CmdStartOrCancelExpedition::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_EXPEDITION_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** Cmd_EnhanceBuilding ***/

CmdEnhanceBuilding::CmdEnhanceBuilding (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	bi = des.Unsigned16();
}

void CmdEnhanceBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).enhance_building(building, bi);
}

void CmdEnhanceBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_ENHANCEBUILDING);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned16(bi);
}
#define PLAYER_CMD_ENHANCEBUILDING_VERSION 1
void CmdEnhanceBuilding::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_ENHANCEBUILDING_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
			bi = fr.Unsigned16();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("enhance building: %s", e.what());
	}
}
void CmdEnhanceBuilding::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_ENHANCEBUILDING_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Now id
	fw.Unsigned16(bi);
}


/*** Cmd_DismantleBuilding ***/
CmdDismantleBuilding::CmdDismantleBuilding (StreamRead & des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void CmdDismantleBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).dismantle_building(building);
}

void CmdDismantleBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_DISMANTLEBUILDING);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_DISMANTLEBUILDING_VERSION 1
void CmdDismantleBuilding::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_DISMANTLEBUILDING_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("dismantle building: %s", e.what());
	}
}
void CmdDismantleBuilding::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_DISMANTLEBUILDING_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

/*** Cmd_EvictWorker ***/
CmdEvictWorker::CmdEvictWorker (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
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
	ser.Unsigned8 (PLCMD_EVICTWORKER);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_EVICTWORKER_VERSION 1
void CmdEvictWorker::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_EVICTWORKER_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Worker>(fr.Unsigned32(), mol);
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("evict worker: %s", e.what());
	}
}
void CmdEvictWorker::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_EVICTWORKER_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** Cmd_ShipScoutDirection ***/
CmdShipScoutDirection::CmdShipScoutDirection (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	dir    = des.Unsigned8();
}

void CmdShipScoutDirection::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->exp_scout_direction(game, dir);
	}
}

void CmdShipScoutDirection::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_SCOUT);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned8 (dir);
}

#define PLAYER_CMD_SHIP_SCOUT_DIRECTION_VERSION 1
void CmdShipScoutDirection::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_SCOUT_DIRECTION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
			// direction
			dir = fr.Unsigned8();
		} else
			throw GameDataError("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("Ship scout: %s", e.what());
	}
}
void CmdShipScoutDirection::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SHIP_SCOUT_DIRECTION_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// direction
	fw.Unsigned8(dir);
}


/*** Cmd_ShipConstructPort ***/
CmdShipConstructPort::CmdShipConstructPort (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	coords = ReadCoords32(&des);
}

void CmdShipConstructPort::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->exp_construct_port(game, coords);
	}
}

void CmdShipConstructPort::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_CONSTRUCT);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	WriteCoords32  (&ser, coords);
}

#define PLAYER_CMD_SHIP_CONSTRUCT_PORT_VERSION 1
void CmdShipConstructPort::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_CONSTRUCT_PORT_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
			// Coords
			coords = ReadCoords32(&fr);
		} else
			throw GameDataError("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("Ship construct port: %s", e.what());
	}
}
void CmdShipConstructPort::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SHIP_CONSTRUCT_PORT_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Coords
	WriteCoords32(&fw, coords);
}


/*** Cmd_ShipExploreIsland ***/
CmdShipExploreIsland::CmdShipExploreIsland (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	clockwise = des.Unsigned8() == 1;
}

void CmdShipExploreIsland::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->exp_explore_island(game, clockwise);
	}
}

void CmdShipExploreIsland::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_EXPLORE);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned8 (clockwise ? 1 : 0);
}

#define PLAYER_CMD_SHIP_EXPLORE_ISLAND_VERSION 1
void CmdShipExploreIsland::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_EXPLORE_ISLAND_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
			clockwise = fr.Unsigned8() == 1;
		} else
			throw GameDataError("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipExploreIsland::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SHIP_EXPLORE_ISLAND_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Direction of exploration
	fw.Unsigned8 (clockwise ? 1 : 0);
}


/*** Cmd_ShipSink ***/
CmdShipSink::CmdShipSink (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
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
	ser.Unsigned8 (PLCMD_SHIP_SINK);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}

#define PLAYER_CMD_SHIP_SINK_VERSION 1
void CmdShipSink::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_SINK_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
		} else
			throw GameDataError("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipSink::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SHIP_SINK_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** Cmd_ShipCancelExpedition ***/
CmdShipCancelExpedition::CmdShipCancelExpedition (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
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
	ser.Unsigned8 (PLCMD_SHIP_CANCELEXPEDITION);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}

#define PLAYER_CMD_SHIP_CANCELEXPEDITION_VERSION 1
void CmdShipCancelExpedition::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_CANCELEXPEDITION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
		} else
			throw GameDataError("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("Ship explore: %s", e.what());
	}
}
void CmdShipCancelExpedition::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SHIP_CANCELEXPEDITION_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** class Cmd_SetWarePriority ***/
CmdSetWarePriority::CmdSetWarePriority
	(const int32_t _duetime, const PlayerNumber _sender,
	 PlayerImmovable & imm,
	 const int32_t type, const WareIndex index, const int32_t priority)
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

#define PLAYER_CMD_SETWAREPRIORITY_VERSION 1

void CmdSetWarePriority::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWAREPRIORITY_VERSION);

	PlayerCommand::Write(fw, egbase, mos);

	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(m_serial)));
	fw.Unsigned8(m_type);
	fw.Signed32(m_index);
	fw.Signed32(m_priority);
}

void CmdSetWarePriority::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SETWAREPRIORITY_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			m_serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
			m_type = fr.Unsigned8();
			m_index = fr.Signed32();
			m_priority = fr.Signed32();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("set ware priority: %s", e.what());
	}
}

CmdSetWarePriority::CmdSetWarePriority(StreamRead & des) :
	PlayerCommand(0, des.Unsigned8()),
	m_serial     (des.Unsigned32()),
	m_type       (des.Unsigned8()),
	m_index      (des.Signed32()),
	m_priority   (des.Signed32())
{}

void CmdSetWarePriority::serialize(StreamWrite & ser)
{
	ser.Unsigned8(PLCMD_SETWAREPRIORITY);
	ser.Unsigned8(sender());
	ser.Unsigned32(m_serial);
	ser.Unsigned8(m_type);
	ser.Signed32(m_index);
	ser.Signed32(m_priority);
}

/*** class Cmd_SetWareMaxFill ***/
CmdSetWareMaxFill::CmdSetWareMaxFill
	(const int32_t _duetime, const PlayerNumber _sender,
	 PlayerImmovable & imm,
	 const WareIndex index, const uint32_t max_fill)
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

#define PLAYER_CMD_SETWAREMAXFILL_SIZE_VERSION 1

void CmdSetWareMaxFill::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWAREMAXFILL_SIZE_VERSION);

	PlayerCommand::Write(fw, egbase, mos);

	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(m_serial)));
	fw.Signed32(m_index);
	fw.Unsigned32(m_max_fill);
}

void CmdSetWareMaxFill::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SETWAREMAXFILL_SIZE_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			m_serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
			m_index = fr.Signed32();
			m_max_fill = fr.Unsigned32();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("set ware max fill: %s", e.what());
	}
}

CmdSetWareMaxFill::CmdSetWareMaxFill(StreamRead & des) :
	PlayerCommand(0, des.Unsigned8()),
	m_serial     (des.Unsigned32()),
	m_index      (des.Signed32()),
	m_max_fill(des.Unsigned32())
{}

void CmdSetWareMaxFill::serialize(StreamWrite & ser)
{
	ser.Unsigned8(PLCMD_SETWAREMAXFILL);
	ser.Unsigned8(sender());
	ser.Unsigned32(m_serial);
	ser.Signed32(m_index);
	ser.Unsigned32(m_max_fill);
}


CmdChangeTargetQuantity::CmdChangeTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy, const WareIndex _ware_type)
	:
	PlayerCommand(_duetime, _sender),
	m_economy (_economy), m_ware_type(_ware_type)
{}

void CmdChangeTargetQuantity::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned32(economy());
	fw.CString
		(egbase.player(sender()).tribe().get_ware_descr(ware_type())->name());
}

void CmdChangeTargetQuantity::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		PlayerCommand::Read(fr, egbase, mol);
		m_economy   = fr.Unsigned32();
		m_ware_type =
			egbase.player(sender()).tribe().ware_index(fr.CString());
	} catch (const WException & e) {
		throw GameDataError("change target quantity: %s", e.what());
	}
}

CmdChangeTargetQuantity::CmdChangeTargetQuantity(StreamRead & des)
	:
	PlayerCommand(0, des.Unsigned8()),
	m_economy    (des.Unsigned32()),
	m_ware_type  (des.Unsigned8())
{}

void CmdChangeTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (sender());
	ser.Unsigned32(economy());
	ser.Unsigned8 (ware_type());
}


CmdSetWareTargetQuantity::CmdSetWareTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy,
	 const WareIndex _ware_type,
	 const uint32_t _permanent)
	:
	CmdChangeTargetQuantity(_duetime, _sender, _economy, _ware_type),
	m_permanent(_permanent)
{}

void CmdSetWareTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	if
		(economy  () < player.get_nr_economies() &&
		 ware_type() < player.tribe().get_nrwares())
		player.get_economy_by_number(economy())->set_ware_target_quantity
			(ware_type(),  m_permanent, duetime());
}

#define PLAYER_CMD_SETWARETARGETQUANTITY_VERSION 2

void CmdSetWareTargetQuantity::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWARETARGETQUANTITY_VERSION);
	CmdChangeTargetQuantity::Write(fw, egbase, mos);
	fw.Unsigned32(m_permanent);
}

void CmdSetWareTargetQuantity::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version <= PLAYER_CMD_SETWARETARGETQUANTITY_VERSION) {
			CmdChangeTargetQuantity::Read(fr, egbase, mol);
			m_permanent = fr.Unsigned32();
			if (packet_version == 1)
				fr.Unsigned32();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("set ware target quantity: %s", e.what());
	}
}

CmdSetWareTargetQuantity::CmdSetWareTargetQuantity(StreamRead & des)
	:
	CmdChangeTargetQuantity(des),
	m_permanent             (des.Unsigned32())
{
	if (cmdserial() == 1) des.Unsigned32();
}

void CmdSetWareTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SETWARETARGETQUANTITY);
	CmdChangeTargetQuantity::serialize(ser);
	ser.Unsigned32(m_permanent);
}


CmdResetWareTargetQuantity::CmdResetWareTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy,
	 const WareIndex _ware_type)
	:
	CmdChangeTargetQuantity(_duetime, _sender, _economy, _ware_type)
{}

void CmdResetWareTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	const TribeDescr & tribe = player.tribe();
	if
		(economy  () < player.get_nr_economies() &&
		 ware_type() < tribe.get_nrwares())
	{
		const int32_t count =
			tribe.get_ware_descr(ware_type())->default_target_quantity();
		player.get_economy_by_number(economy())->set_ware_target_quantity
			(ware_type(),  count, 0);
	}
}

#define PLAYER_CMD_RESETWARETARGETQUANTITY_VERSION 1

void CmdResetWareTargetQuantity::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.Unsigned16(PLAYER_CMD_RESETWARETARGETQUANTITY_VERSION);
	CmdChangeTargetQuantity::Write(fw, egbase, mos);
}

void CmdResetWareTargetQuantity::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_RESETWARETARGETQUANTITY_VERSION)
			CmdChangeTargetQuantity::Read(fr, egbase, mol);
		else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("reset target quantity: %s", e.what());
	}
}

CmdResetWareTargetQuantity::CmdResetWareTargetQuantity(StreamRead & des)
	: CmdChangeTargetQuantity(des)
{}

void CmdResetWareTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_RESETWARETARGETQUANTITY);
	CmdChangeTargetQuantity::serialize(ser);
}


CmdSetWorkerTargetQuantity::CmdSetWorkerTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy,
	 const WareIndex _ware_type,
	 const uint32_t _permanent)
	:
	CmdChangeTargetQuantity(_duetime, _sender, _economy, _ware_type),
	m_permanent(_permanent)
{}

void CmdSetWorkerTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	if
		(economy  () < player.get_nr_economies() &&
		 ware_type() < player.tribe().get_nrwares())
		player.get_economy_by_number(economy())->set_worker_target_quantity
			(ware_type(),  m_permanent, duetime());
}

#define PLAYER_CMD_SETWORKERTARGETQUANTITY_VERSION 2

void CmdSetWorkerTargetQuantity::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWORKERTARGETQUANTITY_VERSION);
	CmdChangeTargetQuantity::Write(fw, egbase, mos);
	fw.Unsigned32(m_permanent);
}

void CmdSetWorkerTargetQuantity::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version <= PLAYER_CMD_SETWORKERTARGETQUANTITY_VERSION) {
			CmdChangeTargetQuantity::Read(fr, egbase, mol);
			m_permanent = fr.Unsigned32();
			if (packet_version == 1)
				fr.Unsigned32();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("set worker target quantity: %s", e.what());
	}
}

CmdSetWorkerTargetQuantity::CmdSetWorkerTargetQuantity(StreamRead & des)
	:
	CmdChangeTargetQuantity(des),
	m_permanent             (des.Unsigned32())
{
	if (cmdserial() == 1) des.Unsigned32();
}

void CmdSetWorkerTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SETWORKERTARGETQUANTITY);
	CmdChangeTargetQuantity::serialize(ser);
	ser.Unsigned32(m_permanent);
}


CmdResetWorkerTargetQuantity::CmdResetWorkerTargetQuantity
	(const int32_t _duetime, const PlayerNumber _sender,
	 const uint32_t _economy,
	 const WareIndex _ware_type)
	:
	CmdChangeTargetQuantity(_duetime, _sender, _economy, _ware_type)
{}

void CmdResetWorkerTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	const TribeDescr & tribe = player.tribe();
	if
		(economy  () < player.get_nr_economies() &&
		 ware_type() < tribe.get_nrwares())
	{
		const int32_t count =
			tribe.get_ware_descr(ware_type())->default_target_quantity();
		player.get_economy_by_number(economy())->set_worker_target_quantity
			(ware_type(),  count, 0);
	}
}

#define PLAYER_CMD_RESETWORKERTARGETQUANTITY_VERSION 1

void CmdResetWorkerTargetQuantity::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.Unsigned16(PLAYER_CMD_RESETWORKERTARGETQUANTITY_VERSION);
	CmdChangeTargetQuantity::Write(fw, egbase, mos);
}

void CmdResetWorkerTargetQuantity::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_RESETWORKERTARGETQUANTITY_VERSION) {
			CmdChangeTargetQuantity::Read(fr, egbase, mol);
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("reset worker target quantity: %s", e.what());
	}
}

CmdResetWorkerTargetQuantity::CmdResetWorkerTargetQuantity(StreamRead & des)
	: CmdChangeTargetQuantity(des)
{}

void CmdResetWorkerTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_RESETWORKERTARGETQUANTITY);
	CmdChangeTargetQuantity::serialize(ser);
}


/*** class Cmd_ChangeTrainingOptions ***/
CmdChangeTrainingOptions::CmdChangeTrainingOptions(StreamRead & des)
:
PlayerCommand (0, des.Unsigned8())
{
	serial    = des.Unsigned32();  //  Serial of the building
	attribute = des.Unsigned16();  //  Attribute to modify
	value     = des.Unsigned16();  //  New vale
}

void CmdChangeTrainingOptions::execute (Game & game)
{
	if (upcast(TrainingSite, trainingsite, game.objects().get_object(serial)))
		game.player(sender()).change_training_options
			(*trainingsite, attribute, value);
}

void CmdChangeTrainingOptions::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_CHANGETRAININGOPTIONS);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned16(attribute);
	ser.Unsigned16(value);
}


#define PLAYER_CMD_CHANGETRAININGOPTIONS_VERSION 1
void CmdChangeTrainingOptions::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_CHANGETRAININGOPTIONS_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<TrainingSite>(fr.Unsigned32(), mol);
			attribute = fr.Unsigned16();
			value     = fr.Unsigned16();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("change training options: %s", e.what());
	}
}

void CmdChangeTrainingOptions::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_CHANGETRAININGOPTIONS_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	fw.Unsigned16(attribute);
	fw.Unsigned16(value);
}

/*** class Cmd_DropSoldier ***/

CmdDropSoldier::CmdDropSoldier(StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial  = des.Unsigned32(); //  Serial of the building
	soldier = des.Unsigned32(); //  Serial of soldier
}

void CmdDropSoldier::execute (Game & game)
{
	if (upcast(PlayerImmovable, player_imm, game.objects().get_object(serial)))
		if (upcast(Soldier, s, game.objects().get_object(soldier)))
			game.player(sender()).drop_soldier(*player_imm, *s);
}

void CmdDropSoldier::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_DROPSOLDIER);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned32(soldier);
}

#define PLAYER_CMD_DROPSOLDIER_VERSION 1
void CmdDropSoldier::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_DROPSOLDIER_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PlayerImmovable>(fr.Unsigned32(), mol);
			soldier = get_object_serial_or_zero<Soldier>(fr.Unsigned32(), mol);
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("drop soldier: %s", e.what());
	}
}

void CmdDropSoldier::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_DROPSOLDIER_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	//  site serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	//  soldier serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(soldier)));
}

/*** Cmd_ChangeSoldierCapacity ***/

CmdChangeSoldierCapacity::CmdChangeSoldierCapacity(StreamRead & des)
:
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	val    = des.Signed16();
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
	ser.Unsigned8 (PLCMD_CHANGESOLDIERCAPACITY);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Signed16(val);
}

#define PLAYER_CMD_CHANGESOLDIERCAPACITY_VERSION 1
void CmdChangeSoldierCapacity::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_CHANGESOLDIERCAPACITY_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
			val = fr.Signed16();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("change soldier capacity: %s", e.what());
	}
}

void CmdChangeSoldierCapacity::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_CHANGESOLDIERCAPACITY_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Now capacity
	fw.Signed16(val);

}

/*** Cmd_EnemyFlagAction ***/

CmdEnemyFlagAction::CmdEnemyFlagAction (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	des.Unsigned8();
	serial = des.Unsigned32();
	des.Unsigned8();
	number = des.Unsigned8();
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
	ser.Unsigned8 (PLCMD_ENEMYFLAGACTION);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (1);
	ser.Unsigned32(serial);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (number);
}

#define PLAYER_CMD_ENEMYFLAGACTION_VERSION 3
void CmdEnemyFlagAction::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_ENEMYFLAGACTION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			fr           .Unsigned8 ();
			serial = get_object_serial_or_zero<Flag>(fr.Unsigned32(), mol);
			fr           .Unsigned8 ();
			number   = fr.Unsigned8 ();
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("enemy flag action: %s", e.what());
	}
}

void CmdEnemyFlagAction::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_ENEMYFLAGACTION_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	// Now action
	fw.Unsigned8 (0);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Now param
	fw.Unsigned8 (sender());
	fw.Unsigned8 (number);
}

/*** struct PlayerMessageCommand ***/

PlayerMessageCommand::PlayerMessageCommand(StreamRead & des) :
PlayerCommand (0, des.Unsigned8()), m_message_id(des.Unsigned32())
{}

#define PLAYER_MESSAGE_CMD_VERSION 1
void PlayerMessageCommand::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_MESSAGE_CMD_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			m_message_id = Message_Id(fr.Unsigned32());
			if (!m_message_id)
				throw GameDataError
					("(player %u): message id is null", sender());
		} else
			throw GameDataError
				("unknown/unhandled version %u", packet_version);
	} catch (const WException & e) {
		throw GameDataError("player message: %s", e.what());
	}
}

void PlayerMessageCommand::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.Unsigned16(PLAYER_MESSAGE_CMD_VERSION);
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned32(mos.message_savers[sender() - 1][message_id()].value());
}


/*** struct Cmd_MessageSetStatusRead ***/

void CmdMessageSetStatusRead::execute (Game & game)
{
	game.player(sender()).messages().set_message_status
		(message_id(), Message::Read);
}

void CmdMessageSetStatusRead::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_MESSAGESETSTATUSREAD);
	ser.Unsigned8 (sender());
	ser.Unsigned32(message_id().value());
}


/*** struct Cmd_MessageSetStatusArchived ***/

void CmdMessageSetStatusArchived::execute (Game & game)
{
	game.player(sender()).messages().set_message_status
		(message_id(), Message::Archived);
}

void CmdMessageSetStatusArchived::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_MESSAGESETSTATUSARCHIVED);
	ser.Unsigned8 (sender());
	ser.Unsigned32(message_id().value());
}

/*** struct Cmd_SetStockPolicy ***/
CmdSetStockPolicy::CmdSetStockPolicy
	(int32_t time, PlayerNumber p,
	 Warehouse & wh, bool isworker, WareIndex ware,
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

			const TribeDescr & tribe = warehouse->descr().tribe();
			if (m_isworker) {
				if (!(m_ware < tribe.get_nrworkers())) {
					log
						("Cmd_SetStockPolicy: sender %u, worker %u out of bounds\n",
						 sender(), m_ware);
					return;
				}
				warehouse->set_worker_policy(m_ware, m_policy);
			} else {
				if (!(m_ware < tribe.get_nrwares())) {
					log
						("Cmd_SetStockPolicy: sender %u, ware %u out of bounds\n",
						 sender(), m_ware);
					return;
				}
				warehouse->set_ware_policy(m_ware, m_policy);
			}
		}
	}
}

CmdSetStockPolicy::CmdSetStockPolicy(StreamRead & des)
	: PlayerCommand(0, des.Unsigned8())
{
	m_warehouse = des.Unsigned32();
	m_isworker = des.Unsigned8();
	m_ware = WareIndex(des.Unsigned8());
	m_policy = static_cast<Warehouse::StockPolicy>(des.Unsigned8());
}

void CmdSetStockPolicy::serialize(StreamWrite & ser)
{
	ser.Unsigned8(PLCMD_SETSTOCKPOLICY);
	ser.Unsigned8(sender());
	ser.Unsigned32(m_warehouse);
	ser.Unsigned8(m_isworker);
	ser.Unsigned8(m_ware);
	ser.Unsigned8(m_policy);
}

#define PLAYER_CMD_SETSTOCKPOLICY_VERSION 1
void CmdSetStockPolicy::Read
	(FileRead & fr, EditorGameBase & egbase, MapObjectLoader & mol)
{
	try {
		uint8_t version = fr.Unsigned8();
		if (version != PLAYER_CMD_SETSTOCKPOLICY_VERSION)
			throw GameDataError("unknown/unhandled version %u", version);
		PlayerCommand::Read(fr, egbase, mol);
		m_warehouse = fr.Unsigned32();
		m_isworker = fr.Unsigned8();
		m_ware = WareIndex(fr.Unsigned8());
		m_policy = static_cast<Warehouse::StockPolicy>(fr.Unsigned8());
	} catch (const std::exception & e) {
		throw GameDataError("Cmd_SetStockPolicy: %s", e.what());
	}
}

void CmdSetStockPolicy::Write
	(FileWrite & fw, EditorGameBase & egbase, MapObjectSaver & mos)
{
	fw.Unsigned8(PLAYER_CMD_SETSTOCKPOLICY_VERSION);
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned32(m_warehouse);
	fw.Unsigned8(m_isworker);
	fw.Unsigned8(m_ware);
	fw.Unsigned8(m_policy);
}

}
