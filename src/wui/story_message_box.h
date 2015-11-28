/*
 * Copyright (C) 2002-2004, 2006-2008, 2010 by the Widelands Development Team
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

#ifndef WL_WUI_STORY_MESSAGE_BOX_H
#define WL_WUI_STORY_MESSAGE_BOX_H

#include <vector>

#include "ui_basic/window.h"

struct StoryMessageBox : public UI::Window {
	StoryMessageBox
		(UI::Panel *,
		 const std::string &, const std::string &, const std::string &,
		 int32_t gposx, int32_t gposy, uint32_t w, uint32_t h);

	bool handle_mousepress(uint8_t btn, int32_t mx, int32_t my) override;

	/// Handle keypresses
	bool handle_key(bool down, SDL_Keysym code) override;

private:
	void clicked_ok();
};

#endif  // end of include guard: WL_WUI_STORY_MESSAGE_BOX_H
