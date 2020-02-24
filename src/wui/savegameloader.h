#ifndef WL_WUI_SAVEGAMELOADER_H
#define WL_WUI_SAVEGAMELOADER_H

#include <string>
#include <vector>

#include "game_io/game_preload_packet.h"
#include "wui/savegamedata.h"

class SavegameLoader {
public:
	SavegameLoader(Widelands::Game& game);

	std::vector<SavegameData> load_files(const std::string directory);

private:
	void load_gamefile(const std::string& gamefilename);
	void add_general_information(SavegameData& gamedata,
	                             const Widelands::GamePreloadPacket& gpdp) const;
	void add_error_info(SavegameData& gamedata, std::string errormessage) const;
	bool is_valid_gametype(const SavegameData& gamedata) const;
	void add_time_info(SavegameData& gamedata, const Widelands::GamePreloadPacket& gpdp) const;
	void add_sub_dir(const std::string& gamefilename);

	Widelands::Game& game_;
};

#endif  // WL_WUI_SAVEGAMELOADER_H
