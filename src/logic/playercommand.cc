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

#include "economy/economy.h"
#include "economy/wares_queue.h"
#include "io/streamwrite.h"
#include "log.h"
#include "logic/game.h"
#include "logic/instances.h"
#include "logic/militarysite.h"
#include "logic/player.h"
#include "logic/ship.h"
#include "logic/soldier.h"
#include "logic/tribe.h"
#include "logic/widelands_fileread.h"
#include "logic/widelands_filewrite.h"
#include "map_io/widelands_map_map_object_loader.h"
#include "map_io/widelands_map_map_object_saver.h"
#include "upcast.h"
#include "wexception.h"

namespace Widelands {

namespace {

// Returns the serial number of the object with the given "object_index" as
// registered by the mol. If object_index is zero (because the object that was
// saved to the file did no longer exist on saving) zero will be returned. That
// means that get_object will always return nullptr and all upcasts will fail -
// so the commands will never do anything when executed.
template<typename T>
Serial get_object_serial_or_zero(uint32_t object_index, Map_Map_Object_Loader& mol) {
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
	PLCMD_CHANGEMILITARYCONFIG             = 17,
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

PlayerCommand::PlayerCommand (const int32_t time, const Player_Number s)
	: GameLogicCommand (time), m_sender(s), m_cmdserial(0)
{}

PlayerCommand * PlayerCommand::deserialize (StreamRead & des)
{
	switch (des.Unsigned8()) {
	case PLCMD_BULLDOZE:                  return new Cmd_Bulldoze                 (des);
	case PLCMD_BUILD:                     return new Cmd_Build                    (des);
	case PLCMD_BUILDFLAG:                 return new Cmd_BuildFlag                (des);
	case PLCMD_BUILDROAD:                 return new Cmd_BuildRoad                (des);
	case PLCMD_FLAGACTION:                return new Cmd_FlagAction               (des);
	case PLCMD_STARTSTOPBUILDING:         return new Cmd_StartStopBuilding        (des);
	case PLCMD_SHIP_EXPEDITION:           return new Cmd_StartOrCancelExpedition  (des);
	case PLCMD_SHIP_SCOUT:                return new Cmd_ShipScoutDirection       (des);
	case PLCMD_SHIP_EXPLORE:              return new Cmd_ShipExploreIsland        (des);
	case PLCMD_SHIP_CONSTRUCT:            return new Cmd_ShipConstructPort        (des);
	case PLCMD_SHIP_SINK:                 return new Cmd_ShipSink                 (des);
	case PLCMD_SHIP_CANCELEXPEDITION:     return new Cmd_ShipCancelExpedition     (des);
	case PLCMD_ENHANCEBUILDING:           return new Cmd_EnhanceBuilding          (des);
	case PLCMD_CHANGETRAININGOPTIONS:     return new Cmd_ChangeTrainingOptions    (des);
	case PLCMD_DROPSOLDIER:               return new Cmd_DropSoldier              (des);
	case PLCMD_CHANGESOLDIERCAPACITY:     return new Cmd_ChangeSoldierCapacity    (des);
	case PLCMD_ENEMYFLAGACTION:           return new Cmd_EnemyFlagAction          (des);
	case PLCMD_SETWAREPRIORITY:           return new Cmd_SetWarePriority          (des);
	case PLCMD_SETWARETARGETQUANTITY:     return new Cmd_SetWareTargetQuantity    (des);
	case PLCMD_RESETWARETARGETQUANTITY:   return new Cmd_ResetWareTargetQuantity  (des);
	case PLCMD_SETWORKERTARGETQUANTITY:   return new Cmd_SetWorkerTargetQuantity  (des);
	case PLCMD_RESETWORKERTARGETQUANTITY: return new Cmd_ResetWorkerTargetQuantity(des);
	case PLCMD_CHANGEMILITARYCONFIG:      return new Cmd_ChangeMilitaryConfig     (des);
	case PLCMD_MESSAGESETSTATUSREAD:      return new Cmd_MessageSetStatusRead     (des);
	case PLCMD_MESSAGESETSTATUSARCHIVED:  return new Cmd_MessageSetStatusArchived (des);
	case PLCMD_SETSTOCKPOLICY:            return new Cmd_SetStockPolicy           (des);
	case PLCMD_SETWAREMAXFILL:            return new Cmd_SetWareMaxFill           (des);
	case PLCMD_DISMANTLEBUILDING:         return new Cmd_DismantleBuilding        (des);
	case PLCMD_EVICTWORKER:               return new Cmd_EvictWorker              (des);
	case PLCMD_MILITARYSITESETSOLDIERPREFERENCE: return new Cmd_MilitarySiteSetSoldierPreference(des);
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
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_COMMAND_VERSION);

	GameLogicCommand::Write(fw, egbase, mos);
	// Now sender
	fw.Unsigned8  (sender   ());
	fw.Unsigned32 (cmdserial());
}

void PlayerCommand::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (2 <= packet_version and packet_version <= PLAYER_COMMAND_VERSION) {
			GameLogicCommand::Read(fr, egbase, mol);
			m_sender    = fr.Unsigned8 ();
			if (not egbase.get_player(m_sender))
				throw game_data_error("player %u does not exist", m_sender);
			m_cmdserial = fr.Unsigned32();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("player command: %s", e.what());
	}
}

/*** class Cmd_Bulldoze ***/

Cmd_Bulldoze::Cmd_Bulldoze (StreamRead & des) :
	PlayerCommand (0, des.Unsigned8()),
	serial        (des.Unsigned32()),
	recurse       (des.Unsigned8())
{}

void Cmd_Bulldoze::execute (Game & game)
{
	if (upcast(PlayerImmovable, pimm, game.objects().get_object(serial)))
		game.player(sender()).bulldoze(*pimm, recurse);
}

void Cmd_Bulldoze::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BULLDOZE);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned8 (recurse);
}
#define PLAYER_CMD_BULLDOZE_VERSION 2
void Cmd_Bulldoze::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if
			(1 <= packet_version and
			 packet_version <= PLAYER_CMD_BULLDOZE_VERSION)
		{
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PlayerImmovable>(fr.Unsigned32(), mol);
			recurse = 2 <= packet_version ? fr.Unsigned8() : false;
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("bulldoze: %s", e.what());
	}
}
void Cmd_Bulldoze::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
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

