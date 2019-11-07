#ifndef SAVEGAMETABLE_H
#define SAVEGAMETABLE_H
#include <vector>

#include "gamedetails.h"
#include "logic/filesystem_constants.h"
#include "ui_basic/table.h"
#include "wui/mapdata.h"

class SavegameTable : public UI::Table<uintptr_t> {
public:
	SavegameTable(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave);

	/// Fill the table with maps and directories.
	void fill(const std::vector<SavegameData>& entries);
	virtual void set_show_filenames(bool show_filenames);

protected:
	const std::string map_filename(const std::string& filename, const std::string& mapname);
	const std::string find_game_type(const SavegameData& savegame);

private:
	bool localize_autosave_;

	virtual void add_columns() = 0;

	virtual void create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                                const SavegameData& savegame) = 0;
	virtual void create_error_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                                const SavegameData& savegame) = 0;
};

class SavegameTableSinglePlayer : public SavegameTable {
public:
	SavegameTableSinglePlayer(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave);

private:
	void add_columns() override;
	void create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame) override;
	void create_error_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame) override;
};

class SavegameTableMultiplayer : public SavegameTable {
public:
	SavegameTableMultiplayer(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave);

private:
	void add_columns() override;
	void create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame) override;
	void create_error_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame) override;
};

class SavegameTableReplay : public SavegameTable {
public:
	SavegameTableReplay(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave);
	void set_show_filenames(bool show_filenames) override;

private:
	bool show_filenames_;
	void add_columns() override;
	void create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame) override;
	void create_error_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame) override;
};

#endif  // SAVEGAMETABLE_H
