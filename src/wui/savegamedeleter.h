#ifndef WL_WUI_SAVEGAMEDELETER_H
#define WL_WUI_SAVEGAMEDELETER_H

#include <string>
#include <vector>

#include "graphic/styles/window_style.h"
#include "ui_basic/panel.h"
#include "wui/savegamedata.h"

/// Encapsualates the deletion of savegames. Is extended by ReplayDeleter to handle deletion of
/// replays
class SavegameDeleter {
public:
	SavegameDeleter(UI::Panel* parent, UI::WindowStyle);

	/// attempt to delete the passed savegames. Returns true, if deletion was actually attempted
	/// (because deletion can be aborted by user via "cancel" in confirmation window)
	bool delete_savegames(const std::vector<SavegameData>& to_be_deleted) const;

	virtual ~SavegameDeleter() {
	}

private:
	bool show_confirmation_window(const std::vector<SavegameData>& selections) const;
	virtual const std::string
	create_header_for_confirmation_window(const size_t no_selections) const;
	void delete_and_count_failures(const std::vector<SavegameData>& to_be_deleted) const;
	virtual uint32_t try_to_delete(const std::vector<SavegameData>& to_be_deleted) const;

	void notify_deletion_failed(const std::vector<SavegameData>& to_be_deleted,
	                            const uint32_t no_failed) const;
	virtual std::string create_header_for_deletion_failed_window(size_t no_to_be_deleted,
	                                                             size_t no_failed) const;

	UI::Panel* parent_;
	UI::WindowStyle style_;
};

class ReplayDeleter : public SavegameDeleter {
public:
	ReplayDeleter(UI::Panel* parent, UI::WindowStyle);

private:
	const std::string
	create_header_for_confirmation_window(const size_t no_selections) const override;
	std::string create_header_for_deletion_failed_window(size_t no_to_be_deleted,
	                                                     size_t no_failed) const override;
	uint32_t try_to_delete(const std::vector<SavegameData>& to_be_deleted) const override;
};

#endif  // WL_WUI_SAVEGAMEDELETER_H