Cmd_Build::Cmd_Build (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	bi = Building_Index(static_cast<Building_Index::value_t>(des.Signed16()));
	coords = des.Coords32  ();
}

void Cmd_Build::execute (Game & game)
{
	// Empty former vector since its a new csite.
	Building::FormerBuildings former_buildings;
	game.player(sender()).build(coords, bi, true, former_buildings);
}

void Cmd_Build::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_BUILD);
	ser.Unsigned8 (sender());
	ser.Signed16  (bi.value());
	ser.Coords32  (coords);
}
#define PLAYER_CMD_BUILD_VERSION 1
void Cmd_Build::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BUILD_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			bi     =
				Building_Index
					(static_cast<Building_Index::value_t>(fr.Unsigned16()));
			coords = fr.Coords32  (egbase.map().extent());
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("build: %s", e.what());
	}
}

void Cmd_Build::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BUILD_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned16(bi.value());
	fw.Coords32  (coords);
}


/*** class Cmd_BuildFlag ***/

Cmd_BuildFlag::Cmd_BuildFlag (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	coords = des.Coords32  ();
}

void Cmd_BuildFlag::execute (Game & game)
{
	game.player(sender()).build_flag(coords);
}

void Cmd_BuildFlag::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BUILDFLAG);
	ser.Unsigned8 (sender());
	ser.Coords32  (coords);
}
#define PLAYER_CMD_BUILDFLAG_VERSION 1
void Cmd_BuildFlag::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BUILDFLAG_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			coords = fr.Coords32(egbase.map().extent());
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("build flag: %s", e.what());
	}
}
void Cmd_BuildFlag::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BUILDFLAG_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	fw.Coords32  (coords);
}

/*** class Cmd_BuildRoad ***/

Cmd_BuildRoad::Cmd_BuildRoad (int32_t t, int32_t p, Path & pa) :
PlayerCommand(t, p),
path         (&pa),
start        (pa.get_start()),
nsteps       (pa.get_nsteps()),
steps        (nullptr)
{}

Cmd_BuildRoad::Cmd_BuildRoad (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	start  = des.Coords32  ();
	nsteps = des.Unsigned16();

	// we cannot completely deserialize the path here because we don't have a Map
	path = nullptr;
	steps = new char[nsteps];

	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		steps[i] = des.Unsigned8();
}

Cmd_BuildRoad::~Cmd_BuildRoad ()
{
	delete path;

	delete[] steps;
}

void Cmd_BuildRoad::execute (Game & game)
{
	if (path == nullptr) {
		assert (steps);

		path = new Path(start);
		for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
			path->append (game.map(), steps[i]);
	}

	game.player(sender()).build_road(*path);
}

void Cmd_BuildRoad::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_BUILDROAD);
	ser.Unsigned8 (sender());
	ser.Coords32  (start);
	ser.Unsigned16(nsteps);

	assert (path || steps);

	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		ser.Unsigned8(path ? (*path)[i] : steps[i]);
}
#define PLAYER_CMD_BUILDROAD_VERSION 1
void Cmd_BuildRoad::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_BUILDROAD_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			start  = fr.Coords32  (egbase.map().extent());
			nsteps = fr.Unsigned16();
			path = nullptr;
			steps = new char[nsteps];
			for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
			steps[i] = fr.Unsigned8();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("build road: %s", e.what());
	}
}
void Cmd_BuildRoad::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_BUILDROAD_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);
	fw.Coords32  (start);
	fw.Unsigned16(nsteps);
	for (Path::Step_Vector::size_type i = 0; i < nsteps; ++i)
		fw.Unsigned8(path ? (*path)[i] : steps[i]);
}


/*** Cmd_FlagAction ***/
Cmd_FlagAction::Cmd_FlagAction (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	des         .Unsigned8 ();
	serial = des.Unsigned32();
}

void Cmd_FlagAction::execute (Game & game)
{
	Player & player = game.player(sender());
	if (upcast(Flag, flag, game.objects().get_object(serial)))
		if (&flag->owner() == &player)
			player.flagaction (*flag);
}

void Cmd_FlagAction::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_FLAGACTION);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (0);
	ser.Unsigned32(serial);
}

#define PLAYER_CMD_FLAGACTION_VERSION 1
void Cmd_FlagAction::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_FLAGACTION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			fr                             .Unsigned8 ();
			serial = get_object_serial_or_zero<Flag>(fr.Unsigned32(), mol);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("flag action: %s", e.what());
	}
}
void Cmd_FlagAction::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
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

Cmd_StartStopBuilding::Cmd_StartStopBuilding (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void Cmd_StartStopBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).start_stop_building(*building);
}

