/*
 * Copyright (C) 2022-2025 by the Widelands Development Team
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

#ifndef WL_UI_FSMENU_MOUSEWHEEL_REPORT_H
#define WL_UI_FSMENU_MOUSEWHEEL_REPORT_H

#include "ui/basic/box.h"
#include "ui/basic/button.h"
#include "ui/basic/multilinetextarea.h"
#include "ui/basic/window.h"
#include "ui/fsmenu/tech_info.h"

namespace FsMenu {

struct InvertedScrollFeedbackWindow : public UI::Window {
	explicit InvertedScrollFeedbackWindow(UI::Panel* parent);

private:
	UI::Box content_;
	UI::MultilineTextarea header_;
	TechInfoBox infobox_;
	UI::Button close_;
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_MOUSEWHEEL_REPORT_H
