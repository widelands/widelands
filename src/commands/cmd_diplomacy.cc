/*
 * Copyright (C) 2004-2025 by the Widelands Development Team
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

#include "commands/cmd_diplomacy.h"

#include <memory>

#include "logic/game.h"
#include "logic/game_controller.h"
#include "logic/player.h"
#include "logic/playersmanager.h"
#include "map_io/map_object_loader.h"
#include "map_io/map_object_saver.h"
#include "wui/interactive_spectator.h"

namespace Widelands {

// CmdDiplomacy
void CmdDiplomacy::execute(Game& game) {
	if (!game.diplomacy_allowed()) {
		return;
	}
	Player& sending_player = *game.get_safe_player(sender());
	auto broadcast_message = [&game](const std::string& heading, const std::string& text) {
		iterate_players_existing(p, game.map().get_nrplayers(), game, player) {
			player->add_message(game, std::unique_ptr<Message>(new Message(
			                             Message::Type::kScenario, game.get_gametime(), _("Diplomacy"),
			                             "images/players/team.png", heading, text)));
		}
		if (upcast(InteractiveSpectator, is, game.get_ibase())) {
			is->log_message(heading, text);
		}
	};

	switch (action_) {
	case DiplomacyAction::kResign:
		if (game.player_manager()->get_player_end_status(sender()) != nullptr) {
			// Ignore if the player lost a moment ago
			return;
		}
		broadcast_message(_("Player Resigned"), format(_("%s has resigned and is now a spectator."),
		                                               sending_player.get_name()));
		game.game_controller()->report_result(sender(), PlayerEndResult::kResigned, "");
		// TODO(Nordfriese): Making the player a spectator is currently left to the win condition.
		// In Endless Game and scenarios this makes it effectively impossible to resign currently.
		break;

	case DiplomacyAction::kLeaveTeam:
		if (sending_player.team_number() == 0) {
			break;
		}
		broadcast_message(
		   _("Player Leaves Team"),
		   format(_("%1$s has left team %2$u and is now teamless."), sending_player.get_name(),
		          static_cast<unsigned>(sending_player.team_number())));
		sending_player.set_team_number(0);
		break;

	case DiplomacyAction::kJoin:
	case DiplomacyAction::kInvite:
		broadcast_message(
		   action_ == DiplomacyAction::kJoin ? _("Team Joining Request") :
		                                       _("Team Joining Invitation"),
		   format(action_ == DiplomacyAction::kJoin ?
		             _("%1$s has requested to join the team of %2$s.") :
		             _("%1$s has invited %2$s to join their team."),
		          sending_player.get_name(), game.get_safe_player(other_player_)->get_name()));
		game.pending_diplomacy_actions().emplace_back(sender(), action_, other_player_);
		// If other_player_ is the interactive player, the IBase
		// will open a confirmation window on next think()
		break;

	case DiplomacyAction::kAcceptJoin:
	case DiplomacyAction::kRefuseJoin:
	case DiplomacyAction::kRetractJoin:
	case DiplomacyAction::kAcceptInvite:
	case DiplomacyAction::kRefuseInvite:
	case DiplomacyAction::kRetractInvite: {
		PlayerNumber cmd_sender = sender();
		bool retract = false;
		if (action_ == DiplomacyAction::kRetractJoin || action_ == DiplomacyAction::kRetractInvite) {
			/* Retracting is treated just like rejection but with a different message. */
			std::swap(other_player_, cmd_sender);
			retract = true;
			action_ = (action_ == DiplomacyAction::kRetractJoin) ? DiplomacyAction::kRefuseJoin :
			                                                       DiplomacyAction::kRefuseInvite;
		}
		assert(other_player_ != cmd_sender);

		const DiplomacyAction original_action =
		   (action_ == DiplomacyAction::kAcceptJoin || action_ == DiplomacyAction::kRefuseJoin) ?
		      DiplomacyAction::kJoin :
		      DiplomacyAction::kInvite;
		for (auto it = game.pending_diplomacy_actions().begin();
		     it != game.pending_diplomacy_actions().end(); ++it) {
			// Note that in the response the numbers of the two players
			// are swapped compared to the original message.
			if (it->action == original_action && it->sender == other_player_ &&
			    it->other == cmd_sender) {
				const bool accept =
				   action_ == DiplomacyAction::kAcceptJoin || action_ == DiplomacyAction::kAcceptInvite;
				std::string fmt_message;
				if (accept) {
					if (original_action == DiplomacyAction::kJoin) {
						fmt_message = _("%1$s has accepted %2$s into their team.");
					} else {
						fmt_message = _("%1$s has accepted the invitation to join the team of %2$s.");
					}
				} else {
					if (original_action == DiplomacyAction::kJoin) {
						if (retract) {
							fmt_message = _("%1$s has retracted the request to join the team of %2$s.");
						} else {
							fmt_message = _("%1$s has denied %2$s membership in their team.");
						}
					} else if (retract) {
						fmt_message = _("%1$s has retracted the invitation to %2$s to join their team.");
					} else {
						fmt_message = _("%1$s has rejected the invitation to join the team of %2$s.");
					}
				}
				broadcast_message(
				   accept ? _("Team Change Accepted") : _("Team Change Rejected"),
				   format(fmt_message, sending_player.get_name(),
				          game.get_safe_player(retract ? cmd_sender : other_player_)->get_name()));

				if (accept) {
					Player* joiner = game.get_safe_player(
					   original_action == DiplomacyAction::kJoin ? other_player_ : cmd_sender);
					Player* other = game.get_safe_player(
					   original_action != DiplomacyAction::kJoin ? other_player_ : cmd_sender);
					if (other->team_number() == 0) {
						// Assign both players to a previously unused team slot
						std::set<TeamNumber> teams;
						iterate_players_existing_const(p, game.map().get_nrplayers(), game, player) {
							teams.insert(player->team_number());
						}
						for (TeamNumber t = 1;; ++t) {
							if (teams.count(t) > 0) {
								continue;
							}
							other->set_team_number(t);
							joiner->set_team_number(t);
							break;
						}
					} else {
						joiner->set_team_number(other->team_number());
					}
				}

				game.pending_diplomacy_actions().erase(it);
				break;
			}
		}
		// If we found nothing, perhaps the command had been sent twice. Ignore.
		break;
	}

	default:
		NEVER_HERE();
	}
}

