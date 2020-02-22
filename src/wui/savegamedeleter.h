#ifndef WL_WUI_SAVEGAMEDELETER_H
#define WL_WUI_SAVEGAMEDELETER_H

#include "savegamedata.h"
#include "ui_basic/panel.h"
#include <stdint.h>
#include <string>
#include <vector>
class SavegameDeleter {
public:
	SavegameDeleter(UI::Panel* parent);
	bool delete_savegames(const std::vector<SavegameData>& to_be_deleted) const;

private:
	bool show_confirmation_window(const std::vector<SavegameData>& selections) const;
	const std::string create_header_for_confirmation_window(const size_t no_selections) const;
	void delete_and_count_failures(const std::vector<SavegameData>& to_be_deleted) const;
	uint32_t try_to_delete(const std::vector<SavegameData>& to_be_deleted) const;
	void notify_deletion_failed(const uint32_t no_to_be_deleted, const uint32_t no_failed) const;

	UI::Panel* parent_;
};

#endif  // WL_WUI_SAVEGAMEDELETER_H