void Cmd_StartStopBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_STARTSTOPBUILDING);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_STOPBUILDING_VERSION 1
void Cmd_StartStopBuilding::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_STOPBUILDING_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("start/stop building: %s", e.what());
	}
}
void Cmd_StartStopBuilding::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_STOPBUILDING_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


Cmd_MilitarySiteSetSoldierPreference::Cmd_MilitarySiteSetSoldierPreference (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	preference = des.Unsigned8();
}

void Cmd_MilitarySiteSetSoldierPreference::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_MILITARYSITESETSOLDIERPREFERENCE);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned8 (preference);
}

void Cmd_MilitarySiteSetSoldierPreference::execute (Game & game)
{
	if (upcast(MilitarySite, building, game.objects().get_object(serial)))
		game.player(sender()).military_site_set_soldier_preference(*building, preference);

}

#define PLAYER_CMD_SOLDIERPREFERENCE_VERSION 1
void Cmd_MilitarySiteSetSoldierPreference::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SOLDIERPREFERENCE_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	fw.Unsigned8(preference);

	// Now serial.
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

void Cmd_MilitarySiteSetSoldierPreference::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try
	{
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SOLDIERPREFERENCE_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			preference = fr.Unsigned8();
			serial = get_object_serial_or_zero<MilitarySite>(fr.Unsigned32(), mol);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("start/stop building: %s", e.what());
	}
}


/*** Cmd_StartOrCancelExpedition ***/

Cmd_StartOrCancelExpedition::Cmd_StartOrCancelExpedition (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void Cmd_StartOrCancelExpedition::execute (Game & game)
{
	if (upcast(Warehouse, warehouse, game.objects().get_object(serial)))
		game.player(sender()).start_or_cancel_expedition(*warehouse);
}

void Cmd_StartOrCancelExpedition::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_EXPEDITION);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_EXPEDITION_VERSION 1
void Cmd_StartOrCancelExpedition::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint16_t const packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_EXPEDITION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Warehouse>(fr.Unsigned32(), mol);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("start/stop building: %s", e.what());
	}
}
void Cmd_StartOrCancelExpedition::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_EXPEDITION_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** Cmd_EnhanceBuilding ***/

Cmd_EnhanceBuilding::Cmd_EnhanceBuilding (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	bi = Building_Index(static_cast<Building_Index::value_t>(des.Unsigned16()));
}

void Cmd_EnhanceBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).enhance_building(building, bi);
}

void Cmd_EnhanceBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_ENHANCEBUILDING);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned16(bi.value());
}
#define PLAYER_CMD_ENHANCEBUILDING_VERSION 1
void Cmd_EnhanceBuilding::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_ENHANCEBUILDING_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
			bi =
				Building_Index
					(static_cast<Building_Index::value_t>(fr.Unsigned16()));
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("enhance building: %s", e.what());
	}
}
void Cmd_EnhanceBuilding::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_ENHANCEBUILDING_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Now id
	fw.Unsigned16(bi.value());
}


/*** Cmd_DismantleBuilding ***/
Cmd_DismantleBuilding::Cmd_DismantleBuilding (StreamRead & des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void Cmd_DismantleBuilding::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		game.player(sender()).dismantle_building(building);
}

void Cmd_DismantleBuilding::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_DISMANTLEBUILDING);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_DISMANTLEBUILDING_VERSION 1
void Cmd_DismantleBuilding::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_DISMANTLEBUILDING_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("dismantle building: %s", e.what());
	}
}
void Cmd_DismantleBuilding::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_DISMANTLEBUILDING_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}

/*** Cmd_EvictWorker ***/
Cmd_EvictWorker::Cmd_EvictWorker (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void Cmd_EvictWorker::execute (Game & game)
{
	upcast(Worker, worker, game.objects().get_object(serial));
	if (worker && worker->owner().player_number() == sender()) {
		worker->evict(game);
	}
}

void Cmd_EvictWorker::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_EVICTWORKER);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}
#define PLAYER_CMD_EVICTWORKER_VERSION 1
void Cmd_EvictWorker::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_EVICTWORKER_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Worker>(fr.Unsigned32(), mol);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("evict worker: %s", e.what());
	}
}
void Cmd_EvictWorker::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_EVICTWORKER_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** Cmd_ShipScoutDirection ***/
Cmd_ShipScoutDirection::Cmd_ShipScoutDirection (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	dir    = des.Unsigned8();
}

void Cmd_ShipScoutDirection::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->exp_scout_direction(game, dir);
	}
}

void Cmd_ShipScoutDirection::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_SCOUT);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned8 (dir);
}

#define PLAYER_CMD_SHIP_SCOUT_DIRECTION_VERSION 1
void Cmd_ShipScoutDirection::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_SCOUT_DIRECTION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
			// direction
			dir = fr.Unsigned8();
		} else
			throw game_data_error("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("Ship scout: %s", e.what());
	}
}
void Cmd_ShipScoutDirection::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
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
Cmd_ShipConstructPort::Cmd_ShipConstructPort (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	coords = des.Coords32();
}

void Cmd_ShipConstructPort::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->exp_construct_port(game, coords);
	}
}

void Cmd_ShipConstructPort::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_CONSTRUCT);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Coords32  (coords);
}

#define PLAYER_CMD_SHIP_CONSTRUCT_PORT_VERSION 1
void Cmd_ShipConstructPort::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_CONSTRUCT_PORT_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
			// Coords
			coords = fr.Coords32();
		} else
			throw game_data_error("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("Ship construct port: %s", e.what());
	}
}
void Cmd_ShipConstructPort::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SHIP_CONSTRUCT_PORT_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));

	// Coords
	fw.Coords32(coords);
}


