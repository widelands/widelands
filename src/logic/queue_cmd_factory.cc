/*
 * Copyright (C) 2002-2004, 2008-2010 by the Widelands Development Team
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

#include "logic/queue_cmd_factory.h"

#include "base/wexception.h"
#include "economy/cmd_call_economy_balance.h"
#include "logic/cmd_calculate_statistics.h"
#include "logic/cmd_incorporate.h"
#include "logic/cmd_luacoroutine.h"
#include "logic/cmd_luascript.h"
#include "logic/map_objects/instances.h"
#include "logic/playercommand.h"
#include "logic/queue_cmd_ids.h"

namespace Widelands {

GameLogicCommand & QueueCmdFactory::create_correct_queue_command
	(uint32_t const id)
{
	switch (id) {
	case QUEUE_CMD_BUILD:
		return *new CmdBuild                     ();
	case QUEUE_CMD_FLAG:
		return *new CmdBuildFlag                 ();
	case QUEUE_CMD_BUILDROAD:
		return *new CmdBuildRoad                 ();
	case QUEUE_CMD_FLAGACTION:
		return *new CmdFlagAction                ();
	case QUEUE_CMD_STOPBUILDING:
		return *new CmdStartStopBuilding         ();
	case QUEUE_CMD_PORT_START_EXPEDITION:
		return *new CmdStartOrCancelExpedition   ();
	case QUEUE_CMD_SHIP_CONSTRUCT_PORT:
		return *new CmdShipConstructPort         ();
	case QUEUE_CMD_SHIP_SCOUT:
		return *new CmdShipScoutDirection        ();
	case QUEUE_CMD_SHIP_EXPLORE:
		return *new CmdShipExploreIsland         ();
	case QUEUE_CMD_SHIP_SINK:
		return *new CmdShipSink                  ();
	case QUEUE_CMD_SHIP_CANCELEXPEDITION:
		return *new CmdShipCancelExpedition      ();
	case QUEUE_CMD_ENHANCEBUILDING:
		return *new CmdEnhanceBuilding           ();
	case QUEUE_CMD_BULLDOZE:
		return *new CmdBulldoze                  ();
	case QUEUE_CMD_DROPSOLDIER:
		return *new CmdDropSoldier               ();
	case QUEUE_CMD_CHANGESOLDIERCAPACITY:
		return *new CmdChangeSoldierCapacity     ();
	case QUEUE_CMD_ENEMYFLAGACTION:
		return *new CmdEnemyFlagAction           ();
	case QUEUE_CMD_SETWAREPRIORITY:
		return *new CmdSetWarePriority           ();
	case QUEUE_CMD_SETWARETARGETQUANTITY:
		return *new CmdSetWareTargetQuantity     ();
	case QUEUE_CMD_RESETWARETARGETQUANTITY:
		return *new CmdResetWareTargetQuantity   ();
	case QUEUE_CMD_SETWORKERTARGETQUANTITY:
		return *new CmdSetWorkerTargetQuantity   ();
	case QUEUE_CMD_RESETWORKERTARGETQUANTITY:
		return *new CmdResetWorkerTargetQuantity ();
	case QUEUE_CMD_MESSAGESETSTATUSREAD:
		return *new CmdMessageSetStatusRead      ();
	case QUEUE_CMD_MESSAGESETSTATUSARCHIVED:
		return *new CmdMessageSetStatusArchived  ();
	case QUEUE_CMD_DESTROY_MAPOBJECT:
		return *new CmdDestroyMapObject        ();
	case QUEUE_CMD_ACT:
		return *new CmdAct                       ();
	case QUEUE_CMD_INCORPORATE:
		return *new CmdIncorporate               ();
	case QUEUE_CMD_LUASCRIPT:
		return *new CmdLuaScript                 ();
	case QUEUE_CMD_LUACOROUTINE:
		return *new CmdLuaCoroutine              ();
	case QUEUE_CMD_CALCULATE_STATISTICS :
		return *new CmdCalculateStatistics       ();
	case QUEUE_CMD_CALL_ECONOMY_BALANCE:
		return *new CmdCallEconomyBalance      ();
	case QUEUE_CMD_SETWAREMAXFILL:
		return *new CmdSetWareMaxFill            ();
	case QUEUE_CMD_DISMANTLEBUILDING:
		return *new CmdDismantleBuilding         ();
	case QUEUE_CMD_EVICTWORKER:
		return *new CmdEvictWorker();
	case QUEUE_CMD_MILITARYSITESETSOLDIERPREFERENCE:
		return *new CmdMilitarySiteSetSoldierPreference();
	default:
		throw wexception("Unknown Queue_Cmd_Id in file: %u", id);
	}
}

}