CmdDiplomacy::CmdDiplomacy(StreamRead& des) : PlayerCommand(Time(0), des.unsigned_8()) {
	action_ = static_cast<DiplomacyAction>(des.unsigned_8());
	other_player_ = des.unsigned_8();
}

void CmdDiplomacy::serialize(StreamWrite& ser) {
	write_id_and_sender(ser);
	ser.unsigned_8(static_cast<uint8_t>(action_));
	ser.unsigned_8(other_player_);
}

constexpr uint8_t kCurrentPacketVersionCmdDiplomacy = 1;

void CmdDiplomacy::read(FileRead& fr, EditorGameBase& egbase, MapObjectLoader& mol) {
	try {
		uint8_t packet_version = fr.unsigned_8();
		if (packet_version == kCurrentPacketVersionCmdDiplomacy) {
			PlayerCommand::read(fr, egbase, mol);
			action_ = static_cast<DiplomacyAction>(fr.unsigned_8());
			other_player_ = fr.unsigned_8();
		} else {
			throw UnhandledVersionError(
			   "CmdDiplomacy", packet_version, kCurrentPacketVersionCmdDiplomacy);
		}
	} catch (const std::exception& e) {
		throw GameDataError("Cmd_Diplomacy: %s", e.what());
	}
}

void CmdDiplomacy::write(FileWrite& fw, EditorGameBase& egbase, MapObjectSaver& mos) {
	fw.unsigned_8(kCurrentPacketVersionCmdDiplomacy);
	PlayerCommand::write(fw, egbase, mos);
	fw.unsigned_8(static_cast<uint8_t>(action_));
	fw.unsigned_8(other_player_);
}

}  // namespace Widelands
