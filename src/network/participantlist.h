/*
 * Copyright (C) 2008-2022 by the Widelands Development Team
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

#ifndef WL_NETWORK_PARTICIPANTLIST_H
#define WL_NETWORK_PARTICIPANTLIST_H

#include <string>

#include "logic/widelands.h"
#include "notifications/signal.h"

// Avoid as many includes as possible
struct GameSettings;
struct UserSettings;
struct RGBColor;

namespace Widelands {
class Game;
class Player;
}  // namespace Widelands

/**
 * Class interface to provide access to the lists of network game participants to the UI.
 * This is a list of participants, not of players. Defeated players and spectators are
 * also handled by this, as well as AIs. The players are indirectly present since each
 * player is always controlled by a human user or an AI.
 * If a player slot is closed, it will not show up here.
 *
 * When passing a \c participant index, the results are ordered: First all human participants
 * are returned, then the AIs.
 * The \c participant indices used by this class are not compatible to other indices,
 * e.g., in GameSettings::players.
 *
 * @note The signal \c participants_updated *must* be triggered by the game if the there are
 *       changes with the participants (e.g., client connects, tribe is selected, ...).
 *       The count of active participants is cached and only recalculated on signal.
 */
class ParticipantList {

public:
	/// Describes the role of a participant inside the game
	enum class ParticipantType {
		/// A human user currently controlling a tribe
		kPlayer,
		/// An AI controlling a tribe
		kAI,
		/// A human spectator of the game
		kSpectator
	};

	/// Counts of current participants
	struct ParticipantCounts {
		/// Number of AIs currently in the game
		int16_t ais;
		/// Number of humans currently in the game
		int16_t humans;
		/// Sum of the other two
		int16_t total;
	};
	/**
	 * Constructor.
	 * @param settings The settings of the current network game.
	 *                 The game might be in the lobby or already started.
	 * @param game A *reference to a pointer* to the game. This way the pointer might be \c nullptr
	 *             when creating the class but will get the right value later on by itself.
	 *             Also, a check for \c nullptr is used to determine whether a game is running.
	 * @param localplayername The name of the network player on this computer.
	 */
	ParticipantList(const GameSettings* settings,
	                Widelands::Game*& game,
	                const std::string& localplayername);

	/**
	 * Returns the counts of currently connected participants.
	 * The contained total - 1 is the highest permitted participant index for the other methods.
	 * @return The counts of participants active in the game.
	 */
	const ParticipantCounts& get_participant_counts() const;

	/**
	 * Returns the type of participant.
	 * @param participant The index of the participant to get data about.
	 * @return The type of participant.
	 */
	ParticipantType get_participant_type(int16_t participant) const;

	/**
	 * Returns the team of the participant when the participant is a player.
	 * A value of \c 0 indicates that the participant has no team.
	 * For spectators, the result is undefined.
	 * @param participant The index of the participant to get data about.
	 * @return The team of players used by the participant.
	 */
	Widelands::TeamNumber get_participant_team(int16_t participant) const;

	/**
	 * Returns whether the local player has someone to chat in its team.
	 * For spectators this is true if there are other spectators.
	 * For players this is true when there are team members that are no AI.
	 * For player slots shared between multiple humans, this is always true.
	 * @param participant The index of the participant to get data about.
	 * @return Whether the participant has someone to chat with in its team.
	 */
	bool needs_teamchat() const;

	/**
	 * Returns the name of the local player.
	 * I.e., the player on the current computer.
	 * @note This method is much more efficient than getting the name for the local playerindex.
	 * @return The player name.
	 */
	const std::string& get_local_playername() const;

	/**
	 * Returns the participant index of the local player.
	 * Might return -1 (invalid index) when the current state of the game settings is invalid.
	 * This can happen for a moment after a client connected to a game.
	 * @note This method is relatively expensive (looping through players).
	 * @return The index of the local player or -1.
	 */
	int16_t get_local_participant_index() const;

	/**
	 * Returns the name of the participant.
	 * This is the name the participant provided when connecting to the server.
	 * The name is also used to display chat messages by this participant.
	 * For AIs this is a descriptive name of the AI.
	 * @param participant The index of the participant to get data about.
	 * @return The name of the participant.
	 */
	const std::string& get_participant_name(int16_t participant) const;

	/**
	 * Returns whether the player used by the participant is defeated or still playing.
	 * For spectators, the result is undefined.
	 * @warning Must only be called when ingame.
	 * @param participant The index of the participant to get data about.
	 * @return Whether the participant has been defeated.
	 */
	bool get_participant_defeated(int16_t participant) const;

	/**
	 * Returns the color of the player used by the participant.
	 * For spectators, the result is undefined.
	 * @param participant The index of the participant to get data about.
	 * @return The playercolor.
	 */
	const RGBColor& get_participant_color(int16_t participant) const;

	/**
	 * Returns whether a game is currently running.
	 * @return Whether a game is running.
	 */
	bool is_ingame() const;

	/**
	 * Returns the ping time of the participant.
	 * Returned is the time that it took the client to return a PING request by the network
	 * relay (the RTT = Round Trip Time).
	 * For AI participant the result is undefined.
	 * In network games that don't use the network relay the result is undefined.
	 * @warning Currently this method isn't implemented yet and always returns 0.
	 * @param participant The index of the participant to get data about.
	 * @return The RTT in milliseconds for this participant up to 255ms.
	 */
	uint8_t get_participant_rtt(int16_t participant) const;

	/// Called when the underlying data was updated and should be re-fetched.
	Notifications::Signal<> participants_updated;

	/**
	 * Called when the RTT for a participant changed.
	 * Passed parameters are the participant index and the new RTT.
	 */
	Notifications::Signal<int16_t, uint8_t> participant_updated_rtt;

private:
	/**
	 * Fetches the UserSettings belonging to the given participant index.
	 * The caller has to make sure that \c participant refers to a human user.
	 * @param participant The index to fetch the data for.
	 * @return The UserSettings entry for the given participant.
	 */
	const UserSettings& participant_to_user(int16_t participant) const;

	/**
	 * Gets the index within GameSettings::players that belongs to the given participant index.
	 * \c participant might refer to a human or AI, but it must be a player and no spectator.
	 * @param participant The index to fetch the data for.
	 * @return The index within GameSettings::players for the given participant.
	 */
	int32_t participant_to_playerindex(int16_t participant) const;

	/**
	 * Fetches the Player belonging to the given participant index.
	 * \c participant might refer to a human or AI, but it must be a player and no spectator.
	 * @note This method must only be used while in a game.
	 * @param participant The index to fetch the data for.
	 * @return A pointer to the Player entry for the given participant.
	 */
	const Widelands::Player* participant_to_player(int16_t participant) const;

	/**
	 * Update \c participant_counts_ based on the game state in \c settings_.
	 */
	void update_participant_counts();

	/// A pointer to the settings of the current game (running or being prepared).
	const GameSettings* settings_;
	/// A reference to the pointer of a currently runnning game.
	Widelands::Game*& game_;
	/// A reference to the user name of the human on this computer.
	const std::string& localplayername_;
	/// Counts of current participants
	ParticipantCounts participant_counts_;
};

#endif  // WL_NETWORK_PARTICIPANTLIST_H