/*** Cmd_ShipExploreIsland ***/
Cmd_ShipExploreIsland::Cmd_ShipExploreIsland (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	clockwise = des.Unsigned8() == 1;
}

void Cmd_ShipExploreIsland::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->exp_explore_island(game, clockwise);
	}
}

void Cmd_ShipExploreIsland::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_EXPLORE);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned8 (clockwise ? 1 : 0);
}

#define PLAYER_CMD_SHIP_EXPLORE_ISLAND_VERSION 1
void Cmd_ShipExploreIsland::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_EXPLORE_ISLAND_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
			clockwise = fr.Unsigned8() == 1;
		} else
			throw game_data_error("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("Ship explore: %s", e.what());
	}
}
void Cmd_ShipExploreIsland::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
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
Cmd_ShipSink::Cmd_ShipSink (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void Cmd_ShipSink::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->sink_ship(game);
	}
}

void Cmd_ShipSink::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_SINK);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}

#define PLAYER_CMD_SHIP_SINK_VERSION 1
void Cmd_ShipSink::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_SINK_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
		} else
			throw game_data_error("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("Ship explore: %s", e.what());
	}
}
void Cmd_ShipSink::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SHIP_SINK_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** Cmd_ShipCancelExpedition ***/
Cmd_ShipCancelExpedition::Cmd_ShipCancelExpedition (StreamRead& des) :
	PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
}

void Cmd_ShipCancelExpedition::execute (Game & game)
{
	upcast(Ship, ship, game.objects().get_object(serial));
	if (ship && ship->get_owner()->player_number() == sender()) {
		ship->exp_cancel(game);
	}
}

void Cmd_ShipCancelExpedition::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SHIP_CANCELEXPEDITION);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
}

#define PLAYER_CMD_SHIP_CANCELEXPEDITION_VERSION 1
void Cmd_ShipCancelExpedition::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SHIP_CANCELEXPEDITION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Ship>(fr.Unsigned32(), mol);
		} else
			throw game_data_error("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("Ship explore: %s", e.what());
	}
}
void Cmd_ShipCancelExpedition::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_SHIP_CANCELEXPEDITION_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now serial
	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(serial)));
}


/*** class Cmd_SetWarePriority ***/
Cmd_SetWarePriority::Cmd_SetWarePriority
	(const int32_t _duetime, const Player_Number _sender,
	 PlayerImmovable & imm,
	 const int32_t type, const Ware_Index index, const int32_t priority)
	:
	PlayerCommand(_duetime, _sender),
	m_serial     (imm.serial()),
	m_type       (type),
	m_index      (index),
	m_priority   (priority)
{}

void Cmd_SetWarePriority::execute(Game & game)
{
	upcast(Building, psite, game.objects().get_object(m_serial));

	if (!psite)
		return;
	if (psite->owner().player_number() != sender())
		return;

	psite->set_priority(m_type, m_index, m_priority);
}

#define PLAYER_CMD_SETWAREPRIORITY_VERSION 1

void Cmd_SetWarePriority::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWAREPRIORITY_VERSION);

	PlayerCommand::Write(fw, egbase, mos);

	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(m_serial)));
	fw.Unsigned8(m_type);
	fw.Signed32(m_index.value());
	fw.Signed32(m_priority);
}

void Cmd_SetWarePriority::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SETWAREPRIORITY_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			m_serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
			m_type = fr.Unsigned8();
			m_index = Ware_Index(static_cast<Ware_Index::value_t>(fr.Signed32()));
			m_priority = fr.Signed32();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("set ware priority: %s", e.what());
	}
}

Cmd_SetWarePriority::Cmd_SetWarePriority(StreamRead & des) :
	PlayerCommand(0, des.Unsigned8()),
	m_serial     (des.Unsigned32()),
	m_type       (des.Unsigned8()),
	m_index      (Ware_Index(static_cast<Ware_Index::value_t>(des.Signed32()))),
	m_priority   (des.Signed32())
{}

void Cmd_SetWarePriority::serialize(StreamWrite & ser)
{
	ser.Unsigned8(PLCMD_SETWAREPRIORITY);
	ser.Unsigned8(sender());
	ser.Unsigned32(m_serial);
	ser.Unsigned8(m_type);
	ser.Signed32(m_index.value());
	ser.Signed32(m_priority);
}

/*** class Cmd_SetWareMaxFill ***/
Cmd_SetWareMaxFill::Cmd_SetWareMaxFill
	(const int32_t _duetime, const Player_Number _sender,
	 PlayerImmovable & imm,
	 const Ware_Index index, const uint32_t max_fill)
	:
	PlayerCommand(_duetime, _sender),
	m_serial     (imm.serial()),
	m_index      (index),
	m_max_fill   (max_fill)
{}

void Cmd_SetWareMaxFill::execute(Game & game)
{
	upcast(Building, b, game.objects().get_object(m_serial));

	if (!b)
		return;
	if (b->owner().player_number() != sender())
		return;

	b->waresqueue(m_index).set_max_fill(m_max_fill);
}

#define PLAYER_CMD_SETWAREMAXFILL_SIZE_VERSION 1

void Cmd_SetWareMaxFill::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWAREMAXFILL_SIZE_VERSION);

	PlayerCommand::Write(fw, egbase, mos);

	fw.Unsigned32(mos.get_object_file_index_or_zero(egbase.objects().get_object(m_serial)));
	fw.Signed32(m_index.value());
	fw.Unsigned32(m_max_fill);
}

