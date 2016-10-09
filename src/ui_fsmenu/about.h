/*
 * Copyright (C) 2016 by the Widelands Development Team
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_UI_FSMENU_ABOUT_H
#define WL_UI_FSMENU_ABOUT_H

#include "ui_basic/button.h"
#include "ui_basic/fileview_panel.h"
#include "ui_basic/textarea.h"
#include "ui_fsmenu/base.h"

/**
 * "Fullscreen "About" information with tabs
 */
class FullscreenMenuAbout : public FullscreenMenuBase {
public:
	FullscreenMenuAbout();

private:
	void fit_to_screen() override;

	uint32_t const butw_;
	uint32_t const buth_;
	uint32_t const hmargin_;
	uint32_t const tab_panel_width_;
	uint32_t const tab_panel_y_;

	UI::Textarea title_;
	UI::Button close_;

	// Tab contents
	UI::FileViewPanel tabs_;
};

#endif  // end of include guard: WL_UI_FSMENU_ABOUT_H
