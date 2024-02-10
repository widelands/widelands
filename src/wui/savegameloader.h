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

#ifndef WL_WUI_SAVEGAMELOADER_H
#define WL_WUI_SAVEGAMELOADER_H

#include <optional>
#include <string>
#include <vector>

#include "base/string.h"
#include "game_io/game_preload_packet.h"
#include "wui/savegamedata.h"

class SavegameLoader {
public:
	explicit SavegameLoader(Widelands::Game& game);
	virtual ~SavegameLoader() = default;
	std::vector<SavegameData> load_files(const std::string& directory);

private:
	[[nodiscard]] virtual bool is_valid_gametype(const SavegameData& gamedata) const = 0;

	void add_general_information(SavegameData& gamedata,
	                             const Widelands::GamePreloadPacket& gpdp) const;
	void add_error_info(SavegameData& gamedata, std::string errormessage) const;
	void add_time_info(SavegameData& gamedata, const Widelands::GamePreloadPacket& gpdp) const;
	void add_sub_dir(const std::string& gamefilename, std::vector<SavegameData>& loaded_games) const;
	void load(const std::string& to_be_loaded, std::vector<SavegameData>& loaded_games) const;
	[[nodiscard]] virtual bool is_valid_savegame(const std::string& filename) const {
		return ends_with(filename, kSavegameExtension);
	}

	Widelands::Game& game_;

protected:
	virtual void load_savegame_from_directory(const std::string& gamefilename,
	                                          std::vector<SavegameData>& loaded_games,
	                                          bool load_for_replay = false) const;
	virtual void load_savegame_from_file(const std::string& gamefilename,
	                                     std::vector<SavegameData>& loaded_games,
	                                     bool load_for_replay = false) const;
};

class ReplayLoader : public SavegameLoader {
public:
	explicit ReplayLoader(Widelands::Game& game);

private:
	[[nodiscard]] bool is_valid_savegame(const std::string& filename) const override {
		return ends_with(filename, kReplayExtension);
	}

	[[nodiscard]] bool is_valid_gametype(const SavegameData& gamedata) const override;

	void load_savegame_from_directory(const std::string& gamefilename,
	                                  std::vector<SavegameData>& loaded_games,
	                                  bool /* load_for_replay */) const override;
	void load_savegame_from_file(const std::string& gamefilename,
	                             std::vector<SavegameData>& loaded_games,
	                             bool /* load_for_replay */) const override;
};

class MultiPlayerLoader : public SavegameLoader {
public:
	explicit MultiPlayerLoader(Widelands::Game& game);

private:
	[[nodiscard]] bool is_valid_gametype(const SavegameData& gamedata) const override;
};

class SinglePlayerLoader : public SavegameLoader {
public:
	explicit SinglePlayerLoader(Widelands::Game& game);

private:
	[[nodiscard]] bool is_valid_gametype(const SavegameData& gamedata) const override;
};

class EverythingLoader : public SavegameLoader {
public:
	explicit EverythingLoader(Widelands::Game& game);

private:
	[[nodiscard]] bool is_valid_savegame(const std::string& filename) const override {
		return ends_with(filename, kSavegameExtension) || ends_with(filename, kReplayExtension);
	}

	[[nodiscard]] bool is_valid_gametype(const SavegameData& gamedata) const override;
};

std::optional<SavegameData> newest_saved_game_or_replay(bool find_replay = false);

#endif  // WL_WUI_SAVEGAMELOADER_H
