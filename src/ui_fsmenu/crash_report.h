/*
 * Copyright (C) 2023 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_CRASH_REPORT_H
#define WL_UI_FSMENU_CRASH_REPORT_H

#include "io/filesystem/filesystem.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"

namespace FsMenu {

class CrashReportWindow : public UI::Window {
public:
	static constexpr const char kReportBugsURL[] = "https://www.widelands.org/wiki/ReportingBugs/";

	CrashReportWindow(FsMenu::MainMenu& menu, const FilenameSet& crash_files);

	void die() override;

private:
	FilenameSet clear_on_close_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_CRASH_REPORT_H
