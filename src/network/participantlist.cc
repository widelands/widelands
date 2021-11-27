#include "network/participantlist.h"

#include "ai/computer_player.h"
#include "logic/game.h"
#include "logic/game_settings.h"
#include "logic/player.h"
#include "logic/playersmanager.h"

ParticipantList::ParticipantList(const GameSettings* settings,
                                 Widelands::Game*& game,
                                 const std::string& localplayername)
   : settings_(settings),
     game_(game),
     localplayername_(localplayername),
     participant_counts_{-1, -1, -1} {
	assert(settings_ != nullptr);
	// The pointer referenced by game_ might be undefined here
	// localplayername_ might be empty here
	update_participant_counts();

	// When the update signal is called, re-calculate the participant counts
	participants_updated.connect(
	   [this]() { update_participant_counts(); }, Notifications::SubscriberPosition::kFront);
}

const ParticipantList::ParticipantCounts& ParticipantList::get_participant_counts() const {
	assert(participant_counts_.total != -1);
	return participant_counts_;
}

// The methods do not return lists on purpose since the data isn't stored in
// lists within GameHost/GameClient either. Creating lists here, returning them,
// and keeping them updated is just asking for trouble

ParticipantList::ParticipantType ParticipantList::get_participant_type(int16_t participant) const {
	assert(participant < participant_counts_.total);
	if (participant >= participant_counts_.humans) {
		return ParticipantList::ParticipantType::kAI;
	}
	if (participant_to_user(participant).position == UserSettings::none()) {
		return ParticipantList::ParticipantType::kSpectator;
	}
	return ParticipantList::ParticipantType::kPlayer;
}

Widelands::TeamNumber ParticipantList::get_participant_team(int16_t participant) const {
	const size_t index = participant_to_playerindex(participant);
	assert(index <= settings_->players.size());
	return settings_->players[index].team;
}

const std::string& ParticipantList::get_participant_name(int16_t participant) const {
	if (participant < participant_counts_.humans) {
		// We can't use the name in the Player entry for normal users since it isn't the selected
		// user name but instead a combined name of all the users for this player
		return participant_to_user(participant).name;
	}
	// It is an AI player. Get its type and resolve it to a pretty name
	const PlayerSettings& ps = settings_->players[participant_to_playerindex(participant)];
	assert(ps.state == PlayerSettings::State::kComputer);
	return AI::ComputerPlayer::get_implementation(ps.ai)->descname;
}

bool ParticipantList::needs_teamchat() const {
	assert(participant_counts_.humans >= 0);
	if (participant_counts_.humans <= 1) {
		// If 0: We have just connected and don't know anything about the users yet
		// If 1: We are the only participant, so there can't be any teams
		return false;
	}

	const int16_t my_index = get_local_participant_index();
	assert(my_index >= 0);
	assert(my_index < participant_counts_.humans);

	// Check whether we are a player or a spectator
	bool found_someone = false;
	if (get_participant_type(my_index) == ParticipantType::kSpectator) {
		// We are a spectator. Check if there are other spectators
		for (int16_t i = 0; i < participant_counts_.humans; ++i) {
			if (get_participant_type(i) != ParticipantType::kSpectator) {
				continue;
			}
			if (found_someone) {
				// The first one we find might be us. If we find a second one,
				// we know that there is a teammate
				return true;
			}
			found_someone = true;
		}
	} else {
		// We are a player. Get our team
		const Widelands::TeamNumber my_team = get_participant_team(my_index);
		if (my_team == 0) {
			// Team 0 is the "no team" entry
			// Special case: There might be one or multiple humans using the same player slot,
			// even when the player slot has no team.
			// In that case, allow team messages to the other humans

			// First pass: Find our index so we can get our player slot/number/position
			int16_t i = my_index;
			uint8_t my_position = UserSettings::not_connected();
			for (const UserSettings& u : settings_->users) {
				if (u.position == UserSettings::not_connected()) {
					continue;
				}
				if (i == 0) {
					my_position = u.position;
					break;
				}
				--i;
			}
			assert(my_position != UserSettings::not_connected());

			// Second pass: Count humans with this player position
			for (const UserSettings& u : settings_->users) {
				if (u.position != my_position) {
					continue;
				}
				if (found_someone) {
					return true;
				}
				found_someone = true;
			}
			// We are all alone (in this game)
			return false;
		}  // end player slot with no team

		// Search for other players with the same team
		for (int16_t i = 0; i < participant_counts_.humans; ++i) {
			if (get_participant_type(i) != ParticipantType::kPlayer) {
				// Skip spectators, they are no team players
				// AIs can't be found here since we only check the first entries that are humans
				continue;
			}
			if (get_participant_team(i) != my_team) {
				continue;
			}
			if (found_someone) {
				return true;
			}
			found_someone = true;
		}
	}
	return false;
}

