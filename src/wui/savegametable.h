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
	              bool is_replay);

	/// Fill the table with maps and directories.
	void fill(const std::vector<SavegameData>& entries);

private:
	bool localize_autosave_;
	bool is_replay_;
	const std::string map_filename(const std::string& filename, const std::string& mapname);
	void add_columns_special();
};

#endif  // SAVEGAMETABLE_H
