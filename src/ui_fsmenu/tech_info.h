/*
 * Copyright (C) 2022 by the Widelands Development Team
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

#include <memory>

#include "ui_basic/box.h"
#include "ui_basic/textarea.h"

namespace FsMenu {

class TechInfoLine : public UI::Box {
	UI::Textarea label_;
	UI::Textarea value_;

public:
	TechInfoLine(UI::Panel* parent, std::string label, std::string value, bool right_to_left);
};

struct TechInfoBox : public UI::Box {
	enum class Type { kAbout, kMousewheelReport };

	explicit TechInfoBox(UI::Panel* parent, TechInfoBox::Type t);
};

}  // namespace FsMenu

#endif  // end of include guard: WL_UI_FSMENU_TECH_INFO_H
