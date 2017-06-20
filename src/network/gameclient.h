/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
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

#ifndef WL_NETWORK_GAMECLIENT_H
#define WL_NETWORK_GAMECLIENT_H

#include "chat/chat.h"
#include "logic/game_controller.h"
#include "logic/game_settings.h"
#include "logic/player_end_result.h"
#include "network/netclient.h"

struct GameClientImpl;

// TODO(unknown): Use composition instead of inheritance
/**
 * GameClient manages the lifetime of a network game in which this computer
 * participates as a client.
 *
 * This includes running the game setup screen and the actual game after
 * launch, as well as dealing with the actual network protocol.
 */
struct GameClient : public GameController,
                    public GameSettingsProvider,
                    private SyncCallback,
                    public ChatProvider {
	GameClient(const std::pair<NetAddress, NetAddress>& host, const std::string& playername, bool internet = false);

	virtual ~GameClient();

	void run();

	// GameController interface
	void think() override;
	void send_player_command(Widelands::PlayerCommand&) override;
	int32_t get_frametime() override;
	GameController::GameType get_game_type() override;

	uint32_t real_speed() override;
	uint32_t desired_speed() override;
	void set_desired_speed(uint32_t speed) override;
	bool is_paused() override;
	void set_paused(bool paused) override;
	void report_result(uint8_t player,
	                   Widelands::PlayerEndResult result,
	                   const std::string& info) override;
	// End GameController interface

	// GameSettingsProvider interface
	const GameSettings& settings() override;

	void set_scenario(bool) override;
	bool can_change_map() override;
	bool can_change_player_state(uint8_t number) override;
	bool can_change_player_tribe(uint8_t number) override;
	bool can_change_player_init(uint8_t number) override;
	bool can_change_player_team(uint8_t number) override;

	bool can_launch() override;

	virtual void set_map(const std::string& mapname,
	                     const std::string& mapfilename,
	                     uint32_t maxplayers,
	                     bool savegame = false) override;
	void set_player_state(uint8_t number, PlayerSettings::State state) override;
	virtual void
	set_player_ai(uint8_t number, const std::string& ai, bool const random_ai = false) override;
	void next_player_state(uint8_t number) override;
	virtual void set_player_tribe(uint8_t number,
	                              const std::string& tribe,
	                              bool const random_tribe = false) override;
	void set_player_init(uint8_t number, uint8_t index) override;
	void set_player_name(uint8_t number, const std::string& name) override;
	void set_player(uint8_t number, const PlayerSettings& ps) override;
	void set_player_number(uint8_t number) override;
	void set_player_team(uint8_t number, Widelands::TeamNumber team) override;
	void set_player_closeable(uint8_t number, bool closeable) override;
	void set_player_shared(uint8_t number, uint8_t shared) override;
	void set_win_condition_script(const std::string&) override;
	std::string get_win_condition_script() override;

	// ChatProvider interface
	void send(const std::string& msg) override;
	const std::vector<ChatMessage>& get_messages() const override;
	bool has_been_set() const override {
		return true;
	}

private:
	/// for unique backupname
	std::string backup_file_name(std::string& path) {
		return path + "~backup";
	}

	void syncreport() override;

	void handle_packet(RecvPacket&);
	void handle_network();
	void send_time();
	void receive_one_player(uint8_t number, StreamRead&);
	void receive_one_user(uint32_t number, StreamRead&);
	void disconnect(const std::string& reason,
	                const std::string& arg = "",
	                bool sendreason = true,
	                bool showmsg = true);

	NetTransferFile* file_;
	GameClientImpl* d;
	bool internet_;
};

#endif  // end of include guard: WL_NETWORK_GAMECLIENT_H
