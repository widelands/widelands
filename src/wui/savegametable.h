#ifndef SAVEGAMETABLE_H
#define SAVEGAMETABLE_H
#include <vector>

#include "gamedetails.h"
#include "ui_basic/table.h"
#include "wui/mapdata.h"

class SavegameTable : public UI::Table<uintptr_t> {
public:
	SavegameTable(
	   UI::Panel* parent, int32_t x, int32_t y, uint32_t w, uint32_t h, UI::PanelStyle style);

	/// Fill the table with maps and directories.
	void fill(const std::vector<SavegameData>& entries);
};

#endif  // SAVEGAMETABLE_H
