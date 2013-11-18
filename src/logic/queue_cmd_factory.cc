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

#include "economy/cmd_call_economy_balance.h"
#include "logic/cmd_calculate_statistics.h"
#include "logic/cmd_incorporate.h"
#include "logic/cmd_luacoroutine.h"
#include "logic/cmd_luascript.h"
#include "logic/instances.h"
#include "logic/playercommand.h"
#include "logic/queue_cmd_ids.h"
#include "wexception.h"

namespace Widelands {

GameLogicCommand & Queue_Cmd_Factory::create_correct_queue_command
	(uint32_t const id)
{
	switch (id) {
	case QUEUE_CMD_BUILD:
		return *new Cmd_Build                     ();
	case QUEUE_CMD_FLAG:
		return *new Cmd_BuildFlag                 ();
	case QUEUE_CMD_BUILDROAD:
		return *new Cmd_BuildRoad                 ();
	case QUEUE_CMD_FLAGACTION:
		return *new Cmd_FlagAction                ();
	case QUEUE_CMD_STOPBUILDING:
		return *new Cmd_StartStopBuilding         ();
	case QUEUE_CMD_PORT_START_EXPEDITION:
		return *new Cmd_StartOrCancelExpedition   ();
	case QUEUE_CMD_SHIP_CONSTRUCT_PORT:
		return *new Cmd_ShipConstructPort         ();
	case QUEUE_CMD_SHIP_SCOUT:
		return *new Cmd_ShipScoutDirection        ();
	case QUEUE_CMD_SHIP_EXPLORE:
		return *new Cmd_ShipExploreIsland         ();
	case QUEUE_CMD_SHIP_SINK:
		return *new Cmd_ShipSink                  ();
	case QUEUE_CMD_SHIP_CANCELEXPEDITION:
		return *new Cmd_ShipCancelExpedition      ();
	case QUEUE_CMD_ENHANCEBUILDING:
		return *new Cmd_EnhanceBuilding           ();
	case QUEUE_CMD_BULLDOZE:
		return *new Cmd_Bulldoze                  ();
	case QUEUE_CMD_DROPSOLDIER:
		return *new Cmd_DropSoldier               ();
	case QUEUE_CMD_CHANGESOLDIERCAPACITY:
		return *new Cmd_ChangeSoldierCapacity     ();
	case QUEUE_CMD_ENEMYFLAGACTION:
		return *new Cmd_EnemyFlagAction           ();
	case QUEUE_CMD_SETWAREPRIORITY:
		return *new Cmd_SetWarePriority           ();
	case QUEUE_CMD_SETWARETARGETQUANTITY:
		return *new Cmd_SetWareTargetQuantity     ();
	case QUEUE_CMD_RESETWARETARGETQUANTITY:
		return *new Cmd_ResetWareTargetQuantity   ();
	case QUEUE_CMD_SETWORKERTARGETQUANTITY:
		return *new Cmd_SetWorkerTargetQuantity   ();
	case QUEUE_CMD_RESETWORKERTARGETQUANTITY:
		return *new Cmd_ResetWorkerTargetQuantity ();
	case QUEUE_CMD_CHANGEMILITARYCONFIG:
		return *new Cmd_ChangeMilitaryConfig      ();
	case QUEUE_CMD_MESSAGESETSTATUSREAD:
		return *new Cmd_MessageSetStatusRead      ();
	case QUEUE_CMD_MESSAGESETSTATUSARCHIVED:
		return *new Cmd_MessageSetStatusArchived  ();
	case QUEUE_CMD_DESTROY_MAPOBJECT:
		return *new Cmd_Destroy_Map_Object        ();
	case QUEUE_CMD_ACT:
		return *new Cmd_Act                       ();
	case QUEUE_CMD_INCORPORATE:
		return *new Cmd_Incorporate               ();
	case QUEUE_CMD_LUASCRIPT:
		return *new Cmd_LuaScript                 ();
	case QUEUE_CMD_LUACOROUTINE:
		return *new Cmd_LuaCoroutine              ();
	case QUEUE_CMD_CALCULATE_STATISTICS :
		return *new Cmd_CalculateStatistics       ();
	case QUEUE_CMD_CALL_ECONOMY_BALANCE:
		return *new Cmd_Call_Economy_Balance      ();
	case QUEUE_CMD_SETWAREMAXFILL:
		return *new Cmd_SetWareMaxFill            ();
	case QUEUE_CMD_DISMANTLEBUILDING:
		return *new Cmd_DismantleBuilding         ();
	case QUEUE_CMD_EVICTWORKER:
		return *new Cmd_EvictWorker();
	case QUEUE_CMD_MILITARYSITESETSOLDIERPREFERENCE:
		return *new Cmd_MilitarySiteSetSoldierPreference();
	default:
		throw wexception("Unknown Queue_Cmd_Id in file: %u", id);
	}
}

}
