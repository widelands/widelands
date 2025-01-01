/*
 * Copyright (C) 2002-2025 by the Widelands Development Team
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
#include "commands/cmd_act.h"
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
#include "commands/cmd_destroy_map_object.h"
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

template <typename... Args>
static GameLogicCommand& do_create_correct_queue_command(QueueCommandTypes const id, Args... args) {
	switch (id) {
	// Player commands
	case QueueCommandTypes::kBuildBuilding:
		return *new CmdBuildBuilding(args...);
	case QueueCommandTypes::kBuildFlag:
		return *new CmdBuildFlag(args...);
	case QueueCommandTypes::kBuildRoad:
		return *new CmdBuildRoad(args...);
	case QueueCommandTypes::kBuildWaterway:
		return *new CmdBuildWaterway(args...);
	case QueueCommandTypes::kFlagAction:
		return *new CmdFlagAction(args...);
	case QueueCommandTypes::kStartStopBuilding:
		return *new CmdStartStopBuilding(args...);
	case QueueCommandTypes::kToggleInfiniteProduction:
		return *new CmdToggleInfiniteProduction(args...);
	case QueueCommandTypes::kEnhanceBuilding:
		return *new CmdEnhanceBuilding(args...);
	case QueueCommandTypes::kBulldoze:
		return *new CmdBulldoze(args...);
	case QueueCommandTypes::kChangeTrainingOptions:
		return *new CmdChangeTrainingOptions(args...);
	case QueueCommandTypes::kDropSoldier:
		return *new CmdDropSoldier(args...);
	case QueueCommandTypes::kChangeSoldierCapacity:
		return *new CmdChangeSoldierCapacity(args...);
	case QueueCommandTypes::kAttack:
		return *new CmdAttack(args...);
	case QueueCommandTypes::kSetWarePriority:
		return *new CmdSetWarePriority(args...);
	case QueueCommandTypes::kSetWareTargetQuantity:
		return *new CmdSetWareTargetQuantity(args...);
	case QueueCommandTypes::kSetWorkerTargetQuantity:
		return *new CmdSetWorkerTargetQuantity(args...);
	case QueueCommandTypes::kSetInputMaxFill:
		return *new CmdSetInputMaxFill(args...);
	case QueueCommandTypes::kMessageSetStatusRead:
		return *new CmdMessageSetStatusRead(args...);
	case QueueCommandTypes::kMessageSetStatusArchived:
		return *new CmdMessageSetStatusArchived(args...);
	case QueueCommandTypes::kSetStockPolicy:
		return *new CmdSetStockPolicy(args...);
	case QueueCommandTypes::kDismantleBuilding:
		return *new CmdDismantleBuilding(args...);
	case QueueCommandTypes::kEvictWorker:
		return *new CmdEvictWorker(args...);
	case QueueCommandTypes::kSetSoldierPreference:
		return *new CmdSetSoldierPreference(args...);
	case QueueCommandTypes::kProposeTrade:
		return *new CmdProposeTrade(args...);
	case QueueCommandTypes::kShipSink:
		return *new CmdShipSink(args...);
	case QueueCommandTypes::kShipCancelExpedition:
		return *new CmdShipCancelExpedition(args...);
	case QueueCommandTypes::kStartOrCancelExpedition:
		return *new CmdStartOrCancelExpedition(args...);
	case QueueCommandTypes::kExpeditionConfig:
		return *new CmdExpeditionConfig(args...);
	case QueueCommandTypes::kShipConstructPort:
		return *new CmdShipConstructPort(args...);
	case QueueCommandTypes::kShipScoutDirection:
		return *new CmdShipScoutDirection(args...);
	case QueueCommandTypes::kShipExploreIsland:
		return *new CmdShipExploreIsland(args...);
	case QueueCommandTypes::kShipSetDestination:
		return *new CmdShipSetDestination(args...);
	case QueueCommandTypes::kShipRefit:
		return *new CmdShipRefit(args...);
	case QueueCommandTypes::kWarshipCommand:
		return *new CmdWarshipCommand(args...);
	case QueueCommandTypes::kToggleMuteMessages:
		return *new CmdToggleMuteMessages(args...);
	case QueueCommandTypes::kMarkMapObjectForRemoval:
		return *new CmdMarkMapObjectForRemoval(args...);
	case QueueCommandTypes::kDiplomacy:
		return *new CmdDiplomacy(args...);
	case QueueCommandTypes::kPinnedNote:
		return *new CmdPinnedNote(args...);
	case QueueCommandTypes::kBuildingName:
		return *new CmdBuildingName(args...);
	case QueueCommandTypes::kFleetTargets:
		return *new CmdFleetTargets(args...);
	case QueueCommandTypes::kPickCustomStartingPosition:
		return *new CmdPickCustomStartingPosition(args...);

	// Non-player game logic commands
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

	// Not a logic command
	case QueueCommandTypes::kDeleteMessage:
	case QueueCommandTypes::kNetCheckSync:
	case QueueCommandTypes::kReplaySyncWrite:
	case QueueCommandTypes::kReplaySyncRead:
	case QueueCommandTypes::kReplayEnd:
	case QueueCommandTypes::kNone:
	default:
		throw wexception("Unknown Queue_Cmd_Id in file: %u", static_cast<unsigned int>(id));
	}
}

GameLogicCommand& create_correct_queue_command(QueueCommandTypes const id) {
	return do_create_correct_queue_command<>(id);
}

PlayerCommand* PlayerCommand::deserialize(StreamRead& des) {
	const uint8_t command_id = des.unsigned_8();
	GameLogicCommand& cmd =
	   do_create_correct_queue_command<StreamRead&>(static_cast<QueueCommandTypes>(command_id), des);
	if (upcast(PlayerCommand, pc, &cmd); pc != nullptr) {
		return pc;
	}
	throw wexception(
	   "Queue_Cmd_Id is not a player command: %u", static_cast<unsigned int>(command_id));
}

}  // namespace Widelands
