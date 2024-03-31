/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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

#include "commands/queue_cmd_ids.h"

#include "base/wexception.h"
#include "commands/cmd_attack.h"
#include "commands/cmd_build_building.h"
#include "commands/cmd_build_flag.h"
#include "commands/cmd_build_road.h"
#include "commands/cmd_build_waterway.h"
#include "commands/cmd_building_name.h"
#include "commands/cmd_bulldoze.h"
#include "commands/cmd_calculate_statistics.h"
#include "commands/cmd_call_economy_balance.h"
#include "commands/cmd_change_soldier_capacity.h"
#include "commands/cmd_change_training_options.h"
#include "commands/cmd_delete_message.h"
#include "commands/cmd_diplomacy.h"
#include "commands/cmd_dismantle_building.h"
#include "commands/cmd_drop_soldier.h"
#include "commands/cmd_enhance_building.h"
#include "commands/cmd_evict_worker.h"
#include "commands/cmd_expedition_config.h"
#include "commands/cmd_flag_action.h"
#include "commands/cmd_fleet_targets.h"
#include "commands/cmd_incorporate.h"
#include "commands/cmd_luacoroutine.h"
#include "commands/cmd_luascript.h"
#include "commands/cmd_mark_map_object_for_removal.h"
#include "commands/cmd_message_set_status_archived.h"
#include "commands/cmd_message_set_status_read.h"
#include "commands/cmd_pick_custom_starting_position.h"
#include "commands/cmd_pinned_note.h"
#include "commands/cmd_propose_trade.h"
#include "commands/cmd_set_input_max_fill.h"
#include "commands/cmd_set_soldier_preference.h"
#include "commands/cmd_set_stock_policy.h"
#include "commands/cmd_set_ware_priority.h"
#include "commands/cmd_set_ware_target_quantity.h"
#include "commands/cmd_set_worker_target_quantity.h"
#include "commands/cmd_ship_cancel_expedition.h"
#include "commands/cmd_ship_construct_port.h"
#include "commands/cmd_ship_explore_island.h"
#include "commands/cmd_ship_refit.h"
#include "commands/cmd_ship_scout_direction.h"
#include "commands/cmd_ship_set_destination.h"
#include "commands/cmd_ship_sink.h"
#include "commands/cmd_start_or_cancel_expedition.h"
#include "commands/cmd_start_stop_building.h"
#include "commands/cmd_toggle_infinite_production.h"
#include "commands/cmd_toggle_mute_messages.h"
#include "commands/cmd_warship_command.h"

namespace Widelands {

GameLogicCommand& create_correct_queue_command(QueueCommandTypes const id) {
	switch (id) {
	case QueueCommandTypes::kBuildBuilding:
		return *new CmdBuildBuilding();
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
	case QueueCommandTypes::kToggleInfiniteProduction:
		return *new CmdToggleInfiniteProduction();
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
	case QueueCommandTypes::kAttack:
		return *new CmdAttack();
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
	case QueueCommandTypes::kSetSoldierPreference:
		return *new CmdSetSoldierPreference();
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
	case QueueCommandTypes::kShipSetDestination:
		return *new CmdShipSetDestination();
	case QueueCommandTypes::kShipRefit:
		return *new CmdShipRefit();
	case QueueCommandTypes::kWarshipCommand:
		return *new CmdWarshipCommand();
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
	case QueueCommandTypes::kDiplomacy:
		return *new CmdDiplomacy();
	case QueueCommandTypes::kPinnedNote:
		return *new CmdPinnedNote();
	case QueueCommandTypes::kBuildingName:
		return *new CmdBuildingName();
	case QueueCommandTypes::kFleetTargets:
		return *new CmdFleetTargets();
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
