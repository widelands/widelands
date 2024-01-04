/*
 * Copyright (C) 2002-2024 by the Widelands Development Team
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
	[[nodiscard]] bool delete_savegames(const std::vector<SavegameData>& to_be_deleted) const;

	virtual ~SavegameDeleter() = default;

private:
	[[nodiscard]] bool show_confirmation_window(const std::vector<SavegameData>& selections) const;
	[[nodiscard]] virtual const std::string
	create_header_for_confirmation_window(size_t no_selections) const;
	void delete_and_count_failures(const std::vector<SavegameData>& to_be_deleted) const;
	[[nodiscard]] virtual uint32_t
	try_to_delete(const std::vector<SavegameData>& to_be_deleted) const;

	void notify_deletion_failed(const std::vector<SavegameData>& to_be_deleted,
	                            uint32_t no_failed) const;
	[[nodiscard]] virtual std::string
	create_header_for_deletion_failed_window(size_t no_to_be_deleted, size_t no_failed) const;

	UI::Panel* parent_;
	UI::WindowStyle style_;
};

class ReplayDeleter : public SavegameDeleter {
public:
	ReplayDeleter(UI::Panel* parent, UI::WindowStyle);

private:
	[[nodiscard]] const std::string
	create_header_for_confirmation_window(size_t no_selections) const override;
	[[nodiscard]] std::string
	create_header_for_deletion_failed_window(size_t no_to_be_deleted,
	                                         size_t no_failed) const override;
	[[nodiscard]] uint32_t
	try_to_delete(const std::vector<SavegameData>& to_be_deleted) const override;
};

#endif  // WL_WUI_SAVEGAMEDELETER_H