void Cmd_SetWareMaxFill::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_SETWAREMAXFILL_SIZE_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			m_serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
			m_index = Ware_Index(static_cast<Ware_Index::value_t>(fr.Signed32()));
			m_max_fill = fr.Unsigned32();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("set ware max fill: %s", e.what());
	}
}

Cmd_SetWareMaxFill::Cmd_SetWareMaxFill(StreamRead & des) :
	PlayerCommand(0, des.Unsigned8()),
	m_serial     (des.Unsigned32()),
	m_index      (Ware_Index(static_cast<Ware_Index::value_t>(des.Signed32()))),
	m_max_fill(des.Unsigned32())
{}

void Cmd_SetWareMaxFill::serialize(StreamWrite & ser)
{
	ser.Unsigned8(PLCMD_SETWAREMAXFILL);
	ser.Unsigned8(sender());
	ser.Unsigned32(m_serial);
	ser.Signed32(m_index.value());
	ser.Unsigned32(m_max_fill);
}


Cmd_ChangeTargetQuantity::Cmd_ChangeTargetQuantity
	(const int32_t _duetime, const Player_Number _sender,
	 const uint32_t _economy, const Ware_Index _ware_type)
	:
	PlayerCommand(_duetime, _sender),
	m_economy (_economy), m_ware_type(_ware_type)
{}

void Cmd_ChangeTargetQuantity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned32(economy());
	fw.CString
		(egbase.player(sender()).tribe().get_ware_descr(ware_type())->name());
}

void Cmd_ChangeTargetQuantity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		PlayerCommand::Read(fr, egbase, mol);
		m_economy   = fr.Unsigned32();
		m_ware_type =
			egbase.player(sender()).tribe().ware_index(fr.CString());
	} catch (const _wexception & e) {
		throw game_data_error("change target quantity: %s", e.what());
	}
}

Cmd_ChangeTargetQuantity::Cmd_ChangeTargetQuantity(StreamRead & des)
	:
	PlayerCommand(0, des.Unsigned8()),
	m_economy    (des.Unsigned32()),
	m_ware_type  (des.Unsigned8())
{}

void Cmd_ChangeTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (sender());
	ser.Unsigned32(economy());
	ser.Unsigned8 (ware_type().value());
}


Cmd_SetWareTargetQuantity::Cmd_SetWareTargetQuantity
	(const int32_t _duetime, const Player_Number _sender,
	 const uint32_t _economy,
	 const Ware_Index _ware_type,
	 const uint32_t _permanent)
	:
	Cmd_ChangeTargetQuantity(_duetime, _sender, _economy, _ware_type),
	m_permanent(_permanent)
{}

void Cmd_SetWareTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	if
		(economy  () < player.get_nr_economies() and
		 ware_type() < player.tribe().get_nrwares())
		player.get_economy_by_number(economy())->set_ware_target_quantity
			(ware_type(),  m_permanent, duetime());
}

#define PLAYER_CMD_SETWARETARGETQUANTITY_VERSION 2

void Cmd_SetWareTargetQuantity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWARETARGETQUANTITY_VERSION);
	Cmd_ChangeTargetQuantity::Write(fw, egbase, mos);
	fw.Unsigned32(m_permanent);
}

void Cmd_SetWareTargetQuantity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version <= PLAYER_CMD_SETWARETARGETQUANTITY_VERSION) {
			Cmd_ChangeTargetQuantity::Read(fr, egbase, mol);
			m_permanent = fr.Unsigned32();
			if (packet_version == 1)
				fr.Unsigned32();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("set ware target quantity: %s", e.what());
	}
}

Cmd_SetWareTargetQuantity::Cmd_SetWareTargetQuantity(StreamRead & des)
	:
	Cmd_ChangeTargetQuantity(des),
	m_permanent             (des.Unsigned32())
{
	if (cmdserial() == 1) des.Unsigned32();
}

void Cmd_SetWareTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SETWARETARGETQUANTITY);
	Cmd_ChangeTargetQuantity::serialize(ser);
	ser.Unsigned32(m_permanent);
}


Cmd_ResetWareTargetQuantity::Cmd_ResetWareTargetQuantity
	(const int32_t _duetime, const Player_Number _sender,
	 const uint32_t _economy,
	 const Ware_Index _ware_type)
	:
	Cmd_ChangeTargetQuantity(_duetime, _sender, _economy, _ware_type)
{}

void Cmd_ResetWareTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	const Tribe_Descr & tribe = player.tribe();
	if
		(economy  () < player.get_nr_economies() and
		 ware_type() < tribe.get_nrwares())
	{
		const int32_t count =
			tribe.get_ware_descr(ware_type())->default_target_quantity();
		player.get_economy_by_number(economy())->set_ware_target_quantity
			(ware_type(),  count, 0);
	}
}

#define PLAYER_CMD_RESETWARETARGETQUANTITY_VERSION 1

void Cmd_ResetWareTargetQuantity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_CMD_RESETWARETARGETQUANTITY_VERSION);
	Cmd_ChangeTargetQuantity::Write(fw, egbase, mos);
}

void Cmd_ResetWareTargetQuantity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_RESETWARETARGETQUANTITY_VERSION)
			Cmd_ChangeTargetQuantity::Read(fr, egbase, mol);
		else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("reset target quantity: %s", e.what());
	}
}

