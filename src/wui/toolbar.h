/*
 * Copyright (C) 2020-2022 by the Widelands Development Team
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

#ifndef WL_WUI_TOOLBAR_H
#define WL_WUI_TOOLBAR_H

#include "graphic/toolbar_imageset.h"
#include "ui_basic/box.h"

class InfoPanel;

/// A horizontal menu bar embellished with background graphics
class MainToolbar : public UI::Panel {
public:
	MainToolbar(InfoPanel&);

	static constexpr uint8_t kButtonSize = 33;

	/// Sets the actual size and position of the toolbar
	void finalize();
	void draw(RenderTarget& dst) override;
	void change_imageset(const ToolbarImageset& images);

	/// A row of buttons and dropdown menus
	UI::Box box;

	bool on_top, draw_background;

private:
	/// The set of background images
	ToolbarImageset imageset_;
	/// How often the left and right images get repeated, calculated from the width of the box
	int repeat_;
};

#endif  // end of include guard: WL_WUI_TOOLBAR_H
