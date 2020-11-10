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