Cmd_ResetWareTargetQuantity::Cmd_ResetWareTargetQuantity(StreamRead & des)
	: Cmd_ChangeTargetQuantity(des)
{}

void Cmd_ResetWareTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_RESETWARETARGETQUANTITY);
	Cmd_ChangeTargetQuantity::serialize(ser);
}


Cmd_SetWorkerTargetQuantity::Cmd_SetWorkerTargetQuantity
	(const int32_t _duetime, const Player_Number _sender,
	 const uint32_t _economy,
	 const Ware_Index _ware_type,
	 const uint32_t _permanent)
	:
	Cmd_ChangeTargetQuantity(_duetime, _sender, _economy, _ware_type),
	m_permanent(_permanent)
{}

void Cmd_SetWorkerTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	if
		(economy  () < player.get_nr_economies() and
		 ware_type() < player.tribe().get_nrwares())
		player.get_economy_by_number(economy())->set_worker_target_quantity
			(ware_type(),  m_permanent, duetime());
}

#define PLAYER_CMD_SETWORKERTARGETQUANTITY_VERSION 2

void Cmd_SetWorkerTargetQuantity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_CMD_SETWORKERTARGETQUANTITY_VERSION);
	Cmd_ChangeTargetQuantity::Write(fw, egbase, mos);
	fw.Unsigned32(m_permanent);
}

void Cmd_SetWorkerTargetQuantity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version <= PLAYER_CMD_SETWORKERTARGETQUANTITY_VERSION) {
			Cmd_ChangeTargetQuantity::Read(fr, egbase, mol);
			m_permanent = fr.Unsigned32();
			if (packet_version == 1)
				fr.Unsigned32();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("set worker target quantity: %s", e.what());
	}
}

Cmd_SetWorkerTargetQuantity::Cmd_SetWorkerTargetQuantity(StreamRead & des)
	:
	Cmd_ChangeTargetQuantity(des),
	m_permanent             (des.Unsigned32())
{
	if (cmdserial() == 1) des.Unsigned32();
}

void Cmd_SetWorkerTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_SETWORKERTARGETQUANTITY);
	Cmd_ChangeTargetQuantity::serialize(ser);
	ser.Unsigned32(m_permanent);
}


Cmd_ResetWorkerTargetQuantity::Cmd_ResetWorkerTargetQuantity
	(const int32_t _duetime, const Player_Number _sender,
	 const uint32_t _economy,
	 const Ware_Index _ware_type)
	:
	Cmd_ChangeTargetQuantity(_duetime, _sender, _economy, _ware_type)
{}

void Cmd_ResetWorkerTargetQuantity::execute(Game & game)
{
	Player & player = game.player(sender());
	const Tribe_Descr & tribe = player.tribe();
	if
		(economy  () < player.get_nr_economies() and
		 ware_type() < tribe.get_nrwares())
	{
		const int32_t count =
			tribe.get_ware_descr(ware_type())->default_target_quantity();
		player.get_economy_by_number(economy())->set_worker_target_quantity
			(ware_type(),  count, 0);
	}
}

#define PLAYER_CMD_RESETWORKERTARGETQUANTITY_VERSION 1

void Cmd_ResetWorkerTargetQuantity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_CMD_RESETWORKERTARGETQUANTITY_VERSION);
	Cmd_ChangeTargetQuantity::Write(fw, egbase, mos);
}

void Cmd_ResetWorkerTargetQuantity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_RESETWORKERTARGETQUANTITY_VERSION) {
			Cmd_ChangeTargetQuantity::Read(fr, egbase, mol);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("reset worker target quantity: %s", e.what());
	}
}

Cmd_ResetWorkerTargetQuantity::Cmd_ResetWorkerTargetQuantity(StreamRead & des)
	: Cmd_ChangeTargetQuantity(des)
{}

void Cmd_ResetWorkerTargetQuantity::serialize(StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_RESETWORKERTARGETQUANTITY);
	Cmd_ChangeTargetQuantity::serialize(ser);
}


/*** class Cmd_ChangeTrainingOptions ***/
Cmd_ChangeTrainingOptions::Cmd_ChangeTrainingOptions(StreamRead & des)
:
PlayerCommand (0, des.Unsigned8())
{
	serial    = des.Unsigned32();  //  Serial of the building
	attribute = des.Unsigned16();  //  Attribute to modify
	value     = des.Unsigned16();  //  New vale
}

void Cmd_ChangeTrainingOptions::execute (Game & game)
{
	if (upcast(TrainingSite, trainingsite, game.objects().get_object(serial)))
		game.player(sender()).change_training_options
			(*trainingsite, attribute, value);
}

void Cmd_ChangeTrainingOptions::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_CHANGETRAININGOPTIONS);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned16(attribute);
	ser.Unsigned16(value);
}


#define PLAYER_CMD_CHANGETRAININGOPTIONS_VERSION 1
void Cmd_ChangeTrainingOptions::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_CHANGETRAININGOPTIONS_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<TrainingSite>(fr.Unsigned32(), mol);
			attribute = fr.Unsigned16();
			value     = fr.Unsigned16();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("change training options: %s", e.what());
	}
}

void Cmd_ChangeTrainingOptions::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
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

Cmd_DropSoldier::Cmd_DropSoldier(StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	serial  = des.Unsigned32(); //  Serial of the building
	soldier = des.Unsigned32(); //  Serial of soldier
}

void Cmd_DropSoldier::execute (Game & game)
{
	if (upcast(PlayerImmovable, player_imm, game.objects().get_object(serial)))
		if (upcast(Soldier, s, game.objects().get_object(soldier)))
			game.player(sender()).drop_soldier(*player_imm, *s);
}

