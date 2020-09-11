/*
 * Copyright (C) 2016-2020 by the Widelands Development Team
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

#include <memory>

#include "ui_basic/fileview_panel.h"
#include "ui_basic/window.h"
#include "ui_fsmenu/main.h"

/**
 * "Fullscreen "About" information with tabs
 */
class FullscreenMenuAbout : public UI::Window {
public:
	explicit FullscreenMenuAbout(FullscreenMenuMain&);
	~FullscreenMenuAbout() override {
	}

	bool handle_key(bool, SDL_Keysym) override;

private:
	void layout() override;

	FullscreenMenuMain& parent_;

	UI::Box box_;
	UI::Button close_;
	UI::FileViewPanel tabs_;

	std::unique_ptr<Notifications::Subscriber<GraphicResolutionChanged>>
	   graphic_resolution_changed_subscriber_;
};

#endif  // end of include guard: WL_UI_FSMENU_ABOUT_H
