#ifndef SAVEGAMETABLE_H
#define SAVEGAMETABLE_H
#include <vector>

#include "gamedetails.h"
#include "logic/filesystem_constants.h"
#include "ui_basic/table.h"
#include "wui/mapdata.h"

class SavegameTable : public UI::Table<uintptr_t> {
public:
	SavegameTable(UI::Panel* parent,
	              int32_t x,
	              int32_t y,
	              uint32_t w,
	              uint32_t h,
	              UI::PanelStyle style,
	              bool is_replay,
	              bool is_single_player,
	              bool show_filenames);

	/// Fill the table with maps and directories.
	void fill(const std::vector<SavegameData>& entries);
	void set_show_filenames(bool show_filenames);

private:
	bool localize_autosave_;
	bool is_replay_;
	bool is_single_player_;
	bool show_filenames_;
	const std::string map_filename(const std::string& filename, const std::string& mapname);
	void add_additional_columns();
	const std::string find_game_type(const SavegameData& savegame);
	void create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame);
	void create_error_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame);
};

#endif  // SAVEGAMETABLE_H
