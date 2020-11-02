#ifndef WL_WUI_SAVEGAMETABLE_H
#define WL_WUI_SAVEGAMETABLE_H

#include "ui_basic/table.h"
#include "wui/savegamedata.h"

/// A Table that displays savegames with support of displaying sub-directories
/// It must be extended to define columns and how entries are created.
class SavegameTable : public UI::Table<uintptr_t> {
public:
	SavegameTable(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave);

	void fill(const std::vector<SavegameData>& entries);
	virtual void set_show_filenames(bool show_filenames);

protected:
	const std::string map_filename(const std::string& filename, const std::string& mapname) const;
	const std::string find_game_type(const SavegameData& savegame) const;
	void create_directory_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                            const SavegameData& savegame);

	UI::FontStyle tooltip_style_, tooltip_header_style_;

private:
	bool localize_autosave_;

	virtual void add_columns() = 0;

	virtual void create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                                const SavegameData& savegame) = 0;
	virtual void create_error_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                                const SavegameData& savegame);
};

/// A Table to display savegames in singleplayer mode (2 columns)
class SavegameTableSinglePlayer : public SavegameTable {
public:
	SavegameTableSinglePlayer(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave);

private:
	void add_columns() override;
	void create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame) override;
};

/// A Table to display savegames in multiplayer mode (3 columns)
class SavegameTableMultiplayer : public SavegameTable {
public:
	SavegameTableMultiplayer(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave);

private:
	void add_columns() override;
	void create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame) override;
};

/// A Table to display replays where filenames can be hidden/shown (3 columns)
class SavegameTableReplay : public SavegameTable {
public:
	SavegameTableReplay(UI::Panel* parent, UI::PanelStyle style, bool localize_autosave);
	void set_show_filenames(bool show_filenames) override;

private:
	bool show_filenames_;
	void add_columns() override;
	void create_valid_entry(UI::Table<uintptr_t const>::EntryRecord& te,
	                        const SavegameData& savegame) override;
};

#endif  // WL_WUI_SAVEGAMETABLE_H