void Cmd_DropSoldier::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_DROPSOLDIER);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Unsigned32(soldier);
}

#define PLAYER_CMD_DROPSOLDIER_VERSION 1
void Cmd_DropSoldier::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_DROPSOLDIER_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<PlayerImmovable>(fr.Unsigned32(), mol);
			soldier = get_object_serial_or_zero<Soldier>(fr.Unsigned32(), mol);
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("drop soldier: %s", e.what());
	}
}

void Cmd_DropSoldier::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
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

Cmd_ChangeSoldierCapacity::Cmd_ChangeSoldierCapacity(StreamRead & des)
:
PlayerCommand (0, des.Unsigned8())
{
	serial = des.Unsigned32();
	val    = des.Signed16();
}

void Cmd_ChangeSoldierCapacity::execute (Game & game)
{
	if (upcast(Building, building, game.objects().get_object(serial)))
		if (&building->owner() == game.get_player(sender()))
			if (upcast(SoldierControl, ctrl, building))
				ctrl->changeSoldierCapacity(val);
}

void Cmd_ChangeSoldierCapacity::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_CHANGESOLDIERCAPACITY);
	ser.Unsigned8 (sender());
	ser.Unsigned32(serial);
	ser.Signed16(val);
}

#define PLAYER_CMD_CHANGESOLDIERCAPACITY_VERSION 1
void Cmd_ChangeSoldierCapacity::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_CHANGESOLDIERCAPACITY_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			serial = get_object_serial_or_zero<Building>(fr.Unsigned32(), mol);
			val = fr.Signed16();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("change soldier capacity: %s", e.what());
	}
}

void Cmd_ChangeSoldierCapacity::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
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

Cmd_EnemyFlagAction::Cmd_EnemyFlagAction (StreamRead & des) :
PlayerCommand (0, des.Unsigned8())
{
	des         .Unsigned8 ();
	serial   = des.Unsigned32();
	des         .Unsigned8 ();
	number   = des.Unsigned8 ();
	retreat  = des.Unsigned8 ();
}

void Cmd_EnemyFlagAction::execute (Game & game)
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
				 and
				 1
				 <
				 player.vision
				 	(Map::get_index
				 	 	(building->get_position(), game.map().get_width())))
				player.enemyflagaction (*flag, sender(), number, retreat);
			else
				log
					("Cmd_EnemyFlagAction::execute: ERROR: wrong player target not "
					 "seen or not hostile.\n");
		}
	}
}

void Cmd_EnemyFlagAction::serialize (StreamWrite & ser) {
	ser.Unsigned8 (PLCMD_ENEMYFLAGACTION);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (1);
	ser.Unsigned32(serial);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (number);
	ser.Unsigned8 (retreat);
}
/// Version 2 and 3 are fully compatible: version 2 soldiers will never retreat
/// but do not crash game.
#define PLAYER_CMD_ENEMYFLAGACTION_VERSION 3
void Cmd_EnemyFlagAction::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_ENEMYFLAGACTION_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			fr           .Unsigned8 ();
			serial = get_object_serial_or_zero<Flag>(fr.Unsigned32(), mol);
			fr           .Unsigned8 ();
			number   = fr.Unsigned8 ();
			retreat  = fr.Unsigned8 ();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("enemy flag action: %s", e.what());
	}
}

void Cmd_EnemyFlagAction::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
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
	fw.Unsigned8 (retreat);
}

/*** Cmd_ChangeMilitaryConfig ***/

Cmd_ChangeMilitaryConfig::Cmd_ChangeMilitaryConfig(StreamRead & des)
:
PlayerCommand (0, des.Unsigned8())
{
	retreat = des.Unsigned8();
	/// Read reserved data
	des.Unsigned8();
	des.Unsigned8();
}

void Cmd_ChangeMilitaryConfig::execute (Game & game)
{
	game.get_player(sender())->set_retreat_percentage(retreat);
}

void Cmd_ChangeMilitaryConfig::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_CHANGEMILITARYCONFIG);
	ser.Unsigned8 (sender());
	ser.Unsigned8 (retreat);
	/// Serialize reserved data
	ser.Unsigned8 (0);
	ser.Unsigned8 (0);
}

#define PLAYER_CMD_CHANGEMILITARYCONFIG_VERSION 1
void Cmd_ChangeMilitaryConfig::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_CMD_CHANGEMILITARYCONFIG_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			Player * plr = egbase.get_player(sender());
			assert(plr);
			retreat = fr.Unsigned8();
			if
				(retreat < plr->tribe().get_military_data().get_min_retreat()
				 or
				 retreat > plr->tribe().get_military_data().get_max_retreat())
				throw game_data_error
					("retreat: value out of range. Received %u, but expected %u-%u",
					 retreat,
					 plr->tribe().get_military_data().get_min_retreat(),
					 plr->tribe().get_military_data().get_max_retreat());
			/// Read reserved data
			fr.Unsigned8();
			fr.Unsigned8();
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("change military config: %s", e.what());
	}
}

void Cmd_ChangeMilitaryConfig::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	// First, write version
	fw.Unsigned16(PLAYER_CMD_CHANGEMILITARYCONFIG_VERSION);
	// Write base classes
	PlayerCommand::Write(fw, egbase, mos);

	// Now retreat
	fw.Unsigned8(retreat);

	// Reserved for future versions
	fw.Unsigned8(0);
	fw.Unsigned8(0);

}


/*** struct PlayerMessageCommand ***/

