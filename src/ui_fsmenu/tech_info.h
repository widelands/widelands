/*
 * Copyright (C) 2022-2024 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_TECH_INFO_H
#define WL_UI_FSMENU_TECH_INFO_H

#include <string>
#include <vector>

#include "ui_basic/box.h"
#include "ui_basic/multilinetextarea.h"

namespace FsMenu {

/*** The data ***/
struct TechInfo {
	// TODO(tothxa): It looks like the mousewheel report can be removed in v1.3
	enum class Type { kAbout, kMousewheelReport };

	explicit TechInfo(TechInfo::Type t);

	// Not localized, to be copied into bugreports
	[[nodiscard]] std::string get_markdown() const;

	// Localized, for the UI
	[[nodiscard]] std::string get_richtext() const;

private:
	struct Entry {
		const std::string label;
		const std::vector<std::string> values;
		const bool localize_values;
	};
	std::vector<TechInfo::Entry> entries_;

	void
	add_plain_entry(const std::string& label, const std::string& single_value, bool localize_value);
};

/*** The UI presentation ***/
struct TechInfoBox : public UI::Box {
	explicit TechInfoBox(UI::Panel* parent, TechInfo::Type t);

private:
	std::string markdown_report_;
	std::string richtext_report_;
	UI::MultilineTextarea report_area_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_TECH_INFO_H