const std::string& ParticipantList::get_local_playername() const {
	return localplayername_;
}

int16_t ParticipantList::get_local_participant_index() const {
	assert(!localplayername_.empty());
	// Find our player index
	for (int16_t my_index = 0; my_index < participant_counts_.humans; ++my_index) {
		if (get_participant_name(my_index) == localplayername_) {
			return my_index;
		}
	}
	// Not found, return error
	// Should not happen normally but might happen directly after connecting
	return -1;
}

bool ParticipantList::get_participant_defeated(int16_t participant) const {
	assert(is_ingame());
	return participant_to_player(participant)->is_defeated();
}

const RGBColor& ParticipantList::get_participant_color(int16_t participant) const {
	assert(get_participant_type(participant) != ParticipantList::ParticipantType::kSpectator);
	// Partially copied code from Player class, but this way also works in lobby
	return kPlayerColors[participant_to_playerindex(participant)];
}

bool ParticipantList::is_ingame() const {
	return (game_ != nullptr);
}

#ifdef NDEBUG
uint8_t ParticipantList::get_participant_rtt(int16_t) const {
#else
uint8_t ParticipantList::get_participant_rtt(int16_t participant) const {
#endif
	assert(participant < participant_counts_.humans);
	// TODO(Notabilis): Implement this function ... and all the Ping-stuff that belongs to it
	// - Maybe show two RTTs per player: To the host and to the netrelay
	// - Offer "autoUpdatePings(bool)" method to have the ping results be periodically refreshed
	// - Add support for the ping signal (participant_updated_rtt)
	// TODO(Notabilis): Add support for LAN games
	return 0;
}

const UserSettings& ParticipantList::participant_to_user(int16_t participant) const {
	assert(participant < participant_counts_.humans);
	assert(participant < static_cast<int16_t>(settings_->users.size()));
	for (const UserSettings& u : settings_->users) {
		if (u.position == UserSettings::not_connected()) {
			continue;
		}
		if (participant == 0) {
			return u;
		}
		--participant;
		assert(participant >= 0);
	}
	NEVER_HERE();
}

int32_t ParticipantList::participant_to_playerindex(int16_t participant) const {
	if (participant >= participant_counts_.humans) {
		// AI
		participant -= participant_counts_.humans;
		assert(settings_ != nullptr);
		assert(participant >= 0);
		assert(static_cast<size_t>(participant) < settings_->players.size());
		for (size_t i = 0; i < settings_->players.size(); ++i) {
			const PlayerSettings& player = settings_->players[i];
			if (player.state != PlayerSettings::State::kComputer) {
				// Ignore open, shared or human player slots
				continue;
			}
			if (participant == 0) {
				return i;
			}
			--participant;
			assert(participant >= 0);
		}
		NEVER_HERE();
	} else {
		// Human user

		// No useful result possible for spectators or semi-connected users
		assert(participant_to_user(participant).position <= UserSettings::highest_playernum());

		// .position is the index within settings_->players and also
		// as .position+1 the index inside game_->player_manager()
		return participant_to_user(participant).position;
	}
}

const Widelands::Player* ParticipantList::participant_to_player(int16_t participant) const {
	assert(participant < participant_counts_.total);
	assert(game_ != nullptr);
	const Widelands::PlayersManager* pm = game_->player_manager();
	assert(pm);
	const int32_t playerindex = participant_to_playerindex(participant);
	assert(playerindex >= 0);
	const Widelands::Player* p = pm->get_player(playerindex + 1);
	assert(p);
	return p;
}

void ParticipantList::update_participant_counts() {

	// Number of connected humans
	participant_counts_.humans = 0;
	for (const UserSettings& u : settings_->users) {
		// settings_->users might contain disconnected humans, filter them out
		if (u.position != UserSettings::not_connected()) {
			++participant_counts_.humans;
		}
	}
	assert(participant_counts_.humans <= static_cast<int16_t>(settings_->users.size()));
	participant_counts_.ais = 0;
	for (const PlayerSettings& player : settings_->players) {

		if (player.state != PlayerSettings::State::kComputer) {
			// Ignore open, shared or human player slots
			continue;
		}
		++participant_counts_.ais;
	}
	participant_counts_.total = participant_counts_.humans + participant_counts_.ais;
}
