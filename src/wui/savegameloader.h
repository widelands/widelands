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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WL_WUI_SAVEGAMELOADER_H
#define WL_WUI_SAVEGAMELOADER_H

#include <string>
#include <vector>

#include "game_io/game_preload_packet.h"
#include "wui/savegamedata.h"

class SavegameLoader {
public:
	SavegameLoader(Widelands::Game& game);
	virtual ~SavegameLoader() {
	}
	std::vector<SavegameData> load_files(const std::string& directory);

private:
	virtual bool is_valid_gametype(const SavegameData& gamedata) const = 0;
	virtual std::string get_savename(const std::string& gamefilename) const;

	void add_general_information(SavegameData& gamedata,
	                             const Widelands::GamePreloadPacket& gpdp) const;
	void add_error_info(SavegameData& gamedata, std::string errormessage) const;
	void add_time_info(SavegameData& gamedata, const Widelands::GamePreloadPacket& gpdp) const;
	void add_sub_dir(const std::string& gamefilename, std::vector<SavegameData>& loaded_games) const;
	void load_savegame_from_directory(const std::string& gamefilename,
	                                  std::vector<SavegameData>& loaded_games) const;
	void load_savegame_from_file(const std::string& gamefilename,
	                             std::vector<SavegameData>& loaded_games) const;
	void load(const std::string& to_be_loaded, std::vector<SavegameData>& loaded_games) const;

	Widelands::Game& game_;
};

class ReplayLoader : public SavegameLoader {
public:
	explicit ReplayLoader(Widelands::Game& game);

private:
	bool is_valid_gametype(const SavegameData& gamedata) const override;
	std::string get_savename(const std::string& gamefilename) const override;
};

class MultiPlayerLoader : public SavegameLoader {
public:
	explicit MultiPlayerLoader(Widelands::Game& game);

private:
	bool is_valid_gametype(const SavegameData& gamedata) const override;
};

class SinglePlayerLoader : public SavegameLoader {
public:
	explicit SinglePlayerLoader(Widelands::Game& game);

private:
	bool is_valid_gametype(const SavegameData& gamedata) const override;
};

class EverythingLoader : public SavegameLoader {
public:
	explicit EverythingLoader(Widelands::Game& game);

private:
	bool is_valid_gametype(const SavegameData& gamedata) const override;
};

#endif  // WL_WUI_SAVEGAMELOADER_H