PlayerMessageCommand::PlayerMessageCommand(StreamRead & des) :
PlayerCommand (0, des.Unsigned8()), m_message_id(des.Unsigned32())
{}

#define PLAYER_MESSAGE_CMD_VERSION 1
void PlayerMessageCommand::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		const uint16_t packet_version = fr.Unsigned16();
		if (packet_version == PLAYER_MESSAGE_CMD_VERSION) {
			PlayerCommand::Read(fr, egbase, mol);
			m_message_id = Message_Id(fr.Unsigned32());
			if (not m_message_id)
				throw game_data_error
					("(player %u): message id is null", sender());
		} else
			throw game_data_error
				("unknown/unhandled version %u", packet_version);
	} catch (const _wexception & e) {
		throw game_data_error("player message: %s", e.what());
	}
}

void PlayerMessageCommand::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned16(PLAYER_MESSAGE_CMD_VERSION);
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned32(mos.message_savers[sender() - 1][message_id()].value());
}


/*** struct Cmd_MessageSetStatusRead ***/

void Cmd_MessageSetStatusRead::execute (Game & game)
{
	game.player(sender()).messages().set_message_status
		(message_id(), Message::Read);
}

void Cmd_MessageSetStatusRead::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_MESSAGESETSTATUSREAD);
	ser.Unsigned8 (sender());
	ser.Unsigned32(message_id().value());
}


/*** struct Cmd_MessageSetStatusArchived ***/

void Cmd_MessageSetStatusArchived::execute (Game & game)
{
	game.player(sender()).messages().set_message_status
		(message_id(), Message::Archived);
}

void Cmd_MessageSetStatusArchived::serialize (StreamWrite & ser)
{
	ser.Unsigned8 (PLCMD_MESSAGESETSTATUSARCHIVED);
	ser.Unsigned8 (sender());
	ser.Unsigned32(message_id().value());
}

/*** struct Cmd_SetStockPolicy ***/
Cmd_SetStockPolicy::Cmd_SetStockPolicy
	(int32_t time, Player_Number p,
	 Warehouse & wh, bool isworker, Ware_Index ware,
	 Warehouse::StockPolicy policy)
: PlayerCommand(time, p)
{
	m_warehouse = wh.serial();
	m_isworker = isworker;
	m_ware = ware;
	m_policy = policy;
}

Cmd_SetStockPolicy::Cmd_SetStockPolicy()
: PlayerCommand(), m_warehouse(0), m_isworker(false), m_policy()
{
}

uint8_t Cmd_SetStockPolicy::id() const
{
	return QUEUE_CMD_SETSTOCKPOLICY;
}

void Cmd_SetStockPolicy::execute(Game & game)
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

			const Tribe_Descr & tribe = warehouse->tribe();
			if (m_isworker) {
				if (!(m_ware < tribe.get_nrworkers())) {
					log
						("Cmd_SetStockPolicy: sender %u, worker %u out of bounds\n",
						 sender(), m_ware.value());
					return;
				}
				warehouse->set_worker_policy(m_ware, m_policy);
			} else {
				if (!(m_ware < tribe.get_nrwares())) {
					log
						("Cmd_SetStockPolicy: sender %u, ware %u out of bounds\n",
						 sender(), m_ware.value());
					return;
				}
				warehouse->set_ware_policy(m_ware, m_policy);
			}
		}
	}
}

Cmd_SetStockPolicy::Cmd_SetStockPolicy(StreamRead & des)
	: PlayerCommand(0, des.Unsigned8())
{
	m_warehouse = des.Unsigned32();
	m_isworker = des.Unsigned8();
	m_ware = Ware_Index(des.Unsigned8());
	m_policy = static_cast<Warehouse::StockPolicy>(des.Unsigned8());
}

void Cmd_SetStockPolicy::serialize(StreamWrite & ser)
{
	ser.Unsigned8(PLCMD_SETSTOCKPOLICY);
	ser.Unsigned8(sender());
	ser.Unsigned32(m_warehouse);
	ser.Unsigned8(m_isworker);
	ser.Unsigned8(m_ware.value());
	ser.Unsigned8(m_policy);
}

#define PLAYER_CMD_SETSTOCKPOLICY_VERSION 1
void Cmd_SetStockPolicy::Read
	(FileRead & fr, Editor_Game_Base & egbase, Map_Map_Object_Loader & mol)
{
	try {
		uint8_t version = fr.Unsigned8();
		if (version != PLAYER_CMD_SETSTOCKPOLICY_VERSION)
			throw game_data_error("unknown/unhandled version %u", version);
		PlayerCommand::Read(fr, egbase, mol);
		m_warehouse = fr.Unsigned32();
		m_isworker = fr.Unsigned8();
		m_ware = Ware_Index(fr.Unsigned8());
		m_policy = static_cast<Warehouse::StockPolicy>(fr.Unsigned8());
	} catch (const std::exception & e) {
		throw game_data_error("Cmd_SetStockPolicy: %s", e.what());
	}
}

void Cmd_SetStockPolicy::Write
	(FileWrite & fw, Editor_Game_Base & egbase, Map_Map_Object_Saver & mos)
{
	fw.Unsigned8(PLAYER_CMD_SETSTOCKPOLICY_VERSION);
	PlayerCommand::Write(fw, egbase, mos);
	fw.Unsigned32(m_warehouse);
	fw.Unsigned8(m_isworker);
	fw.Unsigned8(m_ware.value());
	fw.Unsigned8(m_policy);
}

}
