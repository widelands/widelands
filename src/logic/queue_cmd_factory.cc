/*
 * Copyright (C) 2002-2022 by the Widelands Development Team
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
#include "logic/playercommand.h"

namespace Widelands {

GameLogicCommand& QueueCmdFactory::create_correct_queue_command(QueueCommandTypes const id) {
	switch (id) {
	case QueueCommandTypes::kBuild:
		return *new CmdBuild();
	case QueueCommandTypes::kBuildFlag:
		return *new CmdBuildFlag();
	case QueueCommandTypes::kBuildRoad:
		return *new CmdBuildRoad();
	case QueueCommandTypes::kBuildWaterway:
		return *new CmdBuildWaterway();
	case QueueCommandTypes::kFlagAction:
		return *new CmdFlagAction();
	case QueueCommandTypes::kStartStopBuilding:
		return *new CmdStartStopBuilding();
	case QueueCommandTypes::kEnhanceBuilding:
		return *new CmdEnhanceBuilding();
	case QueueCommandTypes::kBulldoze:
		return *new CmdBulldoze();
	case QueueCommandTypes::kChangeTrainingOptions:
		return *new CmdChangeTrainingOptions();
	case QueueCommandTypes::kDropSoldier:
		return *new CmdDropSoldier();
	case QueueCommandTypes::kChangeSoldierCapacity:
		return *new CmdChangeSoldierCapacity();
	case QueueCommandTypes::kEnemyFlagAction:
		return *new CmdEnemyFlagAction();
	case QueueCommandTypes::kSetWarePriority:
		return *new CmdSetWarePriority();
	case QueueCommandTypes::kSetWareTargetQuantity:
		return *new CmdSetWareTargetQuantity();
	case QueueCommandTypes::kSetWorkerTargetQuantity:
		return *new CmdSetWorkerTargetQuantity();
	case QueueCommandTypes::kSetInputMaxFill:
		return *new CmdSetInputMaxFill();
	case QueueCommandTypes::kMessageSetStatusRead:
		return *new CmdMessageSetStatusRead();
	case QueueCommandTypes::kMessageSetStatusArchived:
		return *new CmdMessageSetStatusArchived();
	case QueueCommandTypes::kSetStockPolicy:
		return *new CmdSetStockPolicy();
	case QueueCommandTypes::kDismantleBuilding:
		return *new CmdDismantleBuilding();
	case QueueCommandTypes::kEvictWorker:
		return *new CmdEvictWorker();
	case QueueCommandTypes::kMilitarysiteSetSoldierPreference:
		return *new CmdMilitarySiteSetSoldierPreference();
	case QueueCommandTypes::kProposeTrade:
		return *new CmdProposeTrade();
	case QueueCommandTypes::kShipSink:
		return *new CmdShipSink();
	case QueueCommandTypes::kShipCancelExpedition:
		return *new CmdShipCancelExpedition();
	case QueueCommandTypes::kStartOrCancelExpedition:
		return *new CmdStartOrCancelExpedition();
	case QueueCommandTypes::kExpeditionConfig:
		return *new CmdExpeditionConfig();
	case QueueCommandTypes::kShipConstructPort:
		return *new CmdShipConstructPort();
	case QueueCommandTypes::kShipScoutDirection:
		return *new CmdShipScoutDirection();
	case QueueCommandTypes::kShipExploreIsland:
		return *new CmdShipExploreIsland();
	case QueueCommandTypes::kDestroyMapObject:
		return *new CmdDestroyMapObject();
	case QueueCommandTypes::kAct:
		return *new CmdAct();
	case QueueCommandTypes::kIncorporate:
		return *new CmdIncorporate();
	case QueueCommandTypes::kLuaScript:
		return *new CmdLuaScript();
	case QueueCommandTypes::kLuaCoroutine:
		return *new CmdLuaCoroutine();
	case QueueCommandTypes::kCalculateStatistics:
		return *new CmdCalculateStatistics();
	case QueueCommandTypes::kCallEconomyBalance:
		return *new CmdCallEconomyBalance();
	case QueueCommandTypes::kToggleMuteMessages:
		return *new CmdToggleMuteMessages();
	case QueueCommandTypes::kMarkMapObjectForRemoval:
		return *new CmdMarkMapObjectForRemoval();
	case QueueCommandTypes::kPickCustomStartingPosition:
		return *new CmdPickCustomStartingPosition();
	case QueueCommandTypes::kDeleteMessage:  // Not a logic command
	case QueueCommandTypes::kNetCheckSync:
	case QueueCommandTypes::kReplaySyncWrite:
	case QueueCommandTypes::kReplaySyncRead:
	case QueueCommandTypes::kReplayEnd:
	case QueueCommandTypes::kNone:
		throw wexception("Unknown Queue_Cmd_Id in file: %u", static_cast<unsigned int>(id));
	}
	NEVER_HERE();
}
}  // namespace Widelands
